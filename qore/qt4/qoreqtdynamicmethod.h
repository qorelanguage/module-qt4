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

#ifndef QOREQTDYNAMICMETHOD_H
#define QOREQTDYNAMICMETHOD_H

#include <smoke.h>
#include <qore/Qore.h>

#include <vector>
#include <memory>

class QObject;
class QMetaMethod;

extern Smoke* qt_Smoke;


DLLLOCAL void emitStaticSignal(QObject *sender,
                               int signalId,
                               const QMetaMethod &qmm,
                               const QoreListNode *args,
                               ExceptionSink *xsink);


/*
class AbstractQoreQtDynamicType {
public:
   DLLLOCAL virtual ~QoreQtAbstractDynamicTypeHelper() {}
   // convert QT void * arg to Qore arg
   DLLLOCAL virtual void qtArgToQore(QoreListNode &args, void *arg) = 0;

   // convert Qore arg to QT void * arg
   DLLLOCAL virtual void qoreToQt(void *&ptr, void *&save, const AbstractQoreNode *val) = 0;

   // delete temporary memory
   DLLLOCAL virtual void deleteTemporary(void *ptr) = 0;

   DLLLOCAL virtual void qoreToQtReturn(void *rv, const AbstractQoreNode *val) = 0;
   DLLLOCAL virtual const char *getName() const = 0;

   DLLLOCAL virtual bool identify(const char *&p) = 0;
};

class SimpleQoreQtDynamicType {
private:
   const char *name;
   int size;

public:
   DLLLOCAL SimpleQoreQtDynamicType(const char *n) : name(n), size(strlen(n)) {
   }

   DLLLOCAL virtual const char *getName() const {
      return name;
   }

   DLLLOCAL bool identify(const char *&p) {
      if (!strncmp(name, p, size)) {
     // make sure there is no more to the passed type
     const char *p1 = p + size;
     while (*p1 == ' ' || *p1 == '\t')
        ++p1;

     if (!*p1 || *p1 == ',' || *p1 == ')') {
        p = p1;
        return true;
     }
      }
      return false;
   }
};
*/

// parent type for Signals and slots
class QoreQtDynamicMethod {
public:
    typedef std::vector<Smoke::Type> SmokeTypeList;
    SmokeTypeList typeList;

    DLLLOCAL virtual ~QoreQtDynamicMethod() {
    }
    DLLLOCAL virtual bool isSignal() const = 0;
    DLLLOCAL virtual bool isSlot() const = 0;

    DLLLOCAL int identifyAndAddTypes(const char *sig, const char *p, ExceptionSink *xsink) {
        return identifyTypes(typeList, sig, p, xsink);
    }

    // static functions
    DLLLOCAL static int identifyTypes(SmokeTypeList &typeList, const char *sig, const char *p, ExceptionSink *xsink);

    DLLLOCAL static int addType(SmokeTypeList &typeList, const char *b, int len, const char *sig, ExceptionSink *xsink);

    DLLLOCAL static void qtToQore(const Smoke::Type &t, void *arg, QoreListNode *args);

    DLLLOCAL static void qoreToQt(ExceptionSink *xsink, const Smoke::Type &qtType, Smoke::StackItem &si,
                                  void *&ptr, void *&save, const AbstractQoreNode *val, const char *cname, const char *mname, int index = -1, bool value_required = false);

    DLLLOCAL static void qoreToQtDirect(const Smoke::Type &qtType, void *&ptr, const AbstractQoreNode *val, const char *cname, const char *mname);

    DLLLOCAL static void cleanup(const Smoke::Type &qtType, void *save) {
    }
};

class QoreQtDynamicSlot : public QoreQtDynamicMethod {
public:
   DLLLOCAL QoreQtDynamicSlot(const QoreObject *qo, const QoreMethod *meth, const char *sig, ExceptionSink *xsink);

    DLLLOCAL virtual ~QoreQtDynamicSlot() {
    }

    DLLLOCAL virtual bool isSignal() const {
        return false;
    }
    DLLLOCAL virtual bool isSlot() const {
        return true;
    }

    // convert arguments and call method
    DLLLOCAL void call(QoreObject *self, void **arguments) const;

private:
    Smoke::Type returnType;
    QoreObject *qore_obj;
    const QoreMethod *method;
};

class QoreQtDynamicSignal : public QoreQtDynamicMethod {
public:
    DLLLOCAL QoreQtDynamicSignal(const char *sig, ExceptionSink *xsink);

    DLLLOCAL virtual ~QoreQtDynamicSignal() {
    }

    DLLLOCAL virtual bool isSignal() const {
        return true;
    }
    DLLLOCAL virtual bool isSlot() const {
        return false;
    }

    DLLLOCAL void emitSignal(QObject *obj, int id, const QoreListNode *args, ExceptionSink *xsink);
};

typedef std::vector<QoreQtDynamicMethod *> qore_qt_method_list_t;

class DynamicMethodList : public qore_qt_method_list_t {
public:
    DLLLOCAL ~DynamicMethodList() {
        for (qore_qt_method_list_t::iterator i = begin(), e = end(); i != e; ++i)
            delete *i;
    }
    DLLLOCAL int addMethod(QoreQtDynamicSlot *slot) {
        int id = size();
        push_back(slot);
        return id;
    }
    DLLLOCAL int addMethod(QoreQtDynamicSignal *sig) {
        int id = size();
        push_back(sig);
        return id;
    }
};



#endif
