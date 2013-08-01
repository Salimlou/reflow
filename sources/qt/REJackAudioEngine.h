#ifndef REJACKAUDIOENGINE_H
#define REJACKAUDIOENGINE_H

#include "RETypes.h"

#include <REAudioEngine.h>
#ifdef WIN32
#define _STDINT_H
#endif
#include <jack/jack.h>

class REJackAudioEngine : public REAudioEngine
{
public:
    static REJackAudioEngine* Instance();

public:
    virtual void Initialize(const REAudioSettings& settings);
    virtual void Shutdown();

    virtual void StartRendering();
    virtual void StopRendering();

protected:
    REJackAudioEngine();
    virtual ~REJackAudioEngine();

protected:
    int Process(jack_nframes_t nframes);

private:
    jack_client_t* _jack;
    jack_port_t* _outputPorts[2];
    std::string _error;
    uint32_t _sampleRate;

public:
    static int _JackProcessCallback (jack_nframes_t nframes, void *arg);
};

#endif // REJACKAUDIOENGINE_H
