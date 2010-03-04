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

#include "qoresmokeglobal.h"
#include "typeinfohelpers.h"
#include "qoresmokeclass.h"
#include "qoremarshalling.h"

#include <QIcon>

int QoreQtIntCompatibleTypeInfoHelper::parseEqualImpl(const QoreTypeInfo *typeInfo) const {
   //printd(0, "QoreQtIntCompatibleTypeInfoHelper::parseEqualImpl() this=%p typeInfo=%s\n", this, typeInfoGetName(typeInfo));
   if (!typeInfo) return QTI_NOT_EQUAL;
   qore_type_t t = typeInfoGetType(typeInfo);
   if (t == NT_INT) return QTI_IDENT;
   return ClassMap::Instance()->checkEnum(typeInfoGetName(typeInfo)) ? QTI_AMBIGUOUS : QTI_NOT_EQUAL;
}

bool QoreQtStringCompatibleTypeInfoHelper::checkTypeInstantiationImpl(AbstractQoreNode *&n, ExceptionSink *xsink) const {
   if (!n)
      return false;

   if (n->getType() == NT_STRING)
      return true;

   if (n->getType() != NT_OBJECT)
      return false;

   QoreObject *o = reinterpret_cast<QoreObject *>(n);
   // see if we can get a QChar
   // if so, we let it get converted to the type needed by Qt in commonqoremethod.cpp
   return o->getClass(QC_QCHAR->getID()) ? true : false;
}

bool QRegionTypeHelper::checkTypeInstantiationImpl(AbstractQoreNode *&n, ExceptionSink *xsink) const {
   if (!n || n->getType() != NT_OBJECT)
      return false;

   QoreObject *o = reinterpret_cast<QoreObject *>(n);
   // see if we can get a QRect
   ReferenceHolder<QoreSmokePrivateData> pd(reinterpret_cast<QoreSmokePrivateData *>(o->getReferencedPrivateData(QC_QRECT->getID(), xsink)), xsink);
   if (!pd)
      return false;
   QRegion *qr = new QRegion(*(pd->getObject<QRect>()));
   QoreObject *rv = new QoreObject(QC_QREGION, getProgram());
   QoreSmokePrivateData *data = new QoreSmokePrivateData(SCI_QREGION, qr, rv);
   rv->setPrivate(QC_QREGION->getID(), data);
   n->deref(xsink);
   n = rv;
   return true;
}

bool QBrushTypeHelper::checkTypeInstantiationImpl(AbstractQoreNode *&n, ExceptionSink *xsink) const {
   const QoreBigIntNode *in = dynamic_cast<QoreBigIntNode *>(n);
   if (!in)
      return false;

   QBrush *br;
    
   const char *name = n->getTypeName();
   //printd(0, "QBrushTypeHelper::checkTypeInstantiationImpl() this=%p checking %s\n", this, name);
   if (!strcmp(name, "Qt::BrushStyle")) {
      Qt::BrushStyle i = (Qt::BrushStyle)in->val;
      br = new QBrush(i);
   }
   else if (!strcmp(name, "Qt::GlobalColor") || in->getType() == NT_INT) {
      Qt::GlobalColor i = (Qt::GlobalColor)in->val;
      br = new QBrush(i);
   }
   else
      return false;
   
   QoreObject *rv = new QoreObject(QC_QBRUSH, getProgram());
   QoreSmokePrivateData *data = new QoreSmokePrivateData(SCI_QBRUSH, br, rv);
   rv->setPrivate(QC_QBRUSH->getID(), data);
   n->deref(xsink);
   n = rv;
   return true;
}

bool QColorTypeHelper::checkTypeInstantiationImpl(AbstractQoreNode *&n, ExceptionSink *xsink) const {
   if (!n)
      return false;

   if (n->getType() != NT_INT && strcmp(n->getTypeName(), "Qt::GlobalColor"))
      return false;

   Qt::GlobalColor gc = (Qt::GlobalColor)n->getAsInt();
   n->deref(xsink);

   QColor *qc = new QColor(gc);
   QoreObject *rv = new QoreObject(QC_QCOLOR, getProgram());
   QoreSmokePrivateData *data = new QoreSmokePrivateData(SCI_QCOLOR, qc, rv);
   rv->setPrivate(QC_QCOLOR->getID(), data);
   n = rv;
   return true;
}

bool QVariantTypeHelper::canConvertIntern(qore_type_t t, const QoreClass *qc, const char *name) const {
   //printd(5, "QVariantTypeHelper::canConvertIntern() t=%d qc=%s\n", t, qc ? qc->getName() : "n/a");
   switch (t) {
      case NT_NOTHING: // NOTHING
      case NT_INT:
      case NT_FLOAT:
      case NT_STRING:
      case NT_BOOLEAN:
	 return true;
   }

   if (t != NT_OBJECT)
      return ClassMap::Instance()->checkEnum(name) ? true : false;
      
   if (qc->getClass(QC_QLOCALE->getID())
       || qc->getClass(QC_QICON->getID())
       || qc->getClass(QC_QBYTEARRAY->getID()))
      return true;
      
   return false;
}

bool QVariantTypeHelper::checkTypeInstantiationImpl(AbstractQoreNode *&n, ExceptionSink *xsink) const {
   QVariant *q = 0;

   qore_type_t t = n ? n->getType() : NT_NOTHING;
   if (dynamic_cast<QoreBigIntNode *>(n))
      t = NT_INT;

   // FIXME: implement all conversions
   switch (t) {
      case NT_NOTHING: // NOTHING
	 q = new QVariant();
	 break;
      case NT_INT:
	 q = new QVariant(n->getAsInt());
	 break;
      case NT_FLOAT:
	 q = new QVariant(n->getAsFloat());
	 break;
      case NT_STRING:
	 q = new QVariant(reinterpret_cast<const QoreStringNode*>(n)->getBuffer());
	 break;
	 //case NT_DATE : TODO/FIXME implement date marshalling in all places
      case NT_BOOLEAN:
	 q = new QVariant(n->getAsBool());
	 break;
      case NT_BINARY: {
	 const BinaryNode *b = reinterpret_cast<const BinaryNode *>(n);
	 q = new QVariant( QByteArray((const char *) b->getPtr(), b->size()) );
	 break;
      }
      case NT_OBJECT: {
	 const QoreObject *obj = reinterpret_cast<const QoreObject *>(n);
	 ReferenceHolder<QoreSmokePrivateData> p(xsink);
	    
	 // check for QLocale
	 p = reinterpret_cast<QoreSmokePrivateData*>(obj->getReferencedPrivateData(QC_QLOCALE->getID(), xsink));
	 if (*xsink) {
	    return 0;
	 }
	 if (p) {
	    // only call this once because it's a virtual call (slow)
	    void *o = p->object();
	    q = o ? new QVariant(*(reinterpret_cast<QLocale *>(o))) : 0;
	    // end of QLocale
	 } else {
	    // check for QIcon
	    p = reinterpret_cast<QoreSmokePrivateData*>(obj->getReferencedPrivateData(QC_QICON->getID(), xsink));
	    if (*xsink) {
	       return 0;
	    }
	    if (p) {
	       // only call this once because it's a virtual call (slow)
	       void *o = p->object();
	       q = o ? new QVariant(*(reinterpret_cast<QIcon *>(o))) : 0;
	    } // end of QIcon check
	    else {
	       // QByteArray
	       p = reinterpret_cast<QoreSmokePrivateData*>(obj->getReferencedPrivateData(QC_QBYTEARRAY->getID(), xsink));
	       if (*xsink) {
		  return 0;
	       }
	       if (p) {
		  // only call this once because it's a virtual call (slow)
		  void *o = p->object();
		  q = o ? new QVariant(*(reinterpret_cast<QByteArray *>(o))) : 0;
	       } // end of QByteArray check
	    }
	 }
	 break;
      }
   } // switch

   if (!q)
      return false;

   if (n)
      n->deref(xsink);
   n = Marshalling::createQoreObjectFromNonQObject(QC_QVARIANT, SCI_QVARIANT, q);
   return true;
}

