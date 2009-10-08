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
    if (!qc->getClass(CID_QOBJECT)) {
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

template<class QLISTT>
AbstractQoreNode * QtContainerToQore::listToObject(const Smoke::Type &t, void* ptr, ExceptionSink *xsink) {
    ReferenceHolder<QoreListNode> retList(new QoreListNode(), xsink);
    QLISTT* l = static_cast<QLISTT*>(ptr);
    QByteArray tt(getSubType(t.name));

    QoreClass *qc = ClassNamesMap::Instance()->value(tt.constData());
    if (!qc) {
        return xsink->raiseException("QLIST-MARSHALL-QT", "Unknown QoreClass for QList< ??? > for %s", t.name);
    }
    if (qc->getClass(CID_QOBJECT)) {
        return xsink->raiseException("QLIST-MARSHALL-QT", "QList< ??? > argument %s is QObject based", t.name);
    }

    for (int i = 0; i < l->count(); ++i) {
        QoreObject *o = new QoreObject(qc, getProgram());
        QoreSmokePrivate * p;
        Smoke::ModuleIndex cls = qt_Smoke->findClass(tt.constData());
        if (!cls.smoke) {
            return xsink->raiseException("QLIST-MARSHALL-QT", "Class %s cannot be found in library map", tt.constData());
        }
        p = new QoreSmokePrivateData(cls.index,
                                     Marshalling::constructCopy((void*)&l->at(i),
                                                                tt.constData(),
                                                                xsink)
                                    );
        o->setPrivate(qc->getID(), p);
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
    m_map["QList<QVariant>"] = &listToObject<QList<QVariant> >; // TODO/FIXME: check it!
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
//     m_map["QVector<QTextLength>
}


QoreQVariant * qoreToQVariant(const Smoke::Type & t, const AbstractQoreNode * node, ExceptionSink * xsink) {
//     printd(0, "Marshalling::qoreToQVariant %s\n", t.name);
    QoreQVariant * ret = new QoreQVariant();

    if (node == 0) { // NOTHING
        ret->qvariant = QVariant();
        return ret;
    }

    if (node->getType() == NT_QTENUM) {
        ret->qvariant = QVariant(node->getAsInt());
        return ret;
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
    case NT_OBJECT: {
        const QoreClass *qc = ClassNamesMap::Instance()->value(t.classId);
        const QoreObject *obj = reinterpret_cast<const QoreObject *>(node);
        QoreSmokePrivateData * p = reinterpret_cast<QoreSmokePrivateData*>(obj->getReferencedPrivateData(qc->getID(), xsink));
        ret->qvariant = p && p->object() ? QVariant( *(QVariant*)(p->object()) ) : QVariant();
        ret->status = QoreQVariant::RealQVariant;
        break;
    }
    default:
        xsink->raiseException("QVARIANT-MARSHALL", "Cannot convert type %s (Qore Type %d)", t.name, node->getType());
        ret->status = QoreQVariant::Invalid;
    } // switch

    return ret;
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
      if (qc)
	 break;
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

        if (tname == "uchar*")
            return new QoreStringNode(*(uchar*)i.s_voidp);

        if (tname == "const char*" || tname == "char*")
            return new QoreStringNode(*(char*)i.s_voidp);

        if (tname == "QString" || tname == "QString&")
            return new QoreStringNode(reinterpret_cast<QString*>(i.s_voidp)->toUtf8().data(), QCS_UTF8);

        if (tname.startsWith("QList<")
                || tname.startsWith("QVector<")
                || tname == "QStringList") {
            AbstractQoreNode * aqn = QtContainerToQore::Instance()->marshall(t, i.s_voidp, xsink);
            // TODO/FIXME: how it will raise an exception?
            if (*xsink || !aqn)
                return 0;
            return aqn;
        }

        if (tname == "WId") {
            return new QoreBigIntNode( (unsigned long)*reinterpret_cast<WId*>(i.s_voidp));
        }

// 		if (tname == "void*") {
// 			printd(0, "Marshalling::stackToQore() handling of void* = %p\n", i.s_voidp);
// 			return 0;
// 		}

        printd(0, "Marshalling::stackToQore() unhandled voidp type: '%s'\n", t.name);
        Q_ASSERT_X(0, "unhandled voidp", "Smoke::t_voidp marshalling");
        // TODO/FIXME: more classes
        return 0;
    }
    case Smoke::t_class: {
        QoreClass * c = ClassNamesMap::Instance()->value(t.classId);
        if (!c) {
            xsink->raiseException("QT-RETURN-VALUE", "Unknown returning object type: %s", t.name);
            return 0;
        }
        void *origObj;

        // TODO/FIXME: WTF? Why are there
//             if (flags == Smoke::tf_ref || flags == Smoke::tf_ptr) {
//                printd(0, "Marshalling::stackToQore() is ref or ptr\n");
        //if (strchr(t.name, '*') != 0) {
//                origObj = i.s_class;
        //origObj = *(void **)i.s_class;
//             } else {
        origObj = i.s_class;
//             }

//             printd(0, "Marshalling::stackToQore() %s: %p\n", t.name, origObj);
        if (!origObj) {
            //printd(0, "(WW) Marshalling::stackToQore - origObj = 0.\n");
            return 0;
        }

        QoreClass *qc;
        ReferenceHolder<QoreObject> o(getQoreObject(t.classId, origObj, qc), xsink);
        if (o) {
//             printd(0, "Marshalling::stackToQore() got QoreObject %p\n", o);
            o->ref();
        } else {

	    QoreSmokePrivate *p;

            // now it should be real object
            // o is still required to decide if is it QObject based or not
	    if (c->getClass(CID_QABSTRACTITEMMODEL)) {	       
	       QoreSmokePrivateQAbstractItemModelData *p1;
	       o = doQObject<QoreSmokePrivateQAbstractItemModelData>(origObj, xsink, &p1);
	       p = p1;
	    }
            else if (c->getClass(CID_QOBJECT)) {
	       QoreSmokePrivateQObjectData *p1;
	       o = doQObject<QoreSmokePrivateQObjectData>(origObj, xsink, &p1);
	       p = p1;
            } 
	    else {
	       o = new QoreObject(c, getProgram());
	       // it's not QObject based, just use copy constructor
	       const char * className = qt_Smoke->classes[t.classId].className;
	       
	       //printd(0, "Marshalling::stackToQore() %s: origObj=%p qcid=%d (%s) scid=%d ref=%d ptr=%d stack=%d\n", t.name, origObj, c->getID(), c->getName(), t.classId, flags == Smoke::tf_ref, flags == Smoke::tf_ptr, flags == Smoke::tf_stack);
	     
	       if (iconst && flags == Smoke::tf_ref) {
		  origObj = Marshalling::constructCopy(origObj, className, xsink);
		  if (*xsink)
		     return 0;
	       }
	    
	       p = new QoreSmokePrivateData(t.classId, origObj);
	       o->setPrivate(c->getID(), p);
	    }
	    if (flags == Smoke::tf_ptr)
	       p->setExternallyOwned();
        }
        // it can return already existing object or non-qobject based one
        // qobject based objs are handled in o->getClass(CID_QOBJECT) part
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
        xsink->raiseException("QT-COPY_CONSTRUCTOR", "No copy constructor found for: %s::%s(%s)",
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

} // namespace
