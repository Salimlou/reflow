//
//  REPlaylistBar.h
//  Reflow
//
//  Created by Sebastien on 08/07/11.
//  Copyright 2011 Gargant Studios. All rights reserved.
//

#ifndef Reflow_REPlaylistBar_h
#define Reflow_REPlaylistBar_h

#include "RETypes.h"

class REPlaylistBar
{
    friend class RESong;
    friend class RESequencer;
    
public:
    REPlaylistBar();
    
    bool IntersectsTickRange(double t0, double t1, bool* extended) const;
    
    int IndexInPlaylist() const {return _indexInPlaylist;}
    int IndexInSong() const {return _indexInSong;}
    unsigned long Tick() const {return _tick;}
    unsigned long Duration() const {return _duration;}
    const RETimeSignature& TimeSignature() const {return _timeSignature;}
    
private:
    int16_t _indexInPlaylist;
    int16_t _indexInSong;
    int32_t _tick;
    int32_t _duration;
    int32_t _extendedTick;
    int32_t _extendedDuration;
    RETimeSignature _timeSignature;
};

#endif
