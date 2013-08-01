//
//  RESystem.h
//  Reflow
//
//  Created by Sebastien on 13/04/11.
//  Copyright 2011 Gargant Studios. All rights reserved.
//

#ifndef _RESYSTEM_H_
#define _RESYSTEM_H_

#include "REScoreNode.h"
#include "RESymbol.h"

class RESystem : public REScoreNode
{
    friend class REScore;
    friend class RELayout;
    friend class REHorizontalLayout;
    
public:
    enum SystemFlag {
        RehearsalBand       = (1 << 0),
        ChordNameBand       = (1 << 1),
        
        CodaBand            = (1 << 2),
        DoubleCodaBand      = (1 << 3),
        SegnoBand           = (1 << 4),
        SegnoSegnoBand      = (1 << 5),
        JumpBand            = (1 << 6),
        ToCodaBand          = (1 << 7),
        ToDoubleCodaBand    = (1 << 8),
        FineBand            = (1 << 9),
        RepeatCountBand     = (1 << 10),
        AlternateEndingsBand= (1 << 11),
        
        TempoMarkerBand     = (1 << 12)
    };
    
public:
    RESystem();
    virtual ~RESystem();
    
    virtual Reflow::ScoreNodeType NodeType() const {return Reflow::SystemNode;}
    
public:
    int Index() const {return _index;}
    int IndexInPage() const {return _indexInPage;}
    
    void Clear();
    
    const REScore* Score() const {return _score;}
        
    const RESlice* SystemBar(int idx) const;
    RESlice* SystemBar(int idx);
    
    const RESystemBarVector& Slices() const {return _systemBars;}
    RESystemBarVector& Slices() {return _systemBars;}
    
    const REStaff* Staff(int idx) const;
    REStaff* Staff(int idx);
    
    bool HasStandardStaffForTrack(const RETrack* track) const;
    bool HasTablatureStaffForTrack(const RETrack* track) const;
    
    const REStaff* FirstStaffOfTrack(const RETrack* track) const;
    const REStaff* LastStaffOfTrack(const RETrack* track) const;
    
    bool IsHorizontalSystem() const {return _horizontalSystem;}
    
    const REStaff* StaffAtY(float y, int* lineIndex) const;
    const RESlice* SystemBarAtX(float x, float* relativeX) const;
    
    const RESlice* SystemBarWithBarIndex(int barIndex) const;
    RESlice* SliceWithBarIndex(int barIndex);

    unsigned int SystemBarCount() const {return (unsigned int)_systemBars.size();}
    unsigned int StaffCount() const {return (unsigned int)_staves.size();}
    
    void InsertSystemBar(RESlice* systemBar, int index);
    void InsertStaff(REStaff* staff, int index);
    
    bool HasFlag(SystemFlag flag) const {return 0 != (_flags & flag);}
    void SetFlag(SystemFlag flag) {_flags |= flag;}
    void UnsetFlag(SystemFlag flag) {_flags &= ~flag;}
    
    const RERange& BarRange() const {return _barRange;}
    
    const RESystem* PreviousSibling() const;
    
    bool IsFirstSystemInPage() const;
    bool IsLastSystemInScore() const;
    bool HasSystemBreakOnLastBar() const;
    
    void CalculateBarDimensions();
    
    float ContentWidth() const;
    inline float LeftMargin() const {return _leftMargin;}
    void SetLeftMargin(float m) {_leftMargin = m;}
    
    REPoint OffsetInPage() const;
    
    unsigned int ListGizmos(Reflow::GizmoType type, REGizmoVector& gizmos) const;
    
    void PickNotesInRect(const RERect& rect, RENoteSet* noteSet, REIntSet* affectedBarsSet) const;
    
    void IterateSymbols(const REConstSymbolAnchorOperation& op) const;
    void IterateSymbolsAtPoint(const REPoint& pointInSystem, const REConstSymbolAnchorOperation& op) const;
    
    unsigned int PageNumber() const;
    
public:
    void Draw(REPainter& painter) const;
    void DrawSlice(REPainter& painter, int sliceIndex) const;
    void DrawBandsOfSliceRange(REPainter& painter, const RERange& sliceRange) const;
    void DrawSlursOfSlice(REPainter& painter, int sliceIndex) const;
    
private:
    void _UpdateIndices();
    void _RefreshMetrics();
    
    void DrawTempoMarker(REPainter& painter, int tempo, Reflow::TempoUnitType tempoUnitType,const REPoint& pt) const;
    
    float DirectionSymbolHeight() const {return 26.0;}
    
private:
    const REScore* _score;
    int _index;
    int _indexInPage;
    uint32_t _flags;
    RERange _barRange;
    RESystemBarVector _systemBars;
    REStaffVector _staves;
    float _leftMargin;
    float _rehearsalYOffset;
    float _chordNameYOffset;
    float _codaYOffset, _doubleCodaYOffset, _segnoYOffset, _segnoSegnoYOffset;
    float _jumpYOffset, _toCodaYOffset, _toDoubleCodaYOffset, _fineYOffset;
    float _repeatCountYOffset;
    float _alternateEndingsYOffset;
    float _tempoMarkerYOffset;
    bool _horizontalSystem;
};


#endif
