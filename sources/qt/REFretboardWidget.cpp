#include "REFretboardWidget.h"
#include "REDocumentView.h"
#include "REScoreController.h"
#include "RECursor.h"

#include "REPhrase.h"
#include "REVoice.h"
#include "REStaff.h"
#include "RETrack.h"
#include "REChord.h"
#include "RESystem.h"
#include "REScore.h"
#include "RESong.h"
#include "REBar.h"
#include "RENote.h"
#include "REFunctions.h"
#include "RESequencer.h"
#include "REPainter.h"

#include "REUndoCommand.h"

#include <sstream>
#include <cmath>

#include <QPainter>
#include <QMouseEvent>
#include <QUndoStack>

#define FRETBOARD_NB_CASES			(27)
#define FRETBOARD_STRING_HEIGHT		(11.0)

static float caseSize[] = {
    55.0, 50.0, 45.0, 40.0, 36.0,
    33.0, 30.0, 27.0, 25.0, 22.0,
    20.0, 19.0, 18.0, 17.0, 16.0,
    15.0, 14.0, 13.0, 13.0, 12.0,

    11.0, 11.0, 11.0, 11.0, 11.0,
    11.0, 11.0, 11.0, 11.0, 11.0,
    11.0, 11.0, 11.0, 11.0, 11.0,
};

REFretboardWidget::REFretboardWidget(QWidget *parent) :
    QWidget(parent), _stringCount(6), _documentView(nullptr)
{
}


void REFretboardWidget::ConnectToDocument(REDocumentView* doc)
{
    _documentView = doc;
    RefreshDisplay();
}

void REFretboardWidget::DisconnectFromDocument()
{
    _documentView = nullptr;
    RefreshDisplay();
}

void REFretboardWidget::RefreshDisplay()
{
    update();
}

int REFretboardWidget::stringWithYOffset(float y) const
{
#ifdef WIN32
    int str = roundtol((y-4.0) / FRETBOARD_STRING_HEIGHT);
#else
    int str = lround((y-4.0) / FRETBOARD_STRING_HEIGHT);
#endif
    if(str < 0) str = 0;
    if(str >= _stringCount) str = _stringCount-1;
    return str;
}

float REFretboardWidget::yOffsetOfString(int string) const
{
    return 4.0 + (string * FRETBOARD_STRING_HEIGHT);
}

int REFretboardWidget::fretWithXOffset(float x) const
{
    if(x < 8.0) {return 0;}

    x -= 8;
    for(int i=0; i<FRETBOARD_NB_CASES; ++i)
    {
        if(x < caseSize[i]) {
            return i+1;
        }
        else {
            x -= caseSize[i];
        }
    }
    return 0;
}

float REFretboardWidget::xOffsetOfFret(int fret) const
{
    if(fret == 0) {
        return 4.0;
    }

    if(fret > 30) fret = 30;
    float x = 8.0;
    for(int i=1; i<=fret; ++i)
    {
        x += caseSize[i-1];
    }
    return x;
}

float REFretboardWidget::xOffsetOfCenterOfFret(int fret) const
{
    if(fret == 0) {
        return 4.0;
    }

    if(fret > 30) fret = 30;
    float x = 8.0;
    int i=1;
    for(; i<=fret; ++i)
    {
        x += caseSize[i-1];
    }
    return x - 0.5 * caseSize[i-1];
}

void REFretboardWidget::grayOut(QPainter& painter)
{
    painter.fillRect(rect(), QBrush(QColor::fromRgbF(0, 0, 0, 0.18)));
}

void REFretboardWidget::mousePressEvent(QMouseEvent *event)
{
    if(!_documentView) return;

    REPoint pos(event->pos().x(), event->pos().y());
    int fret = fretWithXOffset(pos.x);
    int string = stringWithYOffset(pos.y);

    auto op = std::bind(&REScoreController::TypeOnVisualFretboard, std::placeholders::_1, string, fret);
    _documentView->UndoStack()->push(new REScoreUndoCommand(_documentView->ScoreController(), op));
}

void REFretboardWidget::paintEvent(QPaintEvent *)
{
    QPainter qpainter(this);
    QRect bounds = rect();
    float h = bounds.height();

    qpainter.setPen(Qt::black);
    qpainter.fillRect(bounds, QBrush(QColor(8, 8, 8)));

    // Draw Frets
    float x = 8.0;
    QBrush fretBrush = QBrush(QColor::fromRgbF(0.35, 0.35, 0.35));
    QPen fretPen1 = QPen(QColor::fromRgbF(0.30, 0.30, 0.30));
    QPen fretPen2 = QPen(QColor::fromRgbF(0.24, 0.24, 0.24));
    for(int i=1; i<FRETBOARD_NB_CASES; ++i)
    {
        x += caseSize[i-1];
        QRectF rc = QRectF(x, 0, 3.0, h);

        qpainter.fillRect(rc, fretBrush);
        qpainter.setPen(fretPen1);
        qpainter.drawLine(QPointF(x+0.5, 0.0), QPointF(x+0.5, h));

        qpainter.setPen(fretPen2);
        qpainter.drawLine(QPointF(x+2.5, 0.0), QPointF(x+2.5, h));
    }

    // Draw strings
    qpainter.setPen(QPen(QColor::fromRgbF(0.75, 0.75, 0.75)));
    QBrush stringBrush = QBrush(QColor::fromRgbF(0.5, 0.5, 0.5));
    for(int i=0; i<_stringCount; ++i)
    {
        float y = yOffsetOfString(i);
        QRectF rc = QRectF(0.0, y, 600.0, 2.0);

        qpainter.fillRect(rc, stringBrush);
        qpainter.drawLine(QPointF(0, y+0.5), QPointF(600, y+0.5));
    }

    // Draw nut
    qpainter.fillRect(QRectF(0, 0, 8, h), QBrush(QColor::fromRgbF(0.1, 0.1, 0.1)));

    qpainter.setPen(QPen(QColor::fromRgbF(0, 0, 0, 0.60)));
    qpainter.drawLine(QPointF(8.0, 0), QPointF(8.0, h));

    qpainter.setPen(QPen(QColor::fromRgbF(0, 0, 0, 0.30)));
    qpainter.drawLine(QPointF(9.0, 0), QPointF(9.0, h));

    // Border
    qpainter.setPen(Qt::black);
    qpainter.setBrush(QBrush());
    qpainter.drawRect(bounds);

    // Small circles
    float y = h - 4.0;
    float x12 = xOffsetOfCenterOfFret(12);
    float x24 = xOffsetOfCenterOfFret(24);

    qpainter.setPen(QPen());
    qpainter.setBrush(QBrush(QColor::fromRgbF(1, 1, 1, 0.40)));

    qpainter.drawEllipse(QRectF(x12-3.0, y, 3.0, 3.0));
    qpainter.drawEllipse(QRectF(x12+3.0, y, 3.0, 3.0));

    qpainter.drawEllipse(QRectF(xOffsetOfCenterOfFret(3)-1.0, y, 3.0, 3.0));
    qpainter.drawEllipse(QRectF(xOffsetOfCenterOfFret(5)-1.0, y, 3.0, 3.0));
    qpainter.drawEllipse(QRectF(xOffsetOfCenterOfFret(7)-1.0, y, 3.0, 3.0));
    qpainter.drawEllipse(QRectF(xOffsetOfCenterOfFret(9)-1.0, y, 3.0, 3.0));
    qpainter.drawEllipse(QRectF(xOffsetOfCenterOfFret(15)-1.0, y, 3.0, 3.0));
    qpainter.drawEllipse(QRectF(xOffsetOfCenterOfFret(17)-1.0, y, 3.0, 3.0));
    qpainter.drawEllipse(QRectF(xOffsetOfCenterOfFret(19)-1.0, y, 3.0, 3.0));
    qpainter.drawEllipse(QRectF(xOffsetOfCenterOfFret(21)-1.0, y, 3.0, 3.0));

    qpainter.drawEllipse(QRectF(x24-2.5, y, 3.0, 3.0));
    qpainter.drawEllipse(QRectF(x24+2.5, y, 3.0, 3.0));

    if(!_documentView) {
        grayOut(qpainter);
        return;
    }

    const REScoreController* scoreController = _documentView->ScoreController();
    const RECursor& tabCursor = scoreController->Cursor();
    int barIndex = tabCursor.BarIndex();
    int voiceIndex = tabCursor.VoiceIndex();

    const REStaff* staff = scoreController->FirstSelectedStaff();
    if(staff == nullptr) {
        grayOut(qpainter);
        return;
    }

    const RETrack* track = staff->Track();

    std::set<REStringFretPair> pitchesInChord;
    std::set<REStringFretPair> pitchesInBar;

    if(!track->IsTablature())
    {
        grayOut(qpainter);
        return;
    }

    // Draw the capo
    int capo = track->Capo();
    if(capo)
    {
        float x = roundf(xOffsetOfCenterOfFret(capo));

        QRectF rc = QRectF(x-8.0, 0, 16.0, h);
        qpainter.setBrush(QBrush(QColor::fromRgbF(0, 0, 0, 0.25)));
        qpainter.drawRoundedRect(rc, 3.0, 2.0);

        rc.adjust(-1, 0, 1, 0);
        qpainter.setPen(QPen(QColor::fromRgbF(0.55, 0.55, 0.55)));
        qpainter.setBrush(QBrush(QColor::fromRgbF(0.45, 0.45, 0.45)));
        qpainter.drawRoundedRect(rc, 3.0, 2.0);
    }

    // Draw Tuning
    {
        QFont font = QFont("Arial", 6);
        qpainter.setFont(font);
        qpainter.setPen(Qt::gray);

        int minStringCount = _stringCount;
        if(track->StringCount() < minStringCount) minStringCount = track->StringCount();
        for(int i=0; i<minStringCount; ++i)
        {
            float y = yOffsetOfString(i);

            int baseNote = track->TuningForString(i);
            std::string tuningName = Reflow::FindNoteName(baseNote, false);
            qpainter.drawText(QPointF(2.0, y+4), QString::fromStdString(tuningName));
        }
    }

    QPen notePen = QPen(QColor::fromRgbF(0.20, 0.72, 0.12, 1.00));
    QBrush noteBrush = QBrush(QColor::fromRgbF(0.20, 0.72, 0.12, 0.85));

    // Draw Notes
    if(_documentView->IsPlaybackRunning())
    {
        RESequencer* sequencer = _documentView->Sequencer();
        int barIndex = sequencer->BarIndexThatsCurrentlyPlaying();
        int tick = sequencer->TickInBarPlaying();

        for(int voiceIndex = 0; voiceIndex < track->VoiceCount(); ++voiceIndex)
        {
            const REVoice* voice = track->Voice(voiceIndex);
            const REPhrase* phrase = voice->Phrase(barIndex);
            if(phrase)
            {
                const REChord* currentChord = NULL;
                const REChord* chordAfter = NULL;
                phrase->ChordsSurroundingTick(tick, &currentChord, &chordAfter);

                for(int chordIndex=0; chordIndex<phrase->ChordCount(); ++chordIndex)
                {
                    const REChord* chord = phrase->Chord(chordIndex);
                    for(int i=0; i<chord->NoteCount(); ++i)
                    {
                        const RENote* note = chord->Note(i);
                        REStringFretPair stringFret(note->String(), note->Fret());
                        pitchesInBar.insert(stringFret);
                        if(chord == currentChord) {
                            pitchesInChord.insert(stringFret);
                        }
                    }
                }

            }
        }
    }
    else
    {
        const REVoice* voice = track->Voice(voiceIndex);
        const REPhrase* phrase = voice->Phrase(barIndex);
        const REBar* bar = (phrase ? phrase->Bar() : NULL);
        if(bar)
        {
            int chordIndex = tabCursor.ChordIndex();
            int stringIndex = tabCursor.LineIndex();

            const REChord* currentChord = phrase->Chord(chordIndex);

            for(int chordIndex=0; chordIndex<phrase->ChordCount(); ++chordIndex)
            {
                const REChord* chord = phrase->Chord(chordIndex);
                for(int i=0; i<chord->NoteCount(); ++i)
                {
                    const RENote* note = chord->Note(i);
                    REStringFretPair stringFret(note->String(), note->Fret());
                    pitchesInBar.insert(stringFret);
                    if(chord == currentChord) {
                        pitchesInChord.insert(stringFret);
                    }
                }
            }

            notePen = QPen(QColor::fromRgbF(0.95, 0.22, 0.12, 1.00));
            noteBrush = QBrush(QColor::fromRgbF(0.95, 0.22, 0.12, 0.85));
        }
    }

    qpainter.setPen(notePen);
    qpainter.setBrush(noteBrush);

    // Notes in current Chord
    std::set<REStringFretPair>::const_iterator pitchesInChordIterator = pitchesInChord.begin();
    for(; pitchesInChordIterator != pitchesInChord.end(); ++pitchesInChordIterator)
    {
        REStringFretPair stringFret = *pitchesInChordIterator;
        float x = xOffsetOfCenterOfFret(stringFret.fret + capo);
        float y = yOffsetOfString(stringFret.string);

        qpainter.drawEllipse(QRectF(x-2.5, y-2.5, 5.0, 5.0));
    }

    // Notes in current Bar
    std::set<REStringFretPair>::const_iterator pitchesInBarIterator = pitchesInBar.begin();
    for(; pitchesInBarIterator != pitchesInBar.end(); ++pitchesInBarIterator)
    {
        REStringFretPair stringFret = *pitchesInBarIterator;
        float x = xOffsetOfCenterOfFret(stringFret.fret + capo);
        float y = yOffsetOfString(stringFret.string);

        qpainter.setBrush(QBrush());
        qpainter.drawEllipse(QRectF(x-2.5, y-2.5, 5.0, 5.0));
    }

}
