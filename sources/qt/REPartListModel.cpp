#include "REPartListModel.h"

#include "RESong.h"
#include "REScoreSettings.h"
#include "REScore.h"

REPartListModel::REPartListModel(REDocumentView *parent) :
    QAbstractItemModel(parent)
{
}

int REPartListModel::rowCount(const QModelIndex &parentIndex) const
{
    QObject* p = QObject::parent();
    REDocumentView* document = qobject_cast<REDocumentView*>(p);
    const RESong* song = document->Song();

    if(document && parentIndex == QModelIndex()) {
        return song->ScoreCount();
    }
    return 0;
}

int REPartListModel::columnCount(const QModelIndex &parent) const
{
    return 1;
}

QModelIndex REPartListModel::index(int row, int column, const QModelIndex &parent) const
{
    return createIndex(row, column);
}
QModelIndex REPartListModel::parent(const QModelIndex &child) const
{
    return QModelIndex();
}
QVariant REPartListModel::data(const QModelIndex &index, int role) const
{
    QObject* p = QObject::parent();
    REDocumentView* document = qobject_cast<REDocumentView*>(p);
    const RESong* song = document->Song();

    if(!index.isValid()) return QVariant();

    const REScoreSettings* scoreSettings = song->Score(index.row());

    if(role == Qt::DisplayRole)
    {
        return QString::fromStdString(scoreSettings->Name());
    }
    return QVariant();
}
