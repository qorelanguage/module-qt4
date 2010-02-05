/*
  Qore Programming Language Qt4 Module

  Copyright 2009 Qore Technologies sro

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

qore_type_t NT_QTENUM = -1;

QoreString * QoreQtEnumNode::getStringRepresentation(bool &del) const {
    del = true;
    QoreString *str = new QoreString();
    getStringRepresentation(*str);
    return str;
}

void QoreQtEnumNode::getStringRepresentation(QoreString &str) const {
    str.sprintf("%s::%d", m_type.name, m_value);
}

bool QoreQtEnumNode::getAsBoolImpl() const {
    return (bool)m_value;
}

int QoreQtEnumNode::getAsIntImpl() const {
    return (int)m_value;
}

int64 QoreQtEnumNode::getAsBigIntImpl() const {
    return (int64)m_value;
}

double QoreQtEnumNode::getAsFloatImpl() const {
    return (double)m_value;
}

QoreString *QoreQtEnumNode::getAsString(bool &del, int foff, class ExceptionSink *xsink) const {
    return getStringRepresentation(del);
}

int QoreQtEnumNode::getAsString(QoreString &str, int foff, class ExceptionSink *xsink) const {
    getStringRepresentation(str);
    return 0;
}

class AbstractQoreNode *QoreQtEnumNode::realCopy() const {
        return new QoreQtEnumNode(m_value, m_type);
    }

bool QoreQtEnumNode::is_equal_soft(const AbstractQoreNode *v, ExceptionSink *xsink) const {
    return (v ? v->getAsInt() : 0) == m_value;
}

bool QoreQtEnumNode::is_equal_hard(const AbstractQoreNode *v, ExceptionSink *xsink) const {
    const QoreQtEnumNode *ps = dynamic_cast<const QoreQtEnumNode *>(v);
    if (!ps)
        return false;

    const Smoke::Type t = ps->smokeType();
    return ps->value() == m_value
           && (!strcmp(m_type.name, t.name));
}

// returns the type name as a c string
const char * QoreQtEnumNode::getTypeName() const {
    return getStaticTypeName();
}

const char * QoreQtEnumNode::getStaticTypeName() {
    return "QtEnum";
}
