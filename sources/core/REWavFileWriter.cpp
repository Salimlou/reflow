//
//  REWavFileWriter.cpp
//  Reflow
//
//  Created by Sebastien Bourgeois on 06/02/12.
//  Copyright (c) 2012 Gargant Studios. All rights reserved.
//

#include "REWavFileWriter.h"
#include "REWriteChunkToFile.h"

REWavFileWriter::REWavFileWriter()
: _sampleRate(44100), _bitsPerSample(16), _nbChannels(2)
{
    
}

bool REWavFileWriter::RenderToFile(const std::string& path, REAudioStream* stream, unsigned int nbSamples)
{
    FILE* f = fopen(path.c_str(), "wb");
    if(f == NULL) return false;
    
    {
        REWriteChunkToFile _riff(f, "RIFF");
        
        const char* wave = "WAVE";
        fwrite(wave, 4, 1, f);
        
        // Format
        {
            REWriteChunkToFile _fmt(f, "fmt ");
            
            uint16_t compressionCode = 0x0001;
            uint16_t blockAlign = _bitsPerSample / 8 * _nbChannels;
            uint32_t avgBytesPerSecond = _sampleRate * blockAlign;
            
            fwrite((const void*)&compressionCode, 2, 1, f);
            fwrite((const void*)&_nbChannels, 2, 1, f);
            fwrite((const void*)&_sampleRate, 4, 1, f);
            fwrite((const void*)&avgBytesPerSecond, 4, 1, f);
            fwrite((const void*)&blockAlign, 2, 1, f);
            fwrite((const void*)&_bitsPerSample, 2, 1, f);
        }
        
        // Data
        {
            REWriteChunkToFile _fmt(f, "data");
            
            const int bufferSize = 512;
            float bufferL[bufferSize];
            float bufferR[bufferSize];
            int16_t interleavedBuffer[2*bufferSize];
            
            unsigned int samplesRemaining = nbSamples;
            while(samplesRemaining)
            {
                unsigned int samplesToRender = std::min<unsigned int>(bufferSize, samplesRemaining);
                
                memset(bufferL, 0, sizeof(float) * bufferSize);
                memset(bufferR, 0, sizeof(float) * bufferSize);
                
                stream->Process(samplesToRender, bufferL, bufferR);
                
                for(unsigned int i=0; i<samplesToRender; ++i)
                {
                    float fsampleL = (float)bufferL[i];
                    float fsampleR = (float)bufferR[i];
                    if(fsampleL >  1.0) {fsampleL =  1.0; _clip=true;}
                    if(fsampleL < -1.0) {fsampleL = -1.0; _clip=true;}
                    if(fsampleR >  1.0) {fsampleR =  1.0; _clip=true;}
                    if(fsampleR < -1.0) {fsampleR = -1.0; _clip=true;}
                    
                    interleavedBuffer[i*2+0] = (int16_t)(32767.0 * fsampleL);
                    interleavedBuffer[i*2+1] = (int16_t)(32767.0 * fsampleR);
                }
                
                // Write to file
                fwrite(interleavedBuffer, sizeof(int16_t) * bufferSize * 2, 1, f);
                
                samplesRemaining -= samplesToRender;
            }
        }
    }
    
    fclose(f);
    return true;
}
