/*
  qoresmokeclass.cpp

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

#include "qoresmokeclass.h"
#include "qoresmokebinding.h"
#include "qoremarshalling.h"
#include "commonqoremethod.h"
#include "qoreqtenumnode.h"

#include <iostream>
#include <QtDebug>
#include <QVariant>
#include <QAbstractItemModel>
#include <map>

// map from class names to QoreClass pointers
qcmap_t parse_class_map;

Smoke::ModuleIndex SMI_QOBJECT;

extern Smoke* qt_Smoke;

QtQoreMap qt_qore_map;

// set of all QWidgets without parents (= windows) that must be deleted
// before QApplication, otherwise a crash will result
QoreWidgetManager QWM;

ClassNamesMap* ClassNamesMap::m_instance = NULL;
ClassMap * ClassMap::m_instance = NULL;

static AbstractQoreNode *f_QOBJECT_connect(const QoreMethod &method, const QoreListNode *params, ExceptionSink *xsink);
static AbstractQoreNode *QOBJECT_connect(const QoreMethod &method, QoreObject *self, QoreSmokePrivateQObjectData *apd, const QoreListNode *params, ExceptionSink *xsink);

class QRegionTypeHelper : public AbstractQoreClassTypeInfoHelper {
public:
   DLLLOCAL QRegionTypeHelper() : AbstractQoreClassTypeInfoHelper("QRegion", QDOM_GUI) {
   }

   DLLEXPORT virtual bool checkTypeInstantiationImpl(AbstractQoreNode *&n, ExceptionSink *xsink) const {
      if (!n || n->getType() != NT_OBJECT)
	 return false;

      QoreObject *o = reinterpret_cast<QoreObject *>(n);
    // see if we can get a QRect
      ReferenceHolder<QoreSmokePrivateData> pd(reinterpret_cast<QoreSmokePrivateData *>(o->getReferencedPrivateData(QC_QRECT->getID(), xsink)), xsink);
      if (!pd)
	 return false;
      QRegion *qr = new QRegion(*(pd->getObject<QRect>()));
      QoreObject *rv = new QoreObject(QC_QREGION, getProgram());
      QoreSmokePrivateData *data = new QoreSmokePrivateData(SCI_QREGION, qr, rv);
      rv->setPrivate(QC_QREGION->getID(), data);
      n->deref(xsink);
      n = rv;
      return true;
   }
   DLLEXPORT virtual int testTypeCompatibilityImpl(const AbstractQoreNode *n) const {
      if (!n || n->getType() != NT_OBJECT || !testObjectClassAccess(reinterpret_cast<const QoreObject *>(n), QC_QREGION))
	 return QTI_NOT_EQUAL;
      return QTI_AMBIGUOUS;
   }
   DLLEXPORT virtual int parseEqualImpl(const QoreTypeInfo *typeInfo) const {
      return typeInfoGetClass(typeInfo) == QC_QREGION ? QTI_AMBIGUOUS : QTI_NOT_EQUAL;
   }
} QRegionTypeHelper;

const QoreMethod *findUserMethod(const QoreClass *qc, const char *name) {
    const QoreMethod *m = qc->findMethod(name);
    return m && m->isUser() ? m : 0;
}

void ClassMap::registerMethods() {
    QByteArray mname;
    Smoke::Method m;
    for (int i = 1; i < qt_Smoke->numMethodMaps; ++i) {
        Smoke::MethodMap &mm = qt_Smoke->methodMaps[i];

        QByteArray cname = qt_Smoke->classes[mm.classId].className;
        QByteArray munged = qt_Smoke->methodNames[mm.name];

        // overloads are < 0. Done in ambiguousMethodList part of this condition
        if (mm.method > 0) {

//             if (!strcmp(qt_Smoke->methodNames[qt_Smoke->methods[mm.method].name], "quit")) {
//                 printd(0, "ClassMap::registerMethods() i=%d %s::%s() %smethod=%d name=%d mname=%d cmeth=%d '%s'\n", i, qt_Smoke->classes[mm.classId].className, qt_Smoke->methodNames[mm.name], qt_Smoke->methods[mm.method].flags & Smoke::mf_static ? "(static) " : "", mm.method, mm.name, qt_Smoke->methods[mm.method].name, qt_Smoke->methods[mm.method].method, qt_Smoke->methodNames[qt_Smoke->methods[mm.method].name]);
//             }

            m = qt_Smoke->methods[mm.method];
            mname = qt_Smoke->methodNames[m.name];
            m_map[cname][mname].insert(munged, typeList(m, mm.method));

        } else {
            // turn into ambiguousMethodList index
            int ambIx = -mm.method;

//             if (!strcmp(qt_Smoke->methodNames[qt_Smoke->methods[ambIx].name], "quit")) {
//                 printd(0, "ClassMap::registerMethods() (-) i=%d %s::%s() %smethod=%d name=%d mname=%d '%s'\n", i, qt_Smoke->classes[mm.classId].className, qt_Smoke->methodNames[mm.name], qt_Smoke->methods[ambIx].flags & Smoke::mf_static ? "(static) " : "", ambIx, mm.name, qt_Smoke->methods[ambIx].name, qt_Smoke->methodNames[qt_Smoke->methods[ambIx].name]);
//             }

            while (qt_Smoke->ambiguousMethodList[ambIx]) {
                Smoke::Index ambMethIx = qt_Smoke->ambiguousMethodList[ambIx];
                m = qt_Smoke->methods[ambMethIx];
                mname = qt_Smoke->methodNames[m.name];
                m_map[cname][mname].insert(munged, typeList(m, ambMethIx));

                ++ambIx;
            }
        }
    }
}

void ClassMap::registerMethod(const char *class_name, const char *method_name, const char *munged_name, Smoke::Index method_index, TypeHandler &type_handler) {
//     assert(m_map[class_name][method_name][munged_name].types.count() == 0);

    method_index = qt_Smoke->methodMaps[method_index].method;
    if (method_index < 0)
        method_index = -method_index;

    type_handler.method = method_index;

    m_map[class_name][method_name].insert(munged_name, type_handler);
    //printd(0, "ClassMap::registerMethod(%s, %s, %s)\n", class_name, method_name, munged_name);
}

ClassMap::TypeHandler ClassMap::typeList(Smoke::Method m, Smoke::Index methodIndex) {
    ClassMap::TypeHandler ret;
    Smoke::Index *idx = qt_Smoke->argumentList + m.args;
    while (*idx) {
        ret.types.append(qt_Smoke->types[*idx]);
        idx++;
    }
    ret.method = methodIndex;

    return ret;
}

// add an argument handler to all versions of a specific munged method
void ClassMap::addArgHandler(const char *cls, const char *meth, const char *munged, arg_handler_t arg_handler) {
    /*
      for (MungledToTypes::iterator i = m_map[cls][meth].begin(), e = m_map[cls][meth].end(); i != e; ++i)
        printf("%s\n", i.key().data());
    */

    assert(m_map[cls][meth].values(munged).count());

    for (MungledToTypes::iterator i = m_map[cls][meth].find(munged), e = m_map[cls][meth].end(); i != e && i.key() == munged; ++i)
        i.value().arg_handler = arg_handler;
}

// add an argument handler to all versions of a method
void ClassMap::addArgHandler(const char *cls, const char *meth, arg_handler_t arg_handler) {
#ifdef DEBUG
    if (!m_map[cls][meth].count())
        printd(0, "ERROR: %s::%s() does not exist\n", cls, meth);
    assert(m_map[cls][meth].count());
#endif

    for (MungledToTypes::iterator i = m_map[cls][meth].begin(), e = m_map[cls][meth].end(); i != e; ++i)
        i.value().arg_handler = arg_handler;
}

void ClassMap::setRVHandler(const char *cls, const char *meth, const char *munged, return_value_handler_t rv_handler) {
    assert(m_map[cls][meth].values(munged).count());

    for (MungledToTypes::iterator i = m_map[cls][meth].find(munged), e = m_map[cls][meth].end(); i != e && i.key() == munged; ++i)
        i.value().return_value_handler = rv_handler;
}

void ClassMap::setRVHandler(const char *cls, const char *meth, return_value_handler_t rv_handler) {
    assert(m_map[cls][meth].count());

    for (MungledToTypes::iterator i = m_map[cls][meth].begin(), e = m_map[cls][meth].end(); i != e; ++i)
        i.value().return_value_handler = rv_handler;
}

#ifdef DEBUG
#include <QFile>
void ClassMap::printMapToFile(const QString & fname) {
    QFile f(fname);
    f.open(QIODevice::WriteOnly | QIODevice::Truncate);

    QMap<QByteArray,int> typesUsed;
    foreach (QByteArray ckey, m_map.keys()) {
        f.write("\nCLASS " + ckey + "\n");
        foreach (QByteArray mkey, m_map[ckey].uniqueKeys()) {
            f.write("\tMETHOD " + mkey + "\n");
            foreach (QByteArray mmkey, m_map[ckey][mkey].uniqueKeys()) {
                f.write("\t\tMUNGED " + mmkey + "\n");
                foreach(TypeHandler th, m_map[ckey][mkey].values(mmkey)) {
                    f.write("\t\t\tmethodId: ");
                    QByteArray num;
                    num.setNum(th.method, 10);
                    f.write(num + "\n");
                    f.write("\t\t\tARGS ");
                    foreach(Smoke::Type t, th.types) {
                        if (typesUsed.contains(t.name))
                            typesUsed[t.name]++;
                        else
                            typesUsed[t.name] = 1;
                        f.write(t.name);
                        if (t.flags == Smoke::tf_ref && t.flags != Smoke::tf_const)
                            f.write("(REF) ");
                        f.write("   ");
                    }
                    f.write("\n");
                    f.write("\t\t\tRETURNS ");
                    f.write(qt_Smoke->types[qt_Smoke->methods[th.method].ret].name);
                    if (qt_Smoke->methods[th.method].flags & Smoke::mf_enum)
                        f.write(" (ENUM)");
                    f.write("\n");
                }
                f.write("\n");
            }
        }
    }
    f.close();
    // types
    QFile ft(fname + ".types.txt");
    ft.open(QIODevice::WriteOnly | QIODevice::Truncate);
    QMapIterator<QByteArray, int> i(typesUsed);
    QString temp("%1\n");
    while (i.hasNext()) {
        i.next();
        ft.write(i.key());
        ft.write(temp.arg(i.value(), 15).toUtf8());
    }
    ft.close();
}
#endif

static AbstractQoreNode *QOBJECT_createSignal(QoreObject *self, QoreSmokePrivateQObjectData *qo, const QoreListNode *args, ExceptionSink *xsink) {
   HARD_QORE_PARAM(str, const QoreStringNode, args, 0);
   qo->createSignal(str->getBuffer(), xsink);
   return 0;
}

static AbstractQoreNode *QOBJECT_emit(QoreObject *self, QoreSmokePrivateQObjectData *qo, const QoreListNode *args, ExceptionSink *xsink) {
   HARD_QORE_PARAM(str, const QoreStringNode, args, 0);
   qo->emitSignal(str->getBuffer(), args, xsink);
   return 0;
}

static bool qobject_delete_blocker(QoreObject *self, QoreSmokePrivateQObjectData *data) {
    return data->deleteBlocker(self);
}

#ifdef DEBUG_0
static void dump_parse_class_map() {
   for (qcmap_t::iterator i = parse_class_map.begin(), e = parse_class_map.end(); i != e; ++i)
      printf("'%s' = %p (%s)\n", i->first.c_str(), i->second, i->second->getName());
}
#endif

static QoreClass *getNewClass(const char *name) {
   if (!strcmp(name, "QRegion"))
      return QRegionTypeHelper.getClass();
   return new QoreClass(name, QDOM_GUI);
}

// get type from parse class map or create it and add to map
static const QoreTypeInfo *getInitClassType(const char *name, const Smoke::Type &t) {
   //printd(0, "getInitClassType(%s)\n", name);

   qcmap_t::iterator i = parse_class_map.find(name);
   if (i == parse_class_map.end()) {

      Smoke::ModuleIndex mi = qt_Smoke->findClass(name);
      if (!mi.smoke) {
	 //printd(0, "getInitClassType(%s) skipping mapping for tname=%s\n", name, t.name);
	 return 0;
      }
      QoreClass *qc = getNewClass(name);
      parse_class_map[name] = qc;
      return qc->getTypeInfo();
   }
   return i->second->getTypeInfo();
}

static const QoreTypeInfo *getInitType(const Smoke::Type &t) {
   //int flags = t.flags & 0x30;
   int tid = t.flags & Smoke::tf_elem;
   switch (tid) {
      case Smoke::t_bool:
	 return boolTypeInfo;
      case Smoke::t_char:
      case Smoke::t_uchar:
	 return stringTypeInfo;
      case Smoke::t_short:
      case Smoke::t_ushort:
      case Smoke::t_int:
      case Smoke::t_uint:
      case Smoke::t_long:
      case Smoke::t_ulong:
	 return bigIntTypeInfo;
      case Smoke::t_float:
      case Smoke::t_double:
	 return floatTypeInfo;
      case Smoke::t_enum:
	 return enumTypeInfo;
      case Smoke::t_voidp:
      case Smoke::t_class:
	 break;
      default:
	 //printd(0, "getInitType() tid=%d name=%s class=%d\n", tid, t.name, t.classId);
	 assert(false);
   }

   if (!t.name)
      return 0;

   QByteArray cname(t.name);
   if (cname.startsWith("QList<"))
      return listTypeInfo;

   char *f = cname.data();
   if (cname.startsWith("const "))
      f += 6;
   char *p = strchrs(f, "&*");
   if (p)
      *p = '\0';

   if (tid == Smoke::t_voidp) {
      if (!strcmp(f, "QString")
	  || isptrtype(f, "QString")
	  || !strcmp(f, "char")
	  || !strcmp(f, "uchar")
	  || !strcmp(f, "unsigned char"))
	 return stringTypeInfo;

      if (!strcmp(f, "QStringList"))
	 return listTypeInfo;

      //printd(0, "getInitType() name=%s (%s) class=%d returning NULL\n", t.name, f, t.classId);
      return 0;
   }

   assert(tid == Smoke::t_class);

   if (strchr(f, '<'))
      return 0;

   if (!strcmp(f, "QVariant"))
      return 0;

   // see if the class has been created already
   QoreClass *qc = ClassNamesMap::Instance()->value(f);
   if (qc)
      return qc->getTypeInfo();

   return getInitClassType(f, t);
}

// get class from map, and delete it if it's there, or simply create the class
static QoreClass *getInitClass(const char *name) {
   qcmap_t::iterator i = parse_class_map.find(name);
   if (i == parse_class_map.end())
      return getNewClass(name);

   QoreClass *qc = i->second;
   parse_class_map.erase(i);
   return qc;
}

//static const QoreTypeInfo *getClassType(const char *name) {}

QoreSmokeClass::QoreSmokeClass(const char * className, QoreNamespace &qt_ns) : m_qoreClass(0) {
    m_classId = qt_Smoke->findClass(className);
    // if the class is already registered, skip
    if (ClassNamesMap::Instance()->value(m_classId.index)) {
        return;
    }
    if (!m_classId.smoke) {
        printd(0, "Unknown class %s. Skipping.\n", className);
        return;
    }

    m_class = m_classId.smoke->classes[m_classId.index];

    m_qoreClass = getInitClass(m_class.className);

    //printd(5, "QoreSmokeClass::QoreSmokeClass() processing class %s\n", m_class.className);

    if (!QC_QOBJECT && !strcmp(className, "QObject")) {
        QC_QOBJECT = m_qoreClass;
        m_qoreClass->addMethodExtended("createSignal", (q_method_t)QOBJECT_createSignal, false, QDOM_DEFAULT, nothingTypeInfo, 1, stringTypeInfo, QORE_PARAM_NO_ARG);
        m_qoreClass->addMethodExtended("emit", (q_method_t)QOBJECT_emit, false, QDOM_DEFAULT, nothingTypeInfo, 1, stringTypeInfo, QORE_PARAM_NO_ARG);
        m_qoreClass->setDeleteBlocker((q_delete_blocker_t)qobject_delete_blocker);
        SMI_QOBJECT = m_classId;
    }

    ClassNamesMap::Instance()->addItem(m_classId.index, m_qoreClass);

    // namespace "enums" constants
    m_namespace = 0;

    // add own stuff
    addClassMethods(m_classId.index, true);

    // add parents' stuff
    addSuperClasses(m_classId.index, qt_ns);
    
    if (m_namespace) {
        qt_ns.addNamespace(m_namespace);
        m_namespace = 0;
    }

    qt_ns.addSystemClass(m_qoreClass);
}

QoreSmokeClass::~QoreSmokeClass() {
//     if (m_classId.smoke)
//         assert(!m_namespace);
}

void QoreSmokeClass::addSuperClasses(Smoke::Index ix, QoreNamespace &qt_ns) {
    Smoke::Class c = qt_Smoke->classes[ix];
    for (Smoke::Index *i = qt_Smoke->inheritanceList + c.parents; *i; ++i) {
        QoreClass *parent = ClassNamesMap::Instance()->value(*i);	
        if (!parent) {
            QoreSmokeClass qsc(qt_Smoke->classes[(*i)].className, qt_ns);
	    if (!qsc.m_qoreClass) {
	       printd(0, "QoreSmokeClass::addSuperClasses() cannot add %s as base class of %s (class not found)\n", qt_Smoke->classes[(*i)].className, m_qoreClass->getName());
	       continue;
	    }
            parent = qsc.m_qoreClass;
        }
        if (QC_QOBJECT && parent == QC_QOBJECT) {
            m_qoreClass->setDeleteBlocker((q_delete_blocker_t)qobject_delete_blocker);
        }
        m_qoreClass->addBuiltinVirtualBaseClass(parent);

        addSuperClasses(*i, qt_ns);
    }
}

// FIXME: add special common methods for methods with hard typing
void QoreSmokeClass::addClassMethods(Smoke::Index classIx, bool targetClass) {
    for (Smoke::Index i = 1; i < qt_Smoke->numMethods; ++i) {
        Smoke::Method method = qt_Smoke->methods[i];

        if (classIx != method.classId)//m_classId.index)
            continue;

        const char * methodName = qt_Smoke->methodNames[method.name];
        bool isPrivate = method.flags & Smoke::mf_protected;

        //printd(0, "QoreSmokeClass::addClassMethods() %s::%s() enum=%d static=%d\n", m_qoreClass->getName(), methodName, method.flags & Smoke::mf_enum, method.flags & Smoke::mf_static);

        // only target classes handle enums. Parents do it in their init.
        if (targetClass && (method.flags & Smoke::mf_enum)) {
            if (m_namespace == 0)
                m_namespace = new QoreNamespace(m_class.className);
            // Enum methods are static, so no object is required: 0
            Smoke::StackItem arg[1]; // only one item on stack for retval
            (* m_class.classFn)(method.method, 0, arg);

            //printd(0, "adding enum constant %s::%s (%d)\n", m_class.className, methodName, arg[0].s_enum);
            Smoke::Type t = qt_Smoke->types[method.ret];
            m_namespace->addConstant(methodName, new QoreQtEnumNode(arg[0].s_enum, t), enumTypeInfo);
            continue;
        }

        if (targetClass && (method.flags & Smoke::mf_dtor)) {
            assert(!m_qoreClass->getDestructor());
            m_qoreClass->setDestructor2(common_destructor);
            continue;
        }

	// skip parent constructors
        if ((method.flags & Smoke::mf_ctor) && !targetClass)
	   continue;

	// create return and argument type information for method signature
	type_vec_t argTypeInfo;
	argTypeInfo.reserve(method.numArgs);
	Smoke::Index *idx = qt_Smoke->argumentList + method.args;
	for (unsigned i = 0; i < method.numArgs; ++i) {
	   assert(idx[i]);
	   argTypeInfo.push_back(getInitType(qt_Smoke->types[idx[i]]));
	}

        if ((method.flags & Smoke::mf_ctor)) {
	   const QoreMethod *qm = m_qoreClass->getConstructor();
	   if (qm && qm->existsVariant(argTypeInfo)) {
	      //printd(0, "QoreSmokeClass::addClassMethods() skipping already-created variant %s::%s()\n", m_qoreClass->getName(), methodName);
	      continue;
	   }

	   m_qoreClass->setConstructorExtendedList2(common_constructor, isPrivate, QDOM_DEFAULT, argTypeInfo);
	   continue;
        }

	const QoreTypeInfo *returnTypeInfo = getInitType(qt_Smoke->types[method.ret]);

        if (method.flags & Smoke::mf_static) {
	   // set function pointer for methods with explicit implementations
	   q_static_method2_t func = common_static_method;
	   if (m_qoreClass == QC_QOBJECT && !strcmp(methodName, "connect"))
	      func = f_QOBJECT_connect;

	   const QoreMethod *qm = m_qoreClass->findStaticMethod(methodName);
	   if (qm && qm->existsVariant(argTypeInfo)) {
	      //printd(0, "QoreSmokeClass::addClassMethods() skipping already-created variant (static) %s::%s()\n", m_qoreClass->getName(), methodName);
	      continue;
	   }
	   
	   m_qoreClass->addStaticMethodExtendedList2(methodName, func, isPrivate, QDOM_DEFAULT, returnTypeInfo, argTypeInfo);
	   continue;
        }

	// change name if necessary
	const char *name = methodName;
	
        // common method. See qoreMethodName2Qt().
        if (!strcmp(methodName, "copy")) {
	   name = "qt_copy";
        } else if (!strcmp(methodName, "constructor")) {
	   name = "qt_constructor";
        } else if (!strcmp(methodName, "inherits")) {
	   name = "qt_inherits";
        }
	
	const QoreMethod *qm = m_qoreClass->findMethod(name);
	if (qm && qm->existsVariant(argTypeInfo)) {
	   //printd(0, "QoreSmokeClass::addClassMethods() skipping already-created variant %s::%s()\n", m_qoreClass->getName(), name);
	   continue;
	}

	q_method2_t func = (q_method2_t)common_method;
	if (m_qoreClass == QC_QOBJECT && !strcmp(methodName, "connect"))
	   func = (q_method2_t)QOBJECT_connect;
	    
	m_qoreClass->addMethodExtendedList2(name, func, isPrivate, QDOM_DEFAULT, returnTypeInfo, argTypeInfo);
    }

/*
    {
       // add generic methods for custom conversion
       QoreMethodIterator mi(m_qoreClass);
       while (mi.next()) {
	  const QoreMethod *qm = mi.getMethod();
	  if (qm->existsVariant(0, NULL))
	     continue;
	  
	  if (qm == m_qoreClass->getConstructor())
	     m_qoreClass->setConstructorExtended2(common_constructor, qm->isPrivate());
	  else {
	     q_method2_t func = (q_method2_t)common_method;
	     if (m_qoreClass == QC_QOBJECT && !strcmp(qm->getName(), "connect"))
		func = (q_method2_t)QOBJECT_connect;       
	 
	     m_qoreClass->addMethodExtended2(qm->getName(), func, qm->isPrivate(), QDOM_DEFAULT, qm->getUniqueReturnTypeInfo());
	  }
       }
    }

    {
       // add generic static methods for custom conversions
       QoreStaticMethodIterator mi(m_qoreClass);
       while (mi.next()) {
	  const QoreMethod *qm = mi.getMethod();
	  if (qm->existsVariant(0, NULL))
	     continue;
	  
	  q_static_method2_t func = common_static_method;
	  if (m_qoreClass == QC_QOBJECT && !strcmp(qm->getName(), "connect"))
	     func = f_QOBJECT_connect;
	  
	  m_qoreClass->addStaticMethodExtended2(qm->getName(), func, qm->isPrivate(), QDOM_DEFAULT, qm->getUniqueReturnTypeInfo());
       }
    }
*/
  
    if (!strcmp(m_class.className, "QVariant")) {
       //printd(0, "registered toQore\n");
       m_qoreClass->addMethod2("toQore", (q_method2_t)Marshalling::return_qvariant);
    }
}

static AbstractQoreNode *f_QOBJECT_connect(const QoreMethod &method, const QoreListNode *params, ExceptionSink *xsink) {
    const QoreObject *p = test_object_param(params, 0);

    ReferenceHolder<QoreSmokePrivateQObjectData> sender(reinterpret_cast<QoreSmokePrivateQObjectData *>(p ? p->getReferencedPrivateData(QC_QOBJECT->getID(), xsink) : 0), xsink);
    if (!sender) {
        if (!*xsink)
            xsink->raiseException("QOBJECT-CONNECT-ERROR", "first argument is not a QObject");
        return 0;
    }

    const QoreStringNode *str = test_string_param(params, 1);
    if (!str) {
        xsink->raiseException("QOBJECT-CONNECT-ERROR", "missing signal string as second argument");
        return 0;
    }
    const char *signal = str->getBuffer();

    p = test_object_param(params, 2);
    ReferenceHolder<QoreSmokePrivateQObjectData> receiver(reinterpret_cast<QoreSmokePrivateQObjectData *>(p ? p->getReferencedPrivateData(QC_QOBJECT->getID(), xsink) : 0), xsink);

    if (!p) {
        if (!*xsink)
            xsink->raiseException("QOBJECT-CONNECT-ERROR", "missing receiving object as third argument");
        return 0;
    }

    // get member/slot name
    str = test_string_param(params, 3);
    if (!str) {
        xsink->raiseException("QOBJECT-CONNECT-ERROR", "missing slot as fourth argument");
        return 0;
    }
    const char *member = str->getBuffer();

    /*
    p = get_param(params, 4);
    int conn_type = is_nothing(p) ? Qt::AutoConnection : p->getAsInt();

    bool b = QObject::connect(sender->getQObject(), signal, receiver->getQObject(), member, (enum Qt::ConnectionType)conn_type);
    return get_bool_node(b);
    */
    receiver->connectDynamic(*sender, signal, p, member, xsink);
    return 0;
}

static AbstractQoreNode *QOBJECT_connect(const QoreMethod &method, QoreObject *self, QoreSmokePrivateQObjectData *apd, const QoreListNode *params, ExceptionSink *xsink) {
    // HACK: workaround for $.connect(o, sig, o, slot) call type
    if (num_params(params) == 4)
        return f_QOBJECT_connect(method, params, xsink);

    QoreObject *p = test_object_param(params, 0);
    ReferenceHolder<QoreSmokePrivateQObjectData> sender(p ? (QoreSmokePrivateQObjectData *)p->getReferencedPrivateData(QC_QOBJECT->getID(), xsink) : 0, xsink);

    if (!sender) {
        if (!xsink->isException())
            xsink->raiseException("QOBJECT-CONNECT-PARAM-ERROR", "expecting a QObject object as first argument to QObject::connect()");
        return 0;
    }

    const QoreStringNode *pstr = test_string_param(params, 1);
    if (!pstr) {
        xsink->raiseException("QOBJECT-CONNECT-PARAM-ERROR", "expecting a string as second argument to QObject::connect()");
        return 0;
    }
    const char *signal = pstr->getBuffer();

    pstr = test_string_param(params, 2);
    if (!pstr) {
        xsink->raiseException("QOBJECT-CONNECT-PARAM-ERROR", "expecting a string as third argument to QObject::connect()");
        return 0;
    }
    const char *meth = pstr->getBuffer();

    //p = get_param(params, 3);
    //Qt::ConnectionType type = (Qt::ConnectionType)(p ? p->getAsInt() : 0);
    //return get_bool_node(qo->getQObject()->connect(sender->getQObject(), signal, method, type));

    apd->connectDynamic(*sender, signal, self, meth, xsink);
    return 0;
}

void common_constructor(const QoreClass &myclass, QoreObject *self,
                        const QoreListNode *params, ExceptionSink *xsink) {
    const char * className = myclass.getName();
//     printd(0, className);
    CommonQoreMethod cqm(self, 0, className, className, params, xsink);

    if (!cqm.isValid()) {
//         printd(0, "common_constructor() %s failed to set up constructor call\n", className);
        assert(*xsink);
        return;
    }
    assert(!*xsink);

    //printd(0, "common_constructor() %s set up constructor call, calling constuctor method %d\n", className, cqm.method().method);
    void *qtObj = cqm.callConstructor();
    //printd(0, "common_constructor() %s setting up internal object %p\n", className, qtObj);

    if (!qtObj) {
       assert(*xsink);
       return;
    }

    // Setup internal object
    QoreSmokePrivate *obj;

    bool is_qobject = false;
    if (myclass.getClass(QC_QOBJECT->getID())) {
       if (myclass.getClass(QC_QABSTRACTITEMMODEL->getID()))
	  obj = new QoreSmokePrivateQAbstractItemModelData(cqm.method().classId, (QObject *)qtObj);
       else
	  obj = new QoreSmokePrivateQObjectData(cqm.method().classId, (QObject *)qtObj);

       is_qobject = true;
    } else {
        obj = new QoreSmokePrivateData(cqm.method().classId, qtObj, self);
        //printd(0, "common_constructor() (EE) is not QObject based: %s\n", className);
    }

    self->setPrivate(myclass.getID(), obj);
    cqm.postProcessConstructor(obj);

    if (is_qobject) {
       // set the Qt->QoreObject property last in case there are any qt
       // metacalls before the qt object holder is saved as private data
       QoreQtVirtualFlagHelper vfh;
       QObject *qtBaseObj = static_cast<QObject*>(qtObj);
       qtBaseObj->setProperty(QORESMOKEPROPERTY, reinterpret_cast<qulonglong>(self));
    }

    assert(!*xsink);

    //printd(0, "common_constructor() %s setting private data %p for classid %d objclassid %d self:%p\n",
    //       className, obj, myclass.getID(), self->getClass()->getID(), self);
}

// a helper function to handle conflicting names
const char * qoreMethodName2Qt(const char * name) {
    if (!strcmp(name, "qt_copy")) return "copy";
    else if (!strcmp(name, "qt_constructor")) return "constructor";
    else if (!strcmp(name, "qt_inherits")) return "inherits";
    else return name;
}

AbstractQoreNode * common_method(const QoreMethod &method,
                                 QoreObject *self,
                                 AbstractPrivateData *apd,
                                 const QoreListNode *params,
                                 ExceptionSink *xsink) {
    const char * methodName = qoreMethodName2Qt(method.getName());
    const char * className = method.getClass()->getName();

    QoreSmokePrivate * smc = reinterpret_cast<QoreSmokePrivate*>(apd);
    CommonQoreMethod cqm(self, smc, className, methodName, params, xsink);

    Q_ASSERT_X(smc!=0, "cast", "cannot get QoreSmokeClass from QoreObject");
#ifdef DEBUG
    if (!smc->object()) printd(0, "QT Object not availble when calling %s::%s()\n", className, methodName);
#endif
    Q_ASSERT_X(smc->object()!=0, "cast", "QoreSmokePrivate::object() returned 0");
// if (QByteArray(methodName) == "setColumnWidths") assert(0);
    return cqm.callMethod();
}


AbstractQoreNode * common_static_method(const QoreMethod &method,
                                        const QoreListNode *params,
                                        ExceptionSink *xsink) {
    const char * methodName = qoreMethodName2Qt(method.getName());
    const char * className = method.getClass()->getName();
    CommonQoreMethod cqm(0, 0, className, methodName, params, xsink);
    return cqm.callMethod();
}

void common_destructor(const QoreClass &thisclass, QoreObject *self, AbstractPrivateData *private_data, ExceptionSink *xsink) {
    // unconditionally deref private data on exit to avoid a memory leak
    ReferenceHolder<AbstractPrivateData> ref_helper(private_data, xsink);

    assert(dynamic_cast<QoreSmokePrivate*>(private_data));

    QoreSmokePrivate *p = reinterpret_cast<QoreSmokePrivate*>(private_data);

    void *pobj = p->object();

    if (!pobj) {
        //printd(0, "common_destructor (WW) QoreSmokePrivate's Qt object does not exist anymore\n");
        return;
    }

    if (p->isQObject()) {
        QObject * qtObj = reinterpret_cast<QObject*>(pobj);
        // set property to 0 because QoreObject is being deleted
        {
            QoreQtVirtualFlagHelper vfh;
            qtObj->setProperty(QORESMOKEPROPERTY, (qulonglong)0);
        }

        if (qtObj->parent()) {
	   //printd(0, "common_destructor() %s::destructor() object %p not deleted; has parent\n", thisclass.getName(), qtObj);
            // clear the private data
            p->clear();
            return;
        }
	else if (qtObj->isWidgetType()) {
	   // delete from QoreWidgetManager
	   QWM.remove(reinterpret_cast<QWidget*>(qtObj));
	}
    }

    if (p->externallyOwned()) {
       //printd(0, "common_destructor() %s::destructor(): QT object %p is externally owned\n", thisclass.getName(), p->object());
        p->clear();
        return;
    }

    // if the QApplication object is being deleted, then delete all QWidget objects that still exist
    // because the QApplication destructor will free windowing resources and subsequently deleting
    // any QWidget objects will cause a crash
    if (&thisclass == QC_QAPPLICATION)
       QWM.deleteAll();

    const char * className = qt_Smoke->classes[p->smokeClass()].className;
    QByteArray methodName("~");
    methodName += className;

    CommonQoreMethod cqm(self, p, className, methodName.constData(), 0, xsink);

    //printd(0, "common_destructor %s::destructor() Qt: %p\n", thisclass.getName(), p->object());
    assert(cqm.isValid());
    // call the destructor -- and take the object from the private data first
    (* cqm.smokeClass().classFn)(cqm.method().method, p->takeObject(), cqm.Stack);
}

void emitStaticSignal(QObject *sender, int signalId, const QMetaMethod &qmm, const QoreListNode *args, ExceptionSink *xsink) {
    QList<QByteArray> params = qmm.parameterTypes();

    int num_args = params.size();
    void *sig_args[num_args + 1];
    void *save_args[num_args];
    Smoke::StackItem si[num_args];

    QoreQtDynamicMethod::SmokeTypeList tlist;

    // return return value to 0
    sig_args[0] = 0;

    // iterate through signal parameters to build argument list
    for (int i = 0; i < num_args; ++i) {
        // get argument QoreNode
        const AbstractQoreNode *n = args ? args->retrieve_entry(i + 1) : 0;
//         const char *str = params[i].data();

        QoreQtDynamicMethod::addType(tlist, params[i].data(), params[i].size(), qmm.signature(), xsink);
        if (*xsink)
            return;
        QoreQtDynamicMethod::qoreToQt(xsink, tlist[i], si[i], sig_args[i + 1], save_args[i], n, "QObject", "emit", i + 1);
	if (*xsink)
	   return;
    }

    QMetaObject::activate(sender, signalId, signalId, sig_args);

    // iterate through signal parameters to delete temporary values
    //for (int i = 0; i < num_args; ++i)
    //  tlist[i]->del_arg(save_args[i]);
}
