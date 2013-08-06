#include "RETrackListModel.h"

#include "RESong.h"
#include "RETrack.h"

RETrackListModel::RETrackListModel(REDocumentView *parent) :
    QAbstractItemModel(parent)
{
}

int RETrackListModel::rowCount(const QModelIndex &parentIndex) const
{
    QObject* p = QObject::parent();
    REDocumentView* document = qobject_cast<REDocumentView*>(p);
    const RESong* song = document->Song();

    if(document && parentIndex == QModelIndex()) {
        return song->TrackCount();
    }
    return 0;
}

int RETrackListModel::columnCount(const QModelIndex &parent) const
{
    return 1;
}

QModelIndex RETrackListModel::index(int row, int column, const QModelIndex &parent) const
{
    return createIndex(row, column);
}
QModelIndex RETrackListModel::parent(const QModelIndex &child) const
{
    return QModelIndex();
}
QVariant RETrackListModel::data(const QModelIndex &index, int role) const
{
    QObject* p = QObject::parent();
    REDocumentView* document = qobject_cast<REDocumentView*>(p);
    const RESong* song = document->Song();

    if(!index.isValid()) return QVariant();

    const RETrack* track = song->Track(index.row());

    if(role == Qt::DisplayRole)
    {
        return track ? QString::fromStdString(track->Name()) : QString();
    }
    return QVariant();
}
