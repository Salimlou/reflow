//
//  REAudioExportEngine.cpp
//  Reflow
//
//  Created by Sebastien Bourgeois on 20/02/12.
//  Copyright (c) 2012 Gargant Studios. All rights reserved.
//

#include "REAudioExportEngine.h"
#include "REWriteChunkToFile.h"
#include "RESequencer.h"
#include "RESong.h"
#include "REMusicRack.h"

REAudioExportEngine::REAudioExportEngine(const std::string& filename)
: _filename(filename), _file(NULL), _bitsPerSample(16), _nbChannels(2), _cancelRequested(false)
{
    
}
REAudioExportEngine::~REAudioExportEngine()
{
    
}

void REAudioExportEngine::CancelExport()
{
    _cancelRequested = true;
}

bool REAudioExportEngine::ExportSong(const RESong* song)
{
    _file = fopen(_filename.c_str(), "wb");
    if(_file == NULL)  {
        return false;
    }
    
    // Build the Sequencer
    RESequencer sequencer;
    sequencer.Build(song, this);
    sequencer.StartPlayback();
    
    {
        REWriteChunkToFile _riff(_file, "RIFF");
        
        const char* wave = "WAVE";
        fwrite(wave, 4, 1, _file);
        
        // Format
        {
            REWriteChunkToFile _fmt(_file, "fmt ");
            
            uint32_t sampleRate = (uint32_t)SampleRate();
            uint16_t compressionCode = 0x0001;
            uint16_t blockAlign = _bitsPerSample / 8 * _nbChannels;
            uint32_t avgBytesPerSecond = _sampleRate * blockAlign;
            
            fwrite((const void*)&compressionCode, 2, 1, _file);
            fwrite((const void*)&_nbChannels, 2, 1, _file);
            fwrite((const void*)&sampleRate, 4, 1, _file);
            fwrite((const void*)&avgBytesPerSecond, 4, 1, _file);
            fwrite((const void*)&blockAlign, 2, 1, _file);
            fwrite((const void*)&_bitsPerSample, 2, 1, _file);
        }
        
        // Data
        {
            REWriteChunkToFile _fmt(_file, "data");
            
            const int bufferSize = REFLOW_WORK_BUFFER_SIZE;
            float bufferL[bufferSize];
            float bufferR[bufferSize];
            int16_t interleavedBuffer[2*bufferSize];
            
            while(!sequencer.IsPlaybackFinished() && !_cancelRequested)
            {
                unsigned int samplesToRender = bufferSize;//std::min<unsigned int>(bufferSize, samplesRemaining);
                
                memset(bufferL, 0, sizeof(float) * bufferSize);
                memset(bufferR, 0, sizeof(float) * bufferSize);
                                    
                // Fill work buffer by accumulating playing voices
                {
                    // Lock Racks while processing
                    REAudioEngine::MutexLocker lock_the_racks(_mtx);
                    
                    for(unsigned int i=0; i<_racks.size(); ++i)
                    {
                        REMusicRack* rack = _racks[i];
                        if(rack->RenderingEnabled()) {
                            rack->Render(samplesToRender, bufferL, bufferR);
                        }
                    }
                }
                
                // Convert to int16
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
                fwrite(interleavedBuffer, sizeof(int16_t) * bufferSize * 2, 1, _file);
            }
            
            REPrintf("finished rendering to %s\n", _filename.c_str());
        }
    }
    
    fclose(_file);
    return true;
}

void REAudioExportEngine::Initialize()
{
    REAudioEngine::Initialize(REAudioSettings::DefaultAudioSettings());
}

void REAudioExportEngine::Shutdown()
{
    REAudioEngine::Shutdown();
}

void REAudioExportEngine::StartRendering()
{
    REAudioEngine::StartRendering();
}

void REAudioExportEngine::StopRendering()
{
    REAudioEngine::StopRendering();
}
