/*
  qoresmokebinding.cpp

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

#include "qoresmokeglobal.h"

#include "qoremarshalling.h"
#include "qoresmokebinding.h"
#include "qoresmokeclass.h"
#include "commonqoremethod.h"

#include <iostream>
#include <string>
#include <QVariant>
#include <QMetaObject>
#include <QMetaProperty>
#include <QObject>

using namespace std;

QoreSmokeBinding* QoreSmokeBinding::m_instance = NULL;

QoreObject *getQoreQObject(const QObject *qtObj) {
    QVariant ptr;
    {
        QoreQtVirtualFlagHelper vfh;
        ptr = qtObj->property(QORESMOKEPROPERTY);
    }
    if (!ptr.isValid()) {
        //printd(0, "getQoreQObject(%p) property %s not set\n", qtObj, QORESMOKEPROPERTY);
        return 0;
    }

    QoreObject *o = reinterpret_cast<QoreObject*>(ptr.toULongLong());
    //printd(0, "getQoreQObject(%p) returning QoreObject %p\n", qtObj, o);
    return o;
}

QoreObject *getQoreObject(Smoke::Index classId, void *obj, QoreClass *&qc) {
    assert(obj);

    qc = ClassNamesMap::Instance()->value(classId);

//     printd(0, "getQoreObject() qc=%s qobject=%s\n", qc->getName(), qc->getClass(QC_QOBJECT->getID()) ? "true" : "false");

    assert(qc);
    if (!qc->getClass(QC_QOBJECT->getID()))
        return qt_qore_map.get(obj);

    return getQoreQObject(reinterpret_cast<QObject*>(obj));
}

void QoreSmokeBinding::deleted(Smoke::Index classId, void *obj) {
    assert(obj);

    QoreClass *qc;
    QoreObject *o = getQoreObject(classId, obj, qc);

    //printd(0, "QoreSmokeBinding::deleted() %s::~%s (%p) qore obj=%p\n", className(classId), className(classId), obj, o);

    if (o) {
        ExceptionSink xsink;

        // mark private data as cleared
        ReferenceHolder<QoreSmokePrivateQObjectData> qsd(reinterpret_cast<QoreSmokePrivateQObjectData *>(o->getReferencedPrivateData(qc->getID(), &xsink)), &xsink);
//         printd(0, "QoreSmokeBinding::deleted() external delete Qore class %s (Qt class %s) (qsd=%p)\n", o->getClassName(), className(classId), *qsd);

        if (qsd)
            qsd->externalDelete(o, &xsink);
        else
            // mark qore object as externally deleted
            o->externalDelete(qc->getID(), &xsink);
    }
}

bool QoreSmokeBinding::callMethod(Smoke::Index method, void *obj, Smoke::Stack args, bool isAbstract) {
    //printd(0, "QoreSmokeBinding::callMethod() %s::%s() method=%d obj=%p isAbstract=%s (virt=%s)\n", smoke->classes[smoke->methods[method].classId].className, smoke->methodNames[smoke->methods[method].name], method, obj, isAbstract ? "true" : "false", qore_smoke_is_virtual() ? "true" : "false");

    if (qore_smoke_is_virtual()) {
        qore_smoke_clear_virtual();
        //printd(0, "QoreSmokeBinding::callMethod() qore_smoke_is_virtual, returning to Smoke\n");
        return false;
    }

    ExceptionSink xsink;
    Smoke::Method meth = smoke->methods[method];
    const char * cname = smoke->classes[meth.classId].className;
    const char * mname = smoke->methodNames[meth.name];

    QoreClass *qc;
    QoreObject *o = getQoreObject(meth.classId, obj, qc);

//     if (!strcmp(mname, "paintEngine"))
//        printd(0, "QoreSmokeBinding::callMethod() %s::%s() qore object=%p\n", cname, mname, o);

    if (!o) {
        // we must have an implementation for abstract methods
#ifdef DEBUG
        if (isAbstract)
            printd(0, "trying to execute pure virtual method %s::%s()\n", cname, mname);
#endif
        assert(!isAbstract);
        return false;
    }

    if (!strcmp(mname, "qt_metacall")) {
        //printd(0, "QoreSmokeBinding::callMethod() className: %s::%s obj: %p\n", cname, mname, obj);

        // get the QoreSmokePrivate info
        ReferenceHolder<QoreSmokePrivateQObjectData> qsp(reinterpret_cast<QoreSmokePrivateQObjectData *>(o->getReferencedPrivateData(QC_QOBJECT->getID(), &xsink)), &xsink);
        if (!qsp) {
            assert(xsink);
            return false;
        }

        // if the object has been deleted, or is not a signal/slot invokation, then return false
        if (!qsp->qobject() || args[1].s_enum != QMetaObject::InvokeMetaMethod)
            return false;

	int mc = qsp->getParentMethodCount();
	// see if call is for a method in a parent class
	if (args[2].s_int < mc)
	   return false;

	// get method offset in this class
	args[2].s_int -= mc;

        //printd(0, "QoreSmokeBinding::callMethod() %s::%s() method=%d obj=%p qsp=%p isAbstract=%s\n", smoke->classes[smoke->methods[method].classId].className, smoke->methodNames[smoke->methods[method].name], method, obj, *qsp, isAbstract ? "true" : "false");

	// handle the signal, which is connected to a dynamic method (signal or slot)
	qsp->handleSignal(0, args[2].s_int, (void**)args[3].s_voidp);

	args[0].s_int = -1;
	return true;
    }

    const QoreMethod * qoreMethod = o->getClass()->findMethod(mname);
    //printd(0, "QoreSmokeBinding::callMethod() virtual method %s::%s() method=%p (user: %d)\n", o->getClassName(), mname, qoreMethod, qoreMethod ? qoreMethod->isUser() : 0);
    if (!qoreMethod || !qoreMethod->isUser()) {
        //printd(0, "QoreSmokeBinding::callMethod() virtual method %s::%s() not found\n", o->getClassName(), mname);
        if (isAbstract) {
            xsink.raiseException("QT-ABSTRACT-METHOD-ERROR", "The Qt library tried to execute pure virtual %s::%s(), but this method is not implemented in the %s class", o->getClassName(), mname, o->getClassName());
            xsink.handleExceptions();
            assert(0); // TODO/FIXME: propably won't crash here...
        }
        return false;
    }

    Smoke::Index *idx = smoke->argumentList + meth.args;
    QList<Smoke::Type> typeList;
    while (*idx) {
        typeList.append(smoke->types[*idx]);
        idx++;
    }

    //printd(0, "QoreSmokeBinding::callMethod() creating args list for %s::%s() (arg count: %d)\n", o->getClassName(), mname, typeList.size());

    ReferenceHolder<QoreListNode> qoreArgs(new QoreListNode(), &xsink);

    for (int i = 0; i < typeList.size(); ++i)
        qoreArgs->push(Marshalling::stackToQore(typeList.at(i), args[i + 1], &xsink));

    //printd(0, "QoreSmokeBinding::callMethod() calling method smoke=%s::%s(), qore=%s::%s() args=%d\n", cname, mname, o->getClassName(), mname, typeList.size());
    ReferenceHolder<AbstractQoreNode> aNode(o->evalMethod(mname, *qoreArgs, &xsink), &xsink);

    Smoke::Type &rt = smoke->types[meth.ret];
    if (CommonQoreMethod::qoreToStackStatic(&xsink, args[0], cname, mname, rt, *aNode, -1, 0, true) == -1) {
        xsink.handleExceptions();
        assert(0);
    }

    return true;
}

char* QoreSmokeBinding::className(Smoke::Index classId) {
    return (char*) smoke->classes[classId].className;
}
