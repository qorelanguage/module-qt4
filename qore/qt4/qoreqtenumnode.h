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

#ifndef QOREQTENUMNODE_H
#define QOREQTENUMNODE_H

#include <qore/Qore.h>
#include <smoke.h>


DLLEXPORT extern qore_type_t NT_QTENUM;


class QoreQtEnumNode : public SimpleValueQoreNode {
private:
    Smoke::Type m_type;
    int m_value;

    DLLLOCAL virtual bool getAsBoolImpl() const;
    DLLLOCAL virtual int getAsIntImpl() const;
    DLLLOCAL virtual int64 getAsBigIntImpl() const;
    DLLLOCAL virtual double getAsFloatImpl() const;

public:
    DLLLOCAL QoreQtEnumNode(int v, Smoke::Type t)
            : SimpleValueQoreNode(NT_QTENUM),
            m_value(v),
            m_type(t) {
    }

    DLLLOCAL ~QoreQtEnumNode() {
    }

    Smoke::Type smokeType() const {
        return m_type;
    }

    int value() const {
        return m_value;
    }

    DLLLOCAL virtual QoreString *getStringRepresentation(bool &del) const;

    DLLLOCAL virtual void getStringRepresentation(QoreString &str) const;

    DLLLOCAL virtual QoreString *getAsString(bool &del, int foff, class ExceptionSink *xsink) const;

    DLLLOCAL virtual int getAsString(QoreString &str, int foff, class ExceptionSink *xsink) const;

    DLLLOCAL virtual class AbstractQoreNode *realCopy() const;

    DLLLOCAL virtual bool is_equal_soft(const AbstractQoreNode *v, ExceptionSink *xsink) const;

    DLLLOCAL virtual bool is_equal_hard(const AbstractQoreNode *v, ExceptionSink *xsink) const;

    DLLLOCAL virtual const char *getTypeName() const;

    DLLLOCAL static const char *getStaticTypeName();

    static void registerType() {
        NT_QTENUM = get_next_type_id();
    }

};



#endif
