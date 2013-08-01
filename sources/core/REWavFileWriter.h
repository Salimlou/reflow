//
//  REWavFileWriter.h
//  Reflow
//
//  Created by Sebastien Bourgeois on 06/02/12.
//  Copyright (c) 2012 Gargant Studios. All rights reserved.
//

#ifndef Reflow_REWavFileWriter_h
#define Reflow_REWavFileWriter_h

#include "RETypes.h"

class REWavFileWriter
{
public:
    REWavFileWriter();
    
    void SetSampleRate(uint32_t sampleRate) {_sampleRate = sampleRate;}
    
    bool RenderToFile(const std::string& path, REAudioStream* stream, unsigned int nbSamples);
    
private:
    uint32_t _sampleRate;
    uint16_t _nbChannels;
    uint16_t _bitsPerSample;
    bool _clip;
};


#endif
