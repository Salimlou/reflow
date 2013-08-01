#include "REPianoWidget.h"
#include "REDocumentView.h"
#include "REScoreController.h"
#include "RESong.h"
#include "REScore.h"
#include "REBar.h"
#include "RETrack.h"
#include "REVoice.h"
#include "REPhrase.h"
#include "REChord.h"
#include "RENote.h"
#include "RESystem.h"
#include "REStaff.h"
#include "RESequencer.h"

#include <sstream>
#include <cmath>

#include <QPainter>

static int stepToPitch[] = {
    0,
    2,
    4,
    5,
    7,
    9,
    11
};

static int stepOfPitch[] = {0, 0, 1, 1, 2, 3, 3, 4, 4, 5, 5, 6};

static bool isSharped[] = {false, true, false, true, false, false, true, false, true, false, true, false};


REPianoWidget::REPianoWidget(QWidget *parent) :
    QWidget(parent), _scaleX(1.0f), _offsetX(0.0)
{
    _backgroundImage = QImage(":/keyboard-97.png");
    setMinimumSize(_backgroundImage.size());
    setMinimumHeight(_backgroundImage.height());
}

void REPianoWidget::ConnectToDocument(REDocumentView* doc)
{
    _documentView = doc;
}

void REPianoWidget::DisconnectFromDocument()
{
    _documentView = nullptr;
}

void REPianoWidget::paintEvent(QPaintEvent *)
{
    QPainter painter(this);

    painter.fillRect(rect(), QBrush(QColor::fromRgb(0x33, 0x33, 0x33)));

    painter.drawImage(0, 0, _backgroundImage);
}

float REPianoWidget::KeySpacing() const
{
    return 12.0 * _scaleX;
}

float REPianoWidget::BlackKeyHeight() const
{
    return 72.0;
}

int REPianoWidget::StepWithXOffset(float x) const
{
    int step = (x/KeySpacing());
    return step%7;
}

int REPianoWidget::OctaveWithXOffset(float x) const
{
    int step = (x/KeySpacing());
    int octave = (step/7);
    return octave;
}

int REPianoWidget::AlterationWithXYOffset(float x, float y) const
{
    if(y <= BlackKeyHeight())
    {
        float xmod = fmodf(x, KeySpacing());
        int step = StepWithXOffset(x);
        switch(step)
        {
            case 0: // C
                if(xmod >= 9.0 * _scaleX) return 1;
                break;
            case 1: // D
                if(xmod <= 4.0 * _scaleX) return -1;
                if(xmod >= 11.0 * _scaleX) return 1;
                break;
            case 2: // E
                if(xmod <= 6.0 * _scaleX) return -1;
                break;
            case 3: // F
                if(xmod >= 9.0 * _scaleX) return 1;
                break;
            case 4: // G
                if(xmod <= 4.0 * _scaleX) return -1;
                if(xmod >= 10.0 * _scaleX) return 1;
                break;
            case 5: // A
                if(xmod <= 5.0 * _scaleX) return -1;
                if(xmod >= 11.0 * _scaleX) return 1;
                break;
            case 6: // B
                if(xmod <= 6.0 * _scaleX) return -1;
                break;
        }
    }

    return 0;
}

float REPianoWidget::XOffsetOfCenterOfPitch(unsigned int pitch) const
{

    int octave = (pitch/12);
    int step = stepOfPitch[pitch%12];
    bool sharp = isSharped[pitch%12];

    float x = step * KeySpacing() + (0.5 * KeySpacing());
    if(sharp) {
        x+= (0.5 * KeySpacing());
    }

    return x + (7.0 * KeySpacing() * octave);
}

float REPianoWidget::YOffsetOfRoundOfPitch(unsigned int pitch) const
{
    bool sharp = isSharped[pitch%12];

    if(sharp) {
        return 30.0;
    }
    else {
        return 50.0;
    }
}

RERect REPianoWidget::RectForPitch(unsigned int pitch) const
{
    int octave = (pitch/12);
    int chromatic = pitch % 12;
    int diatonic = stepOfPitch[chromatic];
    bool sharp = isSharped[chromatic];

    float x = diatonic * KeySpacing();
    float w = KeySpacing() - 1;
    float y = 0.0;
    float h = height();

    if(sharp) {
        float dx = 0.75;
        switch(diatonic) {
            case 0:
            case 3:
                dx += 0.05;
                break;
            case 1:
            case 5:
                dx -= 0.05;
                break;
            default: break;
        }
        x += (dx * KeySpacing());
        h = BlackKeyHeight();
        w *= 0.5;
    }

    x += (7.0 * KeySpacing() * octave);

    return RERect(x, y, w, h);
}
