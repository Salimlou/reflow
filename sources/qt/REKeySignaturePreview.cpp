#include "REKeySignaturePreview.h"
#include "REPainter.h"

#include <QPainter>

REKeySignaturePreview::REKeySignaturePreview(QWidget *parent) :
    QWidget(parent)
{
}

void REKeySignaturePreview::SetKeySignature(const REKeySignature& sig)
{
    _keySignature = sig;
    update();
}

float REKeySignaturePreview::YOffsetOfLine(int lineIndex) const
{
    float dy = 7.0;
    return dy * (lineIndex - 4);
}

void REKeySignaturePreview::paintEvent(QPaintEvent *)
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
    painter.DrawMusicSymbol("gclef", 22.0, pt.y()+size, size);

    // Draw accidentals
    float x = 60.0;
    bool useSharps = (_keySignature.SharpCount() > 0);
    int accidentals[7];
    int nbAccidentals = _keySignature.DetermineLinesOfAccidentals(Reflow::TrebleClef, accidentals);
    for(int i=0; i<nbAccidentals; ++i) {
        float y = pt.y() + YOffsetOfLine(accidentals[i]);
        painter.DrawMusicSymbol((useSharps ? "sharp" : "flat"), x, y, size);
        x += size;
    }

    QFont font = QFont("Arial", 22, QFont::Bold);
    qpainter.setFont(font);
    qpainter.drawText(QRectF(pt.x()-35, rc.height() - 30, 100, 30), QString::fromStdString(_keySignature.Name()));
}
