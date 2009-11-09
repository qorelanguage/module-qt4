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

// Custom AbstractPrivateData for any Qt object to be used as
// a Qore object private member. See Qore object/classes implementation.
class QoreSmokePrivate : public AbstractPrivateData {
public:
   DLLLOCAL QoreSmokePrivate(Smoke::Index classID, bool n_is_qobject = false) : m_class(classID), externally_owned(false), is_qobject(n_is_qobject) {}
    DLLLOCAL virtual ~QoreSmokePrivate() {
    }
    DLLLOCAL virtual void * object() = 0;
    DLLLOCAL virtual void clear() = 0;
    // the following is defined as pure virtual so only 1 virtual call has to be made when calling it
    // (it could be implemented here with 2 other virtual calls)
    DLLLOCAL virtual void *takeObject() = 0;
    DLLLOCAL Smoke::Index smokeClass() {
        return m_class;
    }

    DLLLOCAL void setExternallyOwned() {
        externally_owned = true;
    }

    DLLLOCAL bool externallyOwned() const {
        return externally_owned;
    }
    DLLLOCAL const char *getClassName() const {
        return qt_Smoke->classes[m_class].className;
    }
    DLLLOCAL bool isQObject() const {
       return is_qobject;
    }
private:
    Smoke::Index m_class;
    bool externally_owned : 1;
    bool is_qobject : 1;
};

class QoreSmokePrivateData : public QoreSmokePrivate {
    bool save_map;
public:
    DLLLOCAL QoreSmokePrivateData(Smoke::Index classID, void *p, QoreObject *self) : QoreSmokePrivate(classID), m_object(p) {
        // register in object map if object is not derived from QObject and has virtual functions
        save_map = qt_Smoke->classes[classID].flags & Smoke::cf_virtual;
        if (save_map)
            qt_qore_map.add(p, self);
    }
    DLLLOCAL virtual ~QoreSmokePrivateData() {
        // the object must have been destroyed externally and cleared before this destructor is run
        assert(!m_object);
    }
    DLLLOCAL virtual void *object() {
        return m_object;
    }
    DLLLOCAL virtual void clear() {
        if (save_map && m_object)
            qt_qore_map.del(m_object);
        m_object = 0;
    }
    DLLLOCAL virtual void *takeObject() {
        void *p = m_object;
        QoreSmokePrivateData::clear();
        return p;
    }

private:
    void * m_object;
};

class QoreSmokePrivateQObjectData : public QoreSmokePrivate {
public:
   DLLLOCAL QoreSmokePrivateQObjectData(Smoke::Index classID, QObject *p) : QoreSmokePrivate(classID, true), m_qobject(p), obj_ref(false) {
        qt_metaobject_method_count = getParentMetaObject()->methodCount();
        Smoke::ModuleIndex mi = qt_Smoke->findMethod(qt_Smoke->classes[classID].className, "qt_metacall$$?");
        assert(mi.smoke);
        qt_metacall_index = qt_Smoke->methodMaps[mi.index].method;
        assert(qt_metacall_index > 0);
        assert(!strcmp(qt_Smoke->methodNames[qt_Smoke->methods[qt_metacall_index].name], "qt_metacall"));

	if (p->isWidgetType()) {
	   QWidget *qw = reinterpret_cast<QWidget *>(p);
	   // add to QoreWidgetManager if it's a window - so it can be deleted if necessary
	   // when the QApplication object is deleted
	   if (!qw->parent())
	      QWM.add(qw);
	}
    }
    DLLLOCAL virtual ~QoreSmokePrivateQObjectData() {
        //printd(0, "QoreSmokePrivateQObjectData::~QoreSmokePrivateQObjectData() this=%p obj=%p (%s)\n", this, m_qobject.data(), getClassName());
        if (m_qobject.data() && !externallyOwned() && !m_qobject->parent()) {
            // set property to 0 because QoreObject is being deleted
            {
                QoreQtVirtualFlagHelper vfh;
                m_qobject->setProperty(QORESMOKEPROPERTY, (qulonglong)0);
            }

            delete m_qobject;
        }
    }
    DLLLOCAL int metacall(Smoke::Stack args);
    DLLLOCAL bool deleteBlocker(QoreObject *self) {
        //printd(0, "QoreSmokePrivateQObjectData::deleteBlocker(%p) %s obj=%p parent=%p eo=%s\n", self, self->getClassName(), m_qobject.data(), m_qobject.data() ? m_qobject->parent() : 0, externallyOwned() ? "true" : "false");
        if (m_qobject.data() && (m_qobject->parent() || externallyOwned())) {
            if (!obj_ref) {
                obj_ref = true;
                // note that if we call QoreObject::ref() here, it will cause an immediate deadlock!
                self->deleteBlockerRef();
            }
            return true;
        }
        return false;
    }
    DLLLOCAL void externalDelete(QoreObject *obj, ExceptionSink *xsink) {
        QObject *qo = m_qobject.data();
	if (qo && qo->isWidgetType())
	   QWM.remove(reinterpret_cast<QWidget *>(qo));

	// clear object before running destructor
        QoreSmokePrivateQObjectData::clear();
        if (obj_ref) {
            //printd(5, "QoreSmokePrivateQObjectData::externalDelete() deleting object of class %s\n", obj->getClassName());
            obj_ref = false;
	    // FIXME: should doDelete() be called unconditionally?
            // delete the object if necessary (if not already in the destructor)
            if (obj->isValid())
                obj->doDelete(xsink);
            obj->deref(xsink);
        }
    }
    DLLLOCAL virtual void *object() {
        return m_qobject.data();
    }
    DLLLOCAL virtual void clear() {
        m_qobject = 0;
    }
    DLLLOCAL virtual void *takeObject() {
        void *p = (void *)m_qobject.data();
        m_qobject = 0;
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
    bool obj_ref;
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
    DLLLOCAL QoreSmokePrivateQAbstractItemModelData(Smoke::Index classID, QObject *p) : QoreSmokePrivateQObjectData(classID, p) {
    }
    DLLLOCAL virtual ~QoreSmokePrivateQAbstractItemModelData() {
        // dereference all stored data ptrs
    }
    DLLLOCAL AbstractQoreNode *isQoreData(void *data) {
        assert(data);
        QoreAutoRWReadLocker al(rwl);
        AbstractQoreNode *d = reinterpret_cast<AbstractQoreNode *>(data);

	d = node_set.find(d) == node_set.end() ? 0 : d->refSelf();
	//printd(5, "QoreSmokePrivateQAbstractItemModelData::isQoreData(%p) this=%p returning %p (%s: %s)\n", data, this, d, d ? d->getTypeName() : "NOTHING", d && d->getType() == NT_OBJECT ? reinterpret_cast<QoreObject *>(d)->getClassName() : "n/a");

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
    typedef int (*arg_handler_t)(Smoke::Stack &stack, TypeList &types, const QoreListNode *args, CommonQoreMethod &cqm, ExceptionSink *xsink);
    typedef AbstractQoreNode *(*return_value_handler_t)(QoreObject *self, Smoke::Type t, Smoke::StackItem &Stack, CommonQoreMethod &cqm, ExceptionSink *xsink);
    typedef struct type_handler_s {
        TypeList types;
        arg_handler_t arg_handler;
        return_value_handler_t return_value_handler;
        Smoke::Index method;
        DLLLOCAL type_handler_s(TypeList &n_types, arg_handler_t n_handler, return_value_handler_t n_rv_handler, Smoke::Index n_method) :
                types(n_types), arg_handler(n_handler), return_value_handler(n_rv_handler), method(n_method) {}
        DLLLOCAL type_handler_s() : arg_handler(0), return_value_handler(0) {}
    } TypeHandler;
    typedef QMultiMap<QByteArray,TypeHandler> MungledToTypes;
    typedef QMap<QByteArray,MungledToTypes> MethodToMungleds;
    typedef QMap<QByteArray,MethodToMungleds> ClassToMethods;

#ifdef DEBUG
    // "unit" test
    void printMapToFile(const QString & fname);
#endif

    void setRVHandler(const char *className, const char *methodName, const char *mungedMethod, return_value_handler_t rv_handler);
    void setRVHandler(const char *className, const char *methodName, return_value_handler_t rv_handler);

    MungledToTypes * availableMethods(const QByteArray & className,
                                      const QByteArray & methodName) {
        return &m_map[className][methodName];
    }

//     TypeList *availableTypes(const char *className, const char *methodName, const char *mungedName) {
//         assert(m_map[className][methodName][mungedName].types.count() != 0);
//         return &m_map[className][methodName][mungedName].types;
//     }

    static ClassMap* Instance() {
        if (!m_instance) {
            m_instance = new ClassMap();
            m_instance->registerMethods();
        }
        return m_instance;
    }
    DLLLOCAL void addArgHandler(const char *cls, const char *meth, const char *munged, arg_handler_t arg_handler);
    DLLLOCAL void addArgHandler(const char *cls, const char *meth, arg_handler_t arg_handler);
    DLLLOCAL void registerMethod(const char *class_name, const char *method_name, const char *munged_name, Smoke::Index method_index, TypeHandler &type_handler);

private:

    ClassToMethods m_map;

    static ClassMap * m_instance;

    ClassMap() {};
    ClassMap(const ClassMap &);
    //ClassMap& operator=(const ClassMap&) {};
    ~ClassMap() {
        delete m_instance;
    }

    type_handler_s typeList(Smoke::Method m, Smoke::Index index);
    // set the mapping in the class->method->munged->args direction.
    // Smoke supports munged->method->class like direction only
    // unfortunately.
    void registerMethods();
};

// Singleton. Everywhere available map Smoke::Class index -> QoreClass*
// It's used to handle creating Qore objects from Qt ones (returned directly
// from Qt library.
class ClassNamesMap {
public:
    bool contains(Smoke::Index ix) {
        return m_map.contains(ix);
    };
    bool contains(const QByteArray & name) {
        return contains(qt_Smoke->findClass(name.constData()).index);
    }
    void addItem(Smoke::Index key, QoreClass* value) {
        m_map[key] = value;
    };
    QoreClass* value(Smoke::Index key) {
        return m_map.value(key);
    };
    QoreClass* value(const QByteArray & name) {
        return value(qt_Smoke->findClass(name.constData()).index);
    };

    static ClassNamesMap* Instance() {
        if (!m_instance) {
            m_instance = new ClassNamesMap();
        }
        return m_instance;
    }

private:
    static ClassNamesMap * m_instance;
    QMap<Smoke::Index,QoreClass*> m_map;

    ClassNamesMap() {};
    ClassNamesMap(const ClassNamesMap &);
    //ClassNamesMap& operator=(const ClassNamesMap&) {};
    ~ClassNamesMap() {
        delete m_instance;
    }
};

// Initial setup for each Qt class. It creates common Qore binding between
// qt class and qore class wrappers.
// There is also a new namespace created if it's required (enums etc.).
class QoreSmokeClass {
public:
    QoreSmokeClass(const char * className, QoreNamespace &qt_ns);
    ~QoreSmokeClass();

private:
    QoreClass * m_qoreClass;
    QoreNamespace * m_namespace;

    Smoke::ModuleIndex m_classId;
    Smoke::Class m_class;

    // targetClass == false if it's one of parent classes
    void addClassMethods(Smoke::Index methodIx, bool targetClass=true);
    void addSuperClasses(Smoke::Index ix, QoreNamespace &qt_ns);
};

// Functions to handle Qore constructor/any method/any static
// method for object/class instance.
void common_constructor(const QoreClass &myclass,
                        QoreObject *self,
                        const QoreListNode *params,
                        ExceptionSink *xsink);
AbstractQoreNode * common_method(const QoreMethod &method,
                                 QoreObject *self,
                                 AbstractPrivateData *apd,
                                 const QoreListNode *params,
                                 ExceptionSink *xsink);
AbstractQoreNode * common_static_method(const QoreMethod &method,
                                        const QoreListNode *params,
                                        ExceptionSink *xsink);
void common_destructor(const QoreClass &thisclass, QoreObject *self,
                       AbstractPrivateData *apd,
                       ExceptionSink *xsink);

#endif
