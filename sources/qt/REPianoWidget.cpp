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
#include "REUndoCommand.h"

#include <sstream>
#include <cmath>

#include <QPainter>
#include <QMouseEvent>
#include <QUndoStack>

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
    QWidget(parent), _scaleX(1.0f), _offsetX(0.0), _documentView(nullptr)
{
    _backgroundImage = QImage(":/keyboard-97.png");
    setMinimumSize(_backgroundImage.size());
    setMinimumHeight(_backgroundImage.height());

    _offsetX = 7 * KeySpacing();
}

void REPianoWidget::ConnectToDocument(REDocumentView* doc)
{
    _documentView = doc;
    RefreshDisplay();
}

void REPianoWidget::DisconnectFromDocument()
{
    _documentView = nullptr;
    RefreshDisplay();
}

void REPianoWidget::paintEvent(QPaintEvent *)
{
    QPainter painter(this);

    painter.fillRect(rect(), QBrush(QColor::fromRgb(0x33, 0x33, 0x33)));
    painter.drawImage(0, 0, _backgroundImage);

    if(_documentView == nullptr) return;

    const REScoreController* scoreController = _documentView->ScoreController();
    const REScore* score = scoreController->Score();
    const RESequencer* sequencer = scoreController->Sequencer();

    int barIndex = scoreController->Cursor().BarIndex();
    int voiceIndex = scoreController->Cursor().VoiceIndex();
    int staffIndex = scoreController->Cursor().StaffIndex();

    const RESystem* system = score->SystemWithBarIndex(barIndex);
    const REStaff* staff = system ? system->Staff(staffIndex) : NULL;
    if(staff == NULL) {
        return;
    }

    const RETrack* track = staff->Track();

    REIntSet pitchesInChord;
    REIntSet pitchesInBar;

    QColor color;

    if(sequencer && sequencer->IsRunning())
    {
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
                        int pitch = note->Pitch().midi;
                        pitchesInBar.insert(pitch);
                        if(chord == currentChord) {
                            pitchesInChord.insert(pitch);
                        }
                    }
                }

            }
        }

        color = QColor::fromRgb(217, 102, 39, 200);
    }
    else
    {
        const REVoice* voice = track->Voice(voiceIndex);
        const REPhrase* phrase = voice->Phrase(barIndex);
        const REBar* bar = (phrase ? phrase->Bar() : NULL);
        if(bar)
        {
            const REChord* currentChord = scoreController->Cursor().Chord();
            int stringIndex = scoreController->Cursor().LineIndex();

            for(int chordIndex=0; chordIndex<phrase->ChordCount(); ++chordIndex)
            {
                const REChord* chord = phrase->Chord(chordIndex);
                for(int i=0; i<chord->NoteCount(); ++i)
                {
                    const RENote* note = chord->Note(i);
                    int pitch = note->Pitch().midi;
                    pitchesInBar.insert(pitch);
                    if(chord == currentChord) {
                        pitchesInChord.insert(pitch);
                    }
                }
            }

            color = QColor::fromRgb(217, 102, 39, 200);
        }
    }

    // Draw pressed keys
    REIntSet::const_iterator pitchesInChordIterator = pitchesInChord.begin();
    for(; pitchesInChordIterator != pitchesInChord.end(); ++pitchesInChordIterator)
    {
        int pitch = *pitchesInChordIterator;

        int chromatic = pitch % 12;
        bool sharp = isSharped[chromatic];

        RERect rc = RectForPitch(pitch);
        rc.origin.x -= _offsetX;
        float y = YOffsetOfRoundOfPitch(pitch);

        painter.setPen(QPen(color));
        painter.setBrush(QBrush(color.lighter()));
        painter.drawRect(QRect(rc.MiddleX()-3, y-3, 6, 6));
    }
}

void REPianoWidget::mousePressEvent(QMouseEvent *event)
{
    if(!_documentView) return;

    REPoint pos(event->pos().x(), event->pos().y());
    pos.x += _offsetX;

    int step = StepWithXOffset(pos.x);
    int octave = OctaveWithXOffset(pos.x);
    int alter = AlterationWithXYOffset(pos.x, pos.y);
    int pitch = stepToPitch[step] + octave*12 + alter;
    float xmod = fmodf(pos.x, KeySpacing());
    printf("clicked on step %d at octave %d with alter %d => pitch: %d (xmod:%1.2f y:%1.2f)\n", step, octave, alter, pitch, xmod, pos.y);

    REIntVector pitches;
    pitches.push_back(pitch);

    QUndoStack* undoStack = _documentView->UndoStack();
    REScoreController* scoreController = _documentView->ScoreController();
    undoStack->push(new REScoreUndoCommand(scoreController, std::bind(&REScoreController::InsertNotesWithPitches, std::placeholders::_1, pitches)));
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

void REPianoWidget::RefreshDisplay()
{
    update();
}
