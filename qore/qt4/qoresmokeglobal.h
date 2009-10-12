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
#include <qore/QoreRWLock.h>

#include <smoke.h>
#include <qt_smoke.h>

#include <QtDebug>
#include <QHash>

#define QORESMOKEPROPERTY "qoreptr"

extern const QoreClass *QC_QOBJECT, *QC_QWIDGET, *QC_QABSTRACTITEMMODEL, *QC_QVARIANT,
*QC_QLOCALE, *QC_QBRUSH, *QC_QCOLOR, *QC_QDATE, *QC_QDATETIME, *QC_QTIME;

extern Smoke::ModuleIndex SMI_QOBJECT;
extern Smoke::Index SCI_QVARIANT, SCI_QLOCALE;

DLLLOCAL QoreObject *getQoreQObject(const QObject *obj);
DLLLOCAL QoreObject *getQoreObject(Smoke::Index classId, void *obj, QoreClass *&qc);
DLLLOCAL bool isptrtype(const char *var, const char *type);

extern Smoke::ModuleIndex QT_METACALL_ID;

extern QoreThreadLocalStorage<void> qore_qt_virtual_flag;

static inline void qore_smoke_set_virtual() {
    assert(!qore_qt_virtual_flag.get());
    qore_qt_virtual_flag.set((void *)1);
}

static inline void qore_smoke_clear_virtual() {
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

// map from non-qobject Qt objects to QoreObjects
typedef QHash<void *, QoreObject *> qt_qore_map_t;

class QtQoreMap : protected qt_qore_map_t, protected QoreRWLock {
public:
    DLLLOCAL ~QtQoreMap() {
        //assert(empty());
    }
    DLLLOCAL void add(void *qto, QoreObject *qo) {
        QoreAutoRWWriteLocker l(this);
        assert(!contains(qto));
        insert(qto, qo);
    }
    DLLLOCAL QoreObject *get(void *qto) {
        QoreAutoRWReadLocker l(this);
        return value(qto, 0);
    }
    DLLLOCAL void del(void *qto) {
        QoreAutoRWWriteLocker l(this);
        assert(contains(qto));
        remove(qto);
    }
};

DLLLOCAL extern QtQoreMap qt_qore_map;

static inline QoreObject *getQoreMappedObject(void *p) {
    return qt_qore_map.get(p);
}

static inline QoreObject *getQoreMappedObject(Smoke::Index classId, void *p) {
    return qt_Smoke->classes[classId].flags & Smoke::cf_virtual ? qt_qore_map.get(p) : 0;
}

DLLLOCAL const QoreMethod *findUserMethod(const QoreClass *qc, const char *name);

#endif

