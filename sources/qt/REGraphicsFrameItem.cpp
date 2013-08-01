#include "REGraphicsFrameItem.h"

#include <QPainter>

#include <RESystem.h>
#include <REFrame.h>
#include <REPainter.h>

REGraphicsFrameItem::REGraphicsFrameItem(const REFrame* frame)
    : QGraphicsItem(0), _frame(frame)
{
}

QRectF REGraphicsFrameItem::boundingRect() const
{
    return _frame->Bounds().ToQRectF();
}

void REGraphicsFrameItem::paint(QPainter *p, const QStyleOptionGraphicsItem *option, QWidget *widget)
{
    REPainter painter(p);
    p->setRenderHint(QPainter::Antialiasing, true);
    p->setRenderHint(QPainter::TextAntialiasing, true);

    painter.SetFillColor(REColor(0, 0, 0));
    painter.SetStrokeColor(REColor(0, 0, 0));

    const REFontDesc& fontDesc = _frame->Font();
    unsigned int flags = 0;
    if(fontDesc.italic) flags |= REPainter::Italic;
    if(fontDesc.bold) flags |= REPainter::Bold;
    switch(_frame->TextAlignment())
    {
        case Reflow::LeftTextAlign:
            flags |= REPainter::LeftAligned; break;
        case Reflow::RightTextAlign:
            flags |= REPainter::RightAligned; break;
        case Reflow::CenterTextAlign:
        default:
            break;
    }

    painter.DrawTextInRect(_frame->Text(), _frame->Bounds(), fontDesc.name, flags, fontDesc.size, REColor::Black);
}
