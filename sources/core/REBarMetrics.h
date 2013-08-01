//
//  REBarMetrics.h
//  Reflow
//
//  Created by Sebastien on 26/04/11.
//  Copyright 2011 Gargant Studios. All rights reserved.
//

#ifndef _REBARMETRICS_H_
#define _REBARMETRICS_H_

#include "RETypes.h"

#define REFLOW_BAR_METRICS_FIRST    (0x01)
#define REFLOW_BAR_METRICS_LAST     (0x02)

class REBarMetrics
{
    friend class RESong;
    friend class REScore;
    friend class RESlice;
    friend class RELayout;
    
public:  
    struct REBMColumn {
        uint32_t tick;
        float leftSpace;
        float rightSpace;
        float xOffset;
    };
        
public:
    REBarMetrics();
    ~REBarMetrics();
    
public:
    unsigned int ColumnCount() const;
    
    const REBMColumn& Column(int columnIndex) const;
    
    unsigned int TrackCount() const;
    
    int RowIndexOfTrack(int trackIndex, int voiceIndex) const;
    unsigned long TickOfColumn(unsigned int colIdx) const;
    int ColumnIndexAtTick(unsigned long tick) const;
    
    double ContentWidth() const {return _contentWidth;}
    double IdealWidth(unsigned long flags) const;
    double LeadingSpace(unsigned long flags) const;
    double TrailingSpace(unsigned long flags) const;
    
    float TimeSignatureOffset(bool firstInSystem) const;
    float KeySignatureOffset(bool firstInSystem) const;
    float RepeatStartOffset(bool firstInSystem) const;
    float ClefOffset(bool firstInSystem) const;
    
    int ColumnIndexAtOrBeforeTick(unsigned long tick, unsigned long* prevTick, unsigned long* nextTick) const;
    
    bool IsEmpty() const {return _empty;}
    bool IsCollapsibleWithFollowing() const {return _collapsibleWithFollowing;}
    
private:
    void _RefreshColumns();
    
private:
    std::vector<REBMColumn> _columns;
    float _leadingSpaceIfFirst;
    float _leadingSpaceInMiddle;
    float _trailingSpaceInMiddle;
    float _trailingSpaceIfLast;
    float _contentWidth;
    
    float _timeSignatureOffsetIfFirst;
    float _timeSignatureOffsetInMiddle;
    
    float _clefOffsetIfFirst;
    float _clefOffsetInMiddle;
    
    float _keySignatureOffsetIfFirst;
    float _keySignatureOffsetInMiddle;
    
    float _repeatStartOffsetIfFirst;
    float _repeatStartOffsetInMiddle;
    
    bool _empty;
    bool _collapsibleWithFollowing;
};


#endif
