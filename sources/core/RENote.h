//
//  RENote.h
//  Reflow
//
//  Created by Sebastien on 12/04/11.
//  Copyright 2011 Gargant Studios. All rights reserved.
//

#ifndef _RENOTE_H_
#define _RENOTE_H_

#include "RETypes.h"
#include "REBend.h"
#include "REObject.h"

class REGraceNoteMetrics
{
    friend class RENote;
    
public:
    struct Column
    {
        float accidentalX;
        float noteX;
    };
    typedef std::vector<Column> ColumnVector;
    
public:
    REGraceNoteMetrics();
    
    int ColumnCount() const {return (int)_columns.size();}
    float Width() const {return _width;}
    
    float XOffsetOfNote(int column) const;
    float XOffsetOfAccidental(int column) const;
    
protected:
    float _width;
    ColumnVector _columns;
};


class RENote : public REObject
{
	friend class REChord;
    friend class REPhrase;
    
public:
    struct REStandardRep {
        int8_t line;            // 0: F1 in G Clef
        int8_t accidental;      // Reflow::Accidental coded on one byte
        int8_t noteHeadSymbol;  // Reflow::NoteHeadSymbol coded on one byte
        uint8_t accidentalOffset;    // Accidental offset column
        uint8_t flags;
    };
    
    enum NoteFlag {
        TieOrigin       = 0x0001,
        TieDestination  = 0x0002,
        DeadNote        = 0x0004,
        GhostNote       = 0x0008,
        
        LeftStick       = 0x0010,
        RightStick      = 0x0020,
        Legato          = 0x0040,
        BendEffect      = 0x0080,
        
        LiveVelocityDefined   = 0x0100,
        LiveVelocityOverride  = 0x0200,
        LiveDurationDefined   = 0x0400,
        LiveDurationOverride  = 0x0800,
        
        LiveOffsetDefined     = 0x1000,
        LiveOffsetOverride    = 0x2000,
        
        Selected        = 0x8000
    };
    
    enum StandardRepFlag {
        StackedSecondOnOppositeSideWithStemUp = 0x01,
        StackedSecondOnOppositeSideWithStemDown = 0x02
    };
	
public:
    RENote();
    virtual ~RENote();
	
public:
	const REChord* Chord() const {return _parent;}
	REChord* Chord() {return _parent;}
	
	int Index() const {return _index;}
    //void Clear();
	
    RELocator Locator() const ;
    
    int Fret() const {return _fret;}
    int String() const {return _string;}
    const RENotePitch& Pitch() const {return _pitch;}
    
    void SetFret(int fret) {_fret=fret;}
    void SetString(int string) {_string=string;}
    void SetPitch(const RENotePitch& pitch);
    void SetPitchFromMIDI(int midi);
    
    RENotePitch DetermineNewPitch(int midi) const;
    RENotePitch DetermineNewPitch(int midi, int enharmonicHints) const;
    
    int8_t LiveVelocity() const {return _liveVelocity;}
    int16_t LiveDuration() const {return _liveTickDuration;}
    int16_t LiveOffset() const {return _liveTickOffset;}
    
    void IncrementPitch();
    void DecrementPitch();
    void IncrementNotePitch(const RENote* baseNote);
    void DecrementNotePitch(const RENote* baseNote);
    
    const REStandardRep& Representation(bool transposed=false) const {return transposed ? _repIT : _repCT;}
    REStandardRep& Representation(bool transposed=false) {return transposed ? _repIT : _repCT;}
    
    bool HasFlag(NoteFlag flag) const {return 0 != (_flags & flag);}
    void SetFlag(NoteFlag flag) {_flags |= flag;}
    void SetFlag(NoteFlag flag, bool set) {if(set) SetFlag(flag); else UnsetFlag(flag);}
    void UnsetFlag(NoteFlag flag) {_flags &= ~flag;}
    
#ifdef REFLOW_2
    bool IsSelected() const {return HasFlag(RENote::Selected);}
#else
    bool IsSelected() const {return false;}
#endif
    void SetSelected(bool selected) {if(selected) SetFlag(RENote::Selected); else UnsetFlag(RENote::Selected);}
    
    bool HasBend() const;
    void SetBend(const REBend& bend);
    const REBend& Bend() const {return _bend;}
    
    Reflow::SlideOutType SlideOut() const {return _slideOut;}
    Reflow::SlideInType  SlideIn() const {return _slideIn;}
    
    void SetSlideOut(Reflow::SlideOutType slideOut) {_slideOut = slideOut;}
    void SetSlideIn(Reflow::SlideInType slideIn) {_slideIn = slideIn;}
    
    RENote* FindDestinationOfTiedNote();
    RENote* FindOriginOfTiedNote();
    const RENote* FindDestinationOfTiedNote() const;
    const RENote* FindOriginOfTiedNote() const;
    
    const RENote* FindNextNoteOnSameString() const;
    const RENote* FindLastSiblingNoteOnSameStringInLegatoSuite() const;
    const RENote* FindPreviousNoteOnSameString() const;
    
    int EnharmonicHints() const {return _enharmonicHints;}
    void SetEnharmonicHints(int ehints);
    int NextEnharmonicHints() const;
    void ToggleEnharmonicHints();
    
    int GraceNoteCount() const;
    const REGraceNote* GraceNote(int idx) const;
    REGraceNote* GraceNote(int idx);
    void AddGraceNote(REGraceNote* gnote);
    void RemoveGraceNote(int idx);
    void ClearGraceNotes();
    float CalculateGraceNoteMetrics(bool transposed, float unitSpacing, REGraceNoteMetrics* metrics=nullptr) const;
    const REGraceNoteVector& GraceNotes() const {return _graceNotes;}
    
    void EncodeTo(REOutputStream& coder) const;
	void DecodeFrom(REInputStream& decoder);
    
    void WriteJson(REJsonWriter& writer, uint32_t version) const;
    void ReadJson(const REJsonValue& obj, uint32_t version);
    
public:
    bool operator==(const RENote& rhs) const;
    bool operator!=(const RENote& rhs) const {return !(*this == rhs);}
    
private:
	REChord* _parent;
    uint16_t _flags;
	int8_t _index;
    int8_t _fret;
    int8_t _string;
    int8_t _liveVelocity;
    int16_t _liveTickOffset;
    int16_t _liveTickDuration;
    uint8_t _enharmonicHints;   // [0..2] to help choosing the correct Pitch from MIDI
    Reflow::SlideOutType _slideOut;
    Reflow::SlideInType _slideIn;
    REBend _bend;
    
    // Standard Notation
    RENotePitch _pitch;         // Calculated Pitch
    REStandardRep _repCT;   // Rep Concert Tone
    REStandardRep _repIT;   // Rep Instrument Tone
    REGraceNoteVector _graceNotes;
    
#ifdef REFLOW_TRACE_INSTANCES
public:
    static int _instanceCount;
#endif
};

#endif

