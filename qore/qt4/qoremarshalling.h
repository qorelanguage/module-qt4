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

#ifndef QOREMARSHALLING_H
#define QOREMARSHALLING_H

#include <smoke.h>
#include <qore/Qore.h>
#include <QVariant>

class QByteArray;


namespace Marshalling {

template <typename T>
QoreObject *doQObject(void *origObj, ExceptionSink *xsink, T **p = 0);

class QtContainerToQore {
public:

    static QtContainerToQore* Instance() {
        if (!m_instance) {
            m_instance = new QtContainerToQore();
        }
        return m_instance;
    }

    AbstractQoreNode * marshall(const Smoke::Type &t, void* ptr, ExceptionSink *xsink);

private:
    typedef AbstractQoreNode* (*qlist_handler_t)(const Smoke::Type &t, void* ptr, ExceptionSink *xsink);

    static QByteArray getSubType(const char * name);

    template<class QLISTT, class QORET>
    static AbstractQoreNode * listToSimpleValue(const Smoke::Type &t, void* ptr, ExceptionSink *xsink);

    static AbstractQoreNode * listToQStringList(const Smoke::Type &t, void* ptr, ExceptionSink *xsink);
    static AbstractQoreNode * listToQByteArray(const Smoke::Type &t, void* ptr, ExceptionSink *xsink);

    template<class QLISTT>
    static AbstractQoreNode * listToEnum(const Smoke::Type &t, void* ptr, ExceptionSink *xsink);

    template<class QLISTT>
    static AbstractQoreNode * listToQObject(const Smoke::Type &t, void* ptr, ExceptionSink *xsink);

    template<class QLISTT>
    static AbstractQoreNode * listToObject(const Smoke::Type &t, void* ptr, ExceptionSink *xsink);

    QMap<QByteArray,qlist_handler_t> m_map;
    static QtContainerToQore * m_instance;

    QtContainerToQore();
    QtContainerToQore(const QtContainerToQore &);
    //QtContainerToQore& operator=(const QtContainerToQore&) {};
    ~QtContainerToQore() {
        delete m_instance;
    }
};


class QoreQListBase {
public:
    bool isValid;
    QoreQListBase() {
        isValid = false;
    }
    virtual ~QoreQListBase() {}
    virtual void * voidp() = 0;
};

template<class QLIST>
class QoreQList : public QoreQListBase {
public:
    QLIST qlist;

    QoreQList() : QoreQListBase() {};

    void * voidp() {
        return &qlist;
    }
};


class QoreToQtContainer {
public:

    static QoreToQtContainer* Instance() {
        if (!m_instance) {
            m_instance = new QoreToQtContainer();
        }
        return m_instance;
    }

    QoreQListBase * marshall(const Smoke::Type &t, const AbstractQoreNode * ptr, ExceptionSink *xsink);

private:
    typedef QoreQListBase* (*qlist_handler_t)(const Smoke::Type &t, const AbstractQoreNode * ptr, ExceptionSink *xsink);

    static QByteArray getSubType(const char * name);

    template<class QLISTT>
    static QoreQListBase * listToSimpleValue(const Smoke::Type &t, const AbstractQoreNode * ptr, ExceptionSink *xsink);

    static QoreQListBase * listToQStringList(const Smoke::Type &t, const AbstractQoreNode * ptr, ExceptionSink *xsink);

    template<class QLISTT, class SUBTYPET>
    static QoreQListBase * listToEnum(const Smoke::Type &t, const AbstractQoreNode * ptr, ExceptionSink *xsink);

    template<class QLISTT, class SUBTYPET>
    static QoreQListBase * listToObject(const Smoke::Type &t, const AbstractQoreNode * ptr, ExceptionSink *xsink);

    template<class QLISTT, class SUBTYPET>
    static QoreQListBase * listToQObject(const Smoke::Type &t, const AbstractQoreNode * ptr, ExceptionSink *xsink);

    QMap<QByteArray,qlist_handler_t> m_map;
    static QoreToQtContainer * m_instance;

    QoreToQtContainer();
    QoreToQtContainer(const QoreToQtContainer &);
    //QoreToQtContainer& operator=(const QoreToQtContainer&) {};
    ~QoreToQtContainer() {
        delete m_instance;
    }
};


class QoreQVariant {
public:
    // Don't create more statuses! It's used for CommonQoreMethod::getScore()
    enum Status {
        Invalid = 0,
        Valid = 1,
        RealQVariant = 2
    };
    Status status;
    QVariant qvariant;
    QoreQVariant() {
        status = Valid;
    }
    virtual ~QoreQVariant() {}
    virtual void * s_class() {
        return &qvariant;
    }
};


QoreQVariant * qoreToQVariant(const Smoke::Type & t, const AbstractQoreNode * node, ExceptionSink * xsink);

AbstractQoreNode * stackToQore(const Smoke::Type &t, Smoke::StackItem &i, ExceptionSink *xsink);

/*! Create a copy of given non-qobject based object.
Using copy constructor. */
void * constructCopy(void * obj, const char * className,
                     ExceptionSink *xsink);

/*! Lookup for the most specific class for given ptr.
Example: if somebody calls QWidget::event(QEvent *e),
this function will try to setup the real event object for
Qore (for example QHelpEvent) automatically.
*/
Smoke::Index resolveQtClass(void * ptr, Smoke::Index classId);

}

#endif
