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
class QoreSmokePrivate;

namespace Marshalling {

template <typename T>
QoreObject *doQObject(void *origObj, ExceptionSink *xsink, T **p = 0);

class QtContainerToQore {
public:

    DLLLOCAL static QtContainerToQore* Instance() {
        if (!m_instance) {
            m_instance = new QtContainerToQore();
        }
        return m_instance;
    }

    DLLLOCAL AbstractQoreNode * marshall(const Smoke::Type &t, void* ptr, bool delete_temp, ExceptionSink *xsink);

private:
    typedef AbstractQoreNode* (*qlist_handler_t)(const Smoke::Type &t, void* ptr, bool delete_temp, ExceptionSink *xsink);

    DLLLOCAL static QByteArray getSubType(const char * name);

    template<class QLISTT, class QORET>
       DLLLOCAL static AbstractQoreNode * listToSimpleValue(const Smoke::Type &t, void* ptr, bool delete_temp, ExceptionSink *xsink);

    DLLLOCAL static AbstractQoreNode * listToQStringList(const Smoke::Type &t, void* ptr, bool delete_temp, ExceptionSink *xsink);
    DLLLOCAL static AbstractQoreNode * listToQByteArray(const Smoke::Type &t, void* ptr, bool delete_temp, ExceptionSink *xsink);

    template<class QLISTT>
       DLLLOCAL static AbstractQoreNode * listToEnum(const Smoke::Type &t, void* ptr, bool delete_temp, ExceptionSink *xsink);

    template<class QLISTT>
       DLLLOCAL static AbstractQoreNode * listToQObject(const Smoke::Type &t, void* ptr, bool delete_temp, ExceptionSink *xsink);

    //! QList<Foo>
    template<class QLISTT>
       DLLLOCAL static AbstractQoreNode * listToObject(const Smoke::Type &t, void* ptr, bool delete_temp, ExceptionSink *xsink);

    //! QList<Foo*>
    template<class QLISTT>
       DLLLOCAL static AbstractQoreNode * listToObjectPtr(const Smoke::Type &t, void* ptr, bool delete_temp, ExceptionSink *xsink);

    QMap<QByteArray,qlist_handler_t> m_map;
    static QtContainerToQore * m_instance;

    DLLLOCAL QtContainerToQore();
    DLLLOCAL QtContainerToQore(const QtContainerToQore &);
    //DLLLOCAL QtContainerToQore& operator=(const QtContainerToQore&) {};
    DLLLOCAL ~QtContainerToQore() {
        delete m_instance;
    }
};


class QoreQListBase {
public:
    bool isValid;
    DLLLOCAL QoreQListBase() {
        isValid = false;
    }
    DLLLOCAL virtual ~QoreQListBase() {}
    DLLLOCAL virtual void * voidp() = 0;
};

template<class QLIST>
class QoreQList : public QoreQListBase {
public:
    QLIST qlist;

    DLLLOCAL QoreQList() : QoreQListBase() {};

    DLLLOCAL void * voidp() {
        return &qlist;
    }
};


class QoreToQtContainer {
public:

    DLLLOCAL static QoreToQtContainer* Instance() {
        if (!m_instance) {
            m_instance = new QoreToQtContainer();
        }
        return m_instance;
    }

    DLLLOCAL QoreQListBase * marshall(const Smoke::Type &t, const AbstractQoreNode * ptr, ExceptionSink *xsink);

private:
    typedef QoreQListBase* (*qlist_handler_t)(const Smoke::Type &t, const AbstractQoreNode * ptr, ExceptionSink *xsink);

    DLLLOCAL static QByteArray getSubType(const char * name);

    template<class QLISTT>
       DLLLOCAL static QoreQListBase * listToSimpleValue(const Smoke::Type &t, const AbstractQoreNode * ptr, ExceptionSink *xsink);

    DLLLOCAL static QoreQListBase * listToQStringList(const Smoke::Type &t, const AbstractQoreNode * ptr, ExceptionSink *xsink);

    template<class QLISTT, class SUBTYPET>
       DLLLOCAL static QoreQListBase * listToEnum(const Smoke::Type &t, const AbstractQoreNode * ptr, ExceptionSink *xsink);

    template<class QLISTT, class SUBTYPET>
       DLLLOCAL static QoreQListBase * listToObject(const Smoke::Type &t, const AbstractQoreNode * ptr, ExceptionSink *xsink);

    template<class QLISTT, class SUBTYPET>
       DLLLOCAL static QoreQListBase * listToQObject(const Smoke::Type &t, const AbstractQoreNode * ptr, ExceptionSink *xsink);

    QMap<QByteArray,qlist_handler_t> m_map;
    DLLLOCAL static QoreToQtContainer * m_instance;

    DLLLOCAL QoreToQtContainer();
    DLLLOCAL QoreToQtContainer(const QoreToQtContainer &);
    //DLLLOCAL QoreToQtContainer& operator=(const QoreToQtContainer&) {};
    DLLLOCAL ~QoreToQtContainer() {
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
    DLLLOCAL QoreQVariant() {
        status = Valid;
    }
    DLLLOCAL virtual ~QoreQVariant() {}
    DLLLOCAL virtual void * s_class() {
        return &qvariant;
    }
};

//! Convert Qore to QVaraint
DLLLOCAL QoreObject *qoreToQVariantObject(const AbstractQoreNode *node, ExceptionSink *xsink);
DLLLOCAL bool canConvertToQVariant(qore_type_t t);

DLLLOCAL QoreQVariant * qoreToQVariant(const Smoke::Type & t, const AbstractQoreNode * node, ExceptionSink * xsink);

/*! Helper function for CommonQoreMethod::getScore().
It's much more efficient than when there was used qoreToQVariant
*/
DLLLOCAL QoreQVariant::Status qoreToQVariantScore(const Smoke::Type & t, const AbstractQoreNode * node, ExceptionSink * xsink);

DLLLOCAL AbstractQoreNode * stackToQore(const Smoke::Type &t, Smoke::StackItem &i, ExceptionSink *xsink);

/*! Create a copy of given non-qobject based object.
Using copy constructor. */
DLLLOCAL void * constructCopy(void * obj, const char * className,
			      ExceptionSink *xsink);

/*! Lookup for the most specific class for given ptr.
Example: if somebody calls QWidget::event(QEvent *e),
this function will try to setup the real event object for
Qore (for example QHelpEvent) automatically.
*/
DLLLOCAL Smoke::Index resolveQtClass(void * ptr, Smoke::Index classId);

/*! Additional method for QVariant. QVariant::toQore().
It performs a direct conversion from QVariant to native
Qore node. */
DLLLOCAL AbstractQoreNode *return_qvariant(const QoreMethod &method,
					   QoreObject *self,
					   AbstractPrivateData *apd,
					   const QoreListNode *params,
					   ExceptionSink *xsink);

DLLLOCAL QoreObject *createQoreObjectFromNonQObject(const QoreClass *qc, Smoke::Index classId, void *ptr, QoreSmokePrivate **p = 0);
DLLLOCAL QoreObject *createQoreObjectFromNonQObjectExternallyOwned(const QoreClass *qc, Smoke::Index classId, void *ptr, QoreSmokePrivate **p = 0);

}

#endif
