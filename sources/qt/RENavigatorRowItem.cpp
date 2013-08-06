#include "RENavigatorRowItem.h"
#include "RENavigatorScene.h"
#include "REDocumentView.h"
#include "RESong.h"
#include "RETrack.h"
#include "REBar.h"
#include "REVoice.h"
#include "REPhrase.h"
#include "REChord.h"
#include "RENote.h"

#include <QPainter>

RENavigatorRowItem::RENavigatorRowItem(const RERange& range, int trackIndex)
    : QGraphicsItem(nullptr), _barRange(range), _trackIndex(trackIndex)
{
}

void RENavigatorRowItem::CalculateBounds(const RENavigatorScene* navigator)
{
    const REDocumentView* documentView = navigator->DocumentView();
    if(documentView == nullptr) return;

    const RESong* song = documentView->Song();
    const REBar* firstBar = song->Bar(_barRange.FirstIndex());
    const REBar* lastBar = song->Bar(_barRange.LastIndex());

    float x0 = navigator->XOfTick(firstBar->OffsetInTicks());
    float x1 = navigator->XOfTick(lastBar->OffsetInTicks());
    float len = navigator->XOfTick(lastBar->TheoricDurationInTicks());

    _bounds = QSizeF(x1-x0+len, navigator->RowHeight());
}

QRectF RENavigatorRowItem::boundingRect() const
{
    return QRectF(QPointF(0,0), _bounds);
}

void RENavigatorRowItem::paint(QPainter * painter, const QStyleOptionGraphicsItem * option, QWidget * widget)
{
    const RENavigatorScene* navigator = qobject_cast<const RENavigatorScene*>(this->scene());
    if(navigator == nullptr) return;

    const REDocumentView* documentView = navigator->DocumentView();
    if(documentView == nullptr) return;

    const RESong* song = documentView->Song();
    const REBar* firstBar = song->Bar(_barRange.FirstIndex());
    const RETrack* track = song->Track(_trackIndex);

    float offsetX = navigator->XOfTick(firstBar->OffsetInTicks());
    float h = navigator->RowHeight();
    float y0 = h - 4.0;
    float y1 = 4.0;

    painter->fillRect(boundingRect(), QBrush(QColor::fromRgbF(0.95, 0.95, 0.95)));

    int barIndex = _barRange.FirstIndex();
    for(; barIndex <= _barRange.LastIndex(); ++barIndex)
    {
        painter->setPen(QPen(Qt::gray));

        const REBar* bar = song->Bar(barIndex);
        unsigned long tick = bar->OffsetInTicks();
        float barX = navigator->XOfTick(tick) - offsetX;
        float barLen = navigator->XOfTick(bar->TheoricDurationInTicks());

        float deltaPitch = (_maxPitch != _minPitch ? 1.0 / (float)(_maxPitch - _minPitch) : 0.5);

        for(unsigned int voiceIndex=0; voiceIndex < track->VoiceCount(); ++voiceIndex)
        {
            const REVoice* voice = track->Voice(voiceIndex);
            const REPhrase* phrase = voice->Phrase(barIndex);

            unsigned int nbChords = phrase->ChordCount();
            for(unsigned int chordIndex = 0; chordIndex < nbChords; ++chordIndex)
            {
                const REChord* chord = phrase->Chord(chordIndex);
                float chordX = navigator->XOfTick(chord->OffsetInTicks());
                float chordLen = navigator->XOfTick(chord->DurationInTicks());
                unsigned int nbNotes = chord->NoteCount();
                for(unsigned int noteIndex = 0; noteIndex < nbNotes; ++noteIndex)
                {
                    const RENote* note = chord->Note(noteIndex);
                    const RENotePitch& pitch = note->Pitch();
                    int midi = pitch.midi;

                    float t = (float)(midi - _minPitch) * deltaPitch;
                    float y = Reflow::Roundf((1.0 - t) * y0 + t * y1) + 0.5;
                    float x0 = chordX + barX;
                    float x1 = x0 + chordLen;

                    painter->drawLine(QPointF(x0,y), QPointF(x1,y));
                }
            }
        }

        painter->setPen(QPen(Qt::black));
        QPointF p0 = QPointF(barX, 0);
        QPointF p1 = QPointF(p0.x() + barLen, p0.y());
        QPointF p2 = QPointF(p0.x(), p0.y() + h);
        QPointF p3 = QPointF(p1.x(), p1.y() + h);
        painter->drawLine(p0, p1);
        painter->drawLine(p2, p3);
    }

}
