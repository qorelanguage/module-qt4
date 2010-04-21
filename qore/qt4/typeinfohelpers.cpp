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
#include <QDate>
#include <QDateTime>
#include <QTime>

// FIXME: add function to create enums from smoke - do not automatically convert from int

int QoreQtIntCompatibleTypeInfoHelper::parseEqualImpl(const QoreTypeInfo *typeInfo) const {
   //printd(0, "QoreQtIntCompatibleTypeInfoHelper::parseEqualImpl() this=%p typeInfo=%s\n", this, typeInfoGetName(typeInfo));
   if (!typeInfo) return QTI_NOT_EQUAL;
   qore_type_t t = typeInfoGetType(typeInfo);
   if (t == NT_INT) return QTI_IDENT;
   return t == NT_FLOAT || ClassMap::Instance()->checkEnum(typeInfoGetName(typeInfo)) ? QTI_AMBIGUOUS : QTI_NOT_EQUAL;
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

   n->deref(xsink);
   n = Marshalling::createQoreObjectFromNonQObject(QC_QREGION, SCI_QREGION, qr);
   return true;
}

bool QBrushTypeHelper::checkTypeInstantiationImpl(AbstractQoreNode *&n, ExceptionSink *xsink) const {
   QBrush *br;

   if (n && n->getType() == NT_OBJECT) {
      QoreObject *o = reinterpret_cast<QoreObject *>(n);
      // see if we can get a QColor
      PrivateDataRefHolder<QoreSmokePrivateData> pd(o, QC_QCOLOR->getID(), xsink);
      if (!pd)
	 return false;
      br = new QBrush(*(pd->getObject<QColor>()));
   }
   else {
      const QoreBigIntNode *in = dynamic_cast<QoreBigIntNode *>(n);
      if (!in)
	 return false;
    
      const char *name = n->getTypeName();
      //printd(0, "QBrushTypeHelper::checkTypeInstantiationImpl() this=%p checking %s\n", this, name);
      if (!strcmp(name, "Qt::BrushStyle")) {
	 Qt::BrushStyle i = (Qt::BrushStyle)in->val;
	 br = new QBrush(i);
      }
      else if (!strcmp(name, "Qt::GlobalColor") || (in->getType() == NT_INT && in->val >= 0 && in->val < 20)) {
	 Qt::GlobalColor i = (Qt::GlobalColor)in->val;
	 br = new QBrush(i);
      }
      else
	 return false;
   }

   n->deref(xsink);
   n = Marshalling::createQoreObjectFromNonQObject(QC_QBRUSH, SCI_QBRUSH, br);
   return true;
}

bool QColorTypeHelper::checkTypeInstantiationImpl(AbstractQoreNode *&n, ExceptionSink *xsink) const {
   if (!n)
      return false;

   if (n->getType() != NT_INT && strcmp(n->getTypeName(), "Qt::GlobalColor"))
      return false;

   QColor *qc;

   int v = n->getAsInt();
   // treat as RGB value
   if (n->getType() == NT_INT && (v < 0 || v >= 20)) {
      qc = new QColor(QRgb(v));
   }
   else {
      Qt::GlobalColor gc = (Qt::GlobalColor)v;
      qc = new QColor(gc);
   }

   n->deref(xsink);
   n = Marshalling::createQoreObjectFromNonQObject(QC_QCOLOR, SCI_QCOLOR, qc);
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

bool QKeySequenceTypeHelper::canConvertIntern(qore_type_t t, const char *name) const {
   //printd(5, "QVariantTypeHelper::canConvertIntern() t=%d qc=%s\n", t, qc ? qc->getName() : "n/a");
   if (t == NT_INT || t == NT_STRING)
      return true;

   return (!strcmp(name, "QKeySequence::StandardKey") || !strcmp(name, "Qt::Key")) ? true : false;
}

bool QKeySequenceTypeHelper::checkTypeInstantiationImpl(AbstractQoreNode *&n, ExceptionSink *xsink) const {
   if (!n)
      return false;
   qore_type_t t = n->getType();
   const char *name = n->getTypeName();

   QKeySequence *qks;
   if (t == NT_STRING)
      qks = new QKeySequence(reinterpret_cast<const QoreStringNode*>(n)->getBuffer());
   else if (t == NT_INT || !strcmp(name, "Qt::Key"))
      qks = new QKeySequence(reinterpret_cast<const QoreBigIntNode*>(n)->val);
   else if (!strcmp(name, "QKeySequence::StandardKey"))
      qks = new QKeySequence((QKeySequence::StandardKey)(reinterpret_cast<const QoreBigIntNode*>(n)->val));
   else
      return false;

   n->deref(xsink);
   n = Marshalling::createQoreObjectFromNonQObject(QC_QKEYSEQUENCE, SCI_QKEYSEQUENCE, qks);
   return true;
}

bool QDateTypeHelper::checkTypeInstantiationImpl(AbstractQoreNode *&n, ExceptionSink *xsink) const {
   if (!n || n->getType() != NT_DATE)
      return false;

   const DateTimeNode *d = reinterpret_cast<const DateTimeNode *>(n);
   qore_tm info;
   d->getInfo(info);

   QDate *date = new QDate(info.year, info.month, info.day);

   n->deref(xsink);
   n = Marshalling::createQoreObjectFromNonQObject(QC_QDATE, SCI_QDATE, date);
   return true;
}

bool QDateTimeTypeHelper::checkTypeInstantiationImpl(AbstractQoreNode *&n, ExceptionSink *xsink) const {
   if (!n || n->getType() != NT_DATE)
      return false;

   const DateTimeNode *d = reinterpret_cast<const DateTimeNode *>(n);
   QDateTime *date = new QDateTime;

   qore_tm info;
   d->getInfo(info);

   date->setDate(QDate(info.year, info.month, info.day));
   date->setTime(QTime(info.hour, info.minute, info.second, info.us / 1000));

   n->deref(xsink);
   n = Marshalling::createQoreObjectFromNonQObject(QC_QDATETIME, SCI_QDATETIME, date);
   return true;
}

bool QTimeTypeHelper::checkTypeInstantiationImpl(AbstractQoreNode *&n, ExceptionSink *xsink) const {
   if (!n || n->getType() != NT_DATE)
      return false;

   const DateTimeNode *d = reinterpret_cast<const DateTimeNode *>(n);

   qore_tm info;
   d->getInfo(info);

   QTime *time = new QTime(info.hour, info.minute, info.second, info.us / 1000);

   n->deref(xsink);
   n = Marshalling::createQoreObjectFromNonQObject(QC_QTIME, SCI_QTIME, time);
   return true;
}
