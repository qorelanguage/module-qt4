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

#ifndef QORESMOKECLASS_H
#define QORESMOKECLASS_H

#include <qore/Qore.h>
#include <smoke.h>
#include <qore/QoreRWLock.h>

#include <QStringList>
#include <QMap>
#include <QPointer>
#include <QHash>
#include <QMetaMethod>

#include <map>
#include <set>

#include "qoreqtdynamicmethod.h"
#include "qoreqtenumnode.h"

// Custom AbstractPrivateData for any Qt object to be used as
// a Qore object private member. See Qore object/classes implementation.
class QoreSmokePrivate : public AbstractPrivateData {
public:
   DLLLOCAL QoreSmokePrivate(Smoke::Index classID, bool n_is_qobject = false) : m_class(classID), externally_owned(false), is_qobject(n_is_qobject), obj_ref(false) {}
   DLLLOCAL virtual ~QoreSmokePrivate() {
      assert(!obj_ref);
   }
   DLLLOCAL virtual void * object() = 0;
   DLLLOCAL virtual void clear() = 0;
   // the following is defined as pure virtual so only 1 virtual call has to be made when calling it
   // (it could be implemented here with 2 other virtual calls)
   DLLLOCAL virtual void *takeObjectForDelete(QoreObject *self, ExceptionSink *xsink) = 0;
   DLLLOCAL Smoke::Index smokeClass() {
      return m_class;
   }

   DLLLOCAL virtual bool deleteBlocker(QoreObject *self) = 0;
   DLLLOCAL virtual void externalDelete(QoreObject *obj, ExceptionSink *xsink) = 0;

   DLLLOCAL void setExternallyOwned() {
      externally_owned = true;
   }

   DLLLOCAL bool externallyOwned() const {
      return externally_owned;
   }
   DLLLOCAL const char *getClassName() const {
      return qt_Smoke->classes[m_class].className;
   }
   DLLLOCAL Smoke::Index classIndex() const {
      return m_class;
   }
   DLLLOCAL bool isQObject() const {
      return is_qobject;
   }

protected:
   Smoke::Index m_class;
   bool externally_owned : 1;
   bool is_qobject : 1;
   bool obj_ref : 1;
};

class QoreSmokePrivateData : public QoreSmokePrivate {
   bool save_map;
public:
   DLLLOCAL QoreSmokePrivateData(Smoke::Index classID, void *p, QoreObject *self) : QoreSmokePrivate(classID), m_object(p) {
      // register in object map if object has virtual functions
      save_map = qt_Smoke->classes[classID].flags & Smoke::cf_virtual;
      if (save_map)
         qt_qore_map.add(p, self);
   }
   DLLLOCAL virtual ~QoreSmokePrivateData();

   DLLLOCAL virtual bool deleteBlocker(QoreObject *self) {
      printd(5, "QoreSmokePrivateData::deleteBlocker(%p) %s obj=%p eo=%s obj_ref=%d\n", self, self->getClassName(), m_object, externallyOwned() ? "true" : "false", obj_ref);
      if (m_object && externallyOwned()) {
	 if (!obj_ref) {
	    obj_ref = true;
	    // note that if we call QoreObject::ref() here, it will cause an immediate deadlock!
	    self->deleteBlockerRef();
	 }

	 //printd(5, "QoreSmokePrivateData::deleteBlocker(%p) returning true\n", self);
	 return true;
      }
      return false;
   }

   DLLLOCAL virtual void externalDelete(QoreObject *obj, ExceptionSink *xsink) {
      if (m_object) {
         // clear object before running destructor
         clear();
      
         //printd(0, "QoreSmokePrivateData::externalDelete() deleting %s object (obj_ref=%d)\n", obj->getClassName(), obj_ref);
         if (obj->isValid())
            obj->doDelete(xsink);
      }

      if (obj_ref) {
         obj_ref = false;
         obj->deref(xsink);
      }
   }

   DLLLOCAL virtual void *object() {
      return m_object;
   }
   DLLLOCAL virtual void clear() {
      if (save_map && m_object)
         qt_qore_map.del(m_object);
      m_object = 0;
   }
   DLLLOCAL virtual void *takeObjectForDelete(QoreObject *self, ExceptionSink *xsink) {
      void *p = m_object;
      QoreSmokePrivateData::clear();

      if (obj_ref) {
         obj_ref = false;
         self->deref(xsink);
      }

      return p;
   }
   // non-virtual method for getting the object
   template <class T>
   DLLLOCAL T *getObject() {
      return reinterpret_cast<T *>(m_object);
   }

private:
   void * m_object;
};

class QoreSmokePrivateQObjectData : public QoreSmokePrivate {
public:
   DLLLOCAL QoreSmokePrivateQObjectData(Smoke::Index classID, QObject *p, QoreObject *self) : QoreSmokePrivate(classID, true), m_qobject(p) {
      //printd(0, "QoreSmokePrivateQObjectData::QoreSmokePrivateQObjectData() %s qobject=%p self=%p this=%p\n", getClassName(), p, self, this);

      qt_metaobject_method_count = getParentMetaObject()->methodCount();
      Smoke::ModuleIndex mi = qt_Smoke->findMethod(qt_Smoke->classes[classID].className, "qt_metacall$$?");
      assert(mi.smoke);
      qt_metacall_index = qt_Smoke->methodMaps[mi.index].method;
      assert(qt_metacall_index > 0);
      assert(!strcmp(qt_Smoke->methodNames[qt_Smoke->methods[qt_metacall_index].name], "qt_metacall"));
      
      if (p->isWidgetType()) {
	 assert(strcmp(qt_Smoke->classes[classID].className, "QApplication"));
	 QWidget *qw = reinterpret_cast<QWidget *>(p);
	 // add to QoreWidgetManager if it's a window - so it can be deleted if necessary
	 // when the QApplication object is deleted
	 // but do not delete the QDesktopWidget
	 if (!qw->parent() && classID != SCI_QDESKTOPWIDGET)
	    QWM.add(qw);
      }

      qt_qore_map.add(p, self);
   }
   DLLLOCAL virtual ~QoreSmokePrivateQObjectData() {
      //printd(0, "QoreSmokePrivateQObjectData::~QoreSmokePrivateQObjectData() this=%p obj=%p (%s)\n", this, m_qobject.data(), getClassName());
      // set property to 0 because QoreObject is being deleted

      QObject *qo = m_qobject.data();
      if (qo) {
	 {
	    QoreQtVirtualFlagHelper vfh;
	    m_qobject->setProperty(QORESMOKEPROPERTY, (qulonglong)0);
	 }

	 qt_qore_map.del(qo);
	 
	 if (!externallyOwned() && !m_qobject->parent()) {
	    delete m_qobject;
	 }
      }
   }

   DLLLOCAL virtual bool deleteBlocker(QoreObject *self) {
      //printd(5, "QoreSmokePrivateQObjectData::deleteBlocker(%p) %s obj=%p parent=%p eo=%s obj_ref=%d\n", self, self->getClassName(), m_qobject.data(), m_qobject.data() ? m_qobject->parent() : 0, externallyOwned() ? "true" : "false", obj_ref);
      if (m_qobject.data() && (m_qobject->parent() || externallyOwned())) {
	 if (!obj_ref) {
	    obj_ref = true;
	    // note that if we call QoreObject::ref() here, it will cause an immediate deadlock!
	    self->deleteBlockerRef();
	 }

	 //printd(5, "QoreSmokePrivateQObjectData::deleteBlocker(%p) returning true\n", self);
	 return true;
      }
      return false;
   }

    DLLLOCAL virtual void *object() {
        return m_qobject.data();
    }
    DLLLOCAL virtual void clear() {
       //printd(5, "QoreSmokePrivateQObjectData::clear() this=%p obj=%p (%s)\n", this, m_qobject.data(), getClassName());

       assert(m_qobject.data());
       if (m_qobject.data()->isWidgetType())
	  QWM.remove(reinterpret_cast<QWidget *>(m_qobject.data()));

       qt_qore_map.del(m_qobject.data());
       m_qobject = 0;
    }
   DLLLOCAL virtual void *takeObjectForDelete(QoreObject *self, ExceptionSink *xsink) {
        void *p = (void *)m_qobject.data();
	clear();

        if (obj_ref) {
           obj_ref = false;
           self->deref(xsink);
        }

        return p;
    }

    DLLLOCAL QObject *qobject() {
        return m_qobject.data();
    }
    DLLLOCAL Smoke::Index getMetacallIndex() const {
        return qt_metacall_index;
    }

    DLLLOCAL int createSignal(const char *signal, ExceptionSink *xsink) {
        QByteArray theSignal = QMetaObject::normalizedSignature(signal);
        int signalId = signalIndices.value(theSignal, -1);
        if (signalId >= 0) {
            xsink->raiseException("QOBJECT-CREATESIGNAL-ERROR", "signal '%s' already exists in this object (class %s)", signal, getClassName());
            return -1;
        }

        std::auto_ptr<QoreQtDynamicSignal> ds(new QoreQtDynamicSignal(signal, xsink));
        if (*xsink)
            return -1;

        signalId = methodList.addMethod(ds.release());
        signalIndices[theSignal] = signalId;

        //printd(0, "%s::createDynamicSignal() this=%08p id=%d '%s'\n", getClassName(), this, signalId, signal);
        return 0;
    }

    DLLLOCAL void handleSignal(QoreObject *qore_obj, int id, void **arguments) {
        assert(id < (int)methodList.size());

        QoreQtDynamicMethod *dmeth = methodList[id];

        QoreQtDynamicSignal *sig = dynamic_cast<QoreQtDynamicSignal *>(dmeth);
        if (sig) {
            id += qt_metaobject_method_count;
            // warning: this function is listed as "internal"
            QMetaObject::activate(m_qobject, id, id, arguments);
        } else {
            QoreQtDynamicSlot *slot = reinterpret_cast<QoreQtDynamicSlot *>(dmeth);
            slot->call(qore_obj, arguments);
        }
    }

    // receiver = QoreObject corresponding to this object
    DLLLOCAL bool connectDynamic(QoreSmokePrivateQObjectData *sender, const char *signal, const QoreObject *receiver, const char *slot, ExceptionSink *xsink) {
        if (!signal || signal[0] != '2') {
            xsink->raiseException("QT-CONNECT-ERROR", "expecting a value processed with SIGNAL() to give the signal name and signature");
            return false;
        }

        if (!slot || (slot[0] != '1' && slot[0] != '2')) {
            xsink->raiseException("QT-CONNECT-ERROR", "expecting a value processed with SIGNAL() (to connect a signal to a signal) or with SLOT() (to connect a signal to a slot) to give the target (signal or slot) name and signature");
            return false;
        }

        bool is_signal = slot[0] == '2';

        QByteArray theSignal = QMetaObject::normalizedSignature(signal + 1);
        QByteArray theSlot = QMetaObject::normalizedSignature(slot + 1);

        //printd(0, "connectDynamic() sig=%s (%s) slot=%s (%s)\n", signal, theSignal.data(), slot, theSlot.data());

        if (!QMetaObject::checkConnectArgs(theSignal, theSlot)) {
            xsink->raiseException("QT-CONNECT-ERROR", "cannot connect signal '%s' with '%s' due to incompatible arguments", signal + 1, slot + 1);
            //printd(5, "%s::connectDynamic(sender=%08p, signal=%s, slot=%s) this=%08p failed\n", getParentMetaObject()->className(), sender, signal, slot, this);
            return -1;
        }

        int targetId = is_signal ? getSignalIndex(theSlot) : getSlotIndex(receiver, theSlot, slot + 1, xsink);
        if (targetId < 0) {
            if (!*xsink) {
                if (is_signal)
                    xsink->raiseException("QT-CONNECT-ERROR", "target signal '%s' does not exist", slot + 1);
                else
                    xsink->raiseException("QT-CONNECT-ERROR", "target slot '%s' does not exist", slot + 1);
            }
            return -1;
        }

        int signalId = sender->getSignalIndex(theSignal);
        if (signalId < 0) {
            xsink->raiseException("QT-CONNECT-ERROR", "signal '%s' does not exist", signal + 1);
            return -1;
        }

        if (!QMetaObject::connect(sender->m_qobject, signalId, m_qobject, targetId)) {
            xsink->raiseException("QT-CONNECT-ERROR", "connection from signal '%s' to '%s' failed", signal + 1, slot + 1);
            return -1;
        }

        //printd(0, "%s::connectDynamic(sender=%08p, signal=%d:%s (meta=%d), receiver=%d:%s (meta=%d)) receiver=%08p success\n", receiver->getClassName(), sender, signalId, signal, sender->getParentMetaObject()->methodCount(), targetId, slot, getParentMetaObject()->methodCount(), receiver);
        return 0;
    }

    DLLLOCAL void emitSignal(const char *sig, const QoreListNode *args, ExceptionSink *xsink) {
        QByteArray theSignal = QMetaObject::normalizedSignature(sig);
        const QMetaObject *mo = getParentMetaObject();

        int id = mo->indexOfSignal(theSignal);

        if (id >= 0) {
            QMetaMethod qmm = mo->method(id);
            //printd(0, "%s::emitSignal(%s, %p) static signal %d\n", mo->className(), sig, args, id);

            emitStaticSignal(m_qobject.data(), id, qmm, args, xsink);
        } else { // emit dynamic signal
            int signalId = signalIndices.value(theSignal, -1);
            if (signalId < 0) {
                xsink->raiseException("EMIT-ERROR", "no signal found matching signature '%s'; register signals by calling QObject::createSignal(); note that signatures (arguments) must be C-style and must match", sig);
                return;
            }
            //printd(0, "emitSignal(%s, %p) dynamic signal %d\n", sig, args, signalId);
            QoreQtDynamicSignal *sp = reinterpret_cast<QoreQtDynamicSignal *>(methodList[signalId]);
            sp->emitSignal(m_qobject, signalId + mo->methodCount(), args, xsink);
        }
    }

    DLLLOCAL int getParentMethodCount() const {
       return qt_metaobject_method_count;
    }

    DLLLOCAL virtual void externalDelete(QoreObject *obj, ExceptionSink *xsink) {
       QObject *qo = m_qobject.data();
       if (qo && qo->isWidgetType())
	  QWM.remove(reinterpret_cast<QWidget *>(qo));

       // clear object before running destructor
       clear();

       //printd(0, "QoreSmokePrivateQObjectData::externalDelete() deleting %s object (obj_ref=%d)\n", obj->getClassName(), obj_ref);
       if (qo && obj->isValid())
	  obj->doDelete(xsink);

       if (obj_ref) {
	  obj_ref = false;
	  obj->deref(xsink);
       }
    }

protected:
    DLLLOCAL int getSignalIndex(const QByteArray &theSignal) const {
        const QMetaObject *pmo = getParentMetaObject();
        int signalId = pmo->indexOfSignal(theSignal);

        if (signalId >= 0) {
            //printd(5, "%s:getSignalIndex('%s') this=%08p is static (%d)\n", pmo->className(), theSignal.data(), this, signalId);
            return signalId;
        }

        signalId = signalIndices.value(theSignal, -1);
        if (signalId < 0) {
            //printd(5, "%s:getSignalIndex('%s') this=%08p does not exist\n", pmo->className(), theSignal.data(), this);
            return -1;
        }

        //printd(5, "%s:getSignalIndex('%s') this=%08p is dynamic (%d methodid %d)\n", pmo->className(), theSignal.data(), this, signalId, signalId + pmo->methodCount());
        return signalId + pmo->methodCount();
    }

    DLLLOCAL int getSlotIndex(const QoreObject *receiver, const QByteArray &theSlot, const char *sig, ExceptionSink *xsink) {
        // get parent meta object
        const QMetaObject *pmo = getParentMetaObject();

        // see if it's an existing dynamic slot
        int slotId = slotIndices.value(theSlot, -1);
        if (slotId >= 0) {
            //printd(5, "%s:getSlotIndex('%s') is dynamic (%d methodid %d) this=%08p\n", pmo->className(), theSlot.data(), slotId, slotId + pmo->methodCount(), this);
            return slotId + pmo->methodCount();
        }

	// see if such a user method exists
	const char *c = strchr(sig, '(');
	QoreString tmp;
	if (c)
	   tmp.concat(sig, c - sig);
	else
	   tmp.concat(sig);
	tmp.trim();

	const QoreClass *qc = receiver->getClass();
	bool special_method = !tmp.compare("constructor") || !tmp.compare("destructor") || !tmp.compare("copy");
	const QoreMethod *meth = findUserMethod(qc, tmp.getBuffer());
	if (!special_method && meth) {
	   std::auto_ptr<QoreQtDynamicSlot> ds(new QoreQtDynamicSlot(receiver, meth, sig, xsink));
	   if (*xsink)
	      return -1;
	   slotId = methodList.addMethod(ds.release());
	   slotIndices[theSlot] = slotId;
	   //printd(5, "%s::getSlotIndex() this=%08p created new dynamic slot, id=%d method_id=%d: '%s'\n", pmo->className(), this, slotId, slotId + pmo->methodCount(), theSlot.data());
	   
	   return slotId + pmo->methodCount();
	}

	// see if it's a builtin slot
        slotId = pmo->indexOfSlot(theSlot);
        // see if it's a static slot
        if (slotId >= 0) {
            //printd(5, "%s:getSlotIndex('%s') is static (%d) qo=%08p\n", pmo->className(), theSlot.data(), slotId, m_qobject);
            return slotId;
        }

	if (meth)
	   xsink->raiseException("QT-CONNECT-ERROR", "cannot connect a signal to special method '%s()' and no such builtin slot exists in parent QT classes", tmp.getBuffer());
	else
	   xsink->raiseException("QT-CONNECT-ERROR", "cannot connect to unknown method '%s()'", tmp.getBuffer());
	return -1;
    }

    DLLLOCAL const QMetaObject *getParentMetaObject() const {
        QoreQtVirtualFlagHelper fh;
        return m_qobject->metaObject();
    }

    QPointer<QObject> m_qobject;
    Smoke::Index qt_metacall_index;
    int qt_metaobject_method_count;
    QHash<QByteArray, int> slotIndices;    // map slot signatures to indices in the methodList
    QHash<QByteArray, int> signalIndices;  // map signal signatures to signal IDs in the methodList
    DynamicMethodList methodList;          // list of dynamic signals and slots
};

class QoreSmokePrivateQAbstractItemModelData : public QoreSmokePrivateQObjectData {
protected:
    typedef std::set<AbstractQoreNode *> node_set_t;
    node_set_t node_set;
    QoreRWLock rwl;

    // unlocked
    DLLLOCAL void purgeMapIntern(ExceptionSink *xsink) {
        // dereference all stored ptrs
        for (node_set_t::iterator i = node_set.begin(), e = node_set.end(); i != e; ++i) {
	   assert(*i);
	   (*i)->deref(xsink);
	}
        node_set.clear();
    }

public:
    DLLLOCAL QoreSmokePrivateQAbstractItemModelData(Smoke::Index classID, QObject *p, QoreObject *self) : QoreSmokePrivateQObjectData(classID, p, self) {
    }
    DLLLOCAL virtual ~QoreSmokePrivateQAbstractItemModelData() {
        // dereference all stored data ptrs
    }
    DLLLOCAL AbstractQoreNode *isQoreData(void *data) {
        assert(data);
        QoreAutoRWReadLocker al(rwl);
        AbstractQoreNode *d = reinterpret_cast<AbstractQoreNode *>(data);

	d = node_set.find(d) == node_set.end() ? 0 : d->refSelf();
	printd(5, "QoreSmokePrivateQAbstractItemModelData::isQoreData(%p) this=%p returning %p (%s: %s)\n", data, this, d, d ? d->getTypeName() : "NOTHING", d && d->getType() == NT_OBJECT ? reinterpret_cast<QoreObject *>(d)->getClassName() : "n/a");

	return d;
    }

    DLLLOCAL void storeIndex(const AbstractQoreNode *data, ExceptionSink *xsink) {
        assert(data);
        printd(5, "QoreSmokePrivateQAbstractItemModelData::storeIndex(%p) this=%p %s: %s\n", data, this, data->getTypeName(), data->getType() == NT_OBJECT ? reinterpret_cast<const QoreObject *>(data)->getClassName() : "n/a");

        QoreAutoRWWriteLocker al(rwl);
        node_set_t::iterator i = node_set.find(const_cast<AbstractQoreNode *>(data));
	if (i != node_set.end())
	   return;

	node_set.insert(data->refSelf());
    }

    DLLLOCAL void purgeMap(ExceptionSink *xsink) {
        QoreAutoRWWriteLocker al(rwl);
        purgeMapIntern(xsink);
    }
};

class CommonQoreMethod;

// Singleton. A global structure to simplify Smoke mappings
// for our usage. See types below.
// It reduces "full scans" of Smoke lists for searching etc.
// TODO/FIXME: more comments
class ClassMap {
public:
   typedef QList<Smoke::Type> TypeList;
   typedef int (*arg_handler_t)(Smoke::Stack &stack, const TypeList &types, const QoreListNode *args, CommonQoreMethod &cqm, ExceptionSink *xsink);
   typedef AbstractQoreNode *(*return_value_handler_t)(QoreObject *self, const Smoke::Type &t, Smoke::Stack Stack, CommonQoreMethod &cqm, ExceptionSink *xsink);
   typedef struct type_handler_s {
      TypeList types;
      arg_handler_t arg_handler;
      return_value_handler_t return_value_handler;
      Smoke::Index method;
      DLLLOCAL type_handler_s(const TypeList &n_types, arg_handler_t n_handler, return_value_handler_t n_rv_handler, Smoke::Index n_method) :
	 types(n_types), arg_handler(n_handler), return_value_handler(n_rv_handler), method(n_method) {}
      DLLLOCAL type_handler_s() : arg_handler(0), return_value_handler(0), method(0) {}
   } TypeHandler;
   typedef QMap<QByteArray,QoreNamespace *> NameToNamespace;
   typedef QMultiMap<QByteArray,TypeHandler> MungledToTypes;
   typedef QMap<QByteArray,MungledToTypes> MethodToMungleds;
   typedef QMap<QByteArray,MethodToMungleds> ClassToMethods;
   typedef QMap<QByteArray,QoreEnumTypeInfoHelper *> NameToEnumType;

#ifdef DEBUG
    // "unit" test
   DLLLOCAL void printMapToFile(const QString & fname);
#endif

   DLLLOCAL void setRVHandler(const char *className, const char *methodName, const char *mungedMethod, return_value_handler_t rv_handler);
   DLLLOCAL void setRVHandler(const char *className, const char *methodName, return_value_handler_t rv_handler);

   DLLLOCAL MungledToTypes * availableMethods(const QByteArray & className,
					      const QByteArray & methodName) {
      return &m_map[className][methodName];
   }

   DLLLOCAL Smoke::Index getDestructor(const QByteArray &className) {
      QByteArray destructor = "~" + className;
      assert(m_map[className][destructor].begin() != m_map[className][destructor].end());      
      TypeHandler &th = m_map[className][destructor].begin().value();
      return th.method;
   }

   DLLLOCAL const QoreTypeInfo *getEnumTypeInfo(const Smoke::Type &t) {
      return internGetEnumTypeInfoHelper(t)->getTypeInfo();
   }

   DLLLOCAL QoreQtEnumNode *getEnumValue(const Smoke::Type &t, int64 value) {
      return internGetEnumTypeInfoHelper(t)->newValue(value);
   }

   DLLLOCAL bool checkEnum(const char *name) {
      NameToEnumType::iterator i = enummap.find(name);
      return i != enummap.end() ? true : false;
   }
  
   DLLLOCAL const QoreTypeInfo *getEnumType(const char *name) {
      NameToEnumType::iterator i = enummap.find(name);
      assert(i != enummap.end());
      return i.value()->getTypeInfo();
      //return i != enummap.end() ? i.value()->getTypeInfo() : 0;
   }
  
   DLLLOCAL QoreQtEnumNode *getRunTimeEnumValue(const char *name, int64 val) {
      NameToEnumType::iterator i = enummap.find(name);
      assert(i != enummap.end());
      return i.value()->newValue(val);
   }
  
   // enum type must already exist, otherwise we have to use locking for all accesses to the list
   DLLLOCAL QoreQtEnumNode *getRunTimeEnumValue(const Smoke::Type &t, int64 val) {
      NameToEnumType::iterator i = enummap.find(t.name);
      assert(i != enummap.end());
      return i.value()->newValue(val);
   }

//     TypeList *availableTypes(const char *className, const char *methodName, const char *mungedName) {
//         assert(m_map[className][methodName][mungedName].types.count() != 0);
//         return &m_map[className][methodName][mungedName].types;
//     }


   DLLLOCAL static void init() {
      assert(!m_instance);
      new ClassMap();
   }
   DLLLOCAL static ClassMap* Instance() {
      assert(m_instance);
      return m_instance;
   }

   DLLLOCAL void addArgHandler(const char *cls, const char *meth, const char *munged, arg_handler_t arg_handler);
   DLLLOCAL void addArgHandler(const char *cls, const char *meth, arg_handler_t arg_handler);
   DLLLOCAL void registerMethod(const char *class_name, const char *method_name, const char *munged_name, Smoke::Index method_index, TypeHandler &type_handler, const QoreTypeInfo *returnType = 0, const type_vec_t &argTypeList = type_vec_t());

   DLLLOCAL static void del() {
      delete m_instance;
   }
   
private:
   ClassToMethods m_map;
   NameToNamespace nsmap;
   NameToEnumType enummap;

   DLLLOCAL static ClassMap * m_instance;

   // set the mapping in the class->method->munged->args direction.
   // Smoke supports munged->method->class like direction only
   // unfortunately.
   DLLLOCAL ClassMap();
   
   DLLLOCAL ClassMap(const ClassMap &);
   //ClassMap& operator=(const ClassMap&) {};
   DLLLOCAL ~ClassMap() {
      for (NameToEnumType::iterator i = enummap.begin(), e = enummap.end(); i != e; ++i)
	 delete i.value();

      m_instance = 0;
   }
   
   DLLLOCAL static void setupClassHierarchy();

   DLLLOCAL QoreNamespace *getNS(const char *name) {
      NameToNamespace::iterator i = nsmap.find(name);
      if (i != nsmap.end())
	 return i.value();
      QoreNamespace *ns = new QoreNamespace(name);
      nsmap[name] = ns;
      return ns;
   }

   DLLLOCAL const QoreEnumTypeInfoHelper *internGetEnumTypeInfoHelper(const Smoke::Type &t) {
      NameToEnumType::iterator i = enummap.find(t.name);
      if (i == enummap.end())
	 i = enummap.insert(t.name, new QoreEnumTypeInfoHelper(t));
      return i.value();
   }

   DLLLOCAL void addMethod(QoreClass *qc, const Smoke::Class &c, const Smoke::Method &method, const TypeHandler *th);
   DLLLOCAL void addQoreMethods();
};

// Singleton. Everywhere available map Smoke::Class index -> QoreClass*
//                                 and const char * -> QoreClass*
// It's used to handle creating Qore objects from Qt ones (returned directly
// from Qt library.
class ClassNamesMap {
public:
   typedef QMap<Smoke::Index,QoreClass*> m_map_t;
   typedef QHash<QByteArray,QoreClass*> m_name_map_t;

   DLLLOCAL bool contains(Smoke::Index ix) {
      return m_map.contains(ix);
   }
   DLLLOCAL bool contains(const QByteArray & name) {
      return m_name_map.contains(name);
      //return contains(qt_Smoke->findClass(name.constData()).index);
   }
   DLLLOCAL void addItem(Smoke::Index key, QoreClass* value) {
      assert(!m_map.value(key));
      assert(!m_name_map.value(value->getName()));
      m_map[key] = value;
      m_name_map[value->getName()] = value;
   }
   DLLLOCAL QoreClass* value(Smoke::Index key) {
      return m_map.value(key);
   }
   DLLLOCAL QoreClass* value(const QByteArray & name) {
      return m_name_map.value(name);
   }

   DLLLOCAL static ClassNamesMap* Instance() {
      if (!m_instance)
	 m_instance = new ClassNamesMap();
      return m_instance;
   }
   
   DLLLOCAL static void del() {
      delete m_instance;
   }

   DLLLOCAL const m_map_t &getMap() const {
      return m_map;
   }

private:
    static ClassNamesMap * m_instance;
    m_map_t m_map;
    m_name_map_t m_name_map;

    ClassNamesMap() {};
    ClassNamesMap(const ClassNamesMap &);
    //ClassNamesMap& operator=(const ClassNamesMap&) {};
    ~ClassNamesMap() {
       m_instance = 0;
    }
};

DLLLOCAL ClassMap::TypeHandler getTypeHandler(Smoke::Index index);
DLLLOCAL ClassMap::TypeHandler getTypeHandlerFromMapIndex(Smoke::Index index);

// Functions to handle Qore constructor/any method/any static
// method for object/class instance.
void common_constructor(const QoreClass &myclass,
			const type_vec_t &typeList,
			ClassMap::TypeHandler *type_handler, 
                        QoreObject *self,
                        const QoreListNode *params,
                        ExceptionSink *xsink);
AbstractQoreNode * common_method(const QoreMethod &method,
				 const type_vec_t &typeList,
				 ClassMap::TypeHandler *type_handler, 
                                 QoreObject *self,
                                 AbstractPrivateData *apd,
                                 const QoreListNode *params,
                                 ExceptionSink *xsink);
AbstractQoreNode * common_static_method(const QoreMethod &method,
					const type_vec_t &typeList,
					ClassMap::TypeHandler *type_handler, 
                                        const QoreListNode *params,
                                        ExceptionSink *xsink);
void common_destructor(const QoreClass &thisclass, 
		       ClassMap::TypeHandler *type_handler, 
		       QoreObject *self,
                       AbstractPrivateData *apd,
                       ExceptionSink *xsink);

#endif
