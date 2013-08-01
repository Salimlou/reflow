//
//  REBarMetrics.cpp
//  Reflow
//
//  Created by Sebastien on 26/04/11.
//  Copyright 2011 Gargant Studios. All rights reserved.
//

#include "REBarMetrics.h"
#include "RETrackSet.h"

REBarMetrics::REBarMetrics()
: _leadingSpaceIfFirst(40.0), _leadingSpaceInMiddle(10.0), _trailingSpaceInMiddle(10.0), _trailingSpaceIfLast(10.0), _contentWidth(0.0),
  _timeSignatureOffsetIfFirst(0), _timeSignatureOffsetInMiddle(0)
{
    _clefOffsetIfFirst = 0;
    _clefOffsetInMiddle = 0;
    
    _keySignatureOffsetIfFirst = 0;
    _keySignatureOffsetInMiddle = 0;
    
    _repeatStartOffsetIfFirst = 0;
    _repeatStartOffsetInMiddle = 0;
    
    _empty = false;
    _collapsibleWithFollowing = false;
}

REBarMetrics::~REBarMetrics()
{
}

unsigned int REBarMetrics::ColumnCount() const
{
    return (unsigned int)_columns.size();
}

unsigned long REBarMetrics::TickOfColumn(unsigned int colIdx) const
{
    assert(colIdx < ColumnCount());
    return  _columns[colIdx].tick;
}

const REBarMetrics::REBMColumn& REBarMetrics::Column(int columnIndex) const
{
    assert(columnIndex >= 0 && columnIndex < ColumnCount());
    return _columns[columnIndex];
}

int REBarMetrics::ColumnIndexAtTick(unsigned long tick) const
{
    for(unsigned int i=0; i<_columns.size(); ++i)
    {
        const REBarMetrics::REBMColumn& col = _columns[i];
        if(col.tick == tick) {
            return i;
        }
    }
    return -1;
}

int REBarMetrics::ColumnIndexAtOrBeforeTick(unsigned long tick, unsigned long* prevTick, unsigned long* nextTick) const
{
    if(_columns.empty()) {
        return -1;
    }
    
    int i = 1;
    while(i < _columns.size() && _columns[i].tick < tick)
    {
        ++i;
    }

    int firstColumn = i-1;
    const REBarMetrics::REBMColumn& col = _columns[firstColumn];
    *prevTick = col.tick;
    if(firstColumn != _columns.size()-1) 
    {
        *nextTick = _columns[firstColumn+1].tick;
    }
    return firstColumn;
}

double REBarMetrics::IdealWidth(unsigned long flags) const
{
    return _contentWidth 
    + (flags & REFLOW_BAR_METRICS_FIRST ? _leadingSpaceIfFirst : _leadingSpaceInMiddle) 
    + (flags & REFLOW_BAR_METRICS_LAST ? _trailingSpaceIfLast : _trailingSpaceInMiddle);
}

double REBarMetrics::LeadingSpace(unsigned long flags) const
{
    return (flags & REFLOW_BAR_METRICS_FIRST ? _leadingSpaceIfFirst : _leadingSpaceInMiddle) ;
}
double REBarMetrics::TrailingSpace(unsigned long flags) const
{
    return (flags & REFLOW_BAR_METRICS_LAST ? _trailingSpaceIfLast : _trailingSpaceInMiddle);
}

float REBarMetrics::TimeSignatureOffset(bool firstInSystem) const
{
    return firstInSystem ? _timeSignatureOffsetIfFirst : _timeSignatureOffsetInMiddle;
}

float REBarMetrics::KeySignatureOffset(bool firstInSystem) const
{
    return firstInSystem ? _keySignatureOffsetIfFirst : _keySignatureOffsetInMiddle;
}

float REBarMetrics::RepeatStartOffset(bool firstInSystem) const
{
    return firstInSystem ? _repeatStartOffsetIfFirst : _repeatStartOffsetInMiddle;
}

float REBarMetrics::ClefOffset(bool firstInSystem) const
{
    return firstInSystem ? _clefOffsetIfFirst : _clefOffsetInMiddle;
}

void REBarMetrics::_RefreshColumns()
{
    float x = 0.0;
    for(unsigned int i=0; i<_columns.size(); ++i)
    {
        REBarMetrics::REBMColumn& col = _columns[i];
        x += col.leftSpace;
        col.xOffset = x;
        x += col.rightSpace;
    }
    _contentWidth = x;
}
