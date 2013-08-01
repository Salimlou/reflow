#include "RENavigatorHeaderWidget.h"
#include "REDocumentView.h"

#include "REPainter.h"
#include "RESong.h"
#include "REBar.h"
#include "REFunctions.h"

#include <QPainter>

#include <sstream>

RENavigatorHeaderWidget::RENavigatorHeaderWidget(QWidget *parent) :
    QWidget(parent), _documentView(nullptr)
{
}

int RENavigatorHeaderWidget::TickAtX(float x) const
{
    return x * 48.0;
}

float RENavigatorHeaderWidget::XOfTick(int tick) const
{
    return 0.5f + Reflow::Roundf((float)tick / 48.0);
}

int RENavigatorHeaderWidget::BarIndexAtX(float x) const
{
    int tick = TickAtX(x);
    const RESong* song = _documentView->Song();
    const REBar* bar = song->FindBarAtTick(tick);
    return (bar ? bar->Index() : -1);
}

int RENavigatorHeaderWidget::TotalWidth() const
{
    if(_documentView == nullptr) return 0;

    const RESong* song = _documentView->Song();
    const REBar* bar = song->Bar(song->BarCount()-1);

    return XOfTick(bar->OffsetInTicks()) + XOfTick(bar->TheoricDurationInTicks());
}

void RENavigatorHeaderWidget::paintEvent(QPaintEvent *)
{
    if(!_documentView) return;

    QRect frame = geometry();

    QPainter p(this);
    REPainter painter(&p);

    QLinearGradient linearGrad(QPointF(0, 0), QPointF(0, 30));
    linearGrad.setColorAt(0, QColor::fromRgb(90, 90, 90));
    linearGrad.setColorAt(1, QColor::fromRgb(0, 0, 0));
    p.fillRect(rect(), QBrush(linearGrad));


    const RESong* song = _documentView->Song();
    unsigned int nbBars = song->BarCount();
    int firstBarIndex = 0;
    int lastBarIndex = nbBars - 1;

    // Draw Text Elements
    for(int barIndex=firstBarIndex; barIndex <= lastBarIndex; ++barIndex)
    {
        const REBar* bar = song->Bar(barIndex);
        unsigned long tick = bar->OffsetInTicks();
        float x = XOfTick(tick);
        float h = 32.0;

        // Bar Number
        {
            std::ostringstream oss; oss << bar->Index()+1;
            std::string txt = oss.str();

            //painter.DrawText(txt, REPoint(x + 2.0, 8.5), "Arial", 0, 8.0, REColor::Black);
            painter.DrawText(txt, REPoint(x + 2.0, 7.5), "Arial", 0, 8.0, REColor::Gray);
        }

        if(bar->HasFlag(REBar::RehearsalSign))
        {
            //painter.DrawText(bar->RehearsalSignText(), REPoint(x + 2.0, h-13.0), "Arial", REPainter::Bold, 9.0, REColor::Black);
            painter.DrawText(bar->RehearsalSignText(), REPoint(x + 2.0, h-14.0), "Arial", REPainter::Bold, 9.0, REColor::Gray);
        }
        else if(bar->HasTimeSignatureChange())
        {
            const RETimeSignature& ts = bar->TimeSignature();
            std::ostringstream oss; oss << (int)ts.numerator << ":" << (int)ts.denominator;
            std::string txt = oss.str();

            //painter.DrawText(txt, REPoint(x + 2.0, h-13.0), "Arial", REPainter::Bold, 9.0, REColor::Black);
            painter.DrawText(txt, REPoint(x + 2.0, h-14.0), "Arial", REPainter::Bold, 9.0, REColor::Gray);
        }
    }

    // Draw Line Elements
    for(int barIndex=firstBarIndex; barIndex <= lastBarIndex; ++barIndex)
    {
        const REBar* bar = song->Bar(barIndex);
        unsigned long tick = bar->OffsetInTicks();
        float x = XOfTick(tick);
        float h = 32.0;

        painter.SetStrokeColor(REColor::DarkGray);
        painter.StrokeVerticalLine(x, 0, h);
        float dx = XOfTick((4*REFLOW_PULSES_PER_QUARTER)/bar->TimeSignature().denominator);
        for(int i=1; i<bar->TimeSignature().numerator; ++i)
        {
            float x1 = 0.5 + Reflow::Roundf(x + i * dx);
            painter.StrokeVerticalLine(x1, h-6.0, h);
        }

        painter.SetStrokeColor(REColor::Black);
        painter.StrokeHorizontalLine(0, frame.width(), h-0.5);
    }
}

void RENavigatorHeaderWidget::SetDocumentView(REDocumentView* doc)
{
    _documentView = doc;
}
