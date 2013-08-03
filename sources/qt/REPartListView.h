#ifndef REPARTLISTVIEW_H
#define REPARTLISTVIEW_H

#include <QListView>

class REPartListView : public QListView
{
    Q_OBJECT
public:
    explicit REPartListView(QWidget *parent = 0);
    
signals:
    void partSelected(int);
    
protected slots:
    virtual void selectionChanged(const QItemSelection & selected, const QItemSelection & deselected);
};

#endif // REPARTLISTVIEW_H
