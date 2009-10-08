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

#ifndef COMMONQOREMETHOD_H
#define COMMONQOREMETHOD_H

#include <qore/Qore.h>
#include <smoke.h>
#include <QByteArray>
#include <QString>
#include <QMap>
#include <QKeySequence>
#include <QBrush>
#include <QPen>
#include <QColor>

#include "qoresmokeclass.h"
#include "qoremarshalling.h"


class ArgStringSink;
extern ArgStringSink qore_string_sink;

extern Smoke* qt_Smoke;

int get_qstring(QString &qstring, const AbstractQoreNode *n, ExceptionSink *xsink);

class ArgStringList {
    friend class ArgStringSink;

private:
    char **ptr;
    int size, allocated;
    int argc;

protected:
    DLLLOCAL ArgStringList() : ptr(0), size(0), allocated(0) {
//         printd(0, "ArgStringList::ArgStringList() this=%p\n", this);
    }

public:
    DLLLOCAL ~ArgStringList() {
//         printd(0, "ArgStringList::~ArgStringList() this=%p size=%d\n", this, size);
        while (size--) {
            if (ptr[size])
                free(ptr[size]);
        }
        if (ptr)
            free(ptr);
    }

    DLLLOCAL void addStr(const char *str) {
        if (size >= allocated) {
            int d = size >> 2;
            allocated = size + (d < 10 ? 10 : d);
            allocated = (allocated / 16 + 1) * 16; // use complete cache line
            ptr = (char **)realloc(ptr, allocated * sizeof(char *));
        }
        ptr[size++] = str ? strdup(str) : 0;
//         printd(0, "ArgStringList() this=%p ptr=%p added '%s' at %p size=%d allocated=%d\n", this, ptr, ptr[size-1], ptr[size-1], size, allocated);
    }

    DLLLOCAL int *getArgcPtr() {
        return &size;
    }

    DLLLOCAL char **getPtr() {
        return ptr;
    }

    DLLLOCAL int getSize() const {
        return size;
    }
};

// we store all ArgStringList objects in a data structure that will be deleted when the module is deleted
// so that the int &argc, char **argv arguments to QCoreApplication and QApplication are valid during the
// lifetime of the objects (HACK!)
class ArgStringSink : public QoreThreadLock {
private:
    QList<ArgStringList *> list;

public:
    DLLLOCAL ArgStringSink() {}
    DLLLOCAL ~ArgStringSink() {
        foreach (ArgStringList *l, list) {
            delete l;
        }
    }
    DLLLOCAL ArgStringList *get() {
        ArgStringList *l = new ArgStringList();
        lock();
        list.push_back(l);
        unlock();
        return l;
    }
};

// storage for references
struct ref_store_s {
    // flag for field used
    enum ref_type {
        r_none,          // no value
        r_int,           // for integers (q_int)
        r_str,           // for strings (q_str)
        r_bool,          // for bool*
        r_sl,            // for a list of strings (q_sl)
        r_qstr,          // for a QString*
        r_qkeysequence,  // for a QKeySequence
        r_qbrush,        // for a QBrush
        r_qpen,          // for a QPen
        r_qcolor,        // for a QColor
        r_qreal,         // for qreal
        r_container,     // for QLists, QVectors, QMaps etc. - all with templates
        r_qvariant,      // for QVariant
    };

    const ReferenceNode *ref;
    AbstractQoreNode *ref_value;
    enum ref_type type;
    bool have_ref_value;
    union data_store_u {
        int q_int;
        char *q_str;
        bool q_bool;
        QString *q_qstr;
        ArgStringList *q_sl;
        qreal q_qreal;
        QKeySequence *q_qkeysequence;
        QBrush *q_qbrush;
        QPen *q_qpen;
        QColor *q_qcolor;
        Marshalling::QoreQListBase * q_container;
        Marshalling::QoreQVariant * q_qvariant;
    } data;

    DLLLOCAL ref_store_s() : ref(0), ref_value(0), type(r_none), have_ref_value(false) {}
    DLLLOCAL ~ref_store_s() {
//         printd(0, "ref_store_s::~ref_store_s() this=%p, ref=%p, ref_value=%p, type=%d, have_ref_value=%d\n", this, ref, ref_value, type, have_ref_value);
        switch (type) {
        case r_str:
            if (data.q_str) {
//          printd(0, "ref_store_s::~ref_store_s() this=%p freeing string ptr %p\n", this, data.q_str);
                free(data.q_str);
            }
            break;
        case r_qstr:
//           printd(0, "ref_store_s::~ref_store_s() this=%p deleting QString ptr %p\n", this, data.q_qstr);
            delete data.q_qstr;
            break;
        case r_qkeysequence:
//           printd(0, "ref_store_s::~ref_store_s() this=%p deleting QKeySequence ptr %p\n", this, data.q_qkeysequence);
            delete data.q_qkeysequence;
            break;
        case r_qbrush:
//           printd(0, "ref_store_s::~ref_store_s() this=%p deleting QBrush ptr %p\n", this, data.q_qbrush);
            delete data.q_qbrush;
            break;
        case r_qpen:
//           printd(0, "ref_store_s::~ref_store_s() this=%p deleting QPen ptr %p\n", this, data.q_qpen);
            delete data.q_qpen;
            break;
        case r_qcolor:
//           printd(0, "ref_store_s::~ref_store_s() this=%p deleting QColor ptr %p\n", this, data.q_qcolor);
            delete data.q_qcolor;
            break;
        case r_container:
            delete data.q_container;
            break;
        case r_qvariant:
            delete data.q_qvariant;
            break;
        default:
            (void)data; // suppress compiler warning
        }
    }

    DLLLOCAL void save_ref_value(const AbstractQoreNode *v) {
        ref_value = v ? v->refSelf() : 0;
        have_ref_value = true;
    }

    DLLLOCAL void assign(QString *v) {
        type = r_qstr;
        data.q_qstr = v;
    }
    DLLLOCAL void assign(int v) {
        //printd(0, "ref_store_s() this=%p assigning integer %d\n", this, v);
        type = r_int;
        data.q_int = v;
    }
    DLLLOCAL void assign(char *v) {
        //printd(0, "ref_store_s::assign('%s' (%p)) this=%p\n", v, v, this);
        type = r_str;
        data.q_str = v;
    }
    DLLLOCAL void assign(bool v) {
        type = r_bool;
        data.q_bool = v;
    }
    DLLLOCAL void assign(ArgStringList *v) {
        //printd(0, "ref_store_s() this=%p assigning string list %p (size %d)\n", this, v, v->getSize());
        type = r_sl;
        data.q_sl = v;
    }
    DLLLOCAL void assign(qreal v) {
        type = r_qreal;
        data.q_qreal = v;
    }
    DLLLOCAL void assign(QKeySequence *v) {
        type = r_qkeysequence;
        data.q_qkeysequence = v;
    }
    DLLLOCAL void assign(QBrush *v) {
        type = r_qbrush;
        data.q_qbrush = v;
    }
    DLLLOCAL void assign(QPen *v) {
        type = r_qpen;
        data.q_qpen = v;
    }
    DLLLOCAL void assign(QColor *v) {
        type = r_qcolor;
        data.q_qcolor = v;
    }
    DLLLOCAL void assign(Marshalling::QoreQListBase * v) {
        type = r_container;
        data.q_container = v;
    }
    DLLLOCAL void assign(Marshalling::QoreQVariant * v) {
        type = r_qvariant;
        data.q_qvariant = v;
    }

    DLLLOCAL void *getPtr() {
        void *ptr;
        switch (type) {
        case r_int:
            ptr = &data.q_int;
            break;
        case r_str:
            ptr = data.q_str;
            break;
        case r_bool:
            ptr = &data.q_bool;
            break;
        case r_sl:
            ptr = data.q_sl->getPtr();
            break;
        case r_qstr:
            ptr = data.q_qstr;
            break;
        case r_qreal:
            ptr = &data.q_qreal;
            break;
        case r_qkeysequence:
            ptr = data.q_qkeysequence;
            break;
        case r_qbrush:
            ptr = data.q_qbrush;
            break;
        case r_qpen:
            ptr = data.q_qpen;
            break;
        case r_qcolor:
            ptr = data.q_qcolor;
            break;
        case r_container:
            ptr = data.q_container->voidp();
            break;
        case r_qvariant:
            ptr = data.q_qvariant->s_class();
            break;
        default:
            assert(false);
            ptr = 0;
            break;
        }
        //printd(0, "ref_store_s::getPtr() this=%p returning %p\n", this, ptr);
        return ptr;
    }
};

// Helper class to handle method's arguments,
// name munging and searching in Smoke structures.
// It's used as a "library" for common_foo funtions
class CommonQoreMethod {
public:
    CommonQoreMethod(QoreObject *n_self,
             QoreSmokePrivate *n_smc,
                     const char* className,
                     const char* methodName,
                     const QoreListNode* params,
                     ExceptionSink *xsink);
    ~CommonQoreMethod();

    DLLLOCAL QoreObject *getQoreObject() {
       return self;
    }
    DLLLOCAL QoreSmokePrivate *getPrivateData() {
       return smc;
    }
    DLLLOCAL Smoke::Class smokeClass() {
        return qt_Smoke->classes[m_method.classId];
    }
    DLLLOCAL const char* mungedMethod() {
        return m_munged.data();
    }
    DLLLOCAL Smoke::Method &method() {
        return m_method;
    }
    DLLLOCAL bool isValid() {
        return m_valid;
    }
    DLLLOCAL const char *getClassName() const {
        return m_className;
    }
    DLLLOCAL const char *getMethodName() const {
        return m_methodName;
    }
    DLLLOCAL ref_store_s *getRefEntry(int index) {
        checkRefStore();
        ref_store_s &val = (*ref_store)[index];
        return &val;
    }

    DLLLOCAL int getObject(Smoke::Index classId, const AbstractQoreNode *v, ReferenceHolder<QoreSmokePrivate> &c, int index, bool nullOk = false);

    DLLLOCAL AbstractQoreNode *returnValue();
    DLLLOCAL void postProcessConstructor(QoreSmokePrivate *n_smc, Smoke::StackItem rv);
    DLLLOCAL int getArgCount() const {
        return qoreArgCnt;
    }
    DLLLOCAL ClassMap::TypeHandler &getTypeHandler() {
        return type_handler;
    }
    DLLLOCAL void qoreToStack(Smoke::Type t,
                              const AbstractQoreNode * node,
                              int index);

    DLLLOCAL AutoVLock &getVLock() {
        return vl;
    }

    DLLLOCAL void saveParam(const char *key, const AbstractQoreNode *val) {
        assert(type_handler.return_value_handler);
        if (!tparams)
            tparams = new QoreHashNode();
        tparams->setKeyValue(key, val ? val->refSelf() : 0, m_xsink);
    }

    DLLLOCAL const QoreHashNode *getParams() const {
        return tparams;
    }

    DLLLOCAL AbstractQoreNode *callMethod();

    DLLLOCAL void suppressMethod() {
       assert(!suppress_method);
       suppress_method = true;
    }

    // static functions
    DLLLOCAL static int qoreToStackStatic(ExceptionSink *xsink, Smoke::StackItem &si, const char *className, const char *methodName, Smoke::Type t, const AbstractQoreNode *node, int index = -1, CommonQoreMethod *cqm = 0, bool temp = false);

    DLLLOCAL static int getObjectStatic(ExceptionSink *xsink, const char *className, const char *methodName, Smoke::Index classId, const AbstractQoreNode *v, ReferenceHolder<QoreSmokePrivate> &c, int index, bool nullOk = false);

    Smoke::Stack Stack;

private:
    typedef QMap<int,ref_store_s> RefMap;

    const char* m_className;
    const char* m_methodName;
    ExceptionSink * m_xsink;
    QByteArray m_munged;
    Smoke::Method m_method;
    bool m_valid;
    int qoreArgCnt;
    RefMap *ref_store;
    AutoVLock vl;
    ClassMap::TypeHandler type_handler;
    QoreHashNode *tparams;
    QoreObject *self;
    QoreSmokePrivate *smc;
    bool suppress_method;

    DLLLOCAL void checkRefStore() {
        if (!ref_store)
            ref_store = new RefMap;
    }

    DLLLOCAL int getScore(Smoke::Type smoke_type, const AbstractQoreNode *n, int index);
};

#endif
