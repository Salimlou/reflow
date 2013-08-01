//
//  REPlaylistBar.cpp
//  Reflow
//
//  Created by Sebastien on 08/07/11.
//  Copyright 2011 Gargant Studios. All rights reserved.
//

#include "REPlaylistBar.h"


REPlaylistBar::REPlaylistBar()
: _timeSignature(4,4)
{
    
}

bool REPlaylistBar::IntersectsTickRange(double t0, double t1, bool* extended) const
{
    double startTick = (double)_tick / (double)REFLOW_PULSES_PER_QUARTER;
    double lastTick = startTick + (double)_duration / (double)REFLOW_PULSES_PER_QUARTER;
    double extendedStartTick = (double)_extendedTick / (double)REFLOW_PULSES_PER_QUARTER;
    double extendedLastTick = extendedStartTick + (double)_extendedDuration / (double)REFLOW_PULSES_PER_QUARTER;
    
    if(startTick < t1 && lastTick >= t0)
    {
        *extended = false;
        return true;
    }
    else if(extendedStartTick < t1 && extendedLastTick >= t0)
    {
        *extended = true;
        return true;
    }
    else return false;
}
