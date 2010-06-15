/* -*- indent-tabs-mode: nil -*- */
/*
  commonqoremethod.cpp

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

#include "qoresmokeglobal.h"

#include "commonqoremethod.h"
#include "qoremarshalling.h"
#include "qoresmokeclass.h"
#include "commonargument.h"
#include "qoreqtenumnode.h"
#include "qoresmokebinding.h"
#include <QtDebug>

#include <QWidget>

#include <memory>

// returns true to var="QString&" or "QString&" and type = "QString"
bool isptrtype(const char *var, const char *type) {
    int i = 0;

    while (var[i] && type[i]) {
        if (var[i] != type[i] && type[i]) {
            //printd(0, "isptrtype(%s, %s) returning 0\n", var, type);
            return false;
        }
        ++i;
    }
    //printd(0, "isptrtype(%s, %s) returning %d\n", var, type, ((var[i] == '*' || var[i] == '&') && !type[i] && !var[i+1]) ? true : false);
    return ((var[i] == '*' || var[i] == '&') && !type[i] && !var[i+1]) ? true : false;
}

CommonQoreMethod::CommonQoreMethod(const char *cname, const char *mname)
   : Stack(0),
     m_className(cname),
     m_methodName(mname),
     m_xsink(0),
     m_valid(false),
     qoreArgCnt(0),
     temp_store(0),
     vl(0),
     type_handler(0),
     tparams(0),
     self(0),
     smc(0),
     suppress_method(false),
     args (0) {

    ClassMap::MungledToTypes *mMap = ClassMap::Instance()->availableMethods(cname, mname);

    foreach (QByteArray name, mMap->keys()) {
        foreach (const ClassMap::TypeHandler &th, mMap->values(name)) {
	   if (!th.types.count()) {
	      type_handler = &th;
	      break;
	   }
	}
	if (type_handler)
	   break;
    }

    assert(type_handler);

    m_valid = true;

    m_method = qt_Smoke->methods[type_handler->method];
    //printd(0, "CommonQoreMethod::method() method=%d (%s)\n", type_handler.method, qt_Smoke->methodNames[m_method.name]);

    // Create a Smoke stack for return value
    Stack = new Smoke::StackItem[1];
}

CommonQoreMethod::CommonQoreMethod(ClassMap::TypeHandler *th,
				   QoreObject *n_self,
                                   QoreSmokePrivate *n_smc,
                                   const char* className,
                                   const char* methodName,
                                   const QoreListNode* params,
                                   ExceptionSink *xsink)
   : Stack(0),
     m_className(className),
     m_methodName(methodName),
     m_xsink(xsink),
     m_valid(false),
     qoreArgCnt(num_params(params)),
     temp_store(0),
     vl(xsink),
     type_handler(th),
     tparams(0),
     self(n_self),
     smc(n_smc),
     suppress_method(false),
     args(params) {

    m_method = qt_Smoke->methods[type_handler->method];
    m_valid = true;

    //printd(0, "CommonQoreMethod::method() method=%d smoke: %s::%s()\n", type_handler->method, qt_Smoke->classes[m_method.classId].className, qt_Smoke->methodNames[m_method.name]);

    // stack must be larger for its 0th value as a retval
    if (type_handler->arg_handler)  {
        if (type_handler->arg_handler(Stack, type_handler->types, params, *this, xsink))
            m_valid = false;
#if DEBUG
        else {
	   if (*xsink) {
	      xsink->handleExceptions();
	      assert(false);
	   }
	}
#endif
    } else {
        // Create a Smoke stack from params
       Stack = new Smoke::StackItem[type_handler->types.count() + 1];
       //printd(0, "CommonQoreMethod::CommonQoreMethod() allocated stack of size %d\n", type_handler->types.count() + 1);

        int i = 1;
        foreach (Smoke::Type t, type_handler->types) {
            if (qoreToStack(t, get_param(params, i-1), i))
                break;
            ++i;
        }
    }
    if (*xsink)
        m_valid = false;
}

CommonQoreMethod::~CommonQoreMethod() {
//     printd(0, "CommonQoreMethod::~CommonQoreMethod() this=%p valid=%d\n", this, m_valid);

    if (temp_store) {
        for (RefMap::iterator i = temp_store->begin(), e = temp_store->end(); i != e; ++i) {
            temp_store_s &rf = i.value();
            //printd(5, "CommonQoreMethod::~CommonQoreMethod() temp_store[%d].ref=%p\n", i.key(), rf.ref);
            // dereference all saved reference values
            if (rf.ref_value)
                rf.ref_value->deref(m_xsink);

            // write back values to references here
            if (rf.ref) {
                ReferenceHelper ref(rf.ref, vl, m_xsink);
                switch (rf.type) {
                case temp_store_s::r_none:
                    ref.assign(0, m_xsink);
                    break;
                case temp_store_s::r_int:
                    ref.assign(new QoreBigIntNode(rf.data.q_int), m_xsink);
                    break;
                case temp_store_s::r_str:
                    ref.assign(new QoreStringNode(rf.data.q_str), m_xsink);
                    break;
                case temp_store_s::r_qstr: {
                    QoreStringNode *v = new QoreStringNode(rf.data.q_qstr->toUtf8().data(), QCS_UTF8);
                    ref.assign(v, m_xsink);
                }
                case temp_store_s::r_bool:
                    ref.assign(get_bool_node(rf.data.q_bool), m_xsink);
                    break;
                case temp_store_s::r_qreal:
                    ref.assign(new QoreFloatNode(rf.data.q_qreal), m_xsink);
                    break;
		case temp_store_s::r_qpixmap:
		    ref.assign(Marshalling::createQoreObjectFromNonQObject(QC_QPIXMAP, SCI_QPIXMAP, rf.data.q_qpixmap), m_xsink);
		    rf.data.q_qpixmap = 0;
		    break;
                default:
                    m_xsink->raiseException("QT-REFERENCE-BIND", "Unhandled refrence type %d", rf.type);
                }
            }
        }

        delete temp_store;
    }

    delete[] Stack;

    if (tparams)
        tparams->deref(m_xsink);
}

int get_qdate(const AbstractQoreNode *n, QDate &date, ExceptionSink *xsink) {
    qore_type_t t = n ? n->getType() : NT_NOTHING;
    if (t == NT_DATE) {
        const DateTimeNode *d = reinterpret_cast<const DateTimeNode *>(n);
        qore_tm info;
        d->getInfo(info);
        date.setDate(info.year, info.month, info.day);
        return 0;
    }

    if (t != NT_OBJECT) {
        xsink->raiseException("QT-QDATE-ERROR", "cannot convert type '%s' to QDate", n ? n->getTypeName() : "NOTHING");
        return -1;
    }

    const QoreObject *o = reinterpret_cast<const QoreObject *>(n);

    ReferenceHolder<QoreSmokePrivateData> data(xsink);

    data = reinterpret_cast<QoreSmokePrivateData *>(o->getReferencedPrivateData(QC_QDATE->getID(), xsink));
    if (*xsink)
        return -1;

    if (data) {
        QDate *d = reinterpret_cast<QDate *>(data->object());
        assert(d);
        date = *d;
        //date.setDate(d->year(), d->month(), d->day());
        return 0;
    }

    data = reinterpret_cast<QoreSmokePrivateData *>(o->getReferencedPrivateData(QC_QDATETIME->getID(), xsink));
    if (*xsink)
        return -1;

    if (data) {
        QDateTime *d = reinterpret_cast<QDateTime *>(data->object());
        assert(d);
        date = d->date();
        return 0;
    }

    xsink->raiseException("QT-QDATE-ERROR", "class '%s' is not derived from QDate or QDateTime", o->getClassName());
    return -1;
}

int get_qdatetime(const AbstractQoreNode *n, QDateTime &dt, ExceptionSink *xsink) {
    qore_type_t t = n ? n->getType() : NT_NOTHING;
    if (t == NT_DATE) {
        const DateTimeNode *qdt = reinterpret_cast<const DateTimeNode *>(n);

        qore_tm info;
        qdt->getInfo(info);
        dt.setDate(QDate(info.year, info.month, info.day));
        dt.setTime(QTime(info.hour, info.minute, info.second, info.us / 1000));

        return 0;
    }

    if (t != NT_OBJECT) {
        xsink->raiseException("QT-QDATETIME-ERROR", "cannot convert type '%s' to QDateTime", n ? n->getTypeName() : "NOTHING");
        return -1;
    }

    const QoreObject *o = reinterpret_cast<const QoreObject *>(n);

    ReferenceHolder<QoreSmokePrivateData> data(xsink);

    data = reinterpret_cast<QoreSmokePrivateData *>(o->getReferencedPrivateData(QC_QDATE->getID(), xsink));
    if (*xsink)
        return -1;

    if (data) {
        QDate *d = reinterpret_cast<QDate *>(data->object());
        assert(d);

        dt.setTime(QTime());
        dt.setDate(*d);
        return 0;
    }

    data = reinterpret_cast<QoreSmokePrivateData *>(o->getReferencedPrivateData(QC_QTIME->getID(), xsink));
    if (*xsink)
        return -1;

    if (data) {
        QTime *qt = reinterpret_cast<QTime *>(data->object());
        assert(qt);

        dt.setDate(QDate());
        dt.setTime(*qt);

        return 0;
    }

    data = reinterpret_cast<QoreSmokePrivateData *>(o->getReferencedPrivateData(QC_QDATETIME->getID(), xsink));
    if (*xsink)
        return -1;

    if (data) {
        QDateTime *d = reinterpret_cast<QDateTime *>(data->object());
        assert(d);
        dt = *d;
        return 0;
    }

    xsink->raiseException("QT-QDATETIME-ERROR", "class '%s' is not derived from QDateTime, QDate, or QTime", o->getClassName());
    return -1;
}

int get_qtime(const AbstractQoreNode *n, QTime &time, ExceptionSink *xsink) {
    qore_type_t t = n ? n->getType() : NT_NOTHING;
    if (t == NT_DATE) {
        const DateTimeNode *qdt = reinterpret_cast<const DateTimeNode *>(n);
        qore_tm info;
        qdt->getInfo(info);
        time.setHMS(info.hour, info.minute, info.second, info.us / 1000);
        return 0;
    }

    if (t != NT_OBJECT) {
        xsink->raiseException("QT-QTIME-ERROR", "cannot convert type '%s' to QTime", n ? n->getTypeName() : "NOTHING");
        return -1;
    }

    const QoreObject *o = reinterpret_cast<const QoreObject *>(n);

    ReferenceHolder<QoreSmokePrivateData> data(xsink);

    data = reinterpret_cast<QoreSmokePrivateData *>(o->getReferencedPrivateData(QC_QDATETIME->getID(), xsink));
    if (*xsink)
        return -1;

    if (data) {
        QDateTime *d = reinterpret_cast<QDateTime *>(data->object());
        assert(d);
        time = d->time();
        return 0;
    }

    data = reinterpret_cast<QoreSmokePrivateData *>(o->getReferencedPrivateData(QC_QTIME->getID(), xsink));
    if (*xsink)
        return -1;

    if (data) {
        QTime *qt = reinterpret_cast<QTime *>(data->object());
        assert(qt);

        time = *qt;

        return 0;
    }

    xsink->raiseException("QT-QTIME-ERROR", "class '%s' is not derived from QTime or QDateTime", o->getClassName());
    return -1;
}

int get_qstring(Smoke::Type &t, QString &qstring, const AbstractQoreNode *n, ExceptionSink *xsink) {
//     printd(0, "get_qstring() %s NT=%d classId=%d\n", t.name, n->getType(), t.classId);
    if (n && n->getType() == NT_STRING) {
        const QoreStringNode *str = reinterpret_cast<const QoreStringNode *>(n);
        if (str->getEncoding() == QCS_ISO_8859_1) {
            qstring = QString::fromLatin1(str->getBuffer());
        } else if (str->getEncoding() == QCS_USASCII) {
            qstring = QString::fromAscii(str->getBuffer());
        } else {
            TempEncodingHelper estr(str, QCS_UTF8, xsink);
            if (*xsink)
                return -1;
            qstring = QString::fromUtf8(str->getBuffer());
        }
    } else if (n && n->getType() == NT_OBJECT) {
        const QoreObject *obj = reinterpret_cast<const QoreObject *>(n);
        QoreSmokePrivateData * p = 0;
        QoreClass * qc = ClassNamesMap::Instance()->value("QChar");
        if (qc && qc->getClass(qc->getID())) {
            p = reinterpret_cast<QoreSmokePrivateData*>(obj->getReferencedPrivateData(qc->getID(), xsink));
            qstring = p && p->object() ? QString( *(QChar*)(p->object()) ) : QString();
            return 0;
        }
        qc = ClassNamesMap::Instance()->value("QLatin1String");
        if (qc && qc->getClass(qc->getID())) {
            p = reinterpret_cast<QoreSmokePrivateData*>(obj->getReferencedPrivateData(qc->getID(), xsink));
            qstring = p && p->object() ? QString( *(QLatin1String*)(p->object()) ) : QString();
            return 0;
        }
        qc = ClassNamesMap::Instance()->value("QByteArray");
        if (qc && qc->getClass(qc->getID())) {
            p = reinterpret_cast<QoreSmokePrivateData*>(obj->getReferencedPrivateData(qc->getID(), xsink));
            qstring = p && p->object() ? QString( *(QByteArray*)(p->object()) ) : QString();
            return 0;
        }
        xsink->raiseException("QT-GET-QSTRING", "Cannot convert QoreObject (%s) to QString", t.name);
        xsink->handleExceptions();
        assert(0);
    } else {
        QoreStringValueHelper str(n, QCS_UTF8, xsink);
        if (*xsink)
            return -1;
        qstring = QString::fromUtf8(str->getBuffer());
    }

    return 0;
}

template<typename T>
static T get_char(const AbstractQoreNode *node) {
    if (node && node->getType() == NT_STRING) {
        const QoreStringNode *str = reinterpret_cast<const QoreStringNode *>(node);
        return (T)(str->getBuffer()[0]);
    }
    return (T)(node ? node->getAsInt() : 0);
}

int CommonQoreMethod::returnQtObjectOnStack(Smoke::StackItem &si, const char *cname, const char *mname, const AbstractQoreNode *v, Smoke::Type &t, int index, ExceptionSink *xsink, bool temp, temp_store_s *temp_store) {
    int flags = t.flags & 0x30;

    ReferenceHolder<QoreSmokePrivate> c(xsink);

    if (getObjectStatic(xsink, cname, mname, t.classId, v, c, index, flags == Smoke::tf_ptr))
        return -1;

    void *p = c ? c->object() : 0;
    //printd(0, "qoreToStackStatic() %s p=%p flags=%x\n", className, p, flags);
    if (p) {
        // if the object is on the stack, and we are passing a temporary value to QT,
        // and the node is unique, then we must take the value and clear the QoreSmokePrivate
        // data structure, otherwise we need to make a copy, because otherwise the data would
        // be destroyed before QT has a chance to use it and QT would be using an invalid pointer
        if (temp && flags == Smoke::tf_stack) {
            if (v->is_unique())
                c->clear();
            else {
                p = Marshalling::constructCopy(p, qt_Smoke->classes[t.classId].className, xsink);
                if (*xsink)
                    return -1;
		if (temp_store)
		   temp_store->assign(t.classId, p);
            }
        } else if (flags == Smoke::tf_ref) {
	   assert(!temp);
	   /*
	   if (t.flags & Smoke::tf_const) {
	   }
	   else {
	      p = Marshalling::constructCopy(p, qt_Smoke->classes[t.classId].className, xsink);
	      if (*xsink)
		 return -1;
	   }
	   */
	}
    }

    if (!p && flags == Smoke::tf_stack) {
        // construct a new object if we have to
       CommonQoreMethod cqm(t.name, t.name);
       (* cqm.smokeClass().classFn)(cqm.method().method, 0, cqm.Stack);
       p = cqm.Stack[0].s_class;
       if (temp_store)
	  temp_store->assign(t.classId, p);
    }

    si.s_class = p;
    return 0;
}

// FIXME: avoid string comparisons by setting global values to smoke classIds instead
int CommonQoreMethod::qoreToStackStatic(ExceptionSink *xsink,
                                        Smoke::StackItem &si,
                                        const char *className,
                                        const char *methodName,
                                        Smoke::Type t,
                                        const AbstractQoreNode * node,
                                        int index,
                                        CommonQoreMethod *cqm,
                                        bool temp,
					temp_store_s *temp_store) {
    int tid = t.flags & Smoke::tf_elem;
    int flags = t.flags & 0x30;
    bool iconst = t.flags & Smoke::tf_const;
    temp_store_s *rf = 0;

    //printd(0, "CommonQoreMethod::qoreToStackStatic() %s::%s --- index %d cqm=%p '%s' classId=%d const=%s flags=0x%x (ptr=%s ref=%s) type=%d qore='%s' (%p)\n", className, methodName, index, cqm, t.name, (int)t.classId, iconst ? "true" : "false", flags, flags == Smoke::tf_ptr ? "true" : "false", flags == Smoke::tf_ref ? "true" : "false", tid, node ? node->getTypeName() : "n/a", node);

    // handle references and pointers
    if (flags == Smoke::tf_ref || flags == Smoke::tf_ptr) {
        const AbstractQoreNode *v;

        if (node && node->getType() == NT_REFERENCE) {
            if (!cqm) {
                xsink->raiseException("QT-RETURN-ERROR", "cannot return a Qore reference to the QT library in call to %s::%s()", className, methodName);
                return -1;
            }

            rf = cqm->getRefEntry(index - 1);

            // if the value has already been acquired, use the value
            if (rf->have_ref_value)
                v = rf->ref_value;
            else {
                rf->ref = reinterpret_cast<const ReferenceNode *>(node);
                ReferenceHelper ref(rf->ref, cqm->getVLock(), xsink);
                if (!ref)
                    return -1;

                v = ref.getValue();
                rf->save_ref_value(v);
            }
        } else {
            // in case no reference was passed, then we will still pass the value to the function
            // but no value will be written back to Qore, so it will be treated like a normal
            // call
            v = node;
        }

        const char *name = t.name;
        if (iconst) {
            assert(!strncmp(name, "const ", 6));
            name += 6;
            // set reference to null because argument type is "const"
            if (rf)
                rf->ref = 0;
        }

        if (isptrtype(name, "int")) {
            if (!cqm) {
                xsink->raiseException("QT-RETURN-ERROR", "cannot return type '%s' to the Qt library in call to %s::%s()", name, className, methodName);
                return -1;
            }
            cqm->getRefEntry(index - 1)->assign(v ? v->getAsInt() : 0);
        } else if (!strcmp(name, "char**")) {
            if (!cqm) {
                xsink->raiseException("QT-RETURN-ERROR", "cannot return type '%s' to the Qt library in call to %s::%s()", name, className, methodName);
                return -1;
            }

            ArgStringList *sl = qore_string_sink.get();

            if (v && v->getType() == NT_LIST) {
                const QoreListNode *l = reinterpret_cast<const QoreListNode *>(v);
                ConstListIterator li(l);
//                 printd(0, "CommonQoreMethod::qoreToStack() processing list %p size %d\n", l, l->size());
                while (li.next()) {
                    QoreStringNodeValueHelper str(li.getValue());
                    sl->addStr(str->getBuffer());
                }
            } else {
                QoreStringNodeValueHelper str(v);
                sl->addStr(str->getBuffer());
            }
            cqm->getRefEntry(index - 1)->assign(sl);
        } else if (isptrtype(name, "char")) {
            /*
            if (!cqm) {
               xsink->raiseException("QT-RETURN-ERROR", "cannot return type '%s' to the QT library in call to %s::%s()", name, className, methodName);
               return -1;
            }
            */
            if (is_nothing(v) || is_null(v)) {
                si.s_voidp = 0;
            } else {
                QoreStringNodeValueHelper str(v);
                if (cqm) {
                    cqm->getRefEntry(index - 1)->assign(strdup(str->getBuffer()));
                    si.s_voidp = cqm->getRefEntry(index - 1)->getPtr();
                } else {
                    std::auto_ptr<QoreString> tmp(new QoreString(*str));
		    if (temp_store)
		       temp_store->assign((char *)tmp->getBuffer());
                    si.s_voidp = tmp->giveBuffer();
                }
            }
            return 0;
        } else if (isptrtype(name, "QString")) {
            std::auto_ptr<QString> qstr(new QString());
            if (get_qstring(t, *(qstr.get()), v, xsink))
                return -1;

            if (cqm) {
                cqm->getRefEntry(index - 1)->assign(qstr.release());
                si.s_voidp = cqm->getRefEntry(index - 1)->getPtr();
            } else {
	       if (temp_store)
		  temp_store->assign(qstr.get());
	       si.s_voidp = qstr.release();
            }
            return 0;
        } else if (isptrtype(name, "qreal") || isptrtype(name, "double")) {
            if (!cqm) {
                xsink->raiseException("QT-RETURN-ERROR", "cannot return type '%s' to the QT library in call to %s::%s()", name, className, methodName);
                return -1;
            }
            cqm->getRefEntry(index-1)->assign(v ? (qreal)v->getAsFloat() : 0.0);
        } else if (isptrtype(name, "bool")) {
            if (!cqm) {
                xsink->raiseException("QT-RETURN-ERROR", "cannot return type '%s' to the Qt library in call to %s::%s()", name, className, methodName);
                return -1;
            }
            cqm->getRefEntry(index - 1)->assign(v ? v->getAsBool() : false);
        } else if (isptrtype(name, "QKeySequence")
                   && v && (v->getType() == NT_INT || v->getType() == NT_STRING)) {
            if (v->getType() == NT_STRING) {
                if (cqm) {
                    temp_store_s *re = cqm->getRefEntry(index - 1);
                    re->assign(new QKeySequence(reinterpret_cast<const QoreStringNode*>(v)->getBuffer()));
                    si.s_class = re->getPtr();
                } else {
                    QKeySequence *qks = new QKeySequence(reinterpret_cast<const QoreStringNode*>(v)->getBuffer());
                    si.s_class = qks;
                }
                return 0;
            }
            if (cqm) {
                temp_store_s *re = cqm->getRefEntry(index - 1);
                re->assign(new QKeySequence((QKeySequence::StandardKey)v->getAsInt()));
                si.s_class = re->getPtr();
            } else {
                QKeySequence *qks = new QKeySequence((QKeySequence::StandardKey)v->getAsInt());
                si.s_class = qks;
            }
            return 0;
        } else if (v && isptrtype(name, "QBrush")) {
	   if (           //v->getType() == NT_QTENUM ||
	      v->getType() == NT_INT) {
                if (cqm) {
                    temp_store_s *re = cqm->getRefEntry(index - 1);
                    re->assign(new QBrush((Qt::GlobalColor)v->getAsInt()));
                    si.s_class = re->getPtr();
                } else {
                    QBrush *qb = new QBrush((Qt::GlobalColor)v->getAsInt());
                    si.s_class = qb;
                }
                return 0;
            }
            if (v->getType() == NT_OBJECT) {
                const QoreObject *obj = reinterpret_cast<const QoreObject *>(v);
                ReferenceHolder<QoreSmokePrivateData> p(xsink);

                // check for QColor
                p = reinterpret_cast<QoreSmokePrivateData*>(obj->getReferencedPrivateData(QC_QCOLOR->getID(), xsink));
                if (*xsink)
                    return -1;
                if (p) {
                    QColor *qc = reinterpret_cast<QColor *>(p->object());
                    assert(qc);
                    si.s_class = new QBrush(*qc);
                    return 0;
                }
            }
            return returnQtObjectOnStack(si, className, methodName, v, t, index, xsink, temp, temp_store);
        } else if (isptrtype(name, "QPen") && v && (v->getType() == NT_INT)) {
            assert(iconst);
        } else if (isptrtype(name, "QColor") && v && (v->getType() == NT_INT)) {
            assert(iconst);
            if (cqm) {
                temp_store_s *re = cqm->getRefEntry(index - 1);
                re->assign(new QColor((Qt::GlobalColor)v->getAsInt()));
                si.s_class = re->getPtr();
            } else {
                QColor *qc = new QColor((Qt::GlobalColor)v->getAsInt());
                si.s_class = qc;
            }
            return 0;
        } else if (isptrtype(name, "QPaintDevice")) {
            ReferenceHolder<QoreSmokePrivate> c(xsink);

            if (getObjectStatic(xsink, className, methodName, t.classId, v, c, index, flags == Smoke::tf_ptr))
                return -1;

            void *o = c->object();

            QPaintDevice *p;
            if (!o)
                p = 0;
            else {
                // see if it is a QWidget
                const QoreObject *obj = reinterpret_cast<const QoreObject *>(v);
                if (obj->getClass(QC_QWIDGET->getID())) {
                    p = static_cast<QPaintDevice *>(reinterpret_cast<QWidget *>(o));
		} else
		    p = reinterpret_cast<QPaintDevice *>(o);
            }
            si.s_class = p;

            return 0;
        } else if (name[0] == 'Q') {
            // handle Qt-related exceptions classes first
            QByteArray bname(name);
            if (bname.startsWith("QList<") || bname.startsWith("QVector<") || bname == "QStringList&"
                || bname.startsWith("QMap<") || bname.startsWith("QHash<")
                ) {
//                 printd(0, "CommonQoreMethod::qoreToStackStatic handling a list argument %s\n", name);
                Marshalling::QoreQListBase * list = Marshalling::QoreToQtContainer::Instance()->marshall(t, node, xsink);
                if (!list) {
                    // exception is already set
                    return -1;
                }
                if (cqm) {
                    temp_store_s *re = cqm->getRefEntry(index - 1);
                    re->assign(list);
                    si.s_voidp = re->getPtr();
                } else {
		   /// FIXME: memory leak or error here
		   si.s_voidp = list->voidp();
                }
                return 0;
            } else if (bname.startsWith("QVariant")) {
                std::auto_ptr<Marshalling::QoreQVariant> variant(Marshalling::qoreToQVariant(t, node, xsink));
                if (variant.get()->status == Marshalling::QoreQVariant::Invalid) {
//                     printd(0, "CommonQoreMethod::qoreToStackStatic() invalid QVariant returned\n");
                    return -1;
                }
                if (cqm) {
                    assert(iconst);
                    temp_store_s *re = cqm->getRefEntry(index - 1);
                    re->assign(variant.release());
                    si.s_class = re->getPtr();
//                     printd(0, "CommonQoreMethod::qoreToStackStatic() %s::%s() valid QVariant returned (cqm) si.s_class=%p\n", className, methodName, si.s_class);
                } else {
//                     printd(0, "CommonQoreMethod::qoreToStackStatic() valid QVariant returned (no cqm) variant=%p\n", variant.get());
		    if (temp_store) {
		       temp_store->assign(variant.release());
		       si.s_class = variant.get()->s_class();
		    }
		    else
		       si.s_class = new QVariant(variant->qvariant);
                }
                return 0;
            } else if (bname.startsWith("QDateTime")) {
                std::auto_ptr<QDateTime> qdt(new QDateTime());
                if (get_qdatetime(v, *(qdt.get()), xsink))
                    return -1;

                if (cqm) {
                    cqm->getRefEntry(index - 1)->assign(qdt.release());
                    si.s_class = cqm->getRefEntry(index - 1)->getPtr();
                } else {
		   if (temp_store)
		      temp_store->assign(qdt.get());
		   si.s_class = qdt.release();
                }
                return 0;
            } else if (bname.startsWith("QDate")) {
                std::auto_ptr<QDate> qd(new QDate());
                if (get_qdate(v, *(qd.get()), xsink))
                    return -1;

                if (cqm) {
                    cqm->getRefEntry(index - 1)->assign(qd.release());
                    si.s_class = cqm->getRefEntry(index - 1)->getPtr();
                } else {
		   if (temp_store)
		      temp_store->assign(qd.get());
                    si.s_class = qd.release();
                }
                return 0;
            } else if (bname.startsWith("QTime")) {
                std::auto_ptr<QTime> qt(new QTime());
                if (get_qtime(v, *(qt.get()), xsink))
                    return -1;

                if (cqm) {
                    cqm->getRefEntry(index - 1)->assign(qt.release());
                    si.s_class = cqm->getRefEntry(index - 1)->getPtr();
                } else {
		   if (temp_store)
		      temp_store->assign(qt.get());
                    si.s_class = qt.release();
                }
                return 0;
            }
            // finally the generic Q stuff
            else {
	       return returnQtObjectOnStack(si, className, methodName, v, t, index, xsink, temp, temp_store);
            }
        } else {
//             printd(0, "can't handle ref type '%s'\n", t.name);
            xsink->raiseException("QT-ARGUMENT-ERROR", "unable to handle argument of type '%s'", t.name);
            xsink->handleExceptions();
            assert(0);
            return -1;
        }

        assert(cqm);
        si.s_voidp = cqm->getRefEntry(index - 1)->getPtr();
        return 0;
    }

    switch (tid) {
    case Smoke::t_bool:
        si.s_bool = node ? node->getAsBool() : false;
        return 0;
    case Smoke::t_char:
        si.s_char = get_char<char>(node);
        return 0;
    case Smoke::t_uchar:
        si.s_uchar = get_char<unsigned char>(node);
        return 0;
    case Smoke::t_short:
        si.s_short = node ? node->getAsInt() : 0;
        return 0;
    case Smoke::t_ushort:
        si.s_ushort = node ? node->getAsInt() : 0;
        return 0;
    case Smoke::t_int:
        si.s_int = node ? node->getAsInt() : 0;
        //printd(0, "qoreToStackStatic() setting arg %d to int %d\n", index, si.s_int);
        return 0;
    case Smoke::t_uint:
        si.s_uint = node ? node->getAsBigInt() : 0;
        return 0;
    case Smoke::t_long:
        si.s_long = node ? node->getAsBigInt() : 0;
        return 0;
    case Smoke::t_ulong:
        si.s_int = node ? node->getAsBigInt() : 0;
        return 0;
    case Smoke::t_float:
        si.s_float = node ? node->getAsFloat() : 0.0;
        return 0;
    case Smoke::t_double:
        si.s_double = node ? node->getAsFloat() : 0.0;
        return 0;
    case Smoke::t_enum:
        si.s_enum = node ? node->getAsBigInt() : 0;
        return 0;
    }

    if (!t.name)
        return 0;

    if (!strcmp(t.name, "WId")) {
        if (cqm) {
            temp_store_s *re = cqm->getRefEntry(index - 1);
            re->assign(node->getAsInt());
            si.s_voidp = re->getPtr();
            return 0;
        }
    }

    if (!strcmp(t.name, "QVariant")) {
       std::auto_ptr<Marshalling::QoreQVariant> variant(Marshalling::qoreToQVariant(t, node, xsink));
       if (!variant.get() || variant->status == Marshalling::QoreQVariant::Invalid)
	  return -1;
       if (temp_store) {
	  si.s_class = &variant->qvariant;
	  temp_store->assign(variant.release());
       }
       else {
	  std::auto_ptr<QVariant> value(new QVariant(variant->qvariant));
	  si.s_class = value.release();
       }
       return 0;
    }

    // QString has no class in smoke
    // TODO/FIXME: There we should avoid code duplication
    if (!strcmp(t.name, "QString")) {
#ifdef DEBUG
        if (cqm)
            Q_ASSERT_X(0, "QString as a value, not ptr or ref", "never should go here");
#endif
        std::auto_ptr<QString> qstr(new QString());
        if (get_qstring(t, *(qstr.get()), node, xsink))
            return -1;

	if (temp_store)
	   temp_store->assign(qstr.get());

        si.s_voidp = qstr.release();
        return 0;
    }

    if (tid == Smoke::t_voidp) {
        xsink->raiseException("QT-ARGUMENT-ERROR", "DEBUG: need special handler for void* argument to %s::%s()", className, methodName);
        printd(0, "void * (%s) in %s::%s()\n", t.name, className, methodName);
        assert(false);
        return -1;
    }

    if (t.name[0] == 'Q')
       return returnQtObjectOnStack(si, className, methodName, node, t, index, xsink, temp, temp_store);

    xsink->raiseException("QT-ARGUMENT-ERROR", "don't know how to handle arguments ot type '%s'", t.name);
    return -1;
}

int CommonQoreMethod::qoreToStack(Smoke::Type t,
                                  const AbstractQoreNode * node,
                                  int index) {
    return qoreToStackStatic(m_xsink, Stack[index], m_className, m_methodName, t, node, index, this);
}

int CommonQoreMethod::getObject(Smoke::Index classId, const AbstractQoreNode *v, ReferenceHolder<QoreSmokePrivate> &c, int index, bool nullOk) {
    return getObjectStatic(m_xsink, m_className, m_methodName, classId, v, c, index, nullOk);
}

int CommonQoreMethod::getObjectStatic(ExceptionSink *xsink,
                                      const char *className,
                                      const char *methodName,
                                      Smoke::Index classId,
                                      const AbstractQoreNode *v,
                                      ReferenceHolder<QoreSmokePrivate> &c,
                                      int index,
                                      bool nullOk) {
    assert(classId != -1);
//     printd(0, "CommonQoreMethod::getObjectStatic %s::%s %d\n", className, methodName, classId);

    // QStrings, void* etc.
    if (classId == 0)
        return 0;

    const QoreClass *qc = ClassNamesMap::Instance()->value(classId);
    assert(qc);

    if (!v || v->getType() != NT_OBJECT) {
        if (nullOk && is_nothing(v))
            return 0;

        if (index == -1)
            xsink->raiseException("QT-ARGUMENT-ERROR", "expecting an object derived from %s for the return type of %s::%s(), got type %s instead", qc->getName(), className, methodName, v ? v->getTypeName() : "NOTHING");
        else
            xsink->raiseException("QT-ARGUMENT-ERROR", "expecting an object derived from %s as argument %d to %s::%s(), got type %s instead", qc->getName(), index, className, methodName, v ? v->getTypeName() : "NOTHING");
        return -1;
    }

    const QoreObject *o = reinterpret_cast <const QoreObject*>(v);
    c = reinterpret_cast<QoreSmokePrivate*>(o->getReferencedPrivateData(qc->getID(), xsink));
    if (*xsink)
        return -1;

    if (!c) {
        if (index == -1)
            xsink->raiseException("QT-ARGUMENT-ERROR", "expecting an object derived from %s for the return type of %s::%s(), got class %s instead", qc->getName(), className, methodName, o->getClassName());
        else
            xsink->raiseException("QT-ARGUMENT-ERROR", "expecting an object derived from %s as argument %d to %s::%s(), got class %s instead", qc->getName(), index, className, methodName, o->getClassName());
        return -1;
    }

    return 0;
}

AbstractQoreNode *CommonQoreMethod::returnValue() {
    Smoke::Type t = qt_Smoke->types[m_method.ret];
    if (type_handler->return_value_handler)
        return type_handler->return_value_handler(self, t, Stack, *this, m_xsink);
    assert(!tparams);
    return Marshalling::stackToQore(t, Stack[0], m_xsink);
}

void CommonQoreMethod::postProcessConstructor(QoreSmokePrivate *n_smc) {
    if (type_handler->return_value_handler) {
        assert(n_smc);
        assert(!smc);
        smc = n_smc;
        Smoke::Type t;
	if (type_handler->types.size())
	   t = type_handler->types[0];
        type_handler->return_value_handler(self, t, Stack, *this, m_xsink);
        return;
    }
    assert(!tparams);
}

AbstractQoreNode *CommonQoreMethod::callMethod() {
    //printd(0, "CommonQoreMethod::callMethod() %s::%s() flags=0x%x\n", m_className, m_methodName, method().flags);

    if (!isValid())
        return 0;
    if (!suppress_method) {
        assert(m_method.method >= 0);
        // call smoke-qt method if not suppressed by arg handler
        (* smokeClass().classFn)(m_method.method, smc ? smc->object() : 0, Stack);
    }

    // return return value
    return returnValue();
}

void *CommonQoreMethod::callConstructor() {
    // call constructor
    (* smokeClass().classFn)(m_method.method, 0, Stack);

    // 0 argument in args is always the return value. So it's
    // the newly created object for now.
    void *qtObj = Stack[0].s_class;
    assert(qtObj);

    //printd(0, "CommonQoreMethod::callConstructor() %s installing qt bindings, qtObj=%p\n", className, qtObj);

    // install qt bindings. It's mandatory for all smoked objects
    Smoke::StackItem a[2];
    a[1].s_voidp = QoreSmokeBinding::Instance(qt_Smoke);
    (* smokeClass().classFn)(0, qtObj, a);

    return qtObj;
}
