//
//  REMultivoiceIterator.cpp
//  Reflow
//
//  Created by Sebastien Bourgeois on 26/01/12.
//  Copyright (c) 2012 Gargant Studios. All rights reserved.
//

#include "REMultivoiceIterator.h"
#include "RETrack.h"
#include "REVoice.h"
#include "REPhrase.h"
#include "REChord.h"

REMultivoiceIterator::REMultivoiceIterator(const RETrack* track, int barIndex)
: _barIndex(barIndex), _track(track), _firstVoiceIndex(0), _lastVoiceIndex(REFLOW_MAX_VOICES)
{
    for(int i=_firstVoiceIndex; i<=_lastVoiceIndex; ++i) {
        _chords[i] = NULL;
        _chordsBefore[i] = NULL;
        const REPhrase* phrase = _track->Voice(i)->Phrase(_barIndex);
        if(phrase) {
            _chords[i] = phrase->FirstChord();
        }
    }
    _tick = 0;
}

REMultivoiceIterator::REMultivoiceIterator(const RETrack* track, int barIndex, int firstVoiceIndex, int lastVoiceIndex)
: _barIndex(barIndex), _track(track), _firstVoiceIndex(firstVoiceIndex), _lastVoiceIndex(lastVoiceIndex)
{
    for(int i=_firstVoiceIndex; i<=_lastVoiceIndex; ++i) {
        _chords[i] = NULL;
        _chordsBefore[i] = NULL;
        const REPhrase* phrase = _track->Voice(i)->Phrase(_barIndex);
        if(phrase) {
            _chords[i] = phrase->FirstChord();
        }
    }
    _tick = 0;
}

bool REMultivoiceIterator::HasAnyChordAtCurrentTickThisFlag(int flag) const
{
    for(int i=_firstVoiceIndex; i<=_lastVoiceIndex; ++i) {
        const REChord* chord = _chords[i];
        if(chord && chord->HasFlag((REChord::ChordFlag)flag)) {
            return true;
        }
    }
    return false;
}

bool REMultivoiceIterator::IsValid() const
{
    bool foundAny = false;
    for(int i=_firstVoiceIndex; i<=_lastVoiceIndex; ++i) {
        if(_chords[i] != NULL) foundAny = true;
    }
    return foundAny;
}

bool REMultivoiceIterator::Next()
{
    // Finished
    if (!IsValid()) {return false;}
    
    // Retrieve next chords in list
    const REChord* nextChord[REFLOW_MAX_VOICES];
    for(int i=_firstVoiceIndex; i<=_lastVoiceIndex; ++i) 
    {
        if(_chords[i] != NULL) {
            nextChord[i] = _chords[i]->NextSibling();
        }
        else if(_chordsBefore[i] != NULL) {
            nextChord[i] = _chordsBefore[i]->NextSibling();
        }
        else {
            nextChord[i] = NULL;
        }
    }
    
    int minTick = INT_MAX;
    bool found = false;
    for(int i=_firstVoiceIndex; i<=_lastVoiceIndex; ++i) 
    {
        if(nextChord[i]) 
        {
            int tk = nextChord[i]->OffsetInTicks();
            if(!found) {
                found = true;
                minTick = tk;
            }
            else {
                if(tk < minTick) {
                    minTick = tk;
                    found = true;
                }
            }
        }
    }
    
    // No chord after
    if(!found) {
        return false;
    }
    
    // Update current chords and tick
    _tick = minTick;
    for(int i=_firstVoiceIndex; i<=_lastVoiceIndex; ++i)
    {
        if(_chords[i]) {
            _chordsBefore[i] = _chords[i];
        }
        const REChord* nc = nextChord[i];
        if(nc != NULL && nc->OffsetInTicks() == minTick) 
        {
            _chords[i] = nc;
        }
        else {
            _chords[i] = NULL;
        }
    }
    
    return true;
}

