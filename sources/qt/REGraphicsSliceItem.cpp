#include "REGraphicsSliceItem.h"

#include <QPainter>

#include <RESystem.h>
#include <RESlice.h>
#include <REPainter.h>

REGraphicsSliceItem::REGraphicsSliceItem(const RESlice* slice)
    : QGraphicsItem(0), _slice(slice)
{
}

QRectF REGraphicsSliceItem::boundingRect() const
{
    const RESystem* system = _slice->System();
    return QRectF(0, 0, _slice->Width(), system->Height());
}

void REGraphicsSliceItem::paint(QPainter *p, const QStyleOptionGraphicsItem *option,
           QWidget *widget)
 {
     REPainter painter(p);

     painter.SetFillColor(REColor::White);
     painter.FillRect(RERect(boundingRect()));

     p->setRenderHint(QPainter::Antialiasing, true);
     p->setRenderHint(QPainter::TextAntialiasing, true);

     //painter.SetActiveVoiceIndex(self.documentEditor.scoreController->Cursor().VoiceIndex());
     painter.SetActiveVoiceIndex(0);
     painter.SetFillColor(REColor(0, 0, 0));
     painter.SetStrokeColor(REColor(0, 0, 0));

     const RESystem* system = _slice->System();
     system->DrawSlice(painter, _slice->Index());

     painter.Translate(-_slice->XOffset(), 0.0);
     system->DrawBandsOfSliceRange(painter, RERange(_slice->Index(), 1));
 }
