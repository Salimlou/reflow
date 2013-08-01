//
//  REMonoSample.cpp
//  Reflow
//
//  Created by Sebastien on 09/07/11.
//  Copyright 2011 Gargant Studios. All rights reserved.
//

#ifdef _WIN32
#  define _USE_MATH_DEFINES 
#  include <math.h>
#else
#  include <cmath>
#endif

#include <cstring>

#include "REMonoSample.h"

REMonoSample::REMonoSample()
: _data(NULL), _sampleRate(44100.0), _size(0)
{
}

REMonoSample::~REMonoSample()
{
    Clear();
}

void REMonoSample::Initialize(unsigned int sampleCount, float sampleRate)
{
    Clear();
    _size = sampleCount;
    _sampleRate = sampleRate;
    _data = (float*) malloc(sizeof(float) * sampleCount);
}

void REMonoSample::Clear()
{
    if(_data) {free(_data); _data=NULL;}
}

void REMonoSample::FillWithZero()
{
    if(!_data) return;
    
    memset(_data, 0, sizeof(float)*_size);
    
    SetSourceFreq(SampleRate() / (float)_size);
}

void REMonoSample::FillWithSawWaveForm()
{
    if(!_data) return;
    
    float invWaveSize = 1.0f / (float)_size;
    for(unsigned int i=0; i<_size; ++i) {
        _data[i] = (-1.0f + (2.0f * ((float)i * invWaveSize)));
    }
    
    SetSourceFreq(SampleRate() * invWaveSize);
}

void REMonoSample::FillWithPulseWaveForm()
{
    if(!_data) return;
    
    for(unsigned int i=0; i<_size/2; ++i) {
        _data[i] = 1.0;
    }
    for(unsigned int i=_size/2; i<_size; ++i) {
        _data[i] = -1.0;
    }
    
    SetSourceFreq(SampleRate() / (float)_size);
}

void REMonoSample::FillWithSineWaveForm()
{
    if(!_data) return;
    
    float invWaveSize = 1.0f / (float)_size;
    for(unsigned int i=0; i<_size; ++i) {
        float phase = (float)i * invWaveSize;
        _data[i] = sinf(phase * 2.0f * M_PI);
    }
    SetSourceFreq(SampleRate() * invWaveSize);
}

void REMonoSample::FillWithNoise()
{
    if(!_data) return;
    
    for(unsigned int i=0; i<_size; ++i) {
        _data[i] = (float)((rand() % 65536)-32768) / 32768.0f;
    }
    SetSourceFreq(SampleRate() / (float)_size);
}
