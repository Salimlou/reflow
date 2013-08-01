#include "REGraphicsSystemItem.h"

#include <QPainter>

#include <RESystem.h>
#include <REPainter.h>

REGraphicsSystemItem::REGraphicsSystemItem(const RESystem* system, QGraphicsItem* parentItem)
    : QGraphicsItem(parentItem), _system(system)
{
}

QRectF REGraphicsSystemItem::boundingRect() const
 {
    return QRectF(0, 0, _system->Width(), _system->Height());
 }

 void REGraphicsSystemItem::paint(QPainter *p, const QStyleOptionGraphicsItem *option,
            QWidget *widget)
 {
     REPainter painter(p);
     p->setRenderHint(QPainter::Antialiasing, true);
     p->setRenderHint(QPainter::TextAntialiasing, true);

     //painter.SetActiveVoiceIndex(self.documentEditor.scoreController->Cursor().VoiceIndex());
     painter.SetActiveVoiceIndex(0);
     painter.SetFillColor(REColor(0, 0, 0));
     painter.SetStrokeColor(REColor(0, 0, 0));

     _system->Draw(painter);
 }
