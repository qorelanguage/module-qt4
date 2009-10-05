/*
  qoresmokeglobal.h

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

#ifndef _QORE_QORESMOKEGLOBAL_H

#define _QORE_QORESMOKEGLOBAL_H

#include <qore/Qore.h>

#include <smoke.h>
#include <qt_smoke.h>

#include <QtDebug>

DLLLOCAL QoreObject *getQoreObject(Smoke::Index classId, void *obj, QoreClass *&qc);
DLLLOCAL bool isptrtype(const char *var, const char *type);

extern Smoke::ModuleIndex QT_METACALL_ID;

extern QoreThreadLocalStorage<void> qore_qt_virtual_flag;

static inline void qore_smoke_set_virtual() {
    assert(!qore_qt_virtual_flag.get());
    qore_qt_virtual_flag.set((void *)1);
}

static inline void qore_smoke_clear_virtual() {
    assert(qore_qt_virtual_flag.get());
    qore_qt_virtual_flag.set(0);
}

static inline bool qore_smoke_is_virtual() {
    return qore_qt_virtual_flag.get();
}

class QoreQtVirtualFlagHelper {
public:
    DLLLOCAL QoreQtVirtualFlagHelper() {
        qore_smoke_set_virtual();
    }
    DLLLOCAL ~QoreQtVirtualFlagHelper() {
        qore_smoke_clear_virtual();
    }
};

DLLLOCAL const QoreMethod *findUserMethod(const QoreClass *qc, const char *name);

#endif

