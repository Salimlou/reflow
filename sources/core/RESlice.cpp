//
//  RESlice.cpp
//  Reflow
//
//  Created by Sebastien on 01/05/11.
//  Copyright 2011 Gargant Studios. All rights reserved.
//

#include "RESlice.h"
#include "RESystem.h"
#include "RESong.h"
#include "REScore.h"
#include "REBarMetrics.h"

#include <cmath>

RESlice::RESlice()
: _index(-1), _metrics(new REBarMetrics)
{
    
}

RESlice::RESlice(REBarMetrics* bm)
: _index(-1), _metrics(bm)
{
    
}

RESlice::~RESlice()
{
    delete _metrics;
}

const REBarMetrics& RESlice::Metrics() const
{
    return *_metrics;
}

const RESystem* RESlice::System() const
{
    return static_cast<const RESystem*>(_parent);
}

RESystem* RESlice::System()
{
    return static_cast<RESystem*>(_parent);
}

const REBar* RESlice::Bar() const
{
    const RESystem* system = System();
    const REScore* score = System()->Score();
    const RESong* song = score->Song();
    return song->Bar(BarIndex());
}

int RESlice::BarIndex() const
{
    const RESystem* system = System();
    if(system->Score()->Settings().HasFlag(REScoreSettings::UseMultiRests))
    {
        int barIndex = system->BarRange().index;
        const RESlice* sbar = system->SystemBar(0);
        while(sbar && sbar != this) 
        {
            barIndex += sbar->BarCount();
            sbar = sbar->NextSibling();
        }
        return barIndex;
    }
    else {
        return _index + system->BarRange().index;
    }
}

float RESlice::XOffset() const 
{
     return roundf(_position.x);
}

float RESlice::StretchFactor() const
{
    unsigned long flags = 0;
    if(IsFirstInSystem()) flags |= REFLOW_BAR_METRICS_FIRST;
    if(IsLastInSystem()) flags |= REFLOW_BAR_METRICS_LAST;
    
    float contentWidth = _metrics->ContentWidth();
    float leadingSpace = _metrics->LeadingSpace(flags);
    float trailingSpace = _metrics->TrailingSpace(flags);
    
    float realWidth = (Width() - leadingSpace - trailingSpace);
    float stretch = realWidth / contentWidth;
    return stretch;
}

float RESlice::XOffsetOfTick(unsigned int tick) const
{
    unsigned long flags = 0;
    if(IsFirstInSystem()) flags |= REFLOW_BAR_METRICS_FIRST;
    if(IsLastInSystem()) flags |= REFLOW_BAR_METRICS_LAST;
    
    int columnIndex = _metrics->ColumnIndexAtTick(tick);
    
    float contentWidth = _metrics->ContentWidth();
    float leadingSpace = _metrics->LeadingSpace(flags);
    float trailingSpace = _metrics->TrailingSpace(flags);
    if(columnIndex == -1) return leadingSpace;

    float realWidth = (Width() - leadingSpace - trailingSpace);
    float stretch = realWidth / contentWidth;
    
    const REBarMetrics::REBMColumn& column = _metrics->Column(columnIndex);
    return roundf(leadingSpace + column.xOffset * stretch);
}

float RESlice::XOffsetAfterLeadingSpace() const
{
    unsigned long flags = 0;
    if(IsFirstInSystem()) flags |= REFLOW_BAR_METRICS_FIRST;
    if(IsLastInSystem()) flags |= REFLOW_BAR_METRICS_LAST;
    
    float leadingSpace = _metrics->LeadingSpace(flags);
    return roundf(leadingSpace);
}

float RESlice::XOffsetBeforeTrailingSpace() const
{
    unsigned long flags = 0;
    if(IsFirstInSystem()) flags |= REFLOW_BAR_METRICS_FIRST;
    if(IsLastInSystem()) flags |= REFLOW_BAR_METRICS_LAST;
    
    float trailingSpace = _metrics->TrailingSpace(flags);
    return roundf(Width() - trailingSpace);
}

float RESlice::XOffsetAtOrBefore(unsigned int tick) const
{
    unsigned long flags = 0;
    if(IsFirstInSystem()) flags |= REFLOW_BAR_METRICS_FIRST;
    if(IsLastInSystem()) flags |= REFLOW_BAR_METRICS_LAST;
    
    unsigned long before = tick;
    unsigned long after = tick;
    int columnIndex = _metrics->ColumnIndexAtOrBeforeTick(tick, &before, &after);
    
    float contentWidth = _metrics->ContentWidth();
    float leadingSpace = _metrics->LeadingSpace(flags);
    float trailingSpace = _metrics->TrailingSpace(flags);
    if(columnIndex == -1) return leadingSpace;
    
    float realWidth = (Width() - leadingSpace - trailingSpace);
    float stretch = realWidth / contentWidth;
    
    const REBarMetrics::REBMColumn& column = _metrics->Column(columnIndex);
    return roundf(leadingSpace + column.xOffset * stretch);
}

float RESlice::XOffsetOfLastColumn() const
{
    int columnCount = _metrics->ColumnCount();
    if(columnCount == 0) return 0.0;
    
    int tick = _metrics->Column(columnCount-1).tick;
    return XOffsetOfTick(tick);
}

RESlice::QueryColumnResult RESlice::QueryColumnAtX(float x, int* columnIndex, float* snapX) const
{
    unsigned long flags = 0;
    if(IsFirstInSystem()) flags |= REFLOW_BAR_METRICS_FIRST;
    if(IsLastInSystem()) flags |= REFLOW_BAR_METRICS_LAST;
    
    float contentWidth = _metrics->ContentWidth();
    float leadingSpace = _metrics->LeadingSpace(flags);
    float trailingSpace = _metrics->TrailingSpace(flags);
    float realWidth = (Width() - leadingSpace - trailingSpace);
    float stretch = realWidth / contentWidth;
    
    if(x < leadingSpace) {
        return RESlice::InLeadingSpace;
    }
    else if (x > (Width() - trailingSpace)) {
        return RESlice::InTrailingSpace;
    }
    else {
        float invStretch = 1.0 / stretch;
        float localX = (x - leadingSpace) * invStretch;
        unsigned int nbColumns = _metrics->ColumnCount();
        for(unsigned int i=0; i<nbColumns; ++i)
        {
            const REBarMetrics::REBMColumn& column = _metrics->Column(i);
            float dx = localX - column.xOffset;
            if(dx >= -column.leftSpace && dx < column.rightSpace) 
            {
                int res = RESlice::OnColumn;
                if(columnIndex != NULL) *columnIndex = i;
                if(snapX != NULL) {
                    *snapX = roundf(leadingSpace + column.xOffset * stretch);
                }
                if(fabsf(dx-column.leftSpace) < 1.5) 
                {
                    res |= RESlice::OnLeftEdgeOfColumn;
                }
                else if(fabsf(dx-column.rightSpace) < 1.5) {
                    res |= RESlice::OnRightEdgeOfColumn;
                }
                
                return (RESlice::QueryColumnResult)res;
            }
        }
        return RESlice::Nothing;
    }
}

const RESlice* RESlice::NextSibling() const
{
    if(_parent == NULL) return NULL;
    return System()->SystemBar(Index()+1);
}

const RESlice* RESlice::PrevSibling() const
{
    if(_parent == NULL) return NULL;
    return System()->SystemBar(Index()-1);
}

bool RESlice::IsFirstInSystem() const
{
    return _index == 0;
}

bool RESlice::IsLastInSystem() const
{
    const RESystem* system = System();
    return _index == (system ? system->SystemBarCount() - 1 : false);
}





REMultiRestSlice::REMultiRestSlice()
: _repeatCount(1)
{    
}
REMultiRestSlice::REMultiRestSlice(REBarMetrics* bm)
: RESlice(bm), _repeatCount(1)
{
    
}
REMultiRestSlice::~REMultiRestSlice()
{
}
