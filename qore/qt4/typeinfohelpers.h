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
      if (!n)
	 return false;

      if (n->getType() == NT_QTENUM) {
	 QoreBigIntNode *rv = new QoreBigIntNode(reinterpret_cast<QoreQtEnumNode *>(n)->value());
	 n->deref(xsink);
	 n = rv;
	 return true;
      }
      return n->getType() == NT_INT ? true : false;
   }
   DLLEXPORT virtual int testTypeCompatibilityImpl(const AbstractQoreNode *n) const {
      //printd(0, "QoreQtIntCompatibleTypeInfoHelper::testTypeCompatibilityImpl() this=%p n=%p (%s)\n", this, n, n ? n->getTypeName() : "NOTHING");
      if (!n) return QTI_NOT_EQUAL;
      if (n->getType() == NT_INT) return QTI_IDENT;
      return n->getType() == NT_QTENUM ? QTI_AMBIGUOUS : QTI_NOT_EQUAL;
   }
   DLLEXPORT virtual int parseEqualImpl(const QoreTypeInfo *typeInfo) const {
      //printd(0, "QoreQtIntCompatibleTypeInfoHelper::parseEqualImpl() this=%p typeInfo=%s\n", this, typeInfoGetName(typeInfo));
      if (!typeInfo) return QTI_NOT_EQUAL;
      qore_type_t t = typeInfoGetType(typeInfo);
      if (t == NT_INT) return QTI_IDENT;
      return t == NT_QTENUM ? QTI_AMBIGUOUS : QTI_NOT_EQUAL;
   }
};

class QRegionTypeHelper : public AbstractQoreClassTypeInfoHelper {
public:
   DLLLOCAL QRegionTypeHelper() : AbstractQoreClassTypeInfoHelper("QRegion", QDOM_GUI) {
   }

   DLLEXPORT virtual bool checkTypeInstantiationImpl(AbstractQoreNode *&n, ExceptionSink *xsink) const;

   DLLEXPORT virtual int testTypeCompatibilityImpl(const AbstractQoreNode *n) const {
      if (!n || n->getType() != NT_OBJECT || !testObjectClassAccess(reinterpret_cast<const QoreObject *>(n), QC_QREGION))
	 return QTI_NOT_EQUAL;
      return QTI_AMBIGUOUS;
   }
   DLLEXPORT virtual int parseEqualImpl(const QoreTypeInfo *typeInfo) const {
      return typeInfoGetClass(typeInfo) == QC_QREGION ? QTI_AMBIGUOUS : QTI_NOT_EQUAL;
   }
};

class QBrushTypeHelper : public AbstractQoreClassTypeInfoHelper {
public:
   DLLLOCAL QBrushTypeHelper() : AbstractQoreClassTypeInfoHelper("QBrush", QDOM_GUI) {
   }

   DLLEXPORT virtual bool checkTypeInstantiationImpl(AbstractQoreNode *&n, ExceptionSink *xsink) const;

   DLLEXPORT virtual int testTypeCompatibilityImpl(const AbstractQoreNode *n) const {
      return dynamic_cast<const QoreBigIntNode *>(n) ? QTI_AMBIGUOUS : QTI_NOT_EQUAL;
   }
   DLLEXPORT virtual int parseEqualImpl(const QoreTypeInfo *typeInfo) const {
      return typeInfo && (typeInfoGetType(typeInfo) == NT_INT || typeInfoGetType(typeInfo) == NT_QTENUM) ? QTI_AMBIGUOUS : QTI_NOT_EQUAL;
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
      return n->getType() == NT_QTENUM ? QTI_AMBIGUOUS : QTI_NOT_EQUAL;
   }
   DLLEXPORT virtual int parseEqualImpl(const QoreTypeInfo *typeInfo) const {
      if (!typeInfo) return QTI_NOT_EQUAL;
      qore_type_t t = typeInfoGetType(typeInfo);
      return t == NT_QTENUM ? QTI_AMBIGUOUS : QTI_NOT_EQUAL;
   }
};

class QVariantTypeHelper : public AbstractQoreClassTypeInfoHelper {
protected:
   DLLLOCAL bool canConvertIntern(qore_type_t t, const QoreClass *qc) const;

public:
   DLLLOCAL QVariantTypeHelper() : AbstractQoreClassTypeInfoHelper("QVariant", QDOM_GUI) {
   }
   DLLEXPORT virtual bool checkTypeInstantiationImpl(AbstractQoreNode *&n, ExceptionSink *xsink) const;

   DLLEXPORT virtual int testTypeCompatibilityImpl(const AbstractQoreNode *n) const {
      qore_type_t t = n ? n->getType() : NT_NOTHING;
      const QoreClass *qc = t == NT_OBJECT ? reinterpret_cast<const QoreObject *>(n)->getClass() : 0;
      return canConvertIntern(t, qc) ? QTI_AMBIGUOUS : QTI_NOT_EQUAL;
   }
   DLLEXPORT virtual int parseEqualImpl(const QoreTypeInfo *typeInfo) const {
      qore_type_t t = typeInfo ? typeInfoGetType(typeInfo) : NT_NOTHING;
      const QoreClass *qc = typeInfo ? typeInfoGetClass(typeInfo) : 0;
      return canConvertIntern(t, qc) ? QTI_AMBIGUOUS : QTI_NOT_EQUAL;
   }
};

#endif
