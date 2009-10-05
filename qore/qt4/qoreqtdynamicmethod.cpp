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

#include <QByteArray>

#include "qoresmokeglobal.h"
#include "qoreqtdynamicmethod.h"
#include "commonqoremethod.h"


int QoreQtDynamicMethod::identifyTypes(SmokeTypeList &typeList, const char *sig, const char *p, ExceptionSink *xsink) {
    printd(0, "QoreQtDynamicMethod::identifyTypes sig=%s p=%s\n", sig, p);
    while (*p && isblank(*p))
        ++p;
    if (*p != ')')
        while (*p) {
            const char *e = p + 1;
            char paren = 0;
            while (paren || (*e != ')' && *e != ',')) {
                assert(*e);
                switch (*e) {
                case '<':
                case '(':
                case '[':
                    if (!paren)
                        paren = *e;
                    break;
                case '>':
                case ')':
                case ']':
                    if (*e == paren)
                        paren = 0;
                    break;
                }
                ++e;
            }
            addType(typeList, p, e - p, sig, xsink);
            if (*e == ')')
                break;
            p = e + 1;
        }
    return *xsink ? -1 : 0;
}

int QoreQtDynamicMethod::addType(SmokeTypeList &typeList, const char *b, int len, const char *sig, ExceptionSink *xsink) {
    QByteArray tn(b, len);
    printd(0, "QoreQtDynamicMethod::addType() processing type '%s'\n", tn.constData());

    Smoke::Type t;
    t.classId = -1;
    t.name = strdup(tn.constData());

    if (tn == "int") {
        t.flags = Smoke::t_int | Smoke::tf_stack;
    } else if (tn == "bool") {
        t.flags = Smoke::t_bool | Smoke::tf_stack;
    } else if (tn == "uint") {
        t.flags = Smoke::t_uint | Smoke::tf_stack;
    } else if (tn == "long") {
        t.flags = Smoke::t_long | Smoke::tf_stack;
    } else if (tn == "ulong") {
        t.flags = Smoke::t_ulong | Smoke::tf_stack;
    } else if (tn == "double") {
        t.flags = Smoke::t_double | Smoke::tf_stack;
    } else if (tn == "char*" ) {
        t.flags = Smoke::t_voidp | Smoke::tf_ptr;
    } else if (tn == "QString") {
        t.name = "QString&";
        t.flags = Smoke::t_voidp | Smoke::tf_ref;
    }
    // everything else should be pointer to object
    else {
        // TODO/FIXME: there should be a better way! But it looks like it is
        // not implemented in Smoke
        bool found = false;
        Smoke::Type st;
        for (int i = 0; i < qt_Smoke->numTypes; ++i) {
            st = qt_Smoke->types[i];
            if (tn == st.name) {
                printd(0, "QoreQtDynamicMethod::addType() found object/ref to %s\n", st.name);
                t.name = st.name;
                t.flags = st.flags;
                t.classId = st.classId;
                found = true;
                break;
            }
        }
        if (!found) {
            xsink->raiseException("DYNAMIC-METHOD-ERROR", "cannot handle argument type '%s' in '%s'", tn.constData(), sig);
            return -1;
        }
    }

    typeList.push_back(t);
    return 0;
}

void QoreQtDynamicMethod::qtToQore(const Smoke::Type &t, void *arg, QoreListNode *args) {
    assert(t.name);
    Smoke::StackItem si;

    if (t.flags & Smoke::t_voidp)
        si.s_voidp = arg;
    else if (t.flags & Smoke::t_bool)
        si.s_bool = * reinterpret_cast<bool*>(arg);
    else if (t.flags & Smoke::t_int)
        si.s_int = * reinterpret_cast<int*>(arg);
    else if (t.flags & Smoke::t_uint)
        si.s_uint = * reinterpret_cast<uint*>(arg);
    else if (t.flags & Smoke::t_long)
        si.s_long = * reinterpret_cast<long*>(arg);
    else if (t.flags & Smoke::t_ulong)
        si.s_ulong = * reinterpret_cast<ulong*>(arg);
    else if (t.flags & Smoke::t_double)
        si.s_double = * reinterpret_cast<double*>(arg);
    else if (t.flags & Smoke::t_float)
        si.s_float = * reinterpret_cast<float*>(arg);
    else if (t.flags & Smoke::t_enum)
        si.s_enum = * reinterpret_cast<int*>(arg);
    else if (t.flags & Smoke::t_class)
        si.s_class = arg;
    else {
        Q_ASSERT_X(false, "sig/slot", "missing qt to qore handling");
    }

//     if ((t.flags & Smoke::tf_elem) == Smoke::t_bool) {
//         bool *ptr = reinterpret_cast<bool *>(arg);
//         args->push(get_bool_node(*ptr));
//     } else if ((t.flags & Smoke::tf_elem) == Smoke::t_int) {
//         int *ptr = reinterpret_cast<int *>(arg);
//         args->push(new QoreBigIntNode(*ptr));
//     } else
//         assert(false);

//AbstractQoreNode * stackToQore(const Smoke::Type &t, Smoke::StackItem &i, ExceptionSink *xsink);
    ExceptionSink xsink;
    AbstractQoreNode * newNode = Marshalling::stackToQore(t, si, &xsink);
    if (!newNode) {
        xsink.handleExceptions();
        exit(1);
    }
    args->push(newNode);

}

void QoreQtDynamicMethod::qoreToQt(const Smoke::Type &qtType, Smoke::StackItem &si, void *&ptr, void *&save, const AbstractQoreNode *val) {
    save = 0;
    ptr = 0;
    printd(0, "qoreToQt() ptr=%p save=%p, val=%p (%s)\n", ptr, save, val, val ? val->getTypeName() : "n/a");

    ExceptionSink xsink;
    const char * cname = qt_Smoke->classes[qtType.classId].className;
    const char * mname = "//unspecified//";
    if (CommonQoreMethod::qoreToStackStatic(&xsink, si, cname, mname, qtType, val) == -1) {
        xsink.handleExceptions();
        exit(1);
    }

    switch (qtType.flags & Smoke::tf_elem) {
    case Smoke::t_voidp:
        save = si.s_voidp;
        break;
    case Smoke::t_bool:
        save = /*(void *)*/&si.s_bool;
        break;
    case Smoke::t_char:
        save = /*(void *)*/&si.s_char;
        break;
    case Smoke::t_uchar:
        save = /*(void *)*/&si.s_uchar;
        break;
    case Smoke::t_short:
        save = /*(void *)*/&si.s_short;
        break;
    case Smoke::t_ushort:
        save = /*(void *)*/&si.s_ushort;
        break;
    case Smoke::t_int:
        save = /*(void *)*/&si.s_int;
        break;
    case Smoke::t_uint:
        save = /*(void *)*/&si.s_uint;
        break;
    case Smoke::t_long:
        save = /*(void *)*/&si.s_long;
        break;
    case Smoke::t_ulong:
        save = /*(void *)*/&si.s_ulong;
        break;
    case Smoke::t_float:
        save = /*(void *)*/&si.s_float;
        break;
    case Smoke::t_double:
        save = /*(void *)*/&si.s_double;
        break;
    case Smoke::t_enum:
        save = /*(void *)*/&si.s_enum;
        break;
    case Smoke::t_class:
        save = si.s_class;
        break;
    default:
        Q_ASSERT_X(false, "sig/slot", "missing qore to qt handling");
    }
    ptr = &save;
}

void QoreQtDynamicMethod::qoreToQtDirect(const Smoke::Type &qtType, void *&ptr, const AbstractQoreNode *val) {
    printd(0, "qoreToQtDirect() ptr=%p val=%p (%s)\n", ptr, val, val ? val->getTypeName() : "n/a");
    void * save;
    Smoke::StackItem si;
    qoreToQt(qtType, si, ptr, save, val);
//     if ((qtType.flags & Smoke::tf_elem) == Smoke::t_bool) {
//         bool *c = (bool *)ptr;
//         *c = val ? val->getAsBool() : false;
//     } else if ((qtType.flags & Smoke::tf_elem) == Smoke::t_int) {
//         int *c = (int *)ptr;
//         *c = val ? val->getAsInt() : 0;
//     } else
//         // TODO/FIXME: it should be done by existing code, shouldn't it?
//         assert(false);
}



QoreQtDynamicSlot::QoreQtDynamicSlot(const QoreObject *qo, const char *sig, ExceptionSink *xsink) : qore_obj(const_cast<QoreObject *>(qo)) {
    returnType.name = 0;

    if (!sig)
        return;

    //printd(5, "QoreQtDynamicSlot::QoreQtDynamicSlot() processing signature '%s'\n", sig);

    // process signature
    const char *p = strchrs(sig, " (");
    if (!p)
        return;
    if (*p == '(') {
        method = resolveMethod(sig, xsink);
        if (!method)
            return;
    } else {
        const char *p1 = 0;
        while (p && *p && (*p) != '(') {
            p1 = p;
            p = strchrs(p, " (");
        }
        if (!p || !*p) {
            xsink->raiseException("DYNAMIC-SLOT-ERROR", "cannot find slot method name in '%s'", sig);
            return;
        }

        // resolve slot signature to method
        method = resolveMethod(p1, xsink);
        if (!method)
            return;

        const char *tmp = sig;

        // FIXME: add return type detection
        //return_type = qqt_type_list.identify(tmp);
    }
    ++p;
    while (*p && isblank(*p))
        ++p;
    if (*p != ')')
        identifyAndAddTypes(sig, p, xsink);
}

void QoreQtDynamicSlot::call(QoreObject *self, void **arguments) const {
    ExceptionSink xsink;

    // create Qore argument list
    ReferenceHolder<QoreListNode> args(typeList.empty() ? 0 : new QoreListNode(), &xsink);
    for (int i = 0, e = typeList.size(); i < e; ++i)
        qtToQore(typeList[i], arguments[i + 1], *args);

    // call Qore user method
    ReferenceHolder<AbstractQoreNode> rv(qore_obj->evalMethod(*method, *args, &xsink), &xsink);

    // process return value
    if (returnType.name)
        qoreToQtDirect(returnType, arguments[0], *rv);
}

const QoreMethod * QoreQtDynamicSlot::resolveMethod(const char *name, ExceptionSink *xsink) {
    const QoreClass *qc = qore_obj->getClass();
    const char *c = strchr(name, '(');
    QoreString tmp;
    if (c)
        tmp.concat(name, c - name);
    else
        tmp.concat(name);
    // do not allow special methods to be matched
    if (!tmp.compare("constructor") || !tmp.compare("destructor") || !tmp.compare("copy")) {
        xsink->raiseException("DYNAMIC-SLOT-ERROR", "cannot assign special method %s::%s() to a Qt slot!", qc->getName(), tmp.getBuffer());
        return 0;
    }

    const QoreMethod *meth = findUserMethod(qc, tmp.getBuffer());
    if (!meth)
        xsink->raiseException("DYNAMIC-SLOT-ERROR", "method %s::%s() does not exist", qc->getName(), tmp.getBuffer());

    //printd(5, "DynamicSlot::resolveMethod(%08p, '%s') search: %s::%s() resolved to %08p\n", qore_obj, name, qc->getName(), tmp.getBuffer(), meth);
    return meth;
}



QoreQtDynamicSignal::QoreQtDynamicSignal(const char *sig, ExceptionSink *xsink) {
    const char *p = strchr(sig, '(');

    if (!p) {
        xsink->raiseException("DYNAMIC-SIGNAL-ERROR", "invalid signal signature '%s'", sig);
        return;
    }
    ++p;

    identifyAndAddTypes(sig, p, xsink);
}

void QoreQtDynamicSignal::emitSignal(QObject *obj, int id, const QoreListNode *args) {
    int num_args = typeList.size();
    void *sig_args[num_args + 1];
    void *save_args[num_args];
    Smoke::StackItem si[num_args];

    // set return value to 0
    sig_args[0] = 0;

    // iterate through signal parameters to build argument list
    for (int i = 0; i < num_args; ++i) {
        // get argument QoreNode
        const AbstractQoreNode *n = args ? args->retrieve_entry(i + 1) : 0;

        qoreToQt(typeList[i], si[i], sig_args[i + 1], save_args[i], n);
    }
    QMetaObject::activate(obj, id, id, sig_args);

    // iterate through signal parameters to delete temporary values
    //for (int i = 0; i < num_args; ++i)
    // type_list[i]->del_arg(save_args[i]);
}


