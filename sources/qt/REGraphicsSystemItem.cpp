#include "REGraphicsSystemItem.h"

#include <QPainter>
#include <QSettings>

#include <RESystem.h>
#include <REPainter.h>
#include <REScoreController.h>

REGraphicsSystemItem::REGraphicsSystemItem(const REScoreController* scoreController, const RESystem* system, QGraphicsItem* parentItem)
    : QGraphicsItem(parentItem), _scoreController(scoreController), _system(system)
{
}

QRectF REGraphicsSystemItem::boundingRect() const
 {
    return QRectF(0, 0, _system->Width(), _system->Height());
 }

 void REGraphicsSystemItem::paint(QPainter *p, const QStyleOptionGraphicsItem *option,
            QWidget *widget)
 {
     QSettings settings;

     REPainter painter(p);
     p->setRenderHint(QPainter::Antialiasing, true);
     p->setRenderHint(QPainter::TextAntialiasing, true);

     painter.SetDrawingToScreen(true);
     painter.SetForcedToBlack(false);
     painter.SetGrayOutInactiveVoice(settings.value("edit/grayOutInactiveVoice", QVariant(false)).toBool());
     painter.SetActiveVoiceIndex(_scoreController ? (_scoreController->IsEditingLowVoice() ? 1 : 0) : 0);
     painter.SetFillColor(REColor(0, 0, 0));
     painter.SetStrokeColor(REColor(0, 0, 0));

     _system->Draw(painter);
 }
