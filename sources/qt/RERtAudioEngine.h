#ifndef RERTAUDIOENGINE_H
#define RERTAUDIOENGINE_H

#include <RETypes.h>

#include <REAudioEngine.h>
#include <RtAudio.h>

class RERtAudioEngine : public REAudioEngine
{
public:
    static RERtAudioEngine* Instance();

public:
    virtual void Initialize(const REAudioSettings& settings);
    virtual void Shutdown();

    virtual void StartRendering();
    virtual void StopRendering();

protected:
    RERtAudioEngine();
    virtual ~RERtAudioEngine();

private:
    RtAudio _rtAudio;
    unsigned long _dumpFrameCounter;

private:
    int RenderCallback( void *outputBuffer, void *inputBuffer, unsigned int nBufferFrames,
                        double streamTime, RtAudioStreamStatus status, void *userData );

public:
    static int _RERtAudioEngineCallback( void *outputBuffer, void *inputBuffer, unsigned int nBufferFrames,
                    double streamTime, RtAudioStreamStatus status, void *userData );
};

#endif // RERTAUDIOENGINE_H
