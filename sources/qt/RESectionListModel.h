#ifndef RESectionListModel_H
#define RESectionListModel_H

#include <QAbstractItemModel>

#include "REDocumentView.h"

class RESectionListModel : public QAbstractItemModel
{
    Q_OBJECT

    friend class REDocumentView;

public:
    explicit RESectionListModel(REDocumentView *parent = 0);
    
public:
    virtual int rowCount(const QModelIndex &parent = QModelIndex()) const;
    virtual int columnCount(const QModelIndex &parent = QModelIndex()) const;
    virtual QModelIndex index(int row, int column, const QModelIndex &parent = QModelIndex()) const;
    virtual QModelIndex parent(const QModelIndex &child) const;
    virtual QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const;

signals:
    
};

#endif // RESectionListModel_H
