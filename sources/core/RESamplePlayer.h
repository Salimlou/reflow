//
//  RESamplePlayer.h
//  Reflow
//
//  Created by Sebastien on 09/07/11.
//  Copyright 2011 Gargant Studios. All rights reserved.
//

#ifndef Reflow_RESamplePlayer_h
#define Reflow_RESamplePlayer_h

#include "RETypes.h"
#include "RESampleGenerator.h"

class RESamplePlayer : public RESampleGenerator
{
public:
    RESamplePlayer();
    virtual ~RESamplePlayer();
    
    void SetFreqModifier(float freqModifier);
    void SetAmpModifier(float ampModifier);
    
    void Start();
    void Stop();
    void NoteOff();
    bool IsSounding() const;
    
    void Process(unsigned int nbFrames, float* workBufferL, float* workBufferR, float volume, float pan);
    
public:
    void SetSample(REMonoSample* sample);
    
private:
    bool _playing;
    float _time, _freqMod, _ampMod;
    REMonoSample* _sample;
};

#endif
