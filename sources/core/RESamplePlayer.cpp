//
//  RESamplePlayer.cpp
//  Reflow
//
//  Created by Sebastien on 09/07/11.
//  Copyright 2011 Gargant Studios. All rights reserved.
//

#include "RESamplePlayer.h"
#include "REMonoSample.h"

#include <cmath>

RESamplePlayer::RESamplePlayer()
: _sample(NULL), _playing(false), _time(0), _freqMod(1.0), _ampMod(1.0)
{
    
}

RESamplePlayer::~RESamplePlayer()
{
    
}

void RESamplePlayer::SetSample(REMonoSample* sample)
{
    _sample = sample;
}
void RESamplePlayer::SetFreqModifier(float freqModifier)
{
    _freqMod = freqModifier;
}
void RESamplePlayer::SetAmpModifier(float ampModifier)
{
    _ampMod = ampModifier;
}

void RESamplePlayer::Start()
{
    if(_sample == NULL) return;
    
    _playing = true;
    _time = 0.0;
}
void RESamplePlayer::Stop()
{
    _playing = false;
}
void RESamplePlayer::NoteOff()
{
    _playing = false;
}
bool RESamplePlayer::IsSounding() const
{
    return _playing;
}

void RESamplePlayer::Process(unsigned int nbFrames, float* workBufferL, float* workBufferR, float volume, float pan)
{
    if(!_playing || _sample == NULL) return;
    
    const float* data = _sample->Data();
    unsigned int size = _sample->Size();
    float speed = /*1.0f /*/ _freqMod;
    for(unsigned int i=0; i<nbFrames; ++i)
    {
        unsigned long pos = (unsigned long)_time % size;
        float sample = data[pos];
        
        *workBufferL += sample * _ampMod * volume * (1.0 - pan);
        ++workBufferL;
        
        *workBufferR += sample * _ampMod * volume * (1.0 - pan);
        ++workBufferR;
        
        _time += speed;
        if(_time >= size) {
            _time = fmodf(_time, size);
        }
    }
}
