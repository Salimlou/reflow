#include "REPartListView.h"

REPartListView::REPartListView(QWidget *parent) :
    QListView(parent)
{
}

void REPartListView::selectionChanged(const QItemSelection & selected, const QItemSelection & deselected)
{
    QListView::selectionChanged(selected, deselected);
    emit partSelected(selected.indexes().first().row());
}
