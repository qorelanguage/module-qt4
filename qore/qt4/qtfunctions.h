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

#ifndef QTFUNCTIONS_H
#define QTFUNCTIONS_H

#include <qore/Qore.h>


/*! Qt4 C++ qApp global pointer wrapper for Qore. It's a function.
It is equivalent to the pointer returned by the QCoreApplication::instance()
function except that, in GUI applications, it is a pointer to a QApplication instance.
*/
static AbstractQoreNode* f_qApp(const QoreListNode *params, class ExceptionSink *xsink);

/*! SLOT macro reimplementation
 */
static AbstractQoreNode *f_SLOT(const QoreListNode *params, class ExceptionSink *xsink);
/*! SIGNAL macro reimplementation
 */
static AbstractQoreNode *f_SIGNAL(const QoreListNode *params, ExceptionSink *xsink);


/*! qDebug(arg1, ...) wrapper.
It supports "extended" qDebug() handling like (C++) qDebug() << "foo" << barObject << etc;
with syntax (Qore) qDebug("foo", barObject, etc);
A space is inserted between the items, and a newline is appended at the end.
Consult Qt4 documentation (QtGlobal) for its reference.
*/
static AbstractQoreNode *f_qDebug(const QoreListNode *params, class ExceptionSink *xsink);
/*! qWarning(arg1, ...) wrapper.
It behaves as qDebug().
*/
static AbstractQoreNode *f_qWarning(const QoreListNode *params, class ExceptionSink *xsink);
/*! qCritical(arg1, ...) wrapper.
It behaves as qDebug().
*/
static AbstractQoreNode *f_qCritical(const QoreListNode *params, class ExceptionSink *xsink);
/*! qFatal(string) wrapper.
It takes only one string argument to print out and forces application to abort.
*/
static AbstractQoreNode *f_qFatal(const QoreListNode *params, class ExceptionSink *xsink);

/*! qRound(number) wrapper.
It takes any number node (bigint, float...) and rounds it. It returns 0 for unsupported
nodes (string, etc).
*/
static AbstractQoreNode *f_qRound(const QoreListNode *params, class ExceptionSink *xsink);
/*! qsrand(seed) wrapper.
It initializes random number generator qrand() with seed as argument:
qsrand(now_ms());
*/
static AbstractQoreNode *f_qsrand(const QoreListNode *params, class ExceptionSink *xsink);
/*! qrand() wrapper.
It returns a random number. See qsrand().
*/
static AbstractQoreNode *f_qrand(const QoreListNode *params, class ExceptionSink *xsink);
/*! qSwap(ref1, ref2) wrapper.
It takes 2 references to swap their values.
qSwap(\$a, \$b);
*/
static AbstractQoreNode *f_qSwap(const QoreListNode *params, class ExceptionSink *xsink);
/*! qVersion() wrapper.
It prints Qt version as string in 4.5.2 format.
See QT_VERSION and QT_VERSION_STR Qore constants.
*/
static AbstractQoreNode *f_qVersion(const QoreListNode *params, class ExceptionSink *xsink);

/*! Register additional functions and constants.
More constants:
QT_VERSION - number with Qt version. E.g. 0x040100.
QT_VERSION_STR - string with Qt version. See qVersion().
Q_WS_X11 - True on X11 environment - UNIXes, except Mac OS X.
Q_WS_MAC - True on Mac OS X
Q_WS_QWS - True on Qt Embedded Linux.
Q_WS_WIN - True on MS Windows.
*/
void registerQtFunctions(QoreNamespace & qt_ns);

#endif
