/* -*- indent-tabs-mode: nil -*- */
/*
  qoresmokeclass.cpp

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

#include "qoresmokeclass.h"
#include "qoresmokebinding.h"
#include "qoremarshalling.h"
#include "commonqoremethod.h"
#include "qoreqtenumnode.h"
#include "typeinfohelpers.h"
#include "qoreqlist.h"

#include <iostream>
#include <QtDebug>
#include <QVariant>
#include <QAbstractItemModel>
#include <QApplication>
#include <QItemSelection>
#include <map>

extern Smoke* qt_Smoke;

// map from QT objects to QoreObject*
QtQoreMap qt_qore_map;

// set of all QWidgets without parents (= windows) that must be deleted
// before QApplication, otherwise a crash will result
QoreWidgetManager QWM;

ClassNamesMap* ClassNamesMap::m_instance = NULL;
ClassMap * ClassMap::m_instance = NULL;

// FIXME: add type information when adding these first 2 functions
static AbstractQoreNode *f_QOBJECT_connect(const QoreMethod &method, const type_vec_t &typeList, ClassMap::TypeHandler *ptr, const QoreListNode *params, ExceptionSink *xsink);
static AbstractQoreNode *QOBJECT_connect(const QoreMethod &method, const type_vec_t &typeList, ClassMap::TypeHandler *ptr, QoreObject *self, QoreSmokePrivateQObjectData *apd, const QoreListNode *params, ExceptionSink *xsink);
//static AbstractQoreNode *QOBJECT_connect_static(const QoreMethod &method, const type_vec_t &typeList, ClassMap::TypeHandler *ptr, QoreObject *self, QoreSmokePrivateQObjectData *apd, const QoreListNode *params, ExceptionSink *xsink);
static AbstractQoreNode *QOBJECT_createSignal(QoreObject *self, QoreSmokePrivateQObjectData *qo, const QoreListNode *args, ExceptionSink *xsink);
static AbstractQoreNode *QOBJECT_emit(QoreObject *self, QoreSmokePrivateQObjectData *qo, const QoreListNode *args, ExceptionSink *xsink);
static bool qt_delete_blocker(QoreObject *self, QoreSmokePrivate *data);
static AbstractQoreNode *QIMAGE_scanLine(QoreObject *self, QoreSmokePrivateData *qo, const QoreListNode *args, ExceptionSink *xsink);

QRegionTypeHelper typeHelperQRegion;
QWidgetTypeHelper typeHelperQWidget;
QColorTypeHelper  typeHelperQColor;
QVariantTypeHelper typeHelperQVariant;
QBrushTypeHelper typeHelperQBrush;
QKeySequenceTypeHelper typeHelperQKeySequence;
QValidatorTypeHelper typeHelperQValidator;
QCompleterTypeHelper typeHelperQCompleter;
QDateTypeHelper typeHelperQDate;
QDateTimeTypeHelper typeHelperQDateTime;
QTimeTypeHelper typeHelperQTime;

const QoreMethod *findUserMethod(const QoreClass *qc, const char *name) {
    const QoreMethod *m = qc->findMethod(name);
    return m && m->isUser() ? m : 0;
}

static const QoreTypeInfo *getInitType(const Smoke::Type &t, bool &valid, bool param = false, bool runtime = false);
static QoreClass *findCreateQoreClass(Smoke::Index ix);

ClassMap::ClassMap() {
   m_instance = this;

   // add class hierarchy information
   setupClassHierarchy();

   QoreClass *qc = 0;
   QByteArray mname;
   for (int i = 1; i < qt_Smoke->numMethodMaps; ++i) {
      Smoke::MethodMap &mm = qt_Smoke->methodMaps[i];
      Smoke::Class &c = qt_Smoke->classes[mm.classId];

      const char *className = c.className;

      // create new class
      if (!qc || (const Smoke::Class *)qc->getUserData() != &c) {
	 // delete all of this
	 Smoke::ModuleIndex mi = qt_Smoke->findClass(className);
	 if (!mi.smoke) {
	    printd(0, "Unknown class %s. Skipping.\n", className);
	    continue;
	 }

	 qc = findCreateQoreClass(mm.classId);	 
      }

      QByteArray cname = c.className;
      QByteArray munged = qt_Smoke->methodNames[mm.name];

      // overloads are < 0. Done in ambiguousMethodList part of this condition
      if (mm.method > 0) {
	 const Smoke::Method &m = qt_Smoke->methods[mm.method];
	 mname = qt_Smoke->methodNames[m.name];
	 
	 //if (!strcmp(qt_Smoke->methodNames[qt_Smoke->methods[mm.method].name], "quit"))
	 //printd(0, "ClassMap::ClassMap() i=%d %s::%s() %smethod=%d name=%d mname=%d cmeth=%d '%s'\n", i, cname.constData(), munged.constData(), m.flags & Smoke::mf_static ? "(static) " : "", mm.method, mm.name, m.name, m.method, mname.constData());
	 
	 MungledToTypes::iterator i = m_map[cname][mname].insert(munged, getTypeHandler(mm.method));
	 const TypeHandler *th = &i.value();

	 addMethod(qc, c, m, th);	 
      } else {
	 // turn into ambiguousMethodList index
	 int ambIx = -mm.method;
	 
	 while (qt_Smoke->ambiguousMethodList[ambIx]) {
	    Smoke::Index ambMethIx = qt_Smoke->ambiguousMethodList[ambIx];
	    const Smoke::Method &m = qt_Smoke->methods[ambMethIx];
	    mname = qt_Smoke->methodNames[m.name];
	    
	    //if (!strcmp(qt_Smoke->methodNames[qt_Smoke->methods[ambIx].name], "quit"))
	    //printd(0, "ClassMap::ClassMap() (-) i=%d %s::%s() %smethod=%d name=%d mname=%d '%s'\n", i, cname.constData(), munged.constData(), m.flags & Smoke::mf_static ? "(static) " : "", ambIx, mm.name, m.name, mname.constData());

	    // FIXME: maybe only insert in m_map if it is an actual method (and not an enum for example)
	    MungledToTypes::iterator i = m_map[cname][mname].insert(munged, getTypeHandler(ambMethIx));
	    const TypeHandler *th = &i.value();
	    
	    addMethod(qc, c, m, th);	 

	    ++ambIx;
	 }
      }
   }

   // add enum namespaces
   QoreNamespace *qtenum = new QoreNamespace("QtEnum");
   for (NameToNamespace::iterator i = nsmap.begin(), e = nsmap.end(); i != e; ++i)
      qtenum->addNamespace(i.value());

   qt_ns.addNamespace(qtenum);

   // free map memory
   nsmap.clear();

   // recheck method hierarchy
   const ClassNamesMap::m_map_t &m_map = ClassNamesMap::Instance()->getMap();
   for (ClassNamesMap::m_map_t::const_iterator i = m_map.begin(), e = m_map.end(); i != e; ++i)
      i.value()->recheckBuiltinMethodHierarchy();

   // add special qore methods to classes
   addQoreMethods();
}

void ClassMap::addQoreMethods() {
   // add methods to classes

   QoreClass *qc = const_cast<QoreClass *>(QC_QOBJECT);

   // QObject
   qc->addMethodExtended("createSignal", (q_method_t)QOBJECT_createSignal, false, QC_NO_FLAGS, QDOM_DEFAULT, nothingTypeInfo, 1, stringTypeInfo, QORE_PARAM_NO_ARG);
   qc->addMethodExtended("emit", (q_method_t)QOBJECT_emit, false, QC_NO_FLAGS, QDOM_DEFAULT, nothingTypeInfo, 1, stringTypeInfo, QORE_PARAM_NO_ARG);

   // QVariant
   qc = ClassNamesMap::Instance()->value("QVariant");
   //printd(0, "registered QVariant::toQore\n");
   qc->addMethod2("toQore", (q_method2_t)Marshalling::return_qvariant);

   // QImage
   qc = ClassNamesMap::Instance()->value("QImage");
   qc->addMethodExtended("scanLine", (q_method_t)QIMAGE_scanLine, false, QC_NO_FLAGS, QDOM_DEFAULT, binaryTypeInfo, 1, bigIntTypeInfo, QORE_PARAM_NO_ARG);

   // QItemSelection
   qc = ClassNamesMap::Instance()->value("QItemSelection");
   addListMethods<QItemSelection>(qc);
}

void ClassMap::setupClassHierarchy() {
   for (Smoke::Index ix = 1; ix < qt_Smoke->numClasses; ++ix) {
      const Smoke::Class &c = qt_Smoke->classes[ix];
      QoreClass *qc = findCreateQoreClass(ix);
      for (Smoke::Index *i = qt_Smoke->inheritanceList + c.parents; *i; ++i) {
	 QoreClass *parent = findCreateQoreClass(*i);
	 qc->addBuiltinVirtualBaseClass(parent);
      }
   }
}

void ClassMap::addMethod(QoreClass *qc, const Smoke::Class &c, const Smoke::Method &method, const TypeHandler *th) {
   const char *methodName = qt_Smoke->methodNames[method.name];

   //printd(0, "ClassMap::addMethod() %s::%s()\n", qc->getName(), methodName);

   // skip certain methods
   if ((!strcmp(qc->getName(), "QAbstractItemModel") && !strcmp(methodName, "createIndex"))
       || (!strcmp(qc->getName(), "QImage") && !strcmp(methodName, "scanLine"))) {
      //printd(0,"skipping %s::%s()\n", c.className, methodName);
      return;
   }

   if (method.flags & Smoke::mf_enum) {
      QoreNamespace *ns = getNS(qc->getName());

      // Enum methods are static, so no object is required: 0
      Smoke::StackItem arg[1]; // only one item on stack for retval
      (* c.classFn)(method.method, 0, arg);
      
      //printd(0, "adding enum constant %s::%s (%d)\n", c.className, methodName, arg[0].s_enum);
      Smoke::Type t = qt_Smoke->types[method.ret];
      
      ns->addConstant(methodName, getEnumValue(t, arg[0].s_enum));
      return;
   }

   bool isPrivate = method.flags & Smoke::mf_protected;
   
   //if (!strcmp(qc->getName(), "QAbstractTableModel"))
   //printd(0, "ClassMap::addMethod() %s::%s() qc=%p enum=%d static=%d method=%p args=%d th=%p\n", qc->getName(), methodName, qc, method.flags & Smoke::mf_enum, method.flags & Smoke::mf_static, &method, method.numArgs, th);

   if (method.flags & Smoke::mf_dtor) {
      assert(!qc->getDestructor());
      qc->setDestructor3(th, (q_destructor3_t)common_destructor);
      return;
   }

   // create return and argument type information for method signature
   type_vec_t argTypeInfo;
   argTypeInfo.reserve(method.numArgs);
   Smoke::Index *idx = qt_Smoke->argumentList + method.args;
   for (unsigned i = 0; i < method.numArgs; ++i) {
      assert(idx[i]);
      bool valid;
      const QoreTypeInfo *typeInfo = getInitType(qt_Smoke->types[idx[i]], valid, true);
      if (!valid)
	 return;
      argTypeInfo.push_back(typeInfo);
   }

   if ((method.flags & Smoke::mf_ctor)) {
      const QoreMethod *qm = qc->getConstructor();
      if (qm && qm->existsVariant(argTypeInfo)) {
	 //printd(0, "ClassMap::addMethod() skipping already-created variant %s::%s()\n", qc->getName(), methodName);
	 return;
      }

      qc->setConstructorExtendedList3(th, (q_constructor3_t)common_constructor, isPrivate, QC_NO_FLAGS, QDOM_DEFAULT, argTypeInfo);
      return;
   }
   
   bool valid;
   const QoreTypeInfo *returnTypeInfo = getInitType(qt_Smoke->types[method.ret], valid);
   if (!valid)
      return;

   if (method.flags & Smoke::mf_static) {
      // set function pointer for methods with explicit implementations
      q_static_method3_t func = (q_static_method3_t)common_static_method;
      if (qc == QC_QOBJECT && !strcmp(methodName, "connect"))
	 func = (q_static_method3_t)f_QOBJECT_connect;

      const QoreMethod *qm = qc->findLocalStaticMethod(methodName);
      if (qm && qm->existsVariant(argTypeInfo)) {
	 //printd(0, "ClassMap::addMethod() skipping already-created variant (static) %s::%s()\n", qc->getName(), methodName);
	 return;
      }
      
      qc->addStaticMethodExtendedList3(th, methodName, func, isPrivate, QC_NO_FLAGS, QDOM_DEFAULT, returnTypeInfo, argTypeInfo);
      return;
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

   const QoreMethod *qm = qc->findLocalMethod(name);
   if (qm && qm->existsVariant(argTypeInfo)) {
      //printd(0, "ClassMap::addMethod() skipping already-created variant %s::%s()\n", qc->getName(), name);
      return;
   }

   q_method3_t func = (q_method3_t)common_method;
   if (qc == QC_QOBJECT && !strcmp(methodName, "connect"))
      func = (q_method3_t)QOBJECT_connect;
   
   qc->addMethodExtendedList3(th, name, func, isPrivate, QC_NO_FLAGS, QDOM_DEFAULT, returnTypeInfo, argTypeInfo);
}

void ClassMap::registerMethod(const char *class_name, const char *method_name, const char *munged_name, Smoke::Index method_index, ClassMap::TypeHandler &type_handler, const QoreTypeInfo *returnType, const type_vec_t &argTypeList) {
//     assert(m_map[class_name][method_name][munged_name].types.count() == 0);

    method_index = qt_Smoke->methodMaps[method_index].method;
    if (method_index < 0) {
       method_index = qt_Smoke->ambiguousMethodList[-method_index];
       //assert(false);
    }

    type_handler.method = method_index;

    MungledToTypes::iterator i = m_map[class_name][method_name].insert(munged_name, type_handler);
    const TypeHandler *th = &i.value();

    // add method to class if possible
    QoreClass *qc = ClassNamesMap::Instance()->value(class_name);
    assert(qc);

    if (!strcmp(class_name, method_name)) {
       assert(!returnType);
       //printd(5, "ClassMap::registerMethod() registering constructor %s::%s() args=%d th=%p\n", class_name, method_name, argTypeList.size(), th);
       qc->setConstructorExtendedList3(th, (q_constructor3_t)common_constructor, false, QC_NO_FLAGS, QDOM_DEFAULT, argTypeList);
    }
    else {
       //printd(5, "ClassMap::registerMethod() registering function %s::%s() args=%d th=%p\n", class_name, method_name, argTypeList.size(), th);
       qc->addMethodExtendedList3(th, method_name, (q_method3_t)common_method, false, QC_NO_FLAGS, QDOM_DEFAULT, returnType, argTypeList);
    }
    //printd(0, "ClassMap::registerMethod(%s, %s, %s)\n", class_name, method_name, munged_name);
}

ClassMap::TypeHandler getTypeHandlerFromMapIndex(Smoke::Index methodIndex) {
   methodIndex = qt_Smoke->methodMaps[methodIndex].method;
   if (methodIndex < 0) {
      methodIndex = qt_Smoke->ambiguousMethodList[-methodIndex];
      //assert(false);
   }
   return getTypeHandler(methodIndex);
}

ClassMap::TypeHandler getTypeHandler(Smoke::Index methodIndex) {
   ClassMap::TypeHandler ret;
   const Smoke::Method &m = qt_Smoke->methods[methodIndex];

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

static bool qt_delete_blocker(QoreObject *self, QoreSmokePrivate *data) {
    return data->deleteBlocker(self);
}

static AbstractQoreNode *QIMAGE_scanLine(QoreObject *self, QoreSmokePrivateData *qo, const QoreListNode *args, ExceptionSink *xsink) {
   HARD_QORE_PARAM(no, const QoreBigIntNode, args, 0);
   QImage *qi = qo->getObject<QImage>();
   if (!qi)
      return 0;
   uchar *ptr = qi->scanLine(no->val);
   if (!ptr)
      return 0;
   // copy scanline data
   BinaryNode *b = new BinaryNode;
   b->append(ptr, qi->bytesPerLine());
   return b;
}

static QoreClass *findCreateQoreClass(Smoke::Index ix) {
   assert(ix > 0);
   Smoke::Class &c = qt_Smoke->classes[ix];

   QoreClass *qc = ClassNamesMap::Instance()->value(ix);
   if (qc)
      return qc;   

   const char *name = c.className;

   if (typeHelperQRegion.hasClass() && !strcmp(name, "QRegion"))
      qc = typeHelperQRegion.getClass();
   else if (typeHelperQWidget.hasClass() && !strcmp(name, "QWidget"))
      qc = typeHelperQWidget.getClass();
   else if (typeHelperQColor.hasClass() && !strcmp(name, "QColor"))
      qc = typeHelperQColor.getClass();
   else if (typeHelperQVariant.hasClass() && !strcmp(name, "QVariant"))
      qc = typeHelperQVariant.getClass();
   else if (typeHelperQBrush.hasClass() && !strcmp(name, "QBrush"))
      qc = typeHelperQBrush.getClass();
   else if (typeHelperQKeySequence.hasClass() && !strcmp(name, "QKeySequence"))
      qc = typeHelperQKeySequence.getClass();
   else if (typeHelperQValidator.hasClass() && !strcmp(name, "QValidator"))
      qc = typeHelperQValidator.getClass();
   else if (typeHelperQCompleter.hasClass() && !strcmp(name, "QCompleter"))
      qc = typeHelperQCompleter.getClass();
   else if (typeHelperQDate.hasClass() && !strcmp(name, "QDate"))
      qc = typeHelperQDate.getClass();
   else if (typeHelperQDateTime.hasClass() && !strcmp(name, "QDateTime"))
      qc = typeHelperQDateTime.getClass();
   else if (typeHelperQTime.hasClass() && !strcmp(name, "QTime"))
      qc = typeHelperQTime.getClass();
   else {
      qc = new QoreClass(name, QDOM_GUI);

      // process special classes
      if (!QC_QOBJECT && !strcmp(name, "QObject")) {
	 QC_QOBJECT = qc;
      }

      // set the delete blocker on all classes with virtual methods
      if (c.flags & Smoke::cf_virtual) {
         // we must set this immediately or child classes will not be registered as having a delete blocker
         qc->setDeleteBlocker((q_delete_blocker_t)qt_delete_blocker);
      }
   }

   //printd(0, "findCreateQoreClass() ix=%d %s qc=%p id=%d\n", ix, name, qc, qc->getID());

   qc->setUserData(&c);
   ClassNamesMap::Instance()->addItem(ix, qc);

   qt_ns.addSystemClass(qc);

   return qc;
}

// get type from parse class map or create it and add to map
static const QoreTypeInfo *getInitClassType(const char *name, const Smoke::Type &t) {
   QoreClass *qc = findCreateQoreClass(t.classId);
   return qc->getTypeInfo();
}

const QoreTypeInfo *getQtTypeInfo(const Smoke::Type &t, bool &valid) {
   return getInitType(t, valid, false, true);
}

static const QoreTypeInfo *getInitType(const Smoke::Type &t, bool &valid, bool param, bool runtime) {
   valid = true;

   //int flags = t.flags & 0x30;
   int tid = t.flags & Smoke::tf_elem;
   switch (tid) {
      case Smoke::t_bool:
	 return boolTypeInfo;
      case Smoke::t_char:
	 return stringTypeInfo;
      case Smoke::t_uchar:
      case Smoke::t_short:
      case Smoke::t_ushort:
      case Smoke::t_int:
      case Smoke::t_uint:
      case Smoke::t_long:
      case Smoke::t_ulong:
	 return param ? qtIntTypeInfo : bigIntTypeInfo;
      case Smoke::t_float:
      case Smoke::t_double:
	 return floatTypeInfo;
      case Smoke::t_enum:
	 return ClassMap::Instance()->getEnumTypeInfo(t);
      case Smoke::t_voidp:
      case Smoke::t_class:
	 break;
      default:
	 //printd(0, "getInitType() tid=%d name=%s class=%d\n", tid, t.name, t.classId);
	 assert(false);
   }

   if (!t.name)
      return 0;

   const char *f = t.name;
   if (!strncmp(f, "const ", 6))
      f += 6;

   QByteArray cname(f);
   if (cname.startsWith("QList<") || cname.startsWith("QVector<")) {
      return listTypeInfo;
   }

   // find references
   if (isptrtype(f, "bool")
       || isptrtype(f, "int")) {
      return referenceTypeInfo;
   }

   f = cname.data();
   char *p = strchrs(f, "&*");
   if (p)
      *p = '\0';

   if (tid == Smoke::t_voidp) {
      if (!strcmp(f, "QString")
	  || !strcmp(f, "QChar")
	  || isptrtype(f, "QString")
	  || isptrtype(f, "QChar")
	  || !strcmp(f, "char")
	  || !strcmp(f, "uchar")
	  || !strcmp(f, "unsigned char"))
	 return param ? qtStringTypeInfo : stringTypeInfo;

      if (!strcmp(f, "QStringList"))
	 return listTypeInfo;

      //printd(0, "getInitType() name=%s (%s) class=%d cannot handle type; returning valid = false\n", t.name, f, t.classId);
      valid = false;
      return 0;
   }

   assert(tid == Smoke::t_class);

   if (strchr(f, '<'))
      return 0;

   if (runtime) {
      QoreClass *qc = ClassNamesMap::Instance()->value(t.classId);
      return qc ? qc->getTypeInfo() : 0;
   }

   return getInitClassType(f, t);
}

static AbstractQoreNode *f_QOBJECT_connect(const QoreMethod &method, const type_vec_t &typeList, ClassMap::TypeHandler *th, const QoreListNode *params, ExceptionSink *xsink) {
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

/*
static AbstractQoreNode *QOBJECT_connect_static(const QoreMethod &method, const type_vec_t &typeList, ClassMap::TypeHandler *type_handler, QoreObject *self, QoreSmokePrivateQObjectData *apd, const QoreListNode *params, ExceptionSink *xsink) {
    // HACK: workaround for $.connect(o, sig, o, slot) call type
   return f_QOBJECT_connect(method, typeList, type_handler, params, xsink);
}
*/

static AbstractQoreNode *QOBJECT_connect(const QoreMethod &method, const type_vec_t &typeList, ClassMap::TypeHandler *type_handler, QoreObject *self, QoreSmokePrivateQObjectData *apd, const QoreListNode *params, ExceptionSink *xsink) {
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

void common_constructor(const QoreClass &myclass, 
			const type_vec_t &typeList,
			ClassMap::TypeHandler *type_handler,
			QoreObject *self,
                        const QoreListNode *params, 
			ExceptionSink *xsink) {
    const char * className = myclass.getName();
//     printd(0, className);
    CommonQoreMethod cqm(type_handler, self, 0, className, className, params, xsink);

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
       QObject *qobj = reinterpret_cast<QObject *>(qtObj);
       if (myclass.getClass(QC_QABSTRACTITEMMODEL->getID()))
	  obj = new QoreSmokePrivateQAbstractItemModelData(cqm.method().classId, qobj, self);
       else
	  obj = new QoreSmokePrivateQObjectData(cqm.method().classId, qobj, self);

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

    printd(5, "common_constructor() %s private %p (%p) cid %d objcid %d self %p\n", className, obj, qtObj, myclass.getID(), self->getClass()->getID(), self);
}

// a helper function to handle conflicting names
const char * qoreMethodName2Qt(const char * name) {
    if (!strcmp(name, "qt_copy")) return "copy";
    else if (!strcmp(name, "qt_constructor")) return "constructor";
    else if (!strcmp(name, "qt_inherits")) return "inherits";
    else return name;
}

AbstractQoreNode * common_method(const QoreMethod &method,
				 const type_vec_t &typeList,
				 ClassMap::TypeHandler *type_handler,
                                 QoreObject *self,
                                 AbstractPrivateData *apd,
                                 const QoreListNode *params,
                                 ExceptionSink *xsink) {
    const char * methodName = qoreMethodName2Qt(method.getName());
    const char * className = method.getClass()->getName();

    QoreSmokePrivate * smc = reinterpret_cast<QoreSmokePrivate*>(apd);
    if (!smc->object())
       return 0;

    CommonQoreMethod cqm(type_handler, self, smc, className, methodName, params, xsink);

    Q_ASSERT_X(smc!=0, "cast", "cannot get QoreSmokeClass from QoreObject");
#ifdef DEBUG
    if (!smc->object()) printd(0, "QT Object not availble when calling %s::%s()\n", className, methodName);
#endif
    Q_ASSERT_X(smc->object()!=0, "cast", "QoreSmokePrivate::object() returned 0");
// if (QByteArray(methodName) == "setColumnWidths") assert(0);
    return cqm.callMethod();
}


AbstractQoreNode * common_static_method(const QoreMethod &method,
					const type_vec_t &typeList,
					ClassMap::TypeHandler *type_handler,
                                        const QoreListNode *params,
                                        ExceptionSink *xsink) {
    const char * methodName = qoreMethodName2Qt(method.getName());
    const char * className = method.getClass()->getName();
    CommonQoreMethod cqm(type_handler, 0, 0, className, methodName, params, xsink);
    return cqm.callMethod();
}

void common_destructor(const QoreClass &thisclass, ClassMap::TypeHandler *type_handler, QoreObject *self, AbstractPrivateData *private_data, ExceptionSink *xsink) {
   // unconditionally deref private data on exit to avoid a memory leak
   ReferenceHolder<AbstractPrivateData> ref_helper(private_data, xsink);

   assert(dynamic_cast<QoreSmokePrivate*>(private_data));

   QoreSmokePrivate *p = reinterpret_cast<QoreSmokePrivate*>(private_data);

   void *pobj = p->object();

   printd(5, "common_destructor() %s self=%p (%s) pobj=%p qobject=%d externally_owned=%d\n", thisclass.getName(), self, self->getClassName(), pobj, p->isQObject(), p->externallyOwned());

   if (!pobj) {
      //printd(0, "common_destructor (WW) QoreSmokePrivate's Qt object does not exist anymore\n");
      return;
   }

   // if the QApplication object is being deleted, then delete all QWidget objects that still exist
   // because the QApplication destructor will free windowing resources and subsequently deleting
   // any QWidget objects will cause a crash
   if (thisclass.getID() == QC_QAPPLICATION->getID()) {
      //printd(0, "QApplication::destructor() pobj=%p private_data=%p\n", pobj, private_data);
      QWM.deleteAll();
   }

   //printd(5, "deleting object of class %s %p pobj=%p private_data=%p\n", thisclass.getName(), &thisclass, pobj, private_data);

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
   
   const char * className = qt_Smoke->classes[p->smokeClass()].className;
   QByteArray methodName("~");
   methodName += className;

   CommonQoreMethod cqm(type_handler, self, p, className, methodName.constData(), 0, xsink);

   //printd(0, "common_destructor %s::destructor() Qt: %p\n", thisclass.getName(), pobj);
   assert(cqm.isValid());
   // call the destructor -- and take the object from the private data first
   (* cqm.smokeClass().classFn)(cqm.method().method, p->takeObject(), cqm.Stack);
   //printd(0, "common_destructor %s::destructor() Qt: %p returned from smoke destructor\n", thisclass.getName(), pobj);
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

QoreSmokePrivateData::~QoreSmokePrivateData() {
   // if the object still exists here, the QoreObject is being obliterated due to an exception in the derived user constructor
   if (m_object && !externallyOwned()) {
      const Smoke::Class &cls = qt_Smoke->classes[classIndex()];
      Smoke::Index dm = ClassMap::Instance()->getDestructor(cls.className);
      // delete the object with the smoke destructor
      (* cls.classFn)(dm, m_object, 0);
   }
}
