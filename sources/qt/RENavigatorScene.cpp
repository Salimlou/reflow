#include "RENavigatorScene.h"
#include "REDocumentView.h"

#include "REScoreController.h"
#include "REPainter.h"
#include "RESong.h"
#include "RETrack.h"
#include "REBar.h"
#include "RESystem.h"
#include "REStaff.h"

#include <QPainter>
#include <QGraphicsSceneMouseEvent>

#include <sstream>

RENavigatorScene::RENavigatorScene(QObject *parent) :
    QGraphicsScene(parent), _documentView(nullptr), _headerHeight(32.0), _rowHeight(30.0)
{
}

void RENavigatorScene::SetDocumentView(REDocumentView* doc)
{
    _documentView = doc;

    Refresh();
}

void RENavigatorScene::drawBackground(QPainter * painter, const QRectF & rect)
{
    QGraphicsScene::drawBackground(painter, rect);

    if(_documentView == nullptr) return;

    int totalWidth = TotalWidth();

    REScoreController* scoreController = _documentView->ScoreController();
    const REScore* score = (scoreController ? scoreController->Score() : nullptr);
    const RESong* song = (score ? score->Song() : nullptr);

    int firstTrackIndex = 0;
    int lastTrackIndex = song->TrackCount()-1;


    for(int trackIndex=firstTrackIndex; trackIndex<=lastTrackIndex; ++trackIndex)
    {
        const RETrack* track = song->Track(trackIndex);
        bool presentInScore = (score->ContainsTrack(track));
        float rowY = trackIndex * _rowHeight;

        QLinearGradient linearGrad(QPointF(0, rowY), QPointF(0, rowY+_rowHeight));
        linearGrad.setColorAt(0, QColor::fromRgb(50, 50, 50));
        linearGrad.setColorAt(1, QColor::fromRgb(25, 25, 25));
        QBrush b = QBrush(linearGrad);
        painter->fillRect(QRect(0, rowY, totalWidth, _rowHeight), b);
    }
}

void RENavigatorScene::drawForeground(QPainter *painter, const QRectF &rect)
{
    QGraphicsScene::drawForeground(painter, rect);

    if(_documentView == nullptr) return;

    const RESong* song = _documentView->Song();
    const REScoreController* scoreController = _documentView->ScoreController();
    if(scoreController->InferredSelectionKind() == REScoreController::BarRangeSelection)
    {
        int firstBarIndex = scoreController->FirstSelectedBarIndex();
        int lastBarIndex = scoreController->LastSelectedBarIndex();
        int firstTrackIndex = 0;
        int lastTrackIndex = song->TrackCount()-1;

        RERect rc0 = RectOfBar(firstBarIndex, firstTrackIndex);
        RERect rc1 = RectOfBar(lastBarIndex, lastTrackIndex);
        RERect rc = rc0.Union(rc1);

        float alphaFactor = 1.0;

        painter->setPen(QColor::fromRgbF(0.20, 0.25, 0.85, 0.95 * alphaFactor));
        QBrush brush = QBrush(QColor::fromRgbF(0.20, 0.25, 0.85, 0.24 * alphaFactor));

        painter->fillRect(rc.ToQRectF(), brush);
        painter->drawRect(rc.ToQRectF());
    }
    else
    {
        int firstBarIndex = scoreController->FirstSelectedBarIndex();
        int lastBarIndex = scoreController->LastSelectedBarIndex();
        const RETrack* track = scoreController->FirstSelectedTrack();
        if(track)
        {
            RERect rc0 = RectOfBar(firstBarIndex, track->Index());
            RERect rc1 = RectOfBar(lastBarIndex, track->Index());
            RERect rc = rc0.Union(rc1);

            float alphaFactor = 1.0;

            painter->setPen(QColor::fromRgbF(0.20, 0.25, 0.85, 0.95 * alphaFactor));
            QBrush brush = QBrush(QColor::fromRgbF(0.20, 0.25, 0.85, 0.24 * alphaFactor));

            painter->fillRect(rc.ToQRectF(), brush);
            painter->drawRect(rc.ToQRectF());
        }
    }
}

void RENavigatorScene::mousePressEvent(QGraphicsSceneMouseEvent *event)
{
    QPointF pos = event->scenePos();
    bool shiftDown = 0 != (event->modifiers() & Qt::ShiftModifier);

    TapAtPoint(REPoint(pos.x(), pos.y()), shiftDown);

    event->accept();
}

int RENavigatorScene::TickAtX(float x) const
{
    return x * 48.0;
}

float RENavigatorScene::XOfTick(int tick) const
{
    return 0.5f + Reflow::Roundf((float)tick / 48.0);
}

int RENavigatorScene::BarIndexAtX(float x) const
{
    int tick = TickAtX(x);
    const RESong* song = _documentView->Song();
    const REBar* bar = song->FindBarAtTick(tick);
    return (bar ? bar->Index() : -1);
}

int RENavigatorScene::TotalWidth() const
{
    if(_documentView == nullptr) return 0;

    const RESong* song = _documentView->Song();
    const REBar* bar = song->Bar(song->BarCount()-1);

    return XOfTick(bar->OffsetInTicks()) + XOfTick(bar->TheoricDurationInTicks());
}

int RENavigatorScene::TotalHeight() const
{
    if(_documentView == nullptr) return 0;

    const RESong* song = _documentView->Song();
    return song->TrackCount() * _rowHeight;
}

RERect RENavigatorScene::RectOfBar(int barIndex, int trackIndex) const
{
    float y0 = 0.0;
    float h = _rowHeight;

    const RESong* song = _documentView->Song();
    const REBar* bar = song->Bar(barIndex);
    const RETrack* track = song->Track(trackIndex);
    if(bar == NULL || track == NULL) {
        return RERect(0,0,0,0);
    }

    float y = y0 + (trackIndex * h);
    float x = XOfTick(bar->OffsetInTicks());
    float w = XOfTick(bar->TheoricDurationInTicks());

    return RERect(x, y, w, h);
}

int RENavigatorScene::TrackIndexAtY(float y) const
{
    return y / _rowHeight;
}

void RENavigatorScene::TapAtPoint(const REPoint& pt, bool extending)
{
    if(_documentView == nullptr) return;

    int barIndex = BarIndexAtX(pt.x);
    int trackIndex = TrackIndexAtY(pt.y);

    REPrintf("mouseDown on bar %d, track %d\n", barIndex+1, trackIndex+1);

    REScoreController* scoreController = _documentView->ScoreController();
    const REScore* score = (scoreController ? scoreController->Score() : nullptr);
    const RESong* song = (score ? score->Song() : nullptr);
    const RETrack* track = (song ? song->Track(trackIndex) : nullptr);
    const RESystem* system = (score ? score->SystemWithBarIndex(barIndex) : NULL);

    if(track == NULL)
    {
        scoreController->MoveCursorToBar(barIndex);
    }
    else
    {
        const REStaff* firstStaff = system->FirstStaffOfTrack(track);
        if(firstStaff) {
            unsigned long flags = extending ? REScoreController::CursorShiftDown : 0;
            scoreController->MoveCursorTo(firstStaff->Index(), barIndex, 0, 0, flags);
        }
        else {
            scoreController->MoveCursorToBar(barIndex);
        }
    }
}

void RENavigatorScene::Refresh()
{
    clear();

    if(_documentView == nullptr) return;

    REScoreController* scoreController = _documentView->ScoreController();
    const REScore* score = (scoreController ? scoreController->Score() : nullptr);
    const RESong* song = (score ? score->Song() : nullptr);

    int firstBarIndex = 0;
    int lastBarIndex = song->BarCount()-1;

    int firstTrackIndex = 0;
    int lastTrackIndex = song->TrackCount()-1;

    for(int trackIndex=firstTrackIndex; trackIndex<=lastTrackIndex; ++trackIndex)
    {
        const RETrack* track = song->Track(trackIndex);
        bool presentInScore = (score->ContainsTrack(track));
        float rowY = trackIndex * _rowHeight;

        bool buildingBar = false;
        float x0 = 0;
        float x1 = 0;

        // Bar Background and Notes
        for(int barIndex=firstBarIndex; barIndex <= lastBarIndex; ++barIndex)
        {
            const REBar* bar = song->Bar(barIndex);
            bool occupied = !track->IsBarEmptyOrRest(barIndex);
            unsigned long tick = bar->OffsetInTicks();
            float barX = XOfTick(tick);
            float barLen = XOfTick(bar->TheoricDurationInTicks());

            if(buildingBar)
            {
                if(occupied) {
                    x1 = barX + barLen;
                }
                else {
                    QGraphicsRectItem* rect  = addRect(QRectF(x0, rowY+1.0, x1-x0, _rowHeight-1.0), QPen(Qt::gray), QBrush(Qt::white));
                    buildingBar = false;
                }
            }
            else
            {
                if(occupied)
                {
                    buildingBar = true;
                    x0 = barX;
                    x1 = barX + barLen;
                }
            }
        }

        if(buildingBar) {
            QGraphicsRectItem* rect  = addRect(QRectF(x0, rowY+1.0, x1-x0, _rowHeight-1.0), QPen(Qt::gray), QBrush(Qt::white));
            buildingBar = false;
        }
    }
}

