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
protected:
   DLLLOCAL static qore_type_t fake_id;
public:
   DLLLOCAL QoreQtIntCompatibleTypeInfoHelper() : QoreTypeInfoHelper("int") {
      assert(!fake_id);
      fake_id = get_next_type_id();
      assign(fake_id);
      qtIntTypeInfo = getTypeInfo();
   }
   DLLEXPORT virtual bool checkTypeInstantiationImpl(AbstractQoreNode *&n, ExceptionSink *xsink) const {
      //printd(0, "QoreQtIntCompatibleTypeInfoHelper::checkTypeInstantiationImpl() this=%p n=%p (%s)\n", this, n, n ? n->getTypeName() : "NOTHING");
      if (n && n->getType() == NT_FLOAT) {
         int64 val = n->getAsBigInt();
         n->deref(xsink);
         n = new QoreBigIntNode(val);
         return true;
      }
      return dynamic_cast<QoreBigIntNode *>(n) ? true : false;
   }
   DLLEXPORT virtual int testTypeCompatibilityImpl(const AbstractQoreNode *n) const {
      //printd(0, "QoreQtIntCompatibleTypeInfoHelper::testTypeCompatibilityImpl() this=%p n=%p (%s)\n", this, n, n ? n->getTypeName() : "NOTHING");
      return n && (n->getType() == NT_FLOAT || dynamic_cast<const QoreBigIntNode *>(n)) ? QTI_AMBIGUOUS : QTI_NOT_EQUAL;
   }
   DLLEXPORT virtual int parseEqualImpl(const QoreTypeInfo *typeInfo) const;
};

class QoreQtStringCompatibleTypeInfoHelper : public QoreTypeInfoHelper {
protected:
   DLLLOCAL static qore_type_t fake_id;
public:
   DLLLOCAL QoreQtStringCompatibleTypeInfoHelper() : QoreTypeInfoHelper("string") {
      assert(!fake_id);
      fake_id = get_next_type_id();
      assign(fake_id);
      qtStringTypeInfo = getTypeInfo();
   }
   DLLEXPORT virtual bool checkTypeInstantiationImpl(AbstractQoreNode *&n, ExceptionSink *xsink) const;
   DLLEXPORT virtual int testTypeCompatibilityImpl(const AbstractQoreNode *n) const {
      if (!n 
          || (n->getType() != NT_STRING 
              && (n->getType() != NT_OBJECT 
                  || (!testObjectClassAccess(reinterpret_cast<const QoreObject *>(n), QC_QCHAR)))))
	 return QTI_NOT_EQUAL;
      return QTI_AMBIGUOUS;      
   }
   DLLEXPORT virtual int parseEqualImpl(const QoreTypeInfo *typeInfo) const {
      return typeInfoGetType(typeInfo) == NT_STRING 
         || typeInfoParseTestCompatibleClass(typeInfo, QC_QCHAR)
         ? QTI_AMBIGUOUS : QTI_NOT_EQUAL;
   }
};

class QRegionTypeHelper : public AbstractQoreClassTypeInfoHelper {
public:
   DLLLOCAL QRegionTypeHelper() : AbstractQoreClassTypeInfoHelper("QRegion", QDOM_GUI) {
   }

   DLLEXPORT virtual bool checkTypeInstantiationImpl(AbstractQoreNode *&n, ExceptionSink *xsink) const;

   DLLEXPORT virtual int testTypeCompatibilityImpl(const AbstractQoreNode *n) const {
      if (!n || n->getType() != NT_OBJECT || !testObjectClassAccess(reinterpret_cast<const QoreObject *>(n), QC_QRECT))
	 return QTI_NOT_EQUAL;
      return QTI_AMBIGUOUS;
   }
   DLLEXPORT virtual int parseEqualImpl(const QoreTypeInfo *typeInfo) const {
      return typeInfoParseTestCompatibleClass(typeInfo, QC_QRECT) ? QTI_AMBIGUOUS : QTI_NOT_EQUAL;
   }
};

class QBrushTypeHelper : public AbstractQoreClassTypeInfoHelper {
public:
   DLLLOCAL QBrushTypeHelper() : AbstractQoreClassTypeInfoHelper("QBrush", QDOM_GUI) {
   }

   DLLEXPORT virtual bool checkTypeInstantiationImpl(AbstractQoreNode *&n, ExceptionSink *xsink) const;

   DLLEXPORT virtual int testTypeCompatibilityImpl(const AbstractQoreNode *n) const {
      if (!n) return QTI_NOT_EQUAL;
      const char *tn = n->getTypeName();
      //printd(0, "QBrushTypeHelper::testTypeCompatibilityImpl() this=%p checking %s\n", this, tn);
      if (n->getType() == NT_INT
          || !strcmp(tn, "Qt::GlobalColor") 
          || !strcmp(tn, "Qt::BrushStyle"))
         return QTI_AMBIGUOUS;
      
      if (n->getType() == NT_OBJECT && testObjectClassAccess(reinterpret_cast<const QoreObject *>(n), QC_QCOLOR))
         return QTI_AMBIGUOUS;

      return QTI_NOT_EQUAL;
   }
   DLLEXPORT virtual int parseEqualImpl(const QoreTypeInfo *typeInfo) const {
      if (!typeInfo) return QTI_NOT_EQUAL;
      const char *tn = typeInfoGetName(typeInfo);
      //printd(0, "QBrushTypeHelper::parseEqualImpl() this=%p checking %s\n", this, tn);
      return typeInfoGetType(typeInfo) == NT_INT 
	 || !strcmp(tn, "Qt::GlobalColor") 
	 || !strcmp(tn, "Qt::BrushStyle")
         || typeInfoParseTestCompatibleClass(typeInfo, QC_QCOLOR)
         ? QTI_AMBIGUOUS : QTI_NOT_EQUAL;
   }
};

class QWidgetTypeHelper : public AbstractQoreClassTypeInfoHelper {
public:
   DLLLOCAL QWidgetTypeHelper() : AbstractQoreClassTypeInfoHelper("QWidget", QDOM_GUI) {
   }

   DLLEXPORT virtual bool checkTypeInstantiationImpl(AbstractQoreNode *&n, ExceptionSink *xsink) const {
      return is_nothing(n) ? true : false;
   }
   DLLEXPORT virtual int testTypeCompatibilityImpl(const AbstractQoreNode *n) const {
      return is_nothing(n) ? QTI_AMBIGUOUS : QTI_NOT_EQUAL;
   }
   DLLEXPORT virtual int parseEqualImpl(const QoreTypeInfo *typeInfo) const {
      return typeInfo && typeInfoGetType(typeInfo) == NT_NOTHING ? QTI_AMBIGUOUS : QTI_NOT_EQUAL;
   }
};

class QColorTypeHelper : public AbstractQoreClassTypeInfoHelper {
public:
   DLLLOCAL QColorTypeHelper() : AbstractQoreClassTypeInfoHelper("QColor", QDOM_GUI) {
   }

   DLLEXPORT virtual bool checkTypeInstantiationImpl(AbstractQoreNode *&n, ExceptionSink *xsink) const;

   DLLEXPORT virtual int testTypeCompatibilityImpl(const AbstractQoreNode *n) const {
      if (!n) return QTI_NOT_EQUAL;
      return n->getType() == NT_INT || !strcmp(n->getTypeName(), "Qt::GlobalColor") ? QTI_AMBIGUOUS : QTI_NOT_EQUAL;
   }
   DLLEXPORT virtual int parseEqualImpl(const QoreTypeInfo *typeInfo) const {
      if (!typeInfo) return QTI_NOT_EQUAL;
      return typeInfoGetType(typeInfo) == NT_INT 
	 || !strcmp(typeInfoGetName(typeInfo), "Qt::GlobalColor") ? QTI_AMBIGUOUS : QTI_NOT_EQUAL;
   }
};

class QVariantTypeHelper : public AbstractQoreClassTypeInfoHelper {
protected:
   DLLLOCAL bool canConvertIntern(qore_type_t t, const QoreClass *qc, const char *name) const;

public:
   DLLLOCAL QVariantTypeHelper() : AbstractQoreClassTypeInfoHelper("QVariant", QDOM_GUI) {
   }
   DLLEXPORT virtual bool checkTypeInstantiationImpl(AbstractQoreNode *&n, ExceptionSink *xsink) const;

   DLLEXPORT virtual int testTypeCompatibilityImpl(const AbstractQoreNode *n) const {
      qore_type_t t = n ? n->getType() : NT_NOTHING;
      const QoreClass *qc = t == NT_OBJECT ? reinterpret_cast<const QoreObject *>(n)->getClass() : 0;
      return canConvertIntern(t, qc, qc ? 0 : get_type_name(n)) ? QTI_AMBIGUOUS : QTI_NOT_EQUAL;
   }
   DLLEXPORT virtual int parseEqualImpl(const QoreTypeInfo *typeInfo) const {
      qore_type_t t = typeInfo ? typeInfoGetType(typeInfo) : NT_NOTHING;
      const QoreClass *qc = typeInfo ? typeInfoGetClass(typeInfo) : 0;
      return canConvertIntern(t, qc, qc ? 0 : typeInfoGetName(typeInfo)) ? QTI_AMBIGUOUS : QTI_NOT_EQUAL;
   }
};

class QKeySequenceTypeHelper : public AbstractQoreClassTypeInfoHelper {
protected:
   DLLLOCAL bool canConvertIntern(qore_type_t t, const char *name) const;

public:
   DLLLOCAL QKeySequenceTypeHelper() : AbstractQoreClassTypeInfoHelper("QKeySequence", QDOM_GUI) {
   }
   DLLEXPORT virtual bool checkTypeInstantiationImpl(AbstractQoreNode *&n, ExceptionSink *xsink) const;
   DLLEXPORT virtual int testTypeCompatibilityImpl(const AbstractQoreNode *n) const {
      if (!n)
         return QTI_NOT_EQUAL;
      return canConvertIntern(n->getType(), n->getTypeName()) ? QTI_AMBIGUOUS : QTI_NOT_EQUAL;
   }
   DLLEXPORT virtual int parseEqualImpl(const QoreTypeInfo *typeInfo) const {
      if (!typeInfo)
         return QTI_NOT_EQUAL;
      return canConvertIntern(typeInfoGetType(typeInfo), typeInfoGetName(typeInfo)) ? QTI_AMBIGUOUS : QTI_NOT_EQUAL;
   }
};

// don't really need to tag this class with QDOM_GUI...
class QDateTypeHelper : public AbstractQoreClassTypeInfoHelper {
protected:
   // only for subclasses
   DLLLOCAL QDateTypeHelper(const char *name) : AbstractQoreClassTypeInfoHelper(name, QDOM_GUI) {
   }

public:
   DLLLOCAL QDateTypeHelper() : AbstractQoreClassTypeInfoHelper("QDate", QDOM_GUI) {
   }
   DLLEXPORT virtual bool checkTypeInstantiationImpl(AbstractQoreNode *&n, ExceptionSink *xsink) const;
   DLLEXPORT virtual int testTypeCompatibilityImpl(const AbstractQoreNode *n) const {
      qore_type_t t = n ? n->getType() : NT_NOTHING;
      return t == NT_DATE ? QTI_AMBIGUOUS : QTI_NOT_EQUAL;
   }
   DLLEXPORT virtual int parseEqualImpl(const QoreTypeInfo *typeInfo) const {
      return typeInfo && typeInfoGetType(typeInfo) == NT_DATE ? QTI_AMBIGUOUS : QTI_NOT_EQUAL;
   }
};

class QDateTimeTypeHelper : public QDateTypeHelper {
public:
   DLLLOCAL QDateTimeTypeHelper() : QDateTypeHelper("QDateTime") {
   }
   DLLEXPORT virtual bool checkTypeInstantiationImpl(AbstractQoreNode *&n, ExceptionSink *xsink) const;
};

class QTimeTypeHelper : public QDateTypeHelper {
public:
   DLLLOCAL QTimeTypeHelper() : QDateTypeHelper("QTime") {
   }
   DLLEXPORT virtual bool checkTypeInstantiationImpl(AbstractQoreNode *&n, ExceptionSink *xsink) const;
};

// for classes that should also be equivalent to a null ptr
class NullPtrClassTypeHelper : public AbstractQoreClassTypeInfoHelper {
public:
   DLLLOCAL NullPtrClassTypeHelper(const char *name) : AbstractQoreClassTypeInfoHelper(name, QDOM_GUI) {
   }

   DLLEXPORT virtual bool checkTypeInstantiationImpl(AbstractQoreNode *&n, ExceptionSink *xsink) const {
      return is_nothing(n) ? true : false;
   }
   DLLEXPORT virtual int testTypeCompatibilityImpl(const AbstractQoreNode *n) const {
      return is_nothing(n) ? QTI_AMBIGUOUS : QTI_NOT_EQUAL;
   }
   DLLEXPORT virtual int parseEqualImpl(const QoreTypeInfo *typeInfo) const {
      return typeInfo && typeInfoGetType(typeInfo) == NT_NOTHING ? QTI_AMBIGUOUS : QTI_NOT_EQUAL;
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
