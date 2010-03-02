/*
  Qore Programming Language Qt4 Module

  Copyright 2009 -2010 Qore Technologies sro

  This library is free software; you can redistribute it and/or
  modify it under the terms of the GNU Lesser General Public
  License as published by the Free Software Foundation; either
  version 2.1 of the License, or (at your option) any later version.

  This library is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
  Lesser General Public License for more details.

  You should have received a copy of the GNU Lesser General Public
  License along with this library; if not, write to the Free Software
  Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA
*/

#include "qoreqtenumnode.h"

QoreString * QoreQtEnumNode::getStringRepresentation(bool &del) const {
    del = true;
    QoreString *str = new QoreString();
    getStringRepresentation(*str);
    return str;
}

void QoreQtEnumNode::getStringRepresentation(QoreString &str) const {
    str.sprintf("%s::%d", m_type.name, val);
}

bool QoreQtEnumNode::getAsBoolImpl() const {
    return (bool)val;
}

int QoreQtEnumNode::getAsIntImpl() const {
    return (int)val;
}

int64 QoreQtEnumNode::getAsBigIntImpl() const {
    return (int64)val;
}

double QoreQtEnumNode::getAsFloatImpl() const {
    return (double)val;
}

QoreString *QoreQtEnumNode::getAsString(bool &del, int foff, class ExceptionSink *xsink) const {
    return getStringRepresentation(del);
}

int QoreQtEnumNode::getAsString(QoreString &str, int foff, class ExceptionSink *xsink) const {
    getStringRepresentation(str);
    return 0;
}

AbstractQoreNode *QoreQtEnumNode::realCopy() const {
   return new QoreQtEnumNode(type, val, m_type);
}

bool QoreQtEnumNode::is_equal_soft(const AbstractQoreNode *v, ExceptionSink *xsink) const {
    return (v ? v->getAsInt() : 0) == val;
}

bool QoreQtEnumNode::is_equal_hard(const AbstractQoreNode *v, ExceptionSink *xsink) const {
    const QoreQtEnumNode *ps = dynamic_cast<const QoreQtEnumNode *>(v);
    if (!ps)
        return false;

    const Smoke::Type t = ps->smokeType();
    return ps->value() == val
           && (!strcmp(m_type.name, t.name));
}

// returns the type name as a c string
const char * QoreQtEnumNode::getTypeName() const {
   return m_type.name;
}

const char * QoreQtEnumNode::getStaticTypeName() {
    return "QtEnum";
}
