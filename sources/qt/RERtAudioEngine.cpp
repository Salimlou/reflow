#include "RERtAudioEngine.h"
#include "REMusicRack.h"
#include "REMusicDevice.h"
#include "RESoundFont.h"
#include "RESoundFontManager.h"

#include "RESF2Patch.h"
#include "RESF2Generator.h"

#include <thread>

RERtAudioEngine* RERtAudioEngine::Instance()
{
    if(!_instance) {
        _instance = new RERtAudioEngine;
    }
    return static_cast<RERtAudioEngine*>(_instance);
}

RERtAudioEngine::RERtAudioEngine()
    : _dumpFrameCounter(0)
{
}

RERtAudioEngine::~RERtAudioEngine()
{
    Shutdown();
}

void RERtAudioEngine::Initialize(const REAudioSettings& settings)
{
    REAudioEngine::Initialize(settings);

    if(_rtAudio.getDeviceCount() < 1)
    {
       std::cout << "\nNo audio devices found!\n";
       exit( 0 );
    }

    RtAudio::StreamParameters parameters;
    parameters.deviceId = _rtAudio.getDefaultOutputDevice();
    parameters.nChannels = 2;
    parameters.firstChannel = 0;
    unsigned int sampleRate = 44100;
    unsigned int bufferFrames = REFLOW_WORK_BUFFER_SIZE; // 256 sample frames

    // Create an Instant Music Rack for monitoring MIDI input
    _monitorRack = new REMusicRack();
    _monitorRack->SetSampleRate(_sampleRate);
    _monitorDevice = _monitorRack->CreateMusicDevice();
    RESynthMusicDevice* synthMusicDevice = static_cast<RESynthMusicDevice*>(_monitorDevice);
    synthMusicDevice->SetSoundFont(_soundfont);
    AddRack(_monitorRack);
    _monitorRack->SetRenderingEnabled(true);

    try {
        _rtAudio.openStream( &parameters, NULL, RTAUDIO_SINT16,
                        sampleRate, &bufferFrames, &RERtAudioEngine::_RERtAudioEngineCallback, NULL);
    }
    catch ( RtError& e ) {
        e.printMessage();
        exit( 0 );
    }
}

void RERtAudioEngine::Shutdown()
{
    REAudioEngine::Shutdown();

    _rtAudio.closeStream();
}

void RERtAudioEngine::StartRendering()
{
    try {
        _rtAudio.startStream();
    }
    catch ( RtError& e ) {
        e.printMessage();
        exit( 0 );
    }
}

void RERtAudioEngine::StopRendering()
{
    try {
        _rtAudio.stopStream();
    }
    catch ( RtError& e ) {
        e.printMessage();
        exit( 0 );
    }
}

int RERtAudioEngine::RenderCallback( void *outputBuffer, void *inputBuffer, unsigned int nBufferFrames,
                    double streamTime, RtAudioStreamStatus status, void *userData )
{
    // CRITICAL: Process Racks
    //           - This vector can be concurrently modified by Main Thread (AddRack, RemoveRack)
    {
        REAudioEngine::MutexLocker lock_the_racks(_mtx);

        // Give Instant Midi packets to monitoring device
        while (_instantMidiPacketBuffer.PacketAvailable()) {
            REMidiInstantPacket packet = _instantMidiPacketBuffer.Pop();
            _monitorDevice->MidiEvent(packet.data[0], packet.data[1], packet.data[2], 0);
        }

        // Give Instant Midi Clips to monitoring device
        while (_instantMidiClipBuffer.PacketAvailable()) {
            REInstantMidiClip clip = _instantMidiClipBuffer.Pop();
            _RouteInstantMidiClipToDevice(_monitorDevice, &clip);
        }

        int16_t* dataL = ((int16_t*)outputBuffer);
        int16_t* dataR = ((int16_t*)outputBuffer)+1;

        const uint32_t workBufferSize = REFLOW_WORK_BUFFER_SIZE;
        static float workBufferL[workBufferSize];
        static float workBufferR[workBufferSize];

        uint32_t remainingFrames = nBufferFrames;
        while(remainingFrames)
        {
            uint32_t framesToRender = std::min<uint32_t>(workBufferSize, remainingFrames);

            // Reset work buffer
            memset(workBufferL, 0, sizeof(float)*framesToRender);
            memset(workBufferR, 0, sizeof(float)*framesToRender);

#if 0
            // Fill with sine wave
            static float _phase = 0.0;
            float phaseInc = (440.0 * 2.0 * M_PI) / _sampleRate;
            for(unsigned int i=0; i<framesToRender; ++i)
            {
                workBufferL[i] = workBufferR[i] = sinf(_phase);
                _phase += phaseInc;
            }
            _phase = ::fmodf(_phase, 2.0 * M_PI);
#else
            // Fill work buffer by accumulating playing voices
            for(unsigned int i=0; i<_racks.size(); ++i)
            {
                REMusicRack* rack = _racks[i];
                if(rack->RenderingEnabled()) {
                    rack->Render(framesToRender, workBufferL, workBufferR);
                }
            }
#endif

            // Transmit data to output buffer
            for(unsigned int f=0; f<framesToRender; ++f)
            {
                float sampleL = workBufferL[f];
                float sampleR = workBufferR[f];

                if(sampleL >  0.96f) sampleL = 0.96f;
                if(sampleL < -0.96f) sampleL = -0.96f;
                if(sampleR >  0.96f) sampleR = 0.96f;
                if(sampleR < -0.96f) sampleR = -0.96f;
                *dataL = (int16_t)(sampleL * 32768);
                *dataR = (int16_t)(sampleR * 32768);
                dataL += 2;
                dataR += 2;
            }

            remainingFrames -= framesToRender;
        }

    }
    // ~CRITICAL

    // Dump
    _dumpFrameCounter += nBufferFrames;
    if(_dumpFrameCounter >= _sampleRate)
    {
        _dumpFrameCounter %= (unsigned long)_sampleRate;
        REPrintf("Rendering %d frames @%1.2fHz (timestamp: %1.2f frames)\n",
                 (int)nBufferFrames, (float)_sampleRate, (float)streamTime);
    }
    return 0;
}

int RERtAudioEngine::_RERtAudioEngineCallback( void *outputBuffer, void *inputBuffer, unsigned int nBufferFrames,
                double streamTime, RtAudioStreamStatus status, void *userData )
{
    return RERtAudioEngine::Instance()->RenderCallback(outputBuffer, inputBuffer, nBufferFrames, streamTime, status, userData);
}
