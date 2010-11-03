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

#include <QtGlobal>
#include <QCoreApplication>

#include "qtfunctions.h"
#include "qoresmokeglobal.h"


/*! Qt4 C++ qApp global pointer wrapper for Qore. It's a function.
It is equivalent to the pointer returned by the QCoreApplication::instance()
function except that, in GUI applications, it is a pointer to a QApplication instance.
*/
static AbstractQoreNode* f_qApp(const QoreListNode *params, class ExceptionSink *xsink)
{
     QoreClass *qc;
     // TODO/FIXME: this qt_Smoke is valid. It's part of the Qt4 module only
     Smoke::Index ix = qt_Smoke->findClass("QCoreApplication").index;
     QoreObject * o = getQoreObject(ix, QCoreApplication::instance(), qc);
     if (!o) {
          xsink->raiseException("QAPP-ERROR", "Unknown Qore object");
          return 0;
     }
     o->ref();
     //printd(0, "f_qApp classId %d qoreobject %p\n", ix, o);
     return o;
}

/*! SLOT macro reimplementation
 */
static AbstractQoreNode *f_SLOT(const QoreListNode *params, class ExceptionSink *xsink)
{
     // get slot name
     const QoreStringNode *p = test_string_param(params, 0);
     if (!p || !p->strlen()) {
          xsink->raiseException("SLOT-ERROR", "missing slot name");
          return 0;
     }
     QoreStringNode *str = new QoreStringNode("1");
     str->concat(p->getBuffer());
     const char *buf = str->getBuffer();
     int slen = str->strlen();
     if (slen < 3 || buf[slen - 1] != ')')
          str->concat("()");
     return str;
}

/*! SIGNAL macro reimplementation
 */
static AbstractQoreNode *f_SIGNAL(const QoreListNode *params, ExceptionSink *xsink)
{
     // get slot name
     const QoreStringNode *p = test_string_param(params, 0);
     if (!p || !p->strlen()) {
          xsink->raiseException("SIGNAL-ERROR", "missing signal name");
          return 0;
     }
     QoreStringNode *str = new QoreStringNode("2");
     str->concat(p->getBuffer());
     const char *buf = str->getBuffer();
     int slen = str->strlen();
     if (slen < 3 || buf[slen - 1] != ')')
          str->concat("()");
     return str;
}

/*! It simulates extended qDebug() behavior. Like:
    qDebug() << anything << including << objects << etc;
 */
static QoreString* qOutput(const char * prefix, const QoreListNode *params, class ExceptionSink *xsink)
{
     QoreString * str = new QoreString(prefix);
     const AbstractQoreNode * n;
     QoreString * tmp;
     int cnt = num_params(params);

     for (int i = 0; i < cnt; ++i) {
          bool del;
          n = get_param(params, i);
          tmp = n->getAsString(del, -1, xsink);
          str->concat(tmp, xsink);
          if (cnt > (i+1))
               str->concat(" ");
          if (del)
               delete tmp;
     }
     return str;
}

/*! qDebug(arg1, ...) wrapper.
It supports "extended" qDebug() handling like (C++) qDebug() << "foo" << barObject << etc;
with syntax (Qore) qDebug("foo", barObject, etc);
A space is inserted between the items, and a newline is appended at the end.
Consult Qt4 documentation (QtGlobal) for its reference.
*/
static AbstractQoreNode *f_qDebug(const QoreListNode *params, class ExceptionSink *xsink)
{
     QoreString * str = qOutput("qDebug: ", params, xsink);
     if (*xsink)
          return 0;

     qDebug(str->getBuffer(), null);
     return 0;
}

/*! qWarning(arg1, ...) wrapper.
It behaves as qDebug().
*/
static AbstractQoreNode *f_qWarning(const QoreListNode *params, class ExceptionSink *xsink)
{
     QoreString * str = qOutput("qWarning: ", params, xsink);
     if (*xsink)
          return 0;

     qWarning(str->getBuffer(), null);
     return 0;
}

/*! qCritical(arg1, ...) wrapper.
It behaves as qDebug().
*/
static AbstractQoreNode *f_qCritical(const QoreListNode *params, class ExceptionSink *xsink)
{
     QoreString * str = qOutput("qCritical: ", params, xsink);
     if (*xsink)
          return 0;

     qCritical(str->getBuffer(), null);
     return 0;
}

/*! qFatal(string) wrapper.
It takes only one string argument to print out and forces application to abort.
*/
static AbstractQoreNode *f_qFatal(const QoreListNode *params, class ExceptionSink *xsink)
{
     QoreStringNodeHolder str(q_sprintf(params, 0, 0, xsink));
     if (*xsink)
          return 0;

     qFatal(str->getBuffer(), null);
     return 0;
}

/*! qRound(number) wrapper.
It takes any number node (bigint, float...) and rounds it. It returns 0 for unsupported
nodes (string, etc).
*/
static AbstractQoreNode *f_qRound(const QoreListNode *params, class ExceptionSink *xsink)
{
     const AbstractQoreNode *p = get_param(params, 0);
     return new QoreBigIntNode(qRound(p ? p->getAsFloat() : 0.0));
}

/*! qsrand(seed) wrapper.
It initializes random number generator qrand() with seed as argument:
qsrand(now_ms());
*/
static AbstractQoreNode *f_qsrand(const QoreListNode *params, class ExceptionSink *xsink)
{
     const AbstractQoreNode *p = get_param(params, 0);
     qsrand(p ? p->getAsInt() : 0);
     return 0;
}

/*! qrand() wrapper.
It returns a random number. See qsrand().
*/
static AbstractQoreNode *f_qrand(const QoreListNode *params, class ExceptionSink *xsink)
{
     return new QoreBigIntNode(qrand());
}

/*! qSwap(ref1, ref2) wrapper.
It takes 2 references to swap their values.
qSwap(\$a, \$b);
*/
static AbstractQoreNode *f_qSwap(const QoreListNode *params, class ExceptionSink *xsink)
{
     const ReferenceNode *r0 = test_reference_param(params, 0);
     if (!r0) {
          xsink->raiseException("QSWAP-ERROR", "first argument must be a reference");
          return 0;
     }

     const ReferenceNode *r1 = test_reference_param(params, 1);
     if (!r1) {
          xsink->raiseException("QSWAP-ERROR", "second argument must be a reference");
          return 0;
     }

     AutoVLock vl(xsink);
     ReferenceHelper ref0(r0, vl, xsink);
     if (!ref0)
          return 0;

     ReferenceHelper ref1(r1, vl, xsink);
     if (!ref1)
          return 0;

     ref0.swap(ref1);
     return 0;
}

/*! qVersion() wrapper.
It prints Qt version as string in 4.5.2 format.
See QT_VERSION and QT_VERSION_STR Qore constants.
*/
static AbstractQoreNode *f_qVersion(const QoreListNode *params, class ExceptionSink *xsink)
{
     return new QoreStringNode(qVersion());
}


void registerQtFunctions(QoreNamespace & qt_ns)
{
     builtinFunctions.add("qApp",      f_qApp, QDOM_GUI);
     builtinFunctions.add("SLOT",      f_SLOT, QDOM_GUI);
     builtinFunctions.add("SIGNAL",    f_SIGNAL, QDOM_GUI);
     builtinFunctions.add("qDebug",    f_qDebug, QDOM_GUI);
     builtinFunctions.add("qWarning",  f_qWarning, QDOM_GUI);
     builtinFunctions.add("qCritical", f_qCritical, QDOM_GUI);
     builtinFunctions.add("qFatal",    f_qFatal, QDOM_GUI);
     builtinFunctions.add("qRound",    f_qRound, QDOM_GUI);
     builtinFunctions.add("qsrand",    f_qsrand, QDOM_GUI);
     builtinFunctions.add("qrand",     f_qrand, QDOM_GUI);
     builtinFunctions.add("qSwap",     f_qSwap, QDOM_GUI);
     builtinFunctions.add("qVersion",  f_qVersion, QDOM_GUI);

     // additional constants
     qt_ns.addConstant("QT_VERSION", new QoreBigIntNode(QT_VERSION));
     qt_ns.addConstant("QT_VERSION_STR", new QoreStringNode(QT_VERSION_STR));
#ifdef Q_WS_X11
     qt_ns.addConstant("Q_WS_X11", &True);
#else
     qt_ns.addConstant("Q_WS_X11", &False);
#endif
#ifdef Q_WS_MAC
     qt_ns.addConstant("Q_WS_MAC", &True);
#else
     qt_ns.addConstant("Q_WS_MAC", &False);
#endif
#ifdef Q_WS_QWS
     qt_ns.addConstant("Q_WS_QWS", &True);
#else
     qt_ns.addConstant("Q_WS_QWS", &False);
#endif
#ifdef Q_WS_WIN
     qt_ns.addConstant("Q_WS_WIN", &True);
#else
     qt_ns.addConstant("Q_WS_WIN", &False);
#endif
}
