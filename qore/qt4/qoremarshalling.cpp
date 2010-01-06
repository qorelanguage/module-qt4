/*
  qoremarshalling.cpp

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
#include "qoreqtenumnode.h"

#include <QByteArray>
#include <QVariant>
#include <QNetworkAddressEntry>
#include <QSslCipher>
#include <QSslError>
#include <QSslCertificate>
#include <QModelIndex>
#include <QFileInfo>
#include <QNetworkInterface>
#include <QPrinterInfo>
#include <QUrl>
#include <QKeySequence>
#include <QTextFrame>
#include <QMdiSubWindow>
#include <QGraphicsWidget>
#include <QDockWidget>
#include <QTableWidgetSelectionRange>
#include <QNetworkCookie>
#include <QGraphicsView>
#include <QUndoStack>
#include <QAbstractButton>
#include <QNetworkProxy>
#include <QFontDatabase>
#include <QWizard>
#include <QLayoutItem>
#include <QIcon>
// #include <>

#include "qoresmokeclass.h"
#include "qoremarshalling.h"
#include "qoresmokebinding.h"
#include "qoreqtenumnode.h"

namespace Marshalling {

QtContainerToQore * QtContainerToQore::m_instance = 0;
QoreToQtContainer * QoreToQtContainer::m_instance = 0;

AbstractQoreNode * QtContainerToQore::marshall(const Smoke::Type &t, void* ptr, ExceptionSink *xsink) {
    QByteArray tname(t.name);
    if (!m_map.contains(tname)) {
        return xsink->raiseException("QLIST-MARSHALL-QT", "calling unknown marshaller Qt-Qore %s", t.name);
    }
    return (*m_map[tname])(t, ptr, xsink);
}

QByteArray QtContainerToQore::getSubType(const char * name) {
    QByteArray tname(name);
    int sep = tname.indexOf("<") + 1;
    // remove < and >
    QByteArray tt = tname.mid(sep, tname.length()-sep-1).replace("*", "");
    return tt;
}

template<class QLISTT, class QORET>
AbstractQoreNode * QtContainerToQore::listToSimpleValue(const Smoke::Type &t, void* ptr, ExceptionSink *xsink) {
//     printd(0, "QtContainerToQore::listToSimpleValue %s\n", t.name);
    ReferenceHolder<QoreListNode> retList(new QoreListNode(), xsink);
    QLISTT* l = static_cast<QLISTT*>(ptr);
    for (int i = 0; i < l->count(); ++i)
        retList->push(new QORET(l->at(i)));
    return retList.release();
}

AbstractQoreNode * QtContainerToQore::listToQStringList(const Smoke::Type &t, void* ptr, ExceptionSink *xsink) {
//     printd(0, "QtContainerToQore::listToSimpleValue %s\n", t.name);
    ReferenceHolder<QoreListNode> retList(new QoreListNode(), xsink);
    QStringList* l = static_cast<QStringList*>(ptr);
    for (int i = 0; i < l->count(); ++i)
        retList->push(new QoreStringNode(l->at(i).toUtf8().constData(), QCS_UTF8));
    return retList.release();
}

AbstractQoreNode * QtContainerToQore::listToQByteArray(const Smoke::Type &t, void* ptr, ExceptionSink *xsink) {
//     printd(0, "QtContainerToQore::listToSimpleValue %s\n", t.name);
    ReferenceHolder<QoreListNode> retList(new QoreListNode(), xsink);
    QList<QByteArray>* l = static_cast<QList<QByteArray>*>(ptr);
    for (int i = 0; i < l->count(); ++i)
        retList->push(new QoreStringNode(l->at(i).constData(), QCS_UTF8));
    return retList.release();
}

template<class QLISTT>
AbstractQoreNode * QtContainerToQore::listToEnum(const Smoke::Type &t, void* ptr, ExceptionSink *xsink) {
    ReferenceHolder<QoreListNode> retList(new QoreListNode(), xsink);
    QLISTT* l = static_cast<QLISTT*>(ptr);
    for (int i = 0; i < l->count(); ++i)
        retList->push(new QoreQtEnumNode(l->at(i), t));
    return retList.release();
}

template<class QLISTT>
AbstractQoreNode * QtContainerToQore::listToQObject(const Smoke::Type &t, void* ptr, ExceptionSink *xsink) {
    ReferenceHolder<QoreListNode> retList(new QoreListNode(), xsink);
    QLISTT* l = static_cast<QLISTT*>(ptr);
    QByteArray tt(getSubType(t.name));

    QoreClass *qc = ClassNamesMap::Instance()->value(tt.constData());
    if (!qc) {
        return xsink->raiseException("QLIST-MARSHALL-QT", "Unknown QoreClass for QList< ??? > for %s", t.name);
    }
    if (!qc->getClass(QC_QOBJECT->getID())) {
        return xsink->raiseException("QLIST-MARSHALL-QT", "QList< ??? > argument %s is not QObject based!", t.name);
    }
    Smoke::ModuleIndex sc = qt_Smoke->findClass(tt.constData());
    if (!sc.smoke) {
        return xsink->raiseException("QLIST-MARSHALL-QT", "Unknown Qt4 Smoke class: QList< ??? > for %s", t.name);
    }

    for (int i = 0; i < l->count(); ++i) {
        QObject * qtObj = reinterpret_cast<QObject*>(l->at(i));
        QoreObject *o = getQoreObject(sc.index, qtObj, qc);
        if (o) {
            o->ref();
        } else {
            o = new QoreObject(qc, getProgram());
//             QObject * qtObj = reinterpret_cast<QObject*>(origObj);
            QoreSmokePrivate * p = new QoreSmokePrivateQObjectData(sc.index, qtObj);
            QoreQtVirtualFlagHelper vfh;
            qtObj->setProperty(QORESMOKEPROPERTY, reinterpret_cast<qulonglong>(o));
            o->setPrivate(qc->getID(), p);
        }
        retList->push(o);
    }

    return retList.release();
}

QoreObject *createQoreObjectFromNonQObject(const QoreClass *theclass, Smoke::Index classId, void *ptr, QoreSmokePrivate **p = 0) {
    // ensure this is not a QObject
    assert(!theclass->getClass(QC_QOBJECT->getID()));
    QoreObject *o = new QoreObject(theclass, getProgram());
    QoreSmokePrivateData *data = new QoreSmokePrivateData(classId, ptr, o);
    o->setPrivate(theclass->getID(), data);
    if (p)
        *p = data;
    return o;
}

template<class QLISTT>
AbstractQoreNode * QtContainerToQore::listToObject(const Smoke::Type &t, void* ptr, ExceptionSink *xsink) {
    ReferenceHolder<QoreListNode> retList(new QoreListNode(), xsink);
    QLISTT* l = static_cast<QLISTT*>(ptr);
    QByteArray tt(getSubType(t.name));

    QoreClass *qc = ClassNamesMap::Instance()->value(tt.constData());
    if (!qc) {
        return xsink->raiseException("QLIST-MARSHALL-QT", "Unknown QoreClass for QList< ??? > for %s", t.name);
    }
    if (qc->getClass(QC_QOBJECT->getID())) {
        return xsink->raiseException("QLIST-MARSHALL-QT", "QList< ??? > argument %s is QObject based", t.name);
    }

    Smoke::ModuleIndex cls = qt_Smoke->findClass(tt.constData());
    if (!cls.smoke) {
        return xsink->raiseException("QLIST-MARSHALL-QT", "Class %s cannot be found in library map", tt.constData());
    }

    bool has_virtual = qt_Smoke->classes[cls.index].flags & Smoke::cf_virtual;
    for (int i = 0; i < l->count(); ++i) {
        void *qto = (void *)&l->at(i);
        QoreObject *o = has_virtual ? getQoreMappedObject(qto) : 0;
        if (o) {
            o->ref();
        } else {
            qto = Marshalling::constructCopy(qto, tt.constData(), xsink);
            if (*xsink)
                return 0;

            o = createQoreObjectFromNonQObject(qc, cls.index, qto);
        }
        retList->push(o);
    }

    return retList.release();
}


//     template<class QMAPT, class QORETKEY, class QORETVAL>
//     static AbstractQoreNode * qmapToQore(const Smoke::Type &t, void* ptr, ExceptionSink *xsink) {
//         ReferenceHolder<QoreHashNode> retList(new QoreHashNode(), xsink);
//         QMAPT* l = static_cast<QMAPT*>(ptr);
//         for (int i = 0; i < l->count(); ++i)
//             retList->push(new QORET(l->at(i)));
//         return retList.release();
//     }

QtContainerToQore::QtContainerToQore() {
    m_map["QStringList"] = &listToQStringList;
    // lists
    m_map["QList<int>"] = &listToSimpleValue<QList<int>, QoreBigIntNode>;
    m_map["QList<qreal>"] = &listToSimpleValue<QList<qreal>, QoreFloatNode>;
//     TODO/FIXME: m_map["QList<QTextOption::Tab> ???
    m_map["QList<QPrinterInfo>"] = &listToObject<QList<QPrinterInfo> >;
    m_map["QList<QWidget*>"] = &listToQObject<QList<QWidget*> >;
    m_map["QList<QNetworkAddressEntry>"] = &listToObject<QList<QNetworkAddressEntry> >;
    m_map["QList<QSslCipher>"] = &listToObject<QList<QSslCipher> >;
    m_map["QList<QFontDatabase::WritingSystem>"] = &listToEnum<QList<QFontDatabase::WritingSystem> >;
    m_map["QList<QSslError>"] = &listToObject<QList<QSslError> >;
    m_map["QList<QSslCertificate>"] = &listToObject<QList<QSslCertificate> >;
    m_map["QList<QModelIndex>"] = &listToObject<QList<QModelIndex> >;
    m_map["QList<QFileInfo>"] = &listToObject<QList<QFileInfo> >;
//     m_map["QList<QPair<qreal,qreal> > hellhound
//     m_map["QList<QTableWidgetItem*> ptr to o
//     m_map["QList<QPrinter::PageSize> enum?
    m_map["QList<QTextFrame*>"] = &listToQObject<QList<QTextFrame*> >;
    m_map["QList<QUrl>"] = &listToObject<QList<QUrl> >;
    m_map["QList<QMdiSubWindow*>"] = &listToQObject<QList<QMdiSubWindow*> >;
    m_map["QList<QNetworkInterface>"] = &listToObject<QList<QNetworkInterface> >;
//     m_map["QList<QStandardItem*> ptr to o
    m_map["QList<QLocale::Country>"] = &listToEnum<QList<QLocale::Country> >;
//     m_map["QList<QGraphicsItem*> ptr to o
    m_map["QList<QGraphicsWidget*>"] = &listToQObject<QList<QGraphicsWidget*> >;
    m_map["QList<QAction*>"] = &listToQObject<QList<QAction*> >;
    m_map["QList<QKeySequence>"] = &listToObject<QList<QKeySequence> >;
//     m_map["QList<QPair<qreal,QPointF> > hellhound
    m_map["QList<QDockWidget*>"] = &listToQObject<QList<QDockWidget*> >;
    m_map["QList<QVariant>"] = &listToObject<QList<QVariant> >;
//     m_map["QList<QTreeWidgetItem*> ptr to o
//     m_map["QList<QTextEdit::ExtraSelection> ??? like QTextOption::Tab
    m_map["QList<QObject*>"] = &listToQObject<QList<QObject*> >;
//     m_map["QList<QSize> ??? new standalone class
//     m_map["QList<QPair<int,int> > hellhound
    m_map["QList<QByteArray>"] = &listToQByteArray;
//     m_map["QList<QImageTextKeyLang>
//     m_map["QList<QListWidgetItem*>
    m_map["QList<QTextBlock>"] = &listToObject<QList<QTextBlock> >;
    m_map["QList<QPolygonF>"] = &listToObject<QList<QPolygonF> >;
//     m_map["QList<QPair<QByteArray,QByteArray> >
//     m_map["QList<QTextLayout::FormatRange>
    m_map["QList<QTableWidgetSelectionRange>"] = &listToObject<QList<QTableWidgetSelectionRange> >;
//     m_map["QList<QPair<QString,QString> >
    m_map["QList<QNetworkCookie>"] = &listToObject<QList<QNetworkCookie> >;
    m_map["QList<QGraphicsView*>"] = &listToQObject<QList<QGraphicsView*> >;
    m_map["QList<QUndoStack*>"] = &listToQObject<QList<QUndoStack*> >;
    m_map["QList<QNetworkProxy>"] = &listToObject<QList<QNetworkProxy> >;
    m_map["QList<QAbstractButton*>"] = &listToQObject<QList<QAbstractButton*> >;
    m_map["QList<QHostAddress>"] = &listToObject<QList<QHostAddress> >;
    // vectors
    m_map["QVector<QRect>"] = &listToObject<QVector<QRect> >;
    m_map["QVector<unsigned int>"] = &listToSimpleValue<QVector<unsigned int>, QoreBigIntNode>;
//     m_map["QVector<QVariant>&
//     m_map["QVector<QPair<qreal,QColor> >
//     m_map["QVector<QTextLength>
    m_map["QVector<qreal>"] = &listToSimpleValue<QVector<qreal>, QoreFloatNode>;
    m_map["QVector<double>"] = &listToSimpleValue<QVector<double>, QoreFloatNode>;
    m_map["QVector<QTextFormat>"] = &listToObject<QVector<QTextFormat> >;
    // maps
//     m_map["QMap<int,QVariant>"] = &qmapToQore<QMap<int,QVariant>, QoreBigIntNode, QoreObjectNode>;
}



QoreQListBase * QoreToQtContainer::marshall(const Smoke::Type &t, const AbstractQoreNode * ptr, ExceptionSink *xsink) {
    QByteArray tname(t.name);
    tname = tname.replace("&", "").replace("const ", "");

    if (!m_map.contains(tname)) {
        xsink->raiseException("QLIST-MARSHALL-QORE", "calling unknown marshaller Qore-Qt: %s", tname.constData());
        return 0;
    }
    return (*m_map[tname])(t, ptr, xsink);
}

QByteArray QoreToQtContainer::getSubType(const char * name) {
    QByteArray tname(name);
    int sep = tname.indexOf("<") + 1;
    // remove < and >
    QByteArray tt = tname.mid(sep, tname.length()-sep-1).replace("*", "").replace("&", "").replace(">", "");
    return tt;
}

template<class QLISTT>
QoreQListBase * QoreToQtContainer::listToSimpleValue(const Smoke::Type &t, const AbstractQoreNode * ptr, ExceptionSink *xsink) {
//     printd(0, "QoreToQtContainer::listToSimpleValue start %d\n", ptr->getType());
    QoreQList<QLISTT> * ret = new QoreQList<QLISTT>();

    switch (ptr->getType()) {
    case NT_LIST: {
        const QoreListNode * ln = reinterpret_cast<const QoreListNode*>(ptr);
        for (uint i = 0; i < ln->size(); ++i) {
            // HACK: getAsFloat is used for all numbers
            ret->qlist.append(ln->retrieve_entry(i)->getAsFloat());
        }
        break;
    }
    case NT_HASH:
        xsink->raiseException("QLIST-MARSHALL-QORE", "Cannot convert Qore hash to list of: %s", t.name);
        return 0;
    default:
        ret->qlist.append(ptr->getAsInt());
    }

    return ret;
}

QoreQListBase * QoreToQtContainer::listToQStringList(const Smoke::Type &t, const AbstractQoreNode * ptr, ExceptionSink *xsink) {
    QoreQList<QStringList> * ret = new QoreQList<QStringList>();

    switch (ptr->getType()) {
    case NT_LIST: {
        const QoreListNode * ln = reinterpret_cast<const QoreListNode*>(ptr);
        for (uint i = 0; i < ln->size(); ++i) {
            QoreStringValueHelper str(ln->retrieve_entry(i));
            ret->qlist.append(str->getBuffer());
        }
        break;
    }
    case NT_HASH:
        xsink->raiseException("QLIST-MARSHALL-QORE", "Cannot convert Qore hash to Qt QStringList: %s", t.name);
        return 0;
    default:
        QoreStringValueHelper str(ptr);
        ret->qlist.append(str->getBuffer());
    }

    return ret;
}

template<class QLISTT, class SUBTYPET>
QoreQListBase * QoreToQtContainer::listToEnum(const Smoke::Type &t, const AbstractQoreNode * ptr, ExceptionSink *xsink) {
    QoreQList<QLISTT> * ret = new QoreQList<QLISTT>();

    switch (ptr->getType()) {
    case NT_LIST: {
        const QoreListNode * ln = reinterpret_cast<const QoreListNode*>(ptr);
        for (uint i = 0; i < ln->size(); ++i) {
            ret->qlist.append((SUBTYPET)ln->retrieve_entry(i)->getAsInt());
        }
        break;
    }
    case NT_HASH:
        xsink->raiseException("QLIST-MARSHALL-QORE", "Cannot convert Qore hash to Qt QList: %s", t.name);
        return 0;
    default:
        ret->qlist.append((SUBTYPET)ptr->getAsInt());
    }

    return ret;
}

template<class QLISTT, class SUBTYPET>
QoreQListBase * QoreToQtContainer::listToObject(const Smoke::Type &t, const AbstractQoreNode * ptr, ExceptionSink *xsink) {
    QoreQList<QLISTT> * ret = new QoreQList<QLISTT>();
    QByteArray tt(getSubType(t.name));

    QoreClass * qc = ClassNamesMap::Instance()->value(tt);
    if (!qc) {
        xsink->raiseException("QLIST-MARSHALL-QORE", "Cannot find class %s", tt.constData());
        return 0;
    }

    switch (ptr->getType()) {
    case NT_LIST: {
        const QoreListNode * ln = reinterpret_cast<const QoreListNode*>(ptr);
        for (uint i = 0; i < ln->size(); ++i) {
            const QoreObject * qo = reinterpret_cast<const QoreObject*>(ln->retrieve_entry(i));
            if (!qo) {
                xsink->raiseException("QLIST-MARSHALL-QORE", "List members must be objects of: %s", tt.constData());
                return 0;
            }
            QoreSmokePrivateData * pd = dynamic_cast<QoreSmokePrivateData*>(qo->getReferencedPrivateData(qc->getID(), xsink));
            if (!pd) {
                xsink->raiseException("QLIST-MARSHALL-QORE", "Cannot get private data from %s", tt.constData());
                return 0;
            }
            ret->qlist.append(*reinterpret_cast<SUBTYPET*>(constructCopy(pd->object(), tt.constData(), xsink)));
        }
        break;
    }
    // TODO/FIXME: remove code duplications here
    case NT_OBJECT: {
        const QoreObject * qo = reinterpret_cast<const QoreObject*>(ptr);
        QoreSmokePrivateData * pd = dynamic_cast<QoreSmokePrivateData*>(qo->getReferencedPrivateData(qc->getID(), xsink));
        if (!pd) {
            xsink->raiseException("QLIST-MARSHALL-QORE", "Cannot get private data from %s", tt.constData());
            return 0;
        }
        ret->qlist.append(*reinterpret_cast<SUBTYPET*>(constructCopy(pd->object(), tt.constData(), xsink)));
        break;
    }
    default: {
        xsink->raiseException("QLIST-MARSHALL-QORE", "Cannot convert type %s to Qt QList: %s", ptr->getType(), t.name);
        return 0;
    }
    }

    return ret;
}

template<class QLISTT, class SUBTYPET>
QoreQListBase * QoreToQtContainer::listToQObject(const Smoke::Type &t, const AbstractQoreNode * ptr, ExceptionSink *xsink) {
    QoreQList<QLISTT> * ret = new QoreQList<QLISTT>();
    QByteArray tt(getSubType(t.name));

    QoreClass * qc = ClassNamesMap::Instance()->value(tt);
    if (!qc) {
        xsink->raiseException("QLIST-MARSHALL-QORE", "Cannot find class %s", tt.constData());
        return 0;
    }

    switch (ptr->getType()) {
    case NT_LIST: {
        const QoreListNode * ln = reinterpret_cast<const QoreListNode*>(ptr);
        for (uint i = 0; i < ln->size(); ++i) {
            const QoreObject * qo = reinterpret_cast<const QoreObject*>(ln->retrieve_entry(i));
            if (!qo) {
                xsink->raiseException("QLIST-MARSHALL-QORE", "List members must be objects of: %s", tt.constData());
                return 0;
            }
            QoreSmokePrivateQObjectData * pd = dynamic_cast<QoreSmokePrivateQObjectData*>(qo->getReferencedPrivateData(qc->getID(), xsink));
            if (!pd) {
                xsink->raiseException("QLIST-MARSHALL-QORE", "Cannot get private data from %s", tt.constData());
                return 0;
            }
            ret->qlist.append(reinterpret_cast<SUBTYPET*>(pd->object()));
        }
        break;
    }
    // TODO/FIXME: remove code duplications here
    case NT_OBJECT: {
        const QoreObject * qo = reinterpret_cast<const QoreObject*>(ptr);
        QoreSmokePrivateQObjectData * pd = dynamic_cast<QoreSmokePrivateQObjectData*>(qo->getReferencedPrivateData(qc->getID(), xsink));
        if (!pd) {
            xsink->raiseException("QLIST-MARSHALL-QORE", "Cannot get private data from %s", tt.constData());
            return 0;
        }
        ret->qlist.append(reinterpret_cast<SUBTYPET*>(pd->object()));
        break;
    }
    default: {
        xsink->raiseException("QLIST-MARSHALL-QORE", "Cannot convert type %s to Qt QList: %s", ptr->getType(), t.name);
        return 0;
    }
    }

    return ret;
}

QoreToQtContainer::QoreToQtContainer() {
    m_map["QStringList"] = &listToQStringList;
    // lists
    m_map["QList<QModelIndex>"] = &listToObject<QList<QModelIndex>, QModelIndex >;
    m_map["QList<QWidget*>"] = &listToQObject<QList<QWidget*>, QWidget >;
    m_map["QList<QKeySequence>"] = &listToObject<QList<QKeySequence>, QKeySequence >;
    m_map["QList<int>"] = &listToSimpleValue<QList<int> >;
    m_map["QList<QVariant>"] = &listToObject<QList<QVariant>, QVariant >;
    m_map["QList<QUrl>"] = &listToObject<QList<QUrl>, QUrl >;
    m_map["QList<QRectF>"] = &listToObject<QList<QRectF>, QRectF >;
//     m_map["QList<QGraphicsItem*>"] =
    m_map["QList<QHostAddress>"] = &listToObject<QList<QHostAddress>, QHostAddress >;
    m_map["QList<QSslError>"] = &listToObject<QList<QSslError>, QSslError >;
//     m_map["QList<QListWidgetItem*>"] =
    m_map["QList<QNetworkCookie>"] = &listToObject<QList<QNetworkCookie>, QNetworkCookie >;
    m_map["QList<QSslCertificate>"] = &listToObject<QList<QSslCertificate>, QSslCertificate >;
    m_map["QList<QSslCipher>"] = &listToObject<QList<QSslCipher>, QSslCipher >;
//     m_map["QList<QStandardItem*>"] =
//     m_map["QList<QTableWidgetItem>"] =
//     m_map["QList<QTreeWidgetItem*>"] =
    m_map["QList<QWizard::WizardButton"] = &listToEnum<QList<QWizard::WizardButton>, QWizard::WizardButton >;
    // vectors
    m_map["QVector<unsigned int>"] = &listToSimpleValue<QVector<unsigned int> >;
    m_map["QVector<QLine>"] = &listToObject<QVector<QLine>, QLine >;
    m_map["QVector<QLineF>"] = &listToObject<QVector<QLineF>, QLineF >;
    m_map["QVector<QPointF>"] = &listToObject<QVector<QPointF>, QPointF >;
    m_map["QVector<QPoint>"] = &listToObject<QVector<QPoint>, QPoint >;
    m_map["QVector<QRect>"] = &listToObject<QVector<QRect>, QRect >;
    m_map["QVector<QRectF>"] = &listToObject<QVector<QRectF>, QRectF >;
    m_map["QVector<qreal>"] = &listToSimpleValue<QVector<qreal> >;
    m_map["QVector<double>"] = &listToSimpleValue<QVector<double> >;
//     m_map["QVector<QTextLength>
}


QoreQVariant *qoreToQVariant(const Smoke::Type & t, const AbstractQoreNode * node, ExceptionSink * xsink) {
//     printd(0, "Marshalling::qoreToQVariant %s %d\n", t.name, node ? node->getType() : 0);
    std::auto_ptr<QoreQVariant> ret(new QoreQVariant());

    // FIXME: implement all conversions
    if (node == 0 || node->getType() == 0) { // NOTHING
        ret->qvariant = QVariant();
        return ret.release();
    }

    if (node->getType() == NT_QTENUM) {
        ret->qvariant = QVariant(node->getAsInt());
        return ret.release();
    }

    switch (node->getType()) {
    case NT_INT:
        ret->qvariant = QVariant(node->getAsInt());
        break;
    case NT_FLOAT:
        ret->qvariant = QVariant(node->getAsFloat());
        break;
    case NT_STRING: {
        ret->qvariant = QVariant(reinterpret_cast<const QoreStringNode*>(node)->getBuffer());
        break;
    }
//         case NT_DATE : TODO/FIXME implement date marshalling in all places
    case NT_BOOLEAN:
        ret->qvariant = QVariant(node->getAsBool());
        break;
    case NT_BINARY: {
        const BinaryNode *b = reinterpret_cast<const BinaryNode *>(node);
        ret->qvariant = QVariant( QByteArray((const char *) b->getPtr(), b->size()) );
        break;
    }
    case NT_OBJECT: {
        const QoreObject *obj = reinterpret_cast<const QoreObject *>(node);
        ReferenceHolder<QoreSmokePrivateData> p(xsink);

        // check for QLocale
        p = reinterpret_cast<QoreSmokePrivateData*>(obj->getReferencedPrivateData(QC_QLOCALE->getID(), xsink));
        if (*xsink) {
            ret->status = QoreQVariant::Invalid;
            return 0;
        }
        if (p) {
            // only call this once because it's a virtual call (slow)
            void *o = p->object();
            ret->qvariant = o ? QVariant(*(reinterpret_cast<QLocale *>(o))) : QVariant();
        } else {
	    // check for QIcon
            p = reinterpret_cast<QoreSmokePrivateData*>(obj->getReferencedPrivateData(QC_QICON->getID(), xsink));
	    if (*xsink) {
	       ret->status = QoreQVariant::Invalid;
	       return 0;
	    }
	    if (p) {
	       // only call this once because it's a virtual call (slow)
	       void *o = p->object();
	       ret->qvariant = o ? QVariant(*(reinterpret_cast<QIcon *>(o))) : QVariant();
	       
	       // note: when adding checks for other classes -> QVariant, must add in qoreToQVariant() as well
	    }
	    else {
	       // check for QVariant
	       p = reinterpret_cast<QoreSmokePrivateData*>(obj->getReferencedPrivateData(QC_QVARIANT->getID(), xsink));
	       if (*xsink) {
		  ret->status = QoreQVariant::Invalid;
		  return 0;
	       }
	       ret->qvariant = p && p->object() ? QVariant( *(QVariant*)(p->object()) ) : QVariant();
	    }
	}
        ret->status = QoreQVariant::RealQVariant;
        break;
    }
    default:
        xsink->raiseException("QVARIANT-MARSHALL", "Cannot convert type %s (Qore Type %d)", t.name, node->getType());
        ret->status = QoreQVariant::Invalid;
    } // switch

    return ret.release();
}

QoreQVariant::Status qoreToQVariantScore(const Smoke::Type & t, const AbstractQoreNode * node, ExceptionSink * xsink) {
    if (node->getType() == NT_QTENUM) {
        return QoreQVariant::Valid;
    }

    switch (node->getType()) {
    case 0: // NOTHING
    case NT_INT:
    case NT_FLOAT:
    case NT_STRING:
    case NT_BOOLEAN:
        return QoreQVariant::Valid;
        break;
    case NT_OBJECT: {
        const QoreObject *obj = reinterpret_cast<const QoreObject *>(node);
        
	if (obj->getClass(QC_QVARIANT->getID()))
	   return QoreQVariant::RealQVariant;

	if (obj->getClass(QC_QLOCALE->getID()))
	   return QoreQVariant::Valid;
	
	if (obj->getClass(QC_QICON->getID()))
	   return QoreQVariant::Valid;
	
	// note: when adding checks for other classes -> QVariant, must add in qoreToQVariant() as well

        return QoreQVariant::Invalid;
        break;
    }
    default:
        return QoreQVariant::Invalid;
    } // switch
    return QoreQVariant::Invalid;
}

template <typename T>
QoreObject *doQObject(void *origObj, ExceptionSink *xsink, T **p = 0) {
    QObject* qtObj = reinterpret_cast<QObject *>(origObj);
    // get real object's class depending on QObject::metaObject
    // it's a must for e.g. sender() call or for objects that
    // are deleted in Qore, but kept in Qt (parenting etc.)
    const QMetaObject *meta;
    {
        QoreQtVirtualFlagHelper vfh;
        meta = qtObj->metaObject();
    }

    const char * cname;
    const QoreClass *qc;
    while (true) {
        cname = meta->className();
        qc = ClassNamesMap::Instance()->value(cname);
        if (qc) {
            printd(0, "doQObject<>(%p) found Qore class %s\n", qtObj, cname);
            break;
        }
        printd(0, "doQObject<>(%p) cannot find Qore class %s, checking parent class\n", qtObj, cname);
        meta = meta->superClass();
        assert(meta);
    }
    Smoke::ModuleIndex cid = qt_Smoke->findClass(cname);
    // New qoreobject for really required Qt class/object
    ReferenceHolder<QoreObject> qto(new QoreObject(qc, getProgram()), xsink);
    assert(cid.smoke);

    T *data = new T(cid.index, qtObj);
    // QObject based obj
    {
        QoreQtVirtualFlagHelper vfh;
        qtObj->setProperty(QORESMOKEPROPERTY, reinterpret_cast<qulonglong>(*qto));
    }
    qto->setPrivate(qc->getID(), data);
    if (p)
        *p = data;
    return qto.release();
}

AbstractQoreNode *return_qvariant(const QoreMethod &method,
                                 QoreObject *self,
                                 AbstractPrivateData *apd,
                                 const QoreListNode *params,
                                 ExceptionSink *xsink)
{
    QoreSmokePrivateData * pd = reinterpret_cast<QoreSmokePrivateData*>(apd);
    assert(pd);
    QVariant qv = *reinterpret_cast<QVariant*>(pd->object());
    //printd(0, "return_qvariant() type=%d\n", qv.type());
    switch (qv.type()) {
    case QVariant::Invalid:
        return nothing();
    case QVariant::Bool:
        return get_bool_node(qv.toBool());
    case QVariant::Double:
        return new QoreFloatNode(qv.toDouble());
    case QVariant::Int:
        return new QoreBigIntNode(qv.toInt());
    case QVariant::LongLong:
        return new QoreBigIntNode(qv.toLongLong());
    case QVariant::String:
        return new QoreStringNode(qv.toString().toUtf8().data(), QCS_UTF8);
    case QVariant::UInt:
        return new QoreBigIntNode(qv.toUInt());
    case QVariant::ULongLong: // WARNING: precision lost here
        return new QoreBigIntNode((int64)qv.toULongLong());
    case QVariant::Locale:
        return createQoreObjectFromNonQObject(QC_QLOCALE, SCI_QLOCALE, new QLocale(qv.toLocale()));
    case QVariant::Icon:
        return createQoreObjectFromNonQObject(QC_QICON, SCI_QICON, new QIcon(qVariantValue<QIcon>(qv)));

        // FIXME: implement all conversions
    case QVariant::Char:
    case QVariant::Date:
    case QVariant::DateTime:
    case QVariant::Line:
    case QVariant::LineF:
    case QVariant::Point:
    case QVariant::PointF:
    case QVariant::Rect:
    case QVariant::RectF:
    case QVariant::RegExp:
    case QVariant::Size:
    case QVariant::Url:
    case QVariant::Time:
    case QVariant::Map:
    case QVariant::List:
    case QVariant::StringList:
    case QVariant::ByteArray:
    case QVariant::BitArray:
    case QVariant::SizeF:
    case QVariant::Hash:
    case QVariant::Font:
    case QVariant::Pixmap:
    case QVariant::Brush:
    case QVariant::Color:
    case QVariant::Palette:
    case QVariant::Image:
    case QVariant::Polygon:
    case QVariant::Region:
    case QVariant::Bitmap:
    case QVariant::Cursor:
    case QVariant::SizePolicy:
    case QVariant::KeySequence:
    case QVariant::Pen:
    case QVariant::TextLength:
    case QVariant::TextFormat:
    case QVariant::Matrix:
    case QVariant::Transform:
    case QVariant::UserType:
    case QVariant::LastType:
        printd(0, "Missing QVariant implementation\n");
        break;
    }

    return createQoreObjectFromNonQObject(QC_QVARIANT, SCI_QVARIANT, new QVariant(qv));
}

AbstractQoreNode * stackToQore(const Smoke::Type &t, Smoke::StackItem &i, ExceptionSink *xsink) {
//     printd(0, "Marshalling::stackToQore() type: %s, %d\n", t.name, t.flags & Smoke::tf_elem);

    int tid = t.flags & Smoke::tf_elem;
    int flags = t.flags & Smoke::tf_ref;
    bool iconst = t.flags & Smoke::tf_const;

    if (!t.name) {
        return 0;
    }

    switch (tid) {
    case Smoke::t_bool:
        return get_bool_node(i.s_bool);
    case Smoke::t_char:
        return new QoreStringNode(i.s_char);
    case Smoke::t_uchar:
        return new QoreStringNode(i.s_uchar);
    case Smoke::t_short:
        return new QoreBigIntNode(i.s_short);
    case Smoke::t_ushort:
        return new QoreBigIntNode(i.s_ushort);
    case Smoke::t_int:
        return new QoreBigIntNode(i.s_int);
    case Smoke::t_uint:
        return new QoreBigIntNode(i.s_uint);
    case Smoke::t_long:
        return new QoreBigIntNode(i.s_long);
    case Smoke::t_ulong:
        return new QoreBigIntNode(i.s_ulong);
    case Smoke::t_float:
        return new QoreFloatNode(i.s_float);
    case Smoke::t_double:
        return new QoreFloatNode(i.s_double);
    case Smoke::t_enum:
        return new QoreQtEnumNode(i.s_enum, t);
    case Smoke::t_voidp: {
        QByteArray tname(t.name);

        if (tname == "uchar*" || tname == "unsigned char*")
            return new QoreStringNode(*(uchar*)i.s_voidp);

        if (tname == "const char*" || tname == "char*")
            return new QoreStringNode(*(char*)i.s_voidp);

        if (tname == "QString" || tname == "QString&")
            return new QoreStringNode(reinterpret_cast<QString*>(i.s_voidp)->toUtf8().data(), QCS_UTF8);

        if (tname.startsWith("QList<")
                || tname.startsWith("QVector<")
                || tname == "QStringList"
                || tname.startsWith("QMap<")
                || tname.startsWith("QHash<")
                ) {
            AbstractQoreNode * aqn = QtContainerToQore::Instance()->marshall(t, i.s_voidp, xsink);
            if (*xsink || !aqn) {
                xsink->handleExceptions();
                return 0;
            }
            return aqn;
        }

        if (tname == "WId") {
            return new QoreBigIntNode( (unsigned long)*reinterpret_cast<WId*>(i.s_voidp));
        }

        printd(0, "Marshalling::stackToQore() unhandled voidp type: '%s'\n", t.name);
        Q_ASSERT_X(0, "unhandled voidp", "Smoke::t_voidp marshalling");
        // more missing classes will be catch by assertion.
        return 0;
    }
    case Smoke::t_class: {
        void *origObj = i.s_class;
        Smoke::Index classId = resolveQtClass(origObj, t.classId);

        // NOTE: Design change. There will be no default QVariant to
        // Qore conversion. User should call custom QVariant::toQore() method for it.
//         if (classId == SCI_QVARIANT)
//             return return_qvariant(*(reinterpret_cast<QVariant *>(origObj)));

        QoreClass * c = ClassNamesMap::Instance()->value(classId);
        if (!c) {
            xsink->raiseException("QT-RETURN-VALUE", "Unknown returning object type: %s", t.name);
            return 0;
        }

        //printd(0, "Marshalling::stackToQore() %s: %p\n", t.name, origObj);
        if (!origObj) {
            //printd(0, "(WW) Marshalling::stackToQore - origObj = 0.\n");
            return 0;
        }

        QoreClass *qc;
        ReferenceHolder<QoreObject> o(getQoreObject(classId, origObj, qc), xsink);
        if (o) {
//             printd(0, "Marshalling::stackToQore() got QoreObject %p\n", o);
            o->ref();
        } else {
            QoreSmokePrivate *p = 0;
            // now it should be real object
            if (c->getClass(QC_QABSTRACTITEMMODEL->getID())) {
                QoreSmokePrivateQAbstractItemModelData *p1;
                o = doQObject<QoreSmokePrivateQAbstractItemModelData>(origObj, xsink, &p1);
                p = p1;
            } else if (c->getClass(QC_QOBJECT->getID())) {
                QoreSmokePrivateQObjectData *p1;
                o = doQObject<QoreSmokePrivateQObjectData>(origObj, xsink, &p1);
                p = p1;
            } else {
                o = getQoreMappedObject(classId, origObj);
                if (o) {
                    o->ref();
                } else {
                    // it's not QObject based, just use copy constructor
                    const char * className = qt_Smoke->classes[classId].className;

                    //printd(0, "Marshalling::stackToQore() %s: origObj=%p qcid=%d (%s) scid=%d ref=%d ptr=%d stack=%d\n", t.name, origObj, c->getID(), c->getName(), t.classId, flags == Smoke::tf_ref, flags == Smoke::tf_ptr, flags == Smoke::tf_stack);

                    if (iconst && flags == Smoke::tf_ref) {
                        origObj = Marshalling::constructCopy(origObj, className, xsink);
                        if (*xsink)
                            return 0;
                    }
                    o = createQoreObjectFromNonQObject(c, classId, origObj, &p);
                }
            }
            if (p && flags != Smoke::tf_stack)
                p->setExternallyOwned();
        }
        // it can return already existing object or non-qobject based one
        // qobject based objs are handled in o->getClass(QC_QOBJECT->getID()) part
        return o.release();
    } // case Smoke::t_class
    } // switch

    xsink->raiseException("QT-RETURN-VALUE", "Unhandled return type '%s'.", t.name);
    return 0;
}

void * constructCopy(void * obj, const char * className, ExceptionSink *xsink) {
    QByteArray realClassName(className);
    QByteArray copyName(className);
    QByteArray mungedName(copyName + "#");
    QByteArray argName("const " + copyName + "&");
//     printd(0, "Marshalling::constructCopy %s::%s %s %s obj=%p\n", realClassName.constData(),
//            copyName.constData(), mungedName.constData(), argName.constData(), obj);

    bool methodFound = false;
    Smoke::Method method;
    ClassMap::MungledToTypes * mMap = ClassMap::Instance()->availableMethods(realClassName, copyName);
    QList<ClassMap::TypeHandler> candidates = mMap->values(mungedName);

    foreach (ClassMap::type_handler_s t, candidates) {
        if (t.types.count() == 1 && t.types.at(0).name == argName) {
            methodFound = true;
            method = qt_Smoke->methods[t.method];
        }
    }
    if (!methodFound) {
        assert(xsink);
        xsink->raiseException("QT-COPY-ERROR", "No copy constructor found for: %s::%s(%s)",
                              realClassName.constData(), copyName.constData(), argName.constData());
        return 0;
    }

    Smoke::StackItem args[2];
    args[1].s_class = obj;

    Smoke::ClassFn fn = qt_Smoke->classes[method.classId].classFn;
    (*fn)(method.method, 0, args);

    // Initialize the binding for the new instance
    Smoke::StackItem s[2];
    s[1].s_voidp = QoreSmokeBinding::Instance(qt_Smoke);
    (*fn)(0, args[0].s_class, s);

    return args[0].s_class;
}

static Smoke::Index findClass(const char * cname) {
    Smoke::ModuleIndex mi = qt_Smoke->idClass(cname);
    Q_ASSERT_X(mi.smoke, "smoke not found", cname);
    return mi.index;
}

Smoke::Index resolveQtClass(void * ptr, Smoke::Index classId) {
    Smoke::Index ret = classId;
    if (qt_Smoke->isDerivedFromByName(qt_Smoke->classes[classId].className, "QEvent")) {
        QEvent * qevent = (QEvent *) qt_Smoke->cast(ptr, classId, qt_Smoke->idClass("QEvent").index);
        switch (qevent->type()) {
        case QEvent::Timer:
            ret = findClass("QTimerEvent");
            break;
        case QEvent::MouseButtonPress:
        case QEvent::MouseButtonRelease:
        case QEvent::MouseButtonDblClick:
        case QEvent::MouseMove:
            ret = findClass("QMouseEvent");
            break;
        case QEvent::KeyPress:
        case QEvent::KeyRelease:
        case QEvent::ShortcutOverride:
            ret = findClass("QKeyEvent");
            break;
        case QEvent::FocusIn:
        case QEvent::FocusOut:
            ret = findClass("QFocusEvent");
            break;
        case QEvent::Enter:
        case QEvent::Leave:
            ret = findClass("QEvent");
            break;
        case QEvent::Paint:
            ret = findClass("QPaintEvent");
            break;
        case QEvent::Move:
            ret = findClass("QMoveEvent");
            break;
        case QEvent::Resize:
            ret = findClass("QResizeEvent");
            break;
        case QEvent::Create:
        case QEvent::Destroy:
            ret = findClass("QEvent");
            break;
        case QEvent::Show:
            ret = findClass("QShowEvent");
            break;
        case QEvent::Hide:
            ret = findClass("QHideEvent");
        case QEvent::Close:
            ret = findClass("QCloseEvent");
            break;
        case QEvent::Quit:
        case QEvent::ParentChange:
        case QEvent::ParentAboutToChange:
        case QEvent::ThreadChange:
        case QEvent::WindowActivate:
        case QEvent::WindowDeactivate:
        case QEvent::ShowToParent:
        case QEvent::HideToParent:
            ret = findClass("QEvent");
            break;
        case QEvent::Wheel:
            ret = findClass("QWheelEvent");
            break;
        case QEvent::WindowTitleChange:
        case QEvent::WindowIconChange:
        case QEvent::ApplicationWindowIconChange:
        case QEvent::ApplicationFontChange:
        case QEvent::ApplicationLayoutDirectionChange:
        case QEvent::ApplicationPaletteChange:
        case QEvent::PaletteChange:
            ret = findClass("QEvent");
            break;
        case QEvent::Clipboard:
            ret = findClass("QClipboardEvent");
            break;
        case QEvent::Speech:
        case QEvent::MetaCall:
        case QEvent::SockAct:
        case QEvent::WinEventAct:
        case QEvent::DeferredDelete:
            ret = findClass("QEvent");
            break;
        case QEvent::DragEnter:
            ret = findClass("QDragEnterEvent");
            break;
        case QEvent::DragLeave:
            ret = findClass("QDragLeaveEvent");
            break;
        case QEvent::DragMove:
            ret = findClass("QDragMoveEvent");
        case QEvent::Drop:
            ret = findClass("QDropEvent");
            break;
        case QEvent::DragResponse:
            ret = findClass("QDragResponseEvent");
            break;
        case QEvent::ChildAdded:
        case QEvent::ChildRemoved:
        case QEvent::ChildPolished:
            ret = findClass("QChildEvent");
            break;
        case QEvent::ShowWindowRequest:
        case QEvent::PolishRequest:
        case QEvent::Polish:
        case QEvent::LayoutRequest:
        case QEvent::UpdateRequest:
        case QEvent::EmbeddingControl:
        case QEvent::ActivateControl:
        case QEvent::DeactivateControl:
            ret = findClass("QEvent");
            break;
        case QEvent::ContextMenu:
            ret = findClass("QContextMenuEvent");
            break;
        case QEvent::InputMethod:
            ret = findClass("QInputMethodEvent");
            break;
        case QEvent::AccessibilityPrepare:
            ret = findClass("QEvent");
            break;
        case QEvent::TabletMove:
        case QEvent::TabletPress:
        case QEvent::TabletRelease:
            ret = findClass("QTabletEvent");
            break;
        case QEvent::LocaleChange:
        case QEvent::LanguageChange:
        case QEvent::LayoutDirectionChange:
        case QEvent::Style:
        case QEvent::OkRequest:
        case QEvent::HelpRequest:
            ret = findClass("QEvent");
            break;
        case QEvent::IconDrag:
            ret = findClass("QIconDragEvent");
            break;
        case QEvent::FontChange:
        case QEvent::EnabledChange:
        case QEvent::ActivationChange:
        case QEvent::StyleChange:
        case QEvent::IconTextChange:
        case QEvent::ModifiedChange:
        case QEvent::MouseTrackingChange:
            ret = findClass("QEvent");
            break;
        case QEvent::WindowBlocked:
        case QEvent::WindowUnblocked:
        case QEvent::WindowStateChange:
            ret = findClass("QWindowStateChangeEvent");
            break;
        case QEvent::ToolTip:
        case QEvent::WhatsThis:
            ret = findClass("QHelpEvent");
            break;
        case QEvent::StatusTip:
            ret = findClass("QEvent");
            break;
        case QEvent::ActionChanged:
        case QEvent::ActionAdded:
        case QEvent::ActionRemoved:
            ret = findClass("QActionEvent");
            break;
        case QEvent::FileOpen:
            ret = findClass("QFileOpenEvent");
            break;
        case QEvent::Shortcut:
            ret = findClass("QShortcutEvent");
            break;
        case QEvent::WhatsThisClicked:
            ret = findClass("QWhatsThisClickedEvent");
            break;
        case QEvent::ToolBarChange:
            ret = findClass("QToolBarChangeEvent");
            break;
        case QEvent::ApplicationActivated:
        case QEvent::ApplicationDeactivated:
        case QEvent::QueryWhatsThis:
        case QEvent::EnterWhatsThisMode:
        case QEvent::LeaveWhatsThisMode:
        case QEvent::ZOrderChange:
            ret = findClass("QEvent");
            break;
        case QEvent::HoverEnter:
        case QEvent::HoverLeave:
        case QEvent::HoverMove:
            ret = findClass("QHoverEvent");
            break;
        case QEvent::AccessibilityHelp:
        case QEvent::AccessibilityDescription:
            ret = findClass("QEvent");
#if QT_VERSION >= 0x40200
        case QEvent::GraphicsSceneMouseMove:
        case QEvent::GraphicsSceneMousePress:
        case QEvent::GraphicsSceneMouseRelease:
        case QEvent::GraphicsSceneMouseDoubleClick:
            ret = findClass("QGraphicsSceneMouseEvent");
            break;
        case QEvent::GraphicsSceneContextMenu:
            ret = findClass("QGraphicsSceneContextMenuEvent");
            break;
        case QEvent::GraphicsSceneHoverEnter:
        case QEvent::GraphicsSceneHoverMove:
        case QEvent::GraphicsSceneHoverLeave:
            ret = findClass("QGraphicsSceneHoverEvent");
            break;
        case QEvent::GraphicsSceneHelp:
            ret = findClass("QGraphicsSceneHelpEvent");
            break;
        case QEvent::GraphicsSceneDragEnter:
        case QEvent::GraphicsSceneDragMove:
        case QEvent::GraphicsSceneDragLeave:
        case QEvent::GraphicsSceneDrop:
            ret = findClass("QGraphicsSceneDragDropEvent");
            break;
        case QEvent::GraphicsSceneWheel:
            ret = findClass("QGraphicsSceneWheelEvent");
            break;
        case QEvent::KeyboardLayoutChange:
            ret = findClass("QEvent");
            break;
#endif
        default:
            break;
        }
    } else if (qt_Smoke->isDerivedFromByName(qt_Smoke->classes[classId].className, "QGraphicsItem")) {
        QGraphicsItem * item = (QGraphicsItem *) qt_Smoke->cast(ptr, classId, qt_Smoke->idClass("QGraphicsItem").index);
        switch (item->type()) {
        case 1:
            ret = findClass("QGraphicsItem");
            break;
        case 2:
            ret = findClass("QGraphicsPathItem");
            break;
        case 3:
            ret = findClass("QGraphicsRectItem");
        case 4:
            ret = findClass("QGraphicsEllipseItem");
            break;
        case 5:
            ret = findClass("QGraphicsPolygonItem");
            break;
        case 6:
            ret = findClass("QGraphicsLineItem");
            break;
        case 7:
            ret = findClass("QGraphicsItem");
            break;
        case 8:
            ret = findClass("QGraphicsTextItem");
            break;
        case 9:
            ret = findClass("QGraphicsSimpleTextItem");
            break;
        case 10:
            ret = findClass("QGraphicsItemGroup");
            break;
        }
    } else if (qt_Smoke->isDerivedFromByName(qt_Smoke->classes[classId].className, "QLayoutItem")) {
        QLayoutItem * item = (QLayoutItem *) qt_Smoke->cast(ptr, classId, qt_Smoke->idClass("QLayoutItem").index);
        if (item->widget() != 0) {
            ret = findClass("QWidgetItem");
        } else if (item->spacerItem() != 0) {
            ret = findClass("QSpacerItem");
        }
    }

    return ret;
}



} // namespace
