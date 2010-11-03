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

#ifndef QOREQTDYNAMICMETHOD_H
#define QOREQTDYNAMICMETHOD_H

#include <smoke.h>
#include <qore/Qore.h>

#include <vector>
#include <memory>

class QObject;
class QMetaMethod;


DLLLOCAL void emitStaticSignal(Smoke * smoke,
                               QObject *sender,
                               int signalId,
                               const QMetaMethod &qmm,
                               const QoreListNode *args,
                               ExceptionSink *xsink);

struct temp_store_s;

// parent type for Signals and slots
class QoreQtDynamicMethod {
public:
     typedef std::vector<Smoke::Type> SmokeTypeList;
     SmokeTypeList typeList;

    DLLLOCAL QoreQtDynamicMethod(Smoke * smoke)
        : m_smoke(smoke) {
    }
     DLLLOCAL virtual ~QoreQtDynamicMethod() {
     }
     DLLLOCAL virtual bool isSignal() const = 0;
     DLLLOCAL virtual bool isSlot() const = 0;

     DLLLOCAL int identifyAndAddTypes(const char *sig, const char *p, ExceptionSink *xsink) {
          return identifyTypes(m_smoke, typeList, sig, p, xsink);
     }

     // static functions
     DLLLOCAL static int identifyTypes(Smoke * smoke, SmokeTypeList &typeList, const char *sig, const char *p, ExceptionSink *xsink);

     DLLLOCAL static int addType(Smoke * smoke, SmokeTypeList &typeList, const char *b, int len, const char *sig, ExceptionSink *xsink);

     DLLLOCAL static void qtToQore(Smoke * smoke, const Smoke::Type &t, void *arg, QoreListNode *args);

     DLLLOCAL static void qoreToQt(Smoke * smoke, ExceptionSink *xsink, const Smoke::Type &qtType, Smoke::StackItem &si,
                                   void *&ptr, void *&save, const AbstractQoreNode *val, const char *cname,
                                   const char *mname, int index = -1, bool value_required = false, temp_store_s *temp_store = 0);

     DLLLOCAL static void qoreToQtDirect(Smoke * smoke, const Smoke::Type &qtType, void *&ptr, const AbstractQoreNode *val, const char *cname, const char *mname);

     DLLLOCAL static void cleanup(const Smoke::Type &qtType, void *save) {
     }
protected:
    Smoke * m_smoke;
};

class QoreQtDynamicSlot : public QoreQtDynamicMethod {
public:
     DLLLOCAL QoreQtDynamicSlot(Smoke * smoke, const QoreObject *qo, const QoreMethod *meth, const char *sig, ExceptionSink *xsink);

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
     DLLLOCAL QoreQtDynamicSignal(Smoke * smoke, const char *sig, ExceptionSink *xsink);

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
