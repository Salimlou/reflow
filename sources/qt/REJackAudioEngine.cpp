#include "REJackAudioEngine.h"
#include "REMusicRack.h"
#include "REMusicDevice.h"

REJackAudioEngine* REJackAudioEngine::Instance()
{
    if(!_instance) {
        _instance = new REJackAudioEngine;
    }
    return static_cast<REJackAudioEngine*>(_instance);
}

REJackAudioEngine::REJackAudioEngine()
    : _jack(nullptr)
{
    _outputPorts[0] = _outputPorts[1] = NULL;
}

REJackAudioEngine::~REJackAudioEngine()
{
    Shutdown();
}

void REJackAudioEngine::Initialize(const REAudioSettings& settings)
{
    REAudioEngine::Initialize(settings);

    _jack = jack_client_new("reflow");
    if(_jack == 0) {
        _error = "Failed to create Client, is JACK Server running ?";
        return;
    }

    jack_set_process_callback(_jack, &REJackAudioEngine::_JackProcessCallback, this);

    _outputPorts[0] = jack_port_register (_jack, "out.L", JACK_DEFAULT_AUDIO_TYPE, JackPortIsOutput, 0);
    _outputPorts[1] = jack_port_register (_jack, "out.R", JACK_DEFAULT_AUDIO_TYPE, JackPortIsOutput, 0);

    _sampleRate = jack_get_sample_rate (_jack);
}

void REJackAudioEngine::Shutdown()
{

}

void REJackAudioEngine::StartRendering()
{
    if(jack_activate(_jack)) {
        _error = "Failed to activate Jack client";
    }
}

void REJackAudioEngine::StopRendering()
{

}

int REJackAudioEngine::Process(jack_nframes_t nframes)
{
    float *workBufferL = (float*) jack_port_get_buffer (_outputPorts[0], nframes);
    float *workBufferR = (float*) jack_port_get_buffer (_outputPorts[1], nframes);

    memset (workBufferL, 0, sizeof (jack_default_audio_sample_t) * nframes);
    memset (workBufferR, 0, sizeof (jack_default_audio_sample_t) * nframes);

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

       // Fill work buffer by accumulating playing voices
        for(unsigned int i=0; i<_racks.size(); ++i)
        {
            REMusicRack* rack = _racks[i];
            if(rack->RenderingEnabled()) {
                rack->Render(nframes, workBufferL, workBufferR);
            }
        }
    }
    // ~CRITICAL

    return 0;
}

int REJackAudioEngine::_JackProcessCallback (jack_nframes_t nframes, void *arg)
{
    return static_cast<REJackAudioEngine*>(arg)->Process(nframes);
}

