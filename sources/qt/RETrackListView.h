#ifndef RETRACKLISTVIEW_H
#define RETRACKLISTVIEW_H

#include <QListView>

class RETrackListView : public QListView
{
    Q_OBJECT
public:
    explicit RETrackListView(QWidget *parent = 0);
    
signals:
    void trackSelected(int);

public slots:
    
protected slots:
    virtual void selectionChanged(const QItemSelection & selected, const QItemSelection & deselected);

};

#endif // RETRACKLISTVIEW_H
