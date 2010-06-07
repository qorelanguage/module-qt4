/* -*- mode: c++; indent-tabs-mode: nil -*- */
/*
  Qore Programming Language Qt4 Module

  Copyright 2009 - 2010 Qore Technologies sro

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

#ifndef QOREQTENUMNODE_H
#define QOREQTENUMNODE_H

#include <qore/Qore.h>
#include <smoke.h>

class QoreQtEnumNode : public QoreBigIntNode {
private:
   Smoke::Type m_type;

   DLLLOCAL virtual bool getAsBoolImpl() const;
   DLLLOCAL virtual int getAsIntImpl() const;
   DLLLOCAL virtual int64 getAsBigIntImpl() const;
   DLLLOCAL virtual double getAsFloatImpl() const;

public:
   DLLLOCAL QoreQtEnumNode(qore_type_t qoreType, int64 v, Smoke::Type t) : QoreBigIntNode(qoreType, v), m_type(t) {
   }

   DLLLOCAL ~QoreQtEnumNode() {
   }

   DLLLOCAL Smoke::Type smokeType() const {
      return m_type;
   }

   DLLLOCAL bool isEnum(const Smoke::Type &t) const {
      return !strcmp(t.name, m_type.name) && t.flags == m_type.flags;
   }

   DLLLOCAL bool isEnum(const char *n) const {
      return !strcmp(n, m_type.name);
   }

   DLLLOCAL int64 value() const {
      return val;
   }

   DLLLOCAL virtual QoreString *getStringRepresentation(bool &del) const;

   DLLLOCAL virtual void getStringRepresentation(QoreString &str) const;

   DLLLOCAL virtual QoreString *getAsString(bool &del, int foff, ExceptionSink *xsink) const;

   DLLLOCAL virtual int getAsString(QoreString &str, int foff, ExceptionSink *xsink) const;

   DLLLOCAL virtual class AbstractQoreNode *realCopy() const;

   DLLLOCAL virtual bool is_equal_soft(const AbstractQoreNode *v, ExceptionSink *xsink) const;

   DLLLOCAL virtual bool is_equal_hard(const AbstractQoreNode *v, ExceptionSink *xsink) const;

   DLLLOCAL virtual const char *getTypeName() const;

   DLLLOCAL static const char *getStaticTypeName();
};

class QoreEnumTypeInfoHelper : public QoreTypeInfoHelper {
protected:
   Smoke::Type enumType;
   qore_type_t qoreType;

public:
   DLLLOCAL QoreEnumTypeInfoHelper(const Smoke::Type &t) : QoreTypeInfoHelper(t.name), enumType(t), qoreType(get_next_type_id()) {
      assign(qoreType);
      setInt();
      //printd(0, "QoreEnumTypeInfoHelper::QoreEnumTypeInfoHelper() creating %p (%s type %d)\n", this, t.name, qoreType);
   }
   DLLLOCAL virtual ~QoreEnumTypeInfoHelper() {
      //printd(0, "QoreEnumTypeInfoHelper::~QoreEnumTypeInfoHelper() deleting %p\n", this);
   }
   DLLLOCAL QoreQtEnumNode *newValue(int64 val) const {
      return new QoreQtEnumNode(qoreType, val, enumType);
   }
};

#endif
