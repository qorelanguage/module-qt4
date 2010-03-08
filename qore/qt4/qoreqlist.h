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

#ifndef _QOREQTLIST_H
#define _QOREQTLIST_H

#include "qoresmokeclass.h"

template<class QLISTT>
class QoreListClassHelper {
public:
   DLLLOCAL static AbstractQoreNode *isEmpty(const QoreMethod &method, QoreObject *self, QoreSmokePrivateData *apd, const QoreListNode *args, ExceptionSink *xsink) {
      return get_bool_node(apd->getObject<QLISTT>()->isEmpty());
   }

   DLLLOCAL static AbstractQoreNode *count(const QoreMethod &method, QoreObject *self, QoreSmokePrivateData *apd, const QoreListNode *args, ExceptionSink *xsink) {
      return new QoreBigIntNode(apd->getObject<QLISTT>()->count());
   }
};

template<class QLISTT>
DLLLOCAL void addListMethods(QoreClass *qc) {   
   qc->addMethodExtended2("isEmpty", (q_method2_t)QoreListClassHelper<QLISTT>::isEmpty, false, QDOM_DEFAULT, boolTypeInfo);
   qc->addMethodExtended2("count", (q_method2_t)QoreListClassHelper<QLISTT>::count, false, QDOM_DEFAULT, bigIntTypeInfo);
   qc->addMethodExtended2("size", (q_method2_t)QoreListClassHelper<QLISTT>::count, false, QDOM_DEFAULT, bigIntTypeInfo);
}

#endif
