#ifndef RENAVIGATORSCENE_H
#define RENAVIGATORSCENE_H

#include "RETypes.h"

#include <QGraphicsScene>

class REDocumentView;

class RENavigatorScene : public QGraphicsScene
{
    Q_OBJECT
public:
    explicit RENavigatorScene(QObject *parent = 0);

    void SetDocumentView(REDocumentView*);

    int TickAtX(float x) const;
    float XOfTick(int tick) const;
    RERect RectOfBar(int barIndex, int trackIndex) const;
    int BarIndexAtX(float x) const;
    int TrackIndexAtY(float y) const;
    int TotalWidth() const;
    int TotalHeight() const;

    void TapAtPoint(const REPoint& pt, bool extending);

    void Refresh();

protected:
    void drawBackground(QPainter * painter, const QRectF & rect);

private:
    REDocumentView* _documentView;
    float _headerHeight;
    float _rowHeight;
};

#endif // RENAVIGATORSCENE_H
