/* -*- indent-tabs-mode: nil -*- */
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

bool QRegionTypeHelper::acceptInputImpl(AbstractQoreNode *&n, ExceptionSink *xsink) const {
   qore_type_t t = get_node_type(n);

   if (t != NT_OBJECT)
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

bool QBrushTypeHelper::acceptInputImpl(AbstractQoreNode *&n, ExceptionSink *xsink) const {
   qore_type_t t = get_node_type(n);

   QBrush *br;

   if (t == NT_OBJECT) {
      QoreObject *o = reinterpret_cast<QoreObject *>(n);
      // see if we can get a QColor
      PrivateDataRefHolder<QoreSmokePrivateData> pd(o, QC_QCOLOR->getID(), xsink);
      if (!pd)
         return false;
      br = new QBrush(*(pd->getObject<QColor>()));
   }
   else {
      // not safe to use this pointer until we verify the type
      QoreBigIntNode *in = reinterpret_cast<QoreBigIntNode *>(n);

      const char *name = get_type_name(n);
      //printd(0, "QBrushTypeHelper::checkTypeInstantiationImpl() this=%p checking %s\n", this, name);
      if (n && !strcmp(name, "Qt::BrushStyle")) {
	 Qt::BrushStyle i = (Qt::BrushStyle)in->val;
	 br = new QBrush(i);
      }
      else if (n && (!strcmp(name, "Qt::GlobalColor") || (n->getType() == NT_INT && in->val >= 0 && in->val < 20))) {
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

bool QColorTypeHelper::acceptInputImpl(AbstractQoreNode *&n, ExceptionSink *xsink) const {
   if (!n || (n->getType() != NT_INT && strcmp(n->getTypeName(), "Qt::GlobalColor")))
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

bool QVariantTypeHelper::acceptInputImpl(AbstractQoreNode *&n, ExceptionSink *xsink) const {
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

         // keep it synchronized with ClassMap::addQoreMethods()
         q = acceptQtObject<QLocale>(QC_QLOCALE->getID(), obj, xsink);
         if (q) break;

         q = acceptQtObject<QIcon>(QC_QICON->getID(), obj, xsink);
         if (q) break;

         q = acceptQtObject<QByteArray>(QC_QBYTEARRAY->getID(), obj, xsink);
         if (q) break;

         q = acceptQtObject<QFont>(QC_QFONT->getID(), obj, xsink);
         if (q) break;
         
         q = acceptQtObject<QBrush>(QC_QBRUSH->getID(), obj, xsink);
         if (q) break;

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

template<class T>
QVariant * QVariantTypeHelper::acceptQtObject(qore_classid_t classID, const QoreObject *obj, ExceptionSink *xsink) const {
   ReferenceHolder<QoreSmokePrivateData> p(xsink);
   p = reinterpret_cast<QoreSmokePrivateData*>(obj->getReferencedPrivateData(classID, xsink));
   if (*xsink) {
      return 0;
   }
   if (p) {
      // only call this once because it's a virtual call (slow)
      void *o = p->object();
      return o ? new QVariant(*(reinterpret_cast<T *>(o))) : 0;
   }
   return 0; 
}


bool QKeySequenceTypeHelper::acceptInputImpl(AbstractQoreNode *&n, ExceptionSink *xsink) const {
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

bool QDateTypeHelper::acceptInputImpl(AbstractQoreNode *&n, ExceptionSink *xsink) const {
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

bool QDateTimeTypeHelper::acceptInputImpl(AbstractQoreNode *&n, ExceptionSink *xsink) const {
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

bool QTimeTypeHelper::acceptInputImpl(AbstractQoreNode *&n, ExceptionSink *xsink) const {
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
