//
//  RESequencer.h
//  Reflow
//
//  Created by Sebastien on 26/05/11.
//  Copyright 2011 Gargant Studios. All rights reserved.
//

#ifndef _RESEQUENCER_H_
#define _RESEQUENCER_H_

#include "RETypes.h"
#include "RESongController.h"
#include "RETimeline.h"
#include "REMidiClip.h"

#include <mutex>

class RESequencerImpl;
class RESynthMusicDevice;
class REAudioEngine;

/** RESequencerClip class.
 */
/*class RESequencerClip
{
    friend class RESequencer;
    friend class RESequencerTrack;
    
public:
    RESequencerClip();
    ~RESequencerClip();
    
private:
    REMidiClip* _clip;
};

typedef std::vector<RESequencerClip*> RESequencerClipVector;*/



/** RESequencerTrack class.
 */
class RESequencerTrack
{
    friend class RESequencer;
    
public:
    ~RESequencerTrack();
    
public:
    int32_t DeviceUUID() const {return _deviceUUID;}
    
    const REMidiClip* Clip(int barIndex) const;
    
private:
    int32_t _index;
    int32_t _deviceUUID;
    int32_t _midiProgram;
    int32_t _initialMidiProgram;
    bool _midiProgramChangeRequested;
    bool _mute;
    bool _solo;
    float _volume;
    float _pan;
    int8_t _capo;
    std::string _trackName;
    REMidiClipVector _clips;
    RESynthMusicDevice* _device;
};

typedef std::vector<RESequencerTrack*> RESequencerTrackVector;



/** RESequencer class.
 */
class RESequencer : public REMusicRackDelegate, public RESongControllerDelegate
{
public:  
    RESequencer();
    virtual ~RESequencer();
    
public:
    void Build(const RESong* song);
    void Build(const RESong* song, REAudioEngine* audioEngine);
    void Shutdown();
    
    void StartPlayback();
    void StopPlayback();
    
    bool IsPlaybackFinished() const;
    
    void JumpTo(int barIndex, int tickInBar);

    void SetLoopPlaybackEnabled(bool loopPlayback);
    bool IsLoopPlaybackEnabled() const;
    
    void SetLoopStartIndicator(int barIndex, int tickInBar);
    void SetLoopEndIndicator(int barIndex, int tickInBar);
    
    int BarOfLoopStartIndicator() const;
    int TickInBarOfLoopStartIndicator() const;
    int BarOfLoopEndIndicator() const;
    int TickInBarOfLoopEndIndicator() const;
    
    void SetPreclickBarCount(int barCount);
    int PreclickBarCount() const;
    
    bool IsInitialized() const;
    bool IsRunning() const;
    
    void AddListener(RESequencerListener* listener);
    void RemoveListener(RESequencerListener* listener);
    
    int BarIndexThatsCurrentlyPlaying() const;
    int NextBarIndexThatShouldBePlaying() const;
    unsigned long TickInBarPlaying() const;
    unsigned long TickInPlaylist() const;
    
    void ExportMidiToFile(const std::string& filename) const;
    void SetMergeChannelsOnExport(bool merge);
    
    float SampleRate() const;
    double CurrentBPM() const;
    
    void SetPlaybackRate(double playbackRate);
    double PlaybackRate() const;
    
    std::mutex& RenderingMutex();
    
    const REMusicRack* Rack() const {return _rack;}
    REMusicRack* Rack() {return _rack;}
    
    void SetTrackVolume(int trackIndex, float volume);
    void SetTrackPan(int trackIndex, float pan);
    void SetTrackSolo(int trackIndex, bool solo);
    void SetTrackMute(int trackIndex, bool mute);    
    void SetTrackMidiProgram(int trackIndex, int midiProgram);
    void SetTrackCapo(int trackIndex, int capo);
    
public: // [[CALLED FROM AUDIO RT THREAD]]
    virtual void WillRenderRack (REMusicRack* rack, unsigned int nbFrames, float* workBufferL, float* workBufferR);
    virtual void DidRenderRack (REMusicRack* rack, unsigned int nbFrames, float* workBufferL, float* workBufferR);
    
    virtual void WillRenderDevice (REMusicRack* rack, REMusicDevice* device, unsigned int nbFrames, float* workBufferL, float* workBufferR);
    virtual void DidRenderDevice (REMusicRack* rack, REMusicDevice* device, unsigned int nbFrames, float* workBufferL, float* workBufferR);
    
public: // [[CALLED FROM MAIN UI THREAD]]
    virtual void SongControllerWillModifySong(const RESongController* controller, const RESong* song);
    virtual void SongControllerDidModifySong(const RESongController* controller, const RESong* song, bool successfully);
    virtual void SongControllerDidModifyPhrase(const RESongController* controller, const REPhrase* phrase, bool successfully);
    
protected:
    REMusicDevice* DeviceForTrack(const RETrack* track);
    
    const REPlaylistBar* FirstOccurenceOfBarInPlaylist(int barIndex) const;
    unsigned long PlaylistDurationInTicks() const;
    
    void _ApplyDeltaTicksToPlaylistForBarIndex(REPlaylistBarVector& playlist, int barIndex, int deltaTicksBefore, int deltaTicksAfter);
    void _RebuildSequencer(const RESong*);
    void _RenderTickRange(double t0, double t1, int sampleDelay);
    void _RenderMetronomeClicks(double t0, double t1, const RETimeSignature& ts, REMusicDevice* metronomeDevice, int sampleDelay);
    void _RenderMetronomeSubclicks(double ratio, double volume, double t0, double t1,  REMusicDevice *metronomeDevice, REIntSet& clickDelays, int sampleDelay);
    double _ApplyPreclickDelay(unsigned int nbFrames);
    
    void GenerateMidiTempoData(REOutputStream& data) const;
    void GenerateMidiTrackData(REOutputStream& data, const RESequencerTrack* track) const;
    
private:
    const RESong* _song;
    REAudioEngine* _audioEngine;
    REMusicRack* _rack;
    std::mutex _renderMutex;
    int32_t _nextUUID;
    RESequencerImpl* const _d;
    bool _mergeChannelsOnExport;
    
    // Calculated from Song
    RETempoTimeline* _tempoTimeline;
    REPlaylistBarVector* _playlist;
    RESequencerTrackVector* _tracks;
};


#endif
