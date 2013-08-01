//
//  REMonoSample.h
//  Reflow
//
//  Created by Sebastien on 09/07/11.
//  Copyright 2011 Gargant Studios. All rights reserved.
//

#ifndef Reflow_REMonoSample_h
#define Reflow_REMonoSample_h

#include "RETypes.h"

class REMonoSample
{
public:
    REMonoSample();
    ~REMonoSample();
    
    const float* Data() const {return _data;}
    float* Data() {return _data;}
    
    unsigned int Size() const {return _size;}
    float SampleRate() const {return _sampleRate;}

    float SourceFreq() const {return _freq;}
    void SetSourceFreq(float freq) {_freq=freq;}
    
    void Initialize(unsigned int sampleCount, float sampleRate);
    void Clear();
    
    void FillWithZero();
    void FillWithSawWaveForm();
    void FillWithPulseWaveForm();
    void FillWithSineWaveForm();
    void FillWithNoise();
    
private:
    float* _data;
    unsigned int _size;
    float _sampleRate;
    float _freq;
};


#endif
