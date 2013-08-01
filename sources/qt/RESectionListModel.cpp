#include "RESectionListModel.h"

#include "RESong.h"
#include "REBar.h"

RESectionListModel::RESectionListModel(REDocumentView *parent) :
    QAbstractItemModel(parent)
{
}

int RESectionListModel::rowCount(const QModelIndex &parentIndex) const
{
    QObject* p = QObject::parent();
    REDocumentView* document = qobject_cast<REDocumentView*>(p);
    const RESong* song = document->Song();

    if(document && parentIndex == QModelIndex()) {
        return song->RehearsalSignCount();
    }
    return 0;
}

int RESectionListModel::columnCount(const QModelIndex &parent) const
{
    return 1;
}

QModelIndex RESectionListModel::index(int row, int column, const QModelIndex &parent) const
{
    return createIndex(row, column);
}
QModelIndex RESectionListModel::parent(const QModelIndex &child) const
{
    return QModelIndex();
}
QVariant RESectionListModel::data(const QModelIndex &index, int role) const
{
    QObject* p = QObject::parent();
    REDocumentView* document = qobject_cast<REDocumentView*>(p);
    const RESong* song = document->Song();

    if(!index.isValid()) return QVariant();

    const REBar* bar = song->BarOfRehearsalAtIndex(index.row());

    if(role == Qt::DisplayRole)
    {
        return QString::fromStdString(bar->RehearsalSignText());
    }
    return QVariant();
}
