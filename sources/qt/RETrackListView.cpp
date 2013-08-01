#include "RETrackListView.h"

RETrackListView::RETrackListView(QWidget *parent) :
    QListView(parent)
{
}

void RETrackListView::selectionChanged(const QItemSelection & selected, const QItemSelection & deselected)
{
    QListView::selectionChanged(selected, deselected);
    emit trackSelected(selected.indexes().first().row());
}
