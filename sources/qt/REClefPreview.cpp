#include "REClefPreview.h"
#include "REPainter.h"

#include <QPainter>

REClefPreview::REClefPreview(QWidget *parent) :
    QWidget(parent), _clef(Reflow::TrebleClef), _ottavia(Reflow::NoOttavia)
{
}

void REClefPreview::paintEvent(QPaintEvent *)
{
    QPainter qpainter(this);

    float size = 14.0;
    QRect rc = rect();
    QPointF pt = QPointF(rc.x() + rc.width()/2 - size, rc.y() + rc.height()/2 - size);

    // Background
    qpainter.fillRect(rc, QBrush(Qt::white));

    // Draw Staff Lines
    qpainter.setPen(QPen(Qt::lightGray));
    for(int i=-2; i<=2; ++i) {
        QPointF p0(0, pt.y() + (i * size));
        QPointF p1(rc.width(), pt.y() + (i * size));
        qpainter.drawLine(p0, p1);
    }

    // Draw Clef
    REPainter painter(&qpainter);
    painter.SetStrokeColor(REColor(0,0,0,1));
    painter.SetFillColor(REColor(0,0,0,1));

    float yVA = 0.0;
    float yVB = 0.0;

    switch(_clef)
    {
        case Reflow::TrebleClef:
            //painter.DrawMusicSymbol(Reflow::FontElement_GClef, 22.0, pt.y+size, size);
            painter.DrawMusicSymbol("gclef", 22.0, pt.y()+size, size);
            yVA = pt.y() - size * 4.0;
            yVB = pt.y() + size * 3.5;
            break;
        case Reflow::BassClef:
            //painter.DrawMusicSymbol(Reflow::FontElement_FClef, 22.0, pt.y-size, size);
            painter.DrawMusicSymbol("fclef", 22.0, pt.y()-size, size);
            yVA = pt.y() - size * 4;
            yVB = pt.y() + size * 2;
            break;
    }

    if(_ottavia != Reflow::NoOttavia)
    {
        QFont font = QFont("Arial", size, QFont::Bold);
        qpainter.setFont(font);

        switch(_ottavia)
        {
        case Reflow::Ottavia_8va:
            qpainter.drawText(QRectF(22.0, yVA, 30, 30), QString("8"));
            break;
        case Reflow::Ottavia_8vb:
            qpainter.drawText(QRectF(22.0, yVB, 30, 30), QString("8"));
            break;
        case Reflow::Ottavia_15ma:
            qpainter.drawText(QRectF(22.0, yVA, 30, 30), QString("15"));
            break;
        case Reflow::Ottavia_15mb:
            qpainter.drawText(QRectF(22.0, yVB, 30, 30), QString("15"));
            break;
        }
    }
}

void REClefPreview::SetClef(Reflow::ClefType clef)
{
    _clef = clef;
    update();
}

void REClefPreview::SetOttavia(Reflow::OttaviaType ottavia)
{
    _ottavia = ottavia;
    update();
}
