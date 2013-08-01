//
//  REStandardNotationCompiler.h
//  Reflow
//
//  Created by Sebastien Bourgeois on 17/12/12.
//
//

#ifndef __Reflow__REStandardNotationCompiler__
#define __Reflow__REStandardNotationCompiler__

#include "RETypes.h"
#include "REPitchClass.h"

class REStandardNotationCompiler
{
    friend class REPhrase;
    
public:
    REStandardNotationCompiler();
    
    void InitializeWithPhrase(const REPhrase* phrase, bool transposed);
    
    void ProcessChord(REChord* chord);
    void InitializeKeySignatureAccidentals(int key);
    
    RENotePitch FindPitchInPhrase(const REPhrase* phrase, int chordIndex, int lineIndex) const;
    
protected:
    void CalculateStandardRepresentationOfNote(RENote* note, bool graceNote);
    void CalculateAccidentalOffsets();
    void CalculateSecondIntervalStacking(REChord* chord);
    
protected:
    int8_t _accidentalOnLine[256];
    bool _transposed;
    REPitchClass _transposingInterval;
    int _key;
    Reflow::ClefType _clef;
    Reflow::OttaviaType _ottavia;
    
    // Reset these for each chord
    std::map<int, RENote*> _accidentedNotesMappedByLine;
    REIntSet _lines;
};

#endif /* defined(__Reflow__REStandardNotationCompiler__) */
