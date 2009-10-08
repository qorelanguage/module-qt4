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
