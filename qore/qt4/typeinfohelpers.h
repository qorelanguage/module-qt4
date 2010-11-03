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

#ifndef _QORE_QT_MODULE_TYPEINFOHELPERS_H
#define _QORE_QT_MODULE_TYPEINFOHELPERS_H

#include "qoreqtenumnode.h"

class QoreQtIntCompatibleTypeInfoHelper : public QoreTypeInfoHelper {
public:
     DLLLOCAL QoreQtIntCompatibleTypeInfoHelper() : QoreTypeInfoHelper(NT_INT, "int") {
          setInt();
          addAcceptsType(floatTypeInfo);
          qtIntTypeInfo = getTypeInfo();
          setInputFilter();
     }
     DLLLOCAL virtual bool acceptInputImpl(AbstractQoreNode *&n, ExceptionSink *xsink) const {
          qore_type_t t = get_node_type(n);

          if (t == NT_INT || (t >= QORE_NUM_TYPES && dynamic_cast<const QoreBigIntNode *>(n)))
               return true;

          if (t != NT_FLOAT)
               return false;

          int64 rv = n->getAsBigInt();
          n->deref(xsink);
          n = new QoreBigIntNode(rv);
          return true;
     }
};

class QoreQtStringCompatibleTypeInfoHelper : public QoreTypeInfoHelper {
protected:
public:
     DLLLOCAL QoreQtStringCompatibleTypeInfoHelper() : QoreTypeInfoHelper(NT_STRING, "string") {
          qtStringTypeInfo = getTypeInfo();
          setInputFilter();
     }

     DLLLOCAL virtual bool acceptInputImpl(AbstractQoreNode *&n, ExceptionSink *xsink) const {
          qore_type_t t = get_node_type(n);

          if (t == NT_STRING)
               return true;

          if (t != NT_OBJECT)
               return false;

          QoreObject *o = reinterpret_cast<QoreObject *>(n);
          // see if we can get a QChar
          if (!o->getClass(QC_QCHAR->getID()))
               return false;

          return true;
     }
};

DLLLOCAL extern QoreQtStringCompatibleTypeInfoHelper qtStringTypeInfoHelper;

class QRegionTypeHelper : public AbstractQoreClassTypeInfoHelper {
public:
     DLLLOCAL QRegionTypeHelper() : AbstractQoreClassTypeInfoHelper("QRegion", QDOM_GUI) {
          setInputFilter();
     }

     DLLLOCAL virtual bool acceptInputImpl(AbstractQoreNode *&n, ExceptionSink *xsink) const;
};

class QBrushTypeHelper : public AbstractQoreClassTypeInfoHelper {
public:
     DLLLOCAL QBrushTypeHelper() : AbstractQoreClassTypeInfoHelper("QBrush", QDOM_GUI) {
          setInputFilter();
     }

     DLLLOCAL virtual bool acceptInputImpl(AbstractQoreNode *&n, ExceptionSink *xsink) const;
};

class QWidgetTypeHelper : public AbstractQoreClassTypeInfoHelper {
public:
     DLLLOCAL QWidgetTypeHelper() : AbstractQoreClassTypeInfoHelper("QWidget", QDOM_GUI) {
          addAcceptsType(nothingTypeInfo);
     }
};

class QColorTypeHelper : public AbstractQoreClassTypeInfoHelper {
protected:
     DLLLOCAL virtual bool acceptInputImpl(AbstractQoreNode *&n, ExceptionSink *xsink) const;

public:
     DLLLOCAL QColorTypeHelper() : AbstractQoreClassTypeInfoHelper("QColor", QDOM_GUI) {
          addAcceptsType(bigIntTypeInfo);
          setInputFilter();
     }
};

class QVariantTypeHelper : public AbstractQoreClassTypeInfoHelper {
protected:
     DLLLOCAL virtual bool acceptInputImpl(AbstractQoreNode *&n, ExceptionSink *xsink) const;

private:
     DLLLOCAL template<class T>
     QVariant * acceptQtObject(qore_classid_t classID, const QoreObject *obj, ExceptionSink *xsink) const;

public:
     DLLLOCAL QVariantTypeHelper() : AbstractQoreClassTypeInfoHelper("QVariant", QDOM_GUI) {
          setInputFilter();
          setIntMatch();
          addAcceptsType(bigIntTypeInfo);
          addAcceptsType(nothingTypeInfo);
          addAcceptsType(floatTypeInfo);
          addAcceptsType(stringTypeInfo);
          addAcceptsType(boolTypeInfo);
          addAcceptsType(binaryTypeInfo);
     }
};

class QKeySequenceTypeHelper : public AbstractQoreClassTypeInfoHelper {
protected:
     DLLLOCAL virtual bool acceptInputImpl(AbstractQoreNode *&n, ExceptionSink *xsink) const;

public:
     DLLLOCAL QKeySequenceTypeHelper() : AbstractQoreClassTypeInfoHelper("QKeySequence", QDOM_GUI) {
          setInputFilter();
          addAcceptsType(bigIntTypeInfo);
          addAcceptsType(stringTypeInfo);
     }
};

// don't really need to tag this class with QDOM_GUI...
class QDateTypeHelper : public AbstractQoreClassTypeInfoHelper {
protected:
     // only for subclasses
     DLLLOCAL QDateTypeHelper(const char *name) : AbstractQoreClassTypeInfoHelper(name, QDOM_GUI) {
          setInputFilter();
          addAcceptsType(dateTypeInfo);
     }

     DLLLOCAL virtual bool acceptInputImpl(AbstractQoreNode *&n, ExceptionSink *xsink) const;

public:
     DLLLOCAL QDateTypeHelper() : AbstractQoreClassTypeInfoHelper("QDate", QDOM_GUI) {
     }
};

class QDateTimeTypeHelper : public QDateTypeHelper {
protected:
     DLLLOCAL virtual bool acceptInputImpl(AbstractQoreNode *&n, ExceptionSink *xsink) const;

public:
     DLLLOCAL QDateTimeTypeHelper() : QDateTypeHelper("QDateTime") {
     }
};

class QTimeTypeHelper : public QDateTypeHelper {
protected:
     DLLLOCAL virtual bool acceptInputImpl(AbstractQoreNode *&n, ExceptionSink *xsink) const;

public:
     DLLLOCAL QTimeTypeHelper() : QDateTypeHelper("QTime") {
     }
};

// for classes that should also be equivalent to a null ptr
class NullPtrClassTypeHelper : public AbstractQoreClassTypeInfoHelper {
public:
     DLLLOCAL NullPtrClassTypeHelper(const char *name) : AbstractQoreClassTypeInfoHelper(name, QDOM_GUI) {
          addAcceptsType(nothingTypeInfo);
     }
};

class QValidatorTypeHelper : public NullPtrClassTypeHelper {
public:
     DLLLOCAL QValidatorTypeHelper() : NullPtrClassTypeHelper("QValidator") {}
};

class QCompleterTypeHelper : public NullPtrClassTypeHelper {
public:
     DLLLOCAL QCompleterTypeHelper() : NullPtrClassTypeHelper("QCompleter") {}
};

#endif
