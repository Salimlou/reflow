//
//  REPhrase.h
//  Reflow
//
//  Created by Sebastien on 12/04/11.
//  Copyright 2011 Gargant Studios. All rights reserved.
//

#ifndef _REPHRASE_H_
#define _REPHRASE_H_

#include "RETypes.h"
#include "RELocator.h"
#include "RETickRangeModifier.h"
#include "REChordDiagram.h"

class REPhrase
{
	friend class REVoice;
    friend class REChord;
	friend class RESongController;
    friend class REArchive;
    
public:
    typedef std::pair<int, REChordDiagram> REChordDiagramTickPair;
    typedef std::vector<REChordDiagramTickPair> REPhraseChordDiagramVector;
    
public:
    REPhrase();
    ~REPhrase();
	
public:
	const REChordVector& Chords() const {return _chords;}
	REChordVector& Chords() {return _chords;}
	
	unsigned int ChordCount() const {return (unsigned int)_chords.size();}
	
	const REChord* Chord(int idx) const;
	REChord* Chord(int idx);
    
    const REChord* FirstChord() const;
    const REChord* LastChord() const;
    REChord* FirstChord();
    REChord* LastChord();
    
    void Refresh(bool fixTieFlags=true);
	void Clear();
    bool IsEmpty() const;
    bool IsEmptyOrRest() const;
    
	const REVoice* Voice() const {return _parent;}
	REVoice* Voice() {return _parent;}
    
    const RETrack* Track() const;
    RETrack* Track();
    
    const REBar* Bar() const;
    REBar* Bar();
	
	RELocator Locator() const;
	int Index() const {return _index;}
	
	void AddChord(REChord* chord);
	void InsertChord(REChord* chord, int idx);
	void RemoveChord(int idx);
    void RemoveChord(REChord* chord);
    
    const REChord* ChordAtTick(long tick) const;
    const REChord* ChordAtTimeDiv(const RETimeDiv& div) const;
    REConstChordPair ChordsSurroundingTick(long tick) const;
    void ChordsSurroundingTick(long tick, const REChord** chordLeft, const REChord** chordRight) const;
    
    const RETimeDiv& Duration() const;
    unsigned long DurationInTicks() const;
    bool IsBarComplete() const;
    bool IsBarCompleteOrExceeded() const;
    
    const REPhrase* NextSibling() const;
    const REPhrase* PreviousSibling() const;
    REPhrase* NextSibling();
    REPhrase* PreviousSibling();
    
    void CalculateLineRange(bool transposed, int* outMinLine, int* outMaxLine) const;
    
    RENotePitch PitchFromStd(unsigned int chordIndex, int lineIndex, bool transposingScore) const;
    
    REPhrase* Clone() const;
    void CopyFrom(const REPhrase& phrase);
    
    const REOttaviaRangeModifier& OttaviaModifier() const {return _ottaviaModifier;}
    REOttaviaRangeModifier& OttaviaModifier() {return _ottaviaModifier;}

    bool OnLeftHandStaff() const;
        
    int ChordDiagramCount() const {return _chordDiagrams.size();}
    const REChordDiagram* ChordDiagramAtIndex(int idx) const;
    const REChordDiagram* ChordDiagramAtTick(int tick) const;
    void InsertChordDiagram(int tick, const REChordDiagram& chordDiagram);
    bool HasChordDiagramAtTick(int tick) const;
    int IndexOfChordDiagramAtTick(int tick) const;
    int TickOfChordDiagramAtIndex(int idx) const;
    void RemoveAllChordDiagrams();
    
    void EncodeTo(REOutputStream& coder) const;
	void DecodeFrom(REInputStream& decoder);
    
    void WriteJson(REJsonWriter& writer, uint32_t version) const;
    void ReadJson(const REJsonValue& obj, uint32_t version);
    
public:
    bool operator==(const REPhrase& rhs) const;
    bool operator!=(const REPhrase& rhs) const {return !(*this == rhs);}
    
private:
	void _UpdateIndices();
    void _CalculatePitchesFromTablature(REChord* chord);
    void _CalculateStandardRepresentation(bool transposed=false);
    void _CalculateDrumsRepresentation();
    void _CalculateBeamingGroups();
	void _BeamElements(int firstIndex, int lastIndex);
    void _GroupTupletElements(unsigned int firstIndex, unsigned int lastIndex);
    void _CalculateTupletGroups();
	bool _FixTieFlags();
    
private:
	REVoice* _parent;
	REChordVector _chords;
    REOttaviaRangeModifier _ottaviaModifier;
    REPhraseChordDiagramVector _chordDiagrams;
	int _index;
    uint32_t _flags;
    
    mutable RETimeDiv _duration;
};


#endif
