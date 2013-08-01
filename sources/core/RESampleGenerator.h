//
//  RESampleGenerator.h
//  Reflow
//
//  Created by Sebastien Bourgeois on 11/08/11.
//  Copyright 2011 Gargant Studios. All rights reserved.
//

#ifndef Reflow_RESampleGenerator_h
#define Reflow_RESampleGenerator_h

#include "RETypes.h"

class RESampleGenerator
{
public:  
    virtual ~RESampleGenerator() {};
    
    virtual void SetFreqModifier(float freqModifier) = 0;
    virtual void SetAmpModifier(float ampModifier) = 0;
    
    virtual void Start() = 0;
    virtual void Stop() = 0;
    virtual void NoteOff() = 0;
    virtual bool IsSounding() const = 0;
    
    virtual void Process(unsigned int nbFrames, float* workBufferL, float* workBufferR, float volume, float pan) = 0;
};


#endif
