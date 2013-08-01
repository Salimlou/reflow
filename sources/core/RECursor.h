//
//  RECursor.h
//  ReflowTools
//
//  Created by Sebastien Bourgeois on 13/02/12.
//  Copyright (c) 2012 Gargant Studios. All rights reserved.
//

#ifndef ReflowTools_RECursor_h
#define ReflowTools_RECursor_h

#include "RETypes.h"


/** RECursor
 */
class RECursor
{
    friend class REScoreController;
    friend class RESongController;
    
public:
    enum VoiceSelectionType 
    {
        HighVoiceSelection,
        LowVoiceSelection
    };
    
public:
    RECursor();
    RECursor(const REScore* score);
    
    RECursor& operator=(const RECursor& rhs);
    
    void SetScore(const REScore* score) {_score = score;}
    
    void SetBeat(const REGlobalTimeDiv& gtd) {_beat = gtd;}
    void SetBarIndex(int barIndex) {_beat.bar = barIndex;}
    void SetTimeDiv(const RETimeDiv& td) {_beat.timeDiv = td;}
    void SetTick(unsigned long tick);
    void SetLineIndex(int lineIndex) {_lineIndex = lineIndex;}
    void SetStaffIndex(int staffIndex) {_staffIndex = staffIndex;}

    const REGlobalTimeDiv& Beat() const {return _beat;}
    int BarIndex() const {return _beat.bar;}
    int Tick() const;
    int LineIndex() const {return _lineIndex;}
    int StaffIndex() const {return _staffIndex;}    
    int VoiceIndex() const;
    int ChordIndex() const;
    
    void SetVoiceSelection(VoiceSelectionType type) {_voiceSelectionType = type;}
    VoiceSelectionType VoiceSelection() const {return _voiceSelectionType;}
    
    const RESong* Song() const;
    const REScore* Score() const;
    const RESystem* System() const;
    const RESlice* Slice() const;
    const REStaff* Staff() const;
    const REStandardStaff* StandardStaff() const;
    const RETablatureStaff* TablatureStaff() const;
    const RETrack* Track() const;
    const REBar* Bar() const;
    const REVoice* Voice() const;
    const REPhrase* Phrase() const;
    const REChord* Chord() const;
    const RENote* Note() const;
    
    int GrandStaffHandIndex() const;                        // -1: Not a standard staff, 0: right hand, 1: left hand
    REConstChordPair ChordsSurroundingTick() const;
    
    void ForceValidPosition(bool assertChordOnBeat=true);
    
    void EncodeTo(REOutputStream& coder) const;
	void DecodeFrom(REInputStream& decoder);
    
private:
    const REScore* _score;
    REGlobalTimeDiv _beat;
    int _staffIndex;
    int _lineIndex;
    VoiceSelectionType _voiceSelectionType;
};


#endif
