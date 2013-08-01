//
//  REMultivoiceIterator.h
//  Reflow
//
//  Created by Sebastien Bourgeois on 26/01/12.
//  Copyright (c) 2012 Gargant Studios. All rights reserved.
//

#ifndef Reflow_REMultivoiceIterator_h
#define Reflow_REMultivoiceIterator_h

#include "RETypes.h"

class REMultivoiceIterator
{
    friend class RETrack;
    
public:
    bool IsValid() const;
    bool Next();
    int Tick() const {return _tick;}
    const REChord* Chord(int voice) const {return _chords[voice];}
    
    bool HasAnyChordAtCurrentTickThisFlag(int flag) const;
    
    void SetFirstVoiceIndex(int firstVoiceIndex) {_firstVoiceIndex=firstVoiceIndex;}
    void SetLastVoiceIndex(int lastVoiceIndex) {_lastVoiceIndex=lastVoiceIndex;}
    
private:
    REMultivoiceIterator(const RETrack* track, int barIndex);
    REMultivoiceIterator(const RETrack* track, int barIndex, int firstVoiceIndex, int lastVoiceIndex);
    
private:
    int _barIndex;
    int _firstVoiceIndex;
    int _lastVoiceIndex;
    const RETrack* _track;
    const REChord* _chords[REFLOW_MAX_VOICES];
    const REChord* _chordsBefore[REFLOW_MAX_VOICES];
    int _tick;
};


#endif
