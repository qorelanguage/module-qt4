/* -*- indent-tabs-mode: nil -*- */
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

#include "qoresmokeglobal.h"

#include "qoresmokebinding.h"
#include "qoresmokeclass.h"
#include "commonqoremethod.h"
#include "qoreqtenumnode.h"
#include "qtfunctions.h"
#include "qoremarshalling.h"
#include "typeinfohelpers.h"

#include <QObject>
#include <QDebug>
#include <iostream>
#include <QModelIndex>
#include <QAbstractItemModel>
#include <QDesktopWidget>
#include <QTimer>
#include <QTreeWidgetItem>
#include <QListWidgetItem>

static QoreStringNode *qt_module_init();
static void qt_module_ns_init(QoreNamespace *rns, QoreNamespace *qns);
static void qt_module_delete();

DLLEXPORT char qore_module_name[] = "qt4";
DLLEXPORT char qore_module_version[] = PACKAGE_VERSION;
DLLEXPORT char qore_module_description[] = "QT4 module";
DLLEXPORT char qore_module_author[] = "Qore Technologies, s.r.o.";
DLLEXPORT char qore_module_url[] = "http://qore.org";
DLLEXPORT int qore_module_api_major = QORE_MODULE_API_MAJOR;
DLLEXPORT int qore_module_api_minor = QORE_MODULE_API_MINOR;
DLLEXPORT qore_module_init_t qore_module_init = qt_module_init;
DLLEXPORT qore_module_ns_init_t qore_module_ns_init = qt_module_ns_init;
DLLEXPORT qore_module_delete_t qore_module_delete = qt_module_delete;

// assume LGPL for qt >= 4.5
#if QT_VERSION >= 0x040500
DLLEXPORT qore_license_t qore_module_license = QL_LGPL;
#else
DLLEXPORT qore_license_t qore_module_license = QL_GPL;
#endif

static QoreQtIntCompatibleTypeInfoHelper qtIntTypeInfoHelper;
QoreQtStringCompatibleTypeInfoHelper qtStringTypeInfoHelper;

const QoreTypeInfo *qtIntTypeInfo, *qtStringTypeInfo;

const QoreClass *QC_QOBJECT = 0, *QC_QWIDGET, *QC_QABSTRACTITEMMODEL, *QC_QVARIANT,
                 *QC_QLOCALE, *QC_QBRUSH, *QC_QCOLOR, *QC_QDATE, *QC_QDATETIME, *QC_QTIME, *QC_QICON,
                 *QC_QPIXMAP, *QC_QAPPLICATION, *QC_QTREEWIDGETITEM, *QC_QLISTWIDGETITEM, *QC_QBYTEARRAY,
                 *QC_QRECT, *QC_QREGION, *QC_QCHAR, *QC_QKEYSEQUENCE, *QC_QLAYOUTITEM, *QC_QDESKTOPWIDGET,
                 *QC_QFONT;

Smoke::Index SCI_QVARIANT, SCI_QLOCALE, SCI_QICON, SCI_QRECT, SCI_QREGION, SCI_QCOLOR, SCI_QPIXMAP,
      SCI_QBRUSH, SCI_QDATE, SCI_QDATETIME, SCI_QTIME, SCI_QKEYSEQUENCE, SCI_QLAYOUTITEM,
      SCI_QDESKTOPWIDGET;

extern Smoke* qt_Smoke;

Smoke::ModuleIndex QT_METACALL_ID;

// extern QoreSmokeBinding * qt_binding;

QoreNamespace qt_ns("Qt");

// for saving virtual call state
QoreThreadLocalStorage<void> qore_qt_virtual_flag;

// for saving int &argc, char ** arguments
ArgStringSink qore_string_sink;

// argc_no = argument position for the int &argc argument (1st = 0)
static int argv_handler_intern(int argc_no, Smoke::Stack &stack, const ClassMap::TypeList &types, const QoreListNode *list, CommonQoreMethod &cqm, ExceptionSink *xsink)
{
     if (!list || list->empty()) {
          xsink->raiseException("QT-ARGUMENT-ERROR", "this version of %s::%s() requires a non-empty list for the list argument (recommended to use $ARGV)", cqm.getClassName(), cqm.getMethodName());
          return -1;
     }

     // setup string list and process list
     ArgStringList *sl = qore_string_sink.get();

//     printd(0, "argv_handler_intern() argc_no=%d list=%p (size %d)\n", argc_no, list, list->size());
     ConstListIterator li(list);
     while (li.next()) {
          QoreStringNodeValueHelper str(li.getValue());
          sl->addStr(str->getBuffer());
     }

     // note: we do not use the first entry for the argc reference
     // because it will be assigned by the persistent ArgStringList

     // assign second reference
     temp_store_s *p1 = cqm.getRefEntry(++argc_no);
     p1->assign(sl);

     // set up stack
     stack[argc_no].s_voidp = sl->getArgcPtr();
     stack[++argc_no].s_voidp = p1->getPtr();

     return 0;
}

static int argv_handler_rint_charpp(Smoke::Stack &stack, const ClassMap::TypeList &types, const QoreListNode *args, CommonQoreMethod &cqm, ExceptionSink *xsink)
{
     // Create a Smoke stack from params
     stack = new Smoke::StackItem[3];
     //printd(0, "argv_handler_rint_charpp() allocated stack of size %d\n", 3);

     bool done = false;

     for (int j = 0, e = types.count(); j < e; ++j) {
          const AbstractQoreNode *n = get_param(args, j);
          const Smoke::Type &t = types[j];

          int flags = t.flags & 0x30;

          if (!done && (flags == Smoke::tf_ref) && isptrtype(t.name, "int")) {
               //int argc = n ? n->getAsInt() : 0;

               const QoreListNode *list = test_list_param(args, j + 1);

// 	 printd(0, "argv_handler_rint_charpp() found int& argument at position %d, list is: %p (size %d)\n", j, list, list ? list->size() : 0);

               if (argv_handler_intern(j, stack, types, list, cqm, xsink))
                    return -1;

               ++j;
               done = true;
          } else {
// 	 printd(0, "argv_handler_rint_charpp() processing regular argument at position %d\n", j);
               cqm.qoreToStack(t, n, j + 1);
          }
     }
     return 0;
}

// handles int &argc, char **argv automatically from the global ARGV variable
static int argv_handler_none(Smoke::Stack &stack, const ClassMap::TypeList &types, const QoreListNode *args, CommonQoreMethod &cqm, ExceptionSink *xsink)
{
     // Create a Smoke stack from params
     stack = new Smoke::StackItem[3];
     //printd(0, "argv_handler_none() allocated stack of size %d\n", 3);

     bool found;
     ReferenceHolder<QoreListNode> l(new QoreListNode(), xsink);
     ReferenceHolder<AbstractQoreNode> argv(getProgram()->getGlobalVariableValue("ARGV", found), xsink);
     l->push(getProgram()->getScriptName());
     if (argv && argv->getType() == NT_LIST)
          l->merge(reinterpret_cast<const QoreListNode *>(*argv));

     return argv_handler_intern(0, stack, types, *l, cqm, xsink);
}

// handles int &argc, char **argv from a single list
static int argv_handler_charpp(Smoke::Stack &stack, const ClassMap::TypeList &types, const QoreListNode *args, CommonQoreMethod &cqm, ExceptionSink *xsink)
{
     // Create a Smoke stack from params
     stack = new Smoke::StackItem[3];
     //printd(0, "argv_handler_charpp() allocated stack of size %d\n", 3);

     const AbstractQoreNode *p = get_param(args, 0);
     if (!p || p->getType() != NT_LIST) {
          xsink->raiseException("QT-PARAMETER-ERROR", "this version of %s::%s() expects a list of strings (ex: $ARGV) as the sole argument (type provided: %s)", cqm.getClassName(), cqm.getMethodName(), get_type_name(p));
          return -1;
     }

     return argv_handler_intern(0, stack, types, reinterpret_cast<const QoreListNode *>(p), cqm, xsink);
}

static int createIndex_handler(Smoke::Stack &stack, const ClassMap::TypeList &types, const QoreListNode *args, CommonQoreMethod &cqm, ExceptionSink *xsink)
{
     // Create a Smoke stack from params
     stack = new Smoke::StackItem[types.size() + 1];

     const AbstractQoreNode *n = get_param(args, 0);
     cqm.qoreToStack(types[0], n, 1);
     //int row = n ? n->getAsInt() : 0;

     n = get_param(args, 1);
     cqm.qoreToStack(types[1], n, 2);
     //int column = n ? n->getAsInt() : 0;

     n = get_param(args, 2);
     const Smoke::Type &t = types[2];
     if ((t.flags & Smoke::tf_elem) != Smoke::t_voidp) {
          cqm.qoreToStack(t, n, 3);
          return 0;
     }

     stack[3].s_voidp = const_cast<AbstractQoreNode *>(n);

     if (n) {
          QoreObject *self = cqm.getQoreObject();
          assert(self);
          // get QoreSmokePrivateQAbstractItemModelData ptr
          ReferenceHolder<QoreSmokePrivateQAbstractItemModelData> data(reinterpret_cast<QoreSmokePrivateQAbstractItemModelData *>(self->getReferencedPrivateData(QC_QABSTRACTITEMMODEL->getID(), xsink)), xsink);
          if (*xsink)
               return -1;

          data->storeIndex(n, xsink);
     }

     return (*xsink) ? -1 : 0;
}

template <typename T>
static AbstractQoreNode *rv_handler_internalPointer(QoreObject *self, const Smoke::Type &t, Smoke::Stack Stack, CommonQoreMethod &cqm, ExceptionSink *xsink)
{
     if (!Stack[0].s_voidp)
          return 0;

     QoreSmokePrivate *smc = cqm.getPrivateData();
     assert(smc);
     assert(smc->object());
     T *qmi = reinterpret_cast<T *>(smc->object());
     const QAbstractItemModel *aim = qmi->model();
     if (!aim)
          return 0;

     QoreObject *o = getQoreQObject(aim);
     if (!o)
          return 0;

     PrivateDataRefHolder<QoreSmokePrivateQAbstractItemModelData> c(o, QC_QABSTRACTITEMMODEL->getID(), xsink);
     if (!c)
          return 0;

     return c->isQoreData(Stack[0].s_voidp);
}

static AbstractQoreNode *rv_handler_QWidget_setParent(QoreObject *self, const Smoke::Type &t, Smoke::Stack Stack, CommonQoreMethod &cqm, ExceptionSink *xsink)
{
     QWidget *qw = reinterpret_cast<QWidget *>(cqm.getPrivateData()->object());
     if (!qw)
          return 0;

     if (qw->parent())
          QWM.remove(qw);
     else
          QWM.add(qw);

     return 0;
}

static AbstractQoreNode *rv_handler_QAbstractItemView_reset(QoreObject *self, const Smoke::Type &t, Smoke::Stack Stack, CommonQoreMethod &cqm, ExceptionSink *xsink)
{
     QoreSmokePrivateQAbstractItemModelData *smc = reinterpret_cast<QoreSmokePrivateQAbstractItemModelData *>(cqm.getPrivateData());
     assert(smc);
     smc->purgeMap(xsink);
     return 0;
}

static int do_externally_owned(const Smoke::Type &t, CommonQoreMethod &cqm, const AbstractQoreNode *n, int index, Smoke::Stack &stack, ExceptionSink *xsink)
{
     // get QoreSmokePrivate object
     ReferenceHolder<QoreSmokePrivate> c(xsink);
     if (cqm.getObject(t.classId, n, c, index))
          return -1;

     // set object externally owned flag
     c->setExternallyOwned();

     // assign to argument stack
     stack[index + 1].s_class = c->object();
     return 0;
}

static int setExternallyOwned_handler(Smoke::Stack &stack, const ClassMap::TypeList &types, const QoreListNode *args, CommonQoreMethod &cqm, ExceptionSink *xsink)
{
     // Create a Smoke stack from params
     stack = new Smoke::StackItem[types.size() + 1];
     //printd(0, "setExternallyOwned_handler() %s::%s() allocated stack of size %d\n", cqm.getClassName(), cqm.getMethodName(), types.size() + 1);

     for (int i = 0, e = types.size(); i < e; ++i) {
          const Smoke::Type &t = types[i];
          const AbstractQoreNode *n = get_param(args, i);

          if (t.classId > 0 && !(t.flags & Smoke::tf_const) && t.flags & Smoke::tf_ptr)
               do_externally_owned(t, cqm, n, i, stack, xsink);
          else
               cqm.qoreToStack(t, n, i + 1);
     }
     return 0;
}

class mySingleShotTimer : public QObject {
private:
     int timerId;
     QoreSmokePrivateQObjectData *my_data;
     QoreObject *obj;

public:
     DLLLOCAL mySingleShotTimer() : my_data(0), obj(0) {}
     DLLLOCAL int timerInit(QoreSmokePrivateQObjectData *data, int msec, QoreSmokePrivateQObjectData *receiver_data, const QoreObject *receiver, const char *member, ExceptionSink *xsink) {
          my_data = data;
          data->createSignal("timeout()", xsink);
          assert(!*xsink);

          receiver_data->connectDynamic(data, "2timeout()", receiver, member, xsink);
          if (*xsink)
               return -1;

          timerId = startTimer(msec);
          //printd(0, "mySingleShotTimer::timerInit() msec: %d timerId: %d data: %p (QT=%p)\n", msec, timerId, my_data, my_data->object());
          return 0;
     }

     DLLLOCAL void setObject(QoreObject *n_obj) {
          obj = n_obj;
     }

protected:
     DLLLOCAL virtual void timerEvent(QTimerEvent *) {
          //printd(0, "mySingleShotTimer::timerEvent() timerId: %d data: %p (QT=%p)\n", timerId, my_data, my_data->object());

          // need to kill the timer _before_ we emit timeout() in case the
          // slot connected to timeout calls processEvents()
          if (timerId > 0)
               killTimer(timerId);
          timerId = -1;
          my_data->emitSignal("timeout()", 0, 0);

          ExceptionSink xsink;
          QoreObject *t = obj;
          t->doDelete(&xsink);
          t->deref(&xsink);
     }
};

static int arg_handler_QTimer_singleShot(Smoke::Stack &stack, const ClassMap::TypeList &types, const QoreListNode *args, CommonQoreMethod &cqm, ExceptionSink *xsink)
{
     const AbstractQoreNode *p = get_param(args, 0);
     int msec = p ? p->getAsInt() : 0;
     const QoreObject *o = test_object_param(args, 1);
     if (!o) {
          xsink->raiseException("QTIMER-SINGLESHOT-PARAM-ERROR", "expecting a QObject object as second argument to QTimer::singleShot()");
          return -1;
     }

     PrivateDataRefHolder<QoreSmokePrivateQObjectData> receiver(o, QC_QOBJECT->getID(), xsink);
     if (!receiver) {
          if (!*xsink)
               xsink->raiseException("QTIMER-SINGLESHOT-PARAM-ERROR", "expecting a QObject object as second argument to QTimer::singleShot()");
          return -1;
     }

     const QoreStringNode *pstr = test_string_param(args, 2);
     if (!pstr) {
          xsink->raiseException("QTIMER-SINGLESHOT-PARAM-ERROR", "expecting a string as third argument to QTimer::singleShot()");
          return -1;
     }
     const char *member = pstr->getBuffer();

     mySingleShotTimer *sst = new mySingleShotTimer();
     //printd(0, "arg_handler_QTimer_singleShot() sst=%p\n", sst);
     QoreSmokePrivateQObjectData *data;
     // TODO/FIXME: qt_Smoke is valid here. This is for Qt4 object.
     ReferenceHolder<QoreObject> obj(Marshalling::doQObject<QoreSmokePrivateQObjectData>(qt_Smoke, sst, xsink, &data), xsink);
     if (*xsink)
          return -1;

     if (sst->timerInit(data, msec, *receiver, o, member, xsink))
          return -1;

     sst->setObject(obj.release());
     cqm.suppressMethod();
     return 0;
}

static int arg_handler_QShortcut(Smoke::Stack &stack, const ClassMap::TypeList &types, const QoreListNode *args, CommonQoreMethod &cqm, ExceptionSink *xsink)
{
     // Create a Smoke stack from params
     stack = new Smoke::StackItem[types.size() + 1];
//    printd(0, "QShortcut_handler() allocated stack of size %d (rv handler=%p)\n", types.size() + 1, cqm.getTypeHandler().return_value_handler);

     const AbstractQoreNode *w = 0;
     for (int i = 0, e = types.size(); i < e; ++i) {
          const Smoke::Type &t = types[i];
          const AbstractQoreNode *n = get_param(args, i);

          // get Smoke type ID
//         int tid = t.flags & Smoke::tf_elem;

          // save parent class for afterwards
          if (i == 1)
               w = n;

          // save shortcut targets for setting up connections after object is created
          if (i == 2 && n && n->getType() == NT_STRING) {
               cqm.saveParam("member", n);
               n = 0;
          }
          if (i == 3 && n && n->getType() == NT_STRING) {
               cqm.saveParam("ambiguousMember", n);
               n = 0;
          }

          cqm.qoreToStack(t, n, i + 1);
     }
     if (cqm.getParams())
          cqm.saveParam("parent", w);

     return 0;
}

static AbstractQoreNode *rv_handler_QTreeWidgetItem(QoreObject *self, const Smoke::Type &t, Smoke::Stack Stack, CommonQoreMethod &cqm, ExceptionSink *xsink)
{
     ReferenceHolder<QoreSmokePrivateData> pd(reinterpret_cast<QoreSmokePrivateData *>(self->getReferencedPrivateData(QC_QTREEWIDGETITEM->getID(), xsink)), xsink);
     if (pd) {
          QTreeWidgetItem *qtwi = pd->getObject<QTreeWidgetItem>();
          if (qtwi->parent() || qtwi->treeWidget())
               pd->setExternallyOwned();
     }

     return 0;
}

static AbstractQoreNode *rv_handler_QListWidgetItem(QoreObject *self, const Smoke::Type &t, Smoke::Stack Stack, CommonQoreMethod &cqm, ExceptionSink *xsink)
{
     ReferenceHolder<QoreSmokePrivateData> pd(reinterpret_cast<QoreSmokePrivateData *>(self->getReferencedPrivateData(QC_QLISTWIDGETITEM->getID(), xsink)), xsink);
     if (pd) {
          QListWidgetItem *qlwi = pd->getObject<QListWidgetItem>();
          if (qlwi->listWidget())
               pd->setExternallyOwned();
     }

     return 0;
}

static AbstractQoreNode *rv_handler_QShortcut(QoreObject *self, const Smoke::Type &t, Smoke::Stack Stack, CommonQoreMethod &cqm, ExceptionSink *xsink)
{
     ReferenceHolder<QoreSmokePrivateQObjectData> shortcut(reinterpret_cast<QoreSmokePrivateQObjectData *>(self->getReferencedPrivateData(QC_QOBJECT->getID(), xsink)), xsink);
     if (*xsink)
          return 0;

     const QoreHashNode *p = cqm.getParams();
     if (shortcut && p) {
          // get the widget object
          const AbstractQoreNode *w = p->getKeyValue("parent");
          assert(w);
          assert(w->getType() == NT_OBJECT);
          const QoreObject *parent = reinterpret_cast<const QoreObject *>(w);

          ReferenceHolder<QoreSmokePrivateQObjectData> po(reinterpret_cast<QoreSmokePrivateQObjectData *>(parent->getReferencedPrivateData(QC_QOBJECT->getID(), xsink)), xsink);
          if (*xsink)
               return 0;

          // connect shortcut signals to dynamic slots in parent
          const AbstractQoreNode *m = p->getKeyValue("member");
          // this is actually a QoreStringNode
          if (m) {
               po->connectDynamic(*shortcut, "2activated()", parent, reinterpret_cast<const QoreStringNode *>(m)->getBuffer(), xsink);
          }
          m = p->getKeyValue("ambiguousMember");
          // this is actually a QoreStringNode
          if (m) {
               po->connectDynamic(*shortcut, "2activatedAmbiguously()", parent, reinterpret_cast<const QoreStringNode *>(m)->getBuffer(), xsink);
          }

//       printd(0, "rv_handler_QShortcut() self=%p shortcut=%p qt=%p parent=%p (%s) qtpar=%p\n", self, *shortcut, shortcut->object(), parent, parent->getClassName(), shortcut->qobject()->parent());
     }
     return 0;
}

static int arg_handler_QPixmapCache_find(Smoke::Stack &stack, const ClassMap::TypeList &types, const QoreListNode *args, CommonQoreMethod &cqm, ExceptionSink *xsink)
{
     stack = new Smoke::StackItem[3];

     const AbstractQoreNode *n = get_param(args, 0);
     cqm.qoreToStack(types[0], n, 1);

     const ReferenceNode *r = HARD_QORE_REF(args, 1);
     temp_store_s *rs = cqm.getRefEntry(1);
     rs->ref = r;
     assert(rs);
     if (rs->type != temp_store_s::r_qpixmap) {
          rs->del();
          rs->assign(new QPixmap);
     }
     stack[2].s_class = rs->getPtr();

     return 0;
}

static AbstractQoreNode *rv_handler_spacer_item(QoreObject *self, const Smoke::Type &t, Smoke::Stack Stack, CommonQoreMethod &cqm, ExceptionSink *xsink)
{
     return self ? self->refSelf() : 0;
}

static void setClassInfo(const QoreClass *&qc, const char *name)
{
     qc = ClassNamesMap::Instance()->value(name);
     assert(qc);
}

static void setClassInfo(const QoreClass *&qc, Smoke::Index &sci, const char *name)
{
     Smoke::ModuleIndex sc = qt_Smoke->findClass(name);
     assert(sc.smoke);
     sci = sc.index;
     qc = ClassNamesMap::Instance()->value(name);
     assert(qc);
}

static QoreStringNode *qt_module_init()
{
//     printd(0, "Qt module init\n");

     if (!qt_Smoke) {
          init_qt_Smoke();
     } else {
          return 0;
     }

     // register all classes and methods
     ClassMap::init(qt_Smoke);

     // register qt "addons"
     registerQtFunctions(qt_ns);

     // setup the binding
     QoreSmokeBinding::Instance(qt_Smoke);

     // set global variables to class information
     setClassInfo(QC_QWIDGET, "QWidget");
     setClassInfo(QC_QABSTRACTITEMMODEL, "QAbstractItemModel");
     setClassInfo(QC_QVARIANT, SCI_QVARIANT, "QVariant");
     setClassInfo(QC_QLOCALE, SCI_QLOCALE, "QLocale");
     setClassInfo(QC_QCOLOR, SCI_QCOLOR, "QColor");
     setClassInfo(QC_QBRUSH, SCI_QBRUSH, "QBrush");
     setClassInfo(QC_QDATE, SCI_QDATE, "QDate");
     setClassInfo(QC_QDATETIME, SCI_QDATETIME, "QDateTime");
     setClassInfo(QC_QTIME, SCI_QTIME, "QTime");
     setClassInfo(QC_QICON, SCI_QICON, "QIcon");
     setClassInfo(QC_QPIXMAP, SCI_QPIXMAP, "QPixmap");
     setClassInfo(QC_QAPPLICATION, "QApplication");
     setClassInfo(QC_QTREEWIDGETITEM, "QTreeWidgetItem");
     setClassInfo(QC_QLISTWIDGETITEM, "QListWidgetItem");
     setClassInfo(QC_QBYTEARRAY, "QByteArray");
     setClassInfo(QC_QRECT, SCI_QRECT, "QRect");
     setClassInfo(QC_QREGION, SCI_QREGION, "QRegion");
     setClassInfo(QC_QCHAR, "QChar");
     setClassInfo(QC_QKEYSEQUENCE, SCI_QKEYSEQUENCE, "QKeySequence");
     setClassInfo(QC_QLAYOUTITEM, SCI_QLAYOUTITEM, "QLayoutItem");
     setClassInfo(QC_QDESKTOPWIDGET, SCI_QDESKTOPWIDGET, "QDesktopWidget");
     setClassInfo(QC_QFONT, "QFont");

     // add alternate method argument handlers
     ClassMap &cm = *(ClassMap::Instance());
     ClassMap::TypeHandler argv_charpp_h(qt_Smoke);
     Smoke::Type charpp_t;
     charpp_t.name = "char**";
     charpp_t.classId = 0;
     charpp_t.flags = 0x20;
     argv_charpp_h.types.append(charpp_t);
     argv_charpp_h.arg_handler = argv_handler_charpp;

     ClassMap::TypeHandler argv_none_h(qt_Smoke);
     argv_none_h.arg_handler = argv_handler_none;

     Smoke::ModuleIndex methodIndex;
     methodIndex = qt_Smoke->findMethod("QCoreApplication", "QCoreApplication$?");
     assert(methodIndex.smoke);

     type_vec_t argv_list;
     argv_list.push_back(listTypeInfo);

     // register new methods for the qore module
     cm.registerMethod("QCoreApplication", "QCoreApplication", "QCoreApplication?", methodIndex.index, argv_charpp_h, 0, argv_list);
     cm.registerMethod("QCoreApplication", "QCoreApplication", "QCoreApplication", methodIndex.index, argv_none_h);

     // add argument handlers to existing methods
     cm.addArgHandler("QCoreApplication", "QCoreApplication", argv_handler_rint_charpp);
     cm.addArgHandler("QApplication", "QApplication", argv_handler_rint_charpp);

     methodIndex = qt_Smoke->findMethod("QApplication", "QApplication$?");
     assert(methodIndex.smoke);

     // register new methods for the qore module
     cm.registerMethod("QApplication", "QApplication", "QApplication?", methodIndex.index, argv_charpp_h, 0, argv_list);
     cm.registerMethod("QApplication", "QApplication", "QApplication", methodIndex.index, argv_none_h);

     // QModelIndex and QPersistentModelIndex
     methodIndex = qt_Smoke->findMethod("QModelIndex", "internalPointer");
     assert(methodIndex.smoke);

     ClassMap::TypeHandler internalPointer = getTypeHandlerFromMapIndex(methodIndex.index);
     internalPointer.return_value_handler = rv_handler_internalPointer<QModelIndex>;

     // QModelIndex and QPersistentModelIndex return value handlers
     cm.registerMethod("QModelIndex", "internalPointer", "internalPointer", methodIndex.index, internalPointer);

     methodIndex = qt_Smoke->findMethod("QPersistentModelIndex", "internalPointer");
     assert(methodIndex.smoke);

     internalPointer = getTypeHandlerFromMapIndex(methodIndex.index);
     internalPointer.return_value_handler = rv_handler_internalPointer<QPersistentModelIndex>;
     cm.registerMethod("QPersistentModelIndex", "internalPointer", "internalPointer", methodIndex.index, internalPointer);

     // QAbstractItemModel handlers
     methodIndex = qt_Smoke->findMethod("QAbstractItemModel", "createIndex$$$");
     assert(methodIndex.smoke);

     ClassMap::TypeHandler createIndexHandler = getTypeHandlerFromMapIndex(methodIndex.index);
     assert(createIndexHandler.types.size() == 3);
     Smoke::Type voidp_t;
     voidp_t.name = "void*";
     voidp_t.classId = 0;
     voidp_t.flags = Smoke::t_voidp|Smoke::tf_ptr;
     createIndexHandler.types[2] = voidp_t;

     createIndexHandler.arg_handler = createIndex_handler;

     type_vec_t argv_int_int_any;
     argv_int_int_any.push_back(bigIntTypeInfo);
     argv_int_int_any.push_back(bigIntTypeInfo);
     argv_int_int_any.push_back(anyTypeInfo);

     cm.registerMethod("QAbstractItemModel", "createIndex", "createIndex$$$", methodIndex.index, createIndexHandler, 0, argv_int_int_any);

     // QLayout::addItem() and ::addWidget() handlers
     // add argument handlers to existing methods
     cm.addArgHandler("QLayout", "addItem", setExternallyOwned_handler);
     cm.addArgHandler("QGridLayout", "addItem", setExternallyOwned_handler);
     cm.addArgHandler("QFormLayout", "addItem", setExternallyOwned_handler);
     cm.addArgHandler("QFormLayout", "setItem", setExternallyOwned_handler);
     cm.addArgHandler("QGraphicsGridLayout", "addItem", setExternallyOwned_handler);
     cm.addArgHandler("QGraphicsLinearLayout", "addItem", setExternallyOwned_handler);
     cm.addArgHandler("QGraphicsLinearLayout", "insertItem", setExternallyOwned_handler);
     cm.addArgHandler("QGraphicsScene", "addItem", setExternallyOwned_handler);
     cm.addArgHandler("QGraphicsItemAnimation", "setItem", setExternallyOwned_handler);
     cm.addArgHandler("QListWidget", "addItem", setExternallyOwned_handler);
     cm.addArgHandler("QListWidget", "insertItem", setExternallyOwned_handler);
     cm.addArgHandler("QStackedLayout", "addItem", setExternallyOwned_handler);
     cm.addArgHandler("QBoxLayout", "addItem", setExternallyOwned_handler);
     cm.addArgHandler("QBoxLayout", "addSpacerItem", setExternallyOwned_handler);
     cm.addArgHandler("QBoxLayout", "insertItem", setExternallyOwned_handler);
     cm.addArgHandler("QBoxLayout", "insertSpacerItem", setExternallyOwned_handler);

     cm.addArgHandler("QLayout", "addWidget", setExternallyOwned_handler);
     cm.addArgHandler("QBoxLayout", "addWidget", setExternallyOwned_handler);
     cm.addArgHandler("QGridLayout", "addWidget", setExternallyOwned_handler);
     cm.addArgHandler("QStackedLayout", "addWidget", setExternallyOwned_handler);
     cm.addArgHandler("QSplitter", "addWidget", setExternallyOwned_handler);
     cm.addArgHandler("QWidgetItem", "QWidgetItem", setExternallyOwned_handler);
     cm.addArgHandler("QTreeWidget", "addTopLevelItem", setExternallyOwned_handler);
     cm.addArgHandler("QTreeWidget", "insertTopLevelItem", setExternallyOwned_handler);
     cm.addArgHandler("QTableWidget", "setItem", setExternallyOwned_handler);
     cm.addArgHandler("QStandardItemModel", "setItem", setExternallyOwned_handler);
     cm.addArgHandler("QWidget", "setLayout", setExternallyOwned_handler);
     cm.addArgHandler("QGraphicsWidget", "setLayout", setExternallyOwned_handler);
     cm.addArgHandler("QApplication", "desktop", setExternallyOwned_handler);

     // QShortcut handlers
     cm.addArgHandler("QShortcut", "QShortcut", arg_handler_QShortcut);
     cm.setRVHandler("QShortcut", "QShortcut", rv_handler_QShortcut);

     // QAbstractItemView return value handlers
     cm.setRVHandler("QAbstractItemView", "reset", rv_handler_QAbstractItemView_reset);
     cm.setRVHandler("QListView", "reset", rv_handler_QAbstractItemView_reset);
     cm.setRVHandler("QHeaderView", "reset", rv_handler_QAbstractItemView_reset);
     cm.setRVHandler("QTreeView", "reset", rv_handler_QAbstractItemView_reset);

     // add return value handlers
     cm.setRVHandler("QLayoutItem", "spacerItem", "spacerItem", rv_handler_spacer_item);
     cm.setRVHandler("QWidget", "setParent", rv_handler_QWidget_setParent);
     cm.setRVHandler("QTreeWidgetItem", "QTreeWidgetItem", rv_handler_QTreeWidgetItem);
     cm.setRVHandler("QListWidgetItem", "QListWidgetItem", rv_handler_QListWidgetItem);

     cm.addArgHandler("QTimer", "singleShot", arg_handler_QTimer_singleShot);

     // QPixmapCache::find(string, reference) method
     methodIndex = qt_Smoke->findMethod("QPixmapCache", "find$#");
     assert(methodIndex.smoke);

     ClassMap::TypeHandler findHandler = getTypeHandlerFromMapIndex(methodIndex.index);
     assert(findHandler.types.size() == 2);

     findHandler.arg_handler = arg_handler_QPixmapCache_find;

     type_vec_t argv_str_ref;
     argv_str_ref.push_back(stringTypeInfo);
     argv_str_ref.push_back(referenceTypeInfo);

     cm.registerMethod("QPixmapCache", "find", "find$#", methodIndex.index, findHandler, boolTypeInfo, argv_str_ref, true);
     //cm.addArgHandler("QPixmapCache", "find", arg_handler_QPixmapCache_find);

     // initialize global constants
     QT_METACALL_ID = qt_Smoke->idMethodName("qt_metacall$$?");
     assert(QT_METACALL_ID.smoke);

#ifdef DEBUG
     // TODO/FIXME: remove it after testing!
     //ClassMap::Instance()->printMapToFile("maps.txt");
#endif

     return 0;
}

static void qt_module_ns_init(QoreNamespace *rns, QoreNamespace *qns)
{
//     printd(0, "Qt namespace init\n");
     qns->addInitialNamespace(qt_ns.copy());
}

static void qt_module_delete()
{
     ClassNamesMap::del();
     ClassMap::del();
//     printd(0, "Qt module delete\n");
     delete qt_Smoke;
}
