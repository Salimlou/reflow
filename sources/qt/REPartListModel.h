#ifndef REPARTLISTMODEL_H
#define REPARTLISTMODEL_H

#include <QAbstractItemModel>

#include "REDocumentView.h"

class REPartListModel : public QAbstractItemModel
{
    Q_OBJECT

    friend class REDocumentView;

public:
    explicit REPartListModel(REDocumentView *parent = 0);
    
public:
    virtual int rowCount(const QModelIndex &parent = QModelIndex()) const;
    virtual int columnCount(const QModelIndex &parent = QModelIndex()) const;
    virtual QModelIndex index(int row, int column, const QModelIndex &parent = QModelIndex()) const;
    virtual QModelIndex parent(const QModelIndex &child) const;
    virtual QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const;

signals:
    
public slots:
    
};

#endif // REPARTLISTMODEL_H
