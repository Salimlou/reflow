//
//  RESequencer.cpp
//  Reflow
//
//  Created by Sebastien on 26/05/11.
//  Copyright 2011 Gargant Studios. All rights reserved.
//

#include "RESequencer.h"
#include "RESong.h"
#include "RETrack.h"
#include "REBar.h"
#include "REVoice.h"
#include "REPhrase.h"
#include "REPlaylistBar.h"
#include "REMidiClip.h"
#include "REMusicDevice.h"
#ifdef REFLOW_QT
#  include "RERtAudioEngine.h"
#elif defined(REFLOW_MAC) || defined(REFLOW_IOS)
#  include "RECoreAudioEngine.h"
#else
#  include "REAudioEngine.h"
#endif
#include "REMusicRack.h"
#include "RESoundFontManager.h"
#include "REFunctions.h"

#include <boost/foreach.hpp>
#include <boost/bind.hpp>

class RESequencerImpl
{
public:  
    double playbackTime;
    int currentBarIndexInPlaylist;
    int currentBarIndexInSong;
    unsigned long currentTickInBar;
    unsigned long currentTickInPlaylist;
    unsigned long framesSinceLastUpdateRender;
    RESequencerListenerVector _listeners;
    
    double preclickTime;
    double preclickDuration;
    int preclickBarCount;
    RETimeSignature preclickTimeSignature;
    
    bool loopPlayback;
    double loopStartTimeInPlaylist;
    double loopEndTimeInPlaylist;
    
    int loopStartBarIndex, loopEndBarIndex, loopStartTickInBar, loopEndTickInBar;
    
    double playbackRate;
    double bpm;
    double newBpm;      // This is modified when _RenderTickRange finds a Tempo Marker

    double sampleRate;
    int channelCount;
    bool running;
    
    double TicksFromSamples(double samples) const
    {
        return (samples * bpm * playbackRate) / (sampleRate * 60.0);
    }
    
    double SamplesFromTicks(double ticks) const 
    {
        return (ticks * 60.0 * sampleRate) / (bpm * playbackRate);
    }
    
public:
    ~RESequencerImpl() {
        _listeners.clear();
    }
};



RESequencerTrack::~RESequencerTrack()
{
    BOOST_FOREACH(REMidiClip* clip, _clips) {
        delete clip;
    }
    _clips.clear();
}






RESequencer::RESequencer()
: _d(new RESequencerImpl), _audioEngine(NULL), _song(0), _rack(NULL), _tempoTimeline(NULL),
  _playlist(NULL), _tracks(NULL), _nextUUID(1), _mergeChannelsOnExport(false)
{
    _d->running = false;
    _d->loopPlayback = false;
    _d->loopStartTimeInPlaylist = 0.0;
    _d->loopEndTimeInPlaylist = 4.0;
    _d->playbackRate = 1.0;
}

std::mutex& RESequencer::RenderingMutex()
{
    return _renderMutex;
}

float RESequencer::SampleRate() const
{
    return _d->sampleRate;
}

double RESequencer::CurrentBPM() const
{
    return _d->bpm;
}

void RESequencer::SetPreclickBarCount(int barCount)
{
    _d->preclickBarCount = barCount;
}
int RESequencer::PreclickBarCount() const
{
    return _d->preclickBarCount;
}

RESequencer::~RESequencer()
{
    delete _playlist;
    
    if(_tracks) {
        BOOST_FOREACH(RESequencerTrack* track, *_tracks) {delete track;}
        delete _tracks;
    }
    
    delete _d;
}

void RESequencer::_ApplyDeltaTicksToPlaylistForBarIndex(REPlaylistBarVector& playlist, int barIndex, int deltaTicksBefore, int deltaTicksAfter)
{
    for(int pbarIndex = 0; pbarIndex < playlist.size(); ++pbarIndex)
    {
        REPlaylistBar& pbar = playlist[pbarIndex];
        if(pbar.IndexInSong() == barIndex)
        {
            if(pbar._tick - deltaTicksBefore < pbar._extendedTick) {
                pbar._extendedTick = pbar._tick - deltaTicksBefore;
            }
            
            int extraTicksBefore = pbar._tick - pbar._extendedTick;
            int newDuration = (extraTicksBefore + pbar._duration + deltaTicksAfter);
            if(newDuration > pbar._extendedDuration) {
                pbar._extendedDuration = newDuration;
            }
        }
    }
}

void RESequencer::SetTrackVolume(int trackIndex, float volume)
{
    if(_tracks == NULL) return;
    
    if(trackIndex >= 0 && trackIndex < _tracks->size()) {
        RESequencerTrack* track = _tracks->at(trackIndex);
        if(track) {
            track->_volume = volume;
        }
    }
}
void RESequencer::SetTrackPan(int trackIndex, float pan)
{
    if(_tracks == NULL) return;
    
    if(trackIndex >= 0 && trackIndex < _tracks->size()) {
        RESequencerTrack* track = _tracks->at(trackIndex);
        if(track) {
            track->_pan = pan;
        }
    }    
}
void RESequencer::SetTrackSolo(int trackIndex, bool solo)
{
    if(_tracks == NULL) return;
    
    if(trackIndex >= 0 && trackIndex < _tracks->size()) {
        RESequencerTrack* track = _tracks->at(trackIndex);
        if(track) {
            track->_solo = solo;
        }
    }    
}
void RESequencer::SetTrackMute(int trackIndex, bool mute)
{
    if(_tracks == NULL) return;
    
    if(trackIndex >= 0 && trackIndex < _tracks->size()) {
        RESequencerTrack* track = _tracks->at(trackIndex);
        if(track) {
            track->_mute = mute;
        }
    }    
}
void RESequencer::SetTrackMidiProgram(int trackIndex, int midiProgram)
{
    if(_tracks == NULL) return;
    
    if(trackIndex >= 0 && trackIndex < _tracks->size()) {
        RESequencerTrack* track = _tracks->at(trackIndex);
        if(track) {
            track->_midiProgram = midiProgram;
            track->_midiProgramChangeRequested = true;
        }
    }        
}
void RESequencer::SetTrackCapo(int trackIndex, int capo)
{
    if(_tracks == NULL) return;
    
    if(trackIndex >= 0 && trackIndex < _tracks->size()) {
        RESequencerTrack* track = _tracks->at(trackIndex);
        if(track) {
            track->_capo = capo;
        }
    }        
}

void RESequencer::_RebuildSequencer(const RESong* song)
{
    if(!IsInitialized()) return;
    
    REPrintf("_RebuildSequencer\n");
    
    // Calculate New Playlist
    REPlaylistBarVector* playlist = song->ClonePlaylist();
    
    // Clone the tempo timeline
    RETempoTimeline* tempoTimeline = song->TempoTimeline().Clone();
    
    // Calculate New Sequencer Tracks
    RESequencerTrackVector* tracks = new RESequencerTrackVector;
    for(int trackIndex=0; trackIndex < song->TrackCount(); ++trackIndex)
    {
        const RETrack* track = song->Track(trackIndex);
        RESequencerTrack* seqTrack = new RESequencerTrack;
        {
            seqTrack->_index = trackIndex;
            seqTrack->_mute = track->IsMute();
            seqTrack->_solo = track->IsSolo();
            seqTrack->_volume = track->Volume();
            seqTrack->_pan = track->Pan();
            seqTrack->_capo = track->Capo();
            seqTrack->_midiProgram = track->MIDIProgram();
            seqTrack->_initialMidiProgram = track->MIDIProgram();
            seqTrack->_midiProgramChangeRequested = false;
            seqTrack->_trackName = track->Name();
            
            if(track->_deviceUUID == -1 && _audioEngine != NULL) {
                track->_deviceUUID = _nextUUID++;
            }
            seqTrack->_deviceUUID = track->_deviceUUID;
            
            // Calculate Clips
            for(int barIndex=0; barIndex<song->BarCount(); ++barIndex)
            {
                const REBar* bar = song->Bar(barIndex);
                
                REMidiClip* clip = track->CalculateMidiClipForBar(barIndex);
                int deltaTicksBefore = 0;
                int deltaTicksAfter = 0;
                if(clip->MinTick() < 0) {
                    deltaTicksBefore = abs(clip->MinTick());
                }
                if(clip->MaxTick() > bar->TheoricDurationInTicks()) {
                    deltaTicksAfter = (clip->MaxTick() - bar->TheoricDurationInTicks());
                }
                if(deltaTicksBefore || deltaTicksAfter) {
                    _ApplyDeltaTicksToPlaylistForBarIndex(*playlist, barIndex, deltaTicksBefore, deltaTicksAfter);
                }
                seqTrack->_clips.push_back(clip);
            }
            
            REPrintf("Track (%d) has Device UUID (%d)\n", trackIndex, seqTrack->_deviceUUID);

            // Try to retrieve an existing device in Rack
            int deviceIndex = _rack->IndexOfMusicDeviceWithUUID(seqTrack->_deviceUUID);
            if(deviceIndex != -1){
                seqTrack->_device = static_cast<RESynthMusicDevice*>(_rack->Device(deviceIndex));
                REPrintf("  > found device at index %d\n", deviceIndex);
            }
            else {
                seqTrack->_device = static_cast<RESynthMusicDevice*>(_rack->_NewMusicDevice(seqTrack->_deviceUUID));
                if(_audioEngine) {
                    seqTrack->_device->SetSoundFont(_audioEngine->SoundFont());
                }
                else {
                    seqTrack->_device->SetSoundFont(RESoundFontManager::Instance().DefaultSoundFont());
                }

                REPrintf("  > did not found any device\n");
            }
            
            // MIDI Patch
            for(int channel = 0; channel < 16; ++channel)
            {
                int program = (track->IsDrums() ? 0 : track->MIDIProgram());
                int bank = (track->IsDrums() ? 128 : 0);
                seqTrack->_device->ProcessControllerEvent(channel, 0x00, bank);
                seqTrack->_device->ProcessProgramChangeEvent(channel, program);
                
                // If the track is a guitar, channel should be set to monophonic (one channel per string)
                if(track->IsTablature()) {
                    RESynthChannel* chan = seqTrack->_device->Channel(channel);
                    if(chan) chan->SetMonophonic(true);
                }
            }
        }
        tracks->push_back(seqTrack);
    }
    
    // Delete Old Devices
    REMusicDeviceVector devicesToDelete;
    for(int deviceIndex=0; deviceIndex < _rack->DeviceCount(); ++deviceIndex)
    {
        REMusicDevice* device = _rack->Device(deviceIndex);
        bool found = false;
        for(int trackIndex=0; trackIndex < tracks->size(); ++trackIndex)
        {
            RESequencerTrack* seqTrack = tracks->at(trackIndex);
            if(seqTrack->_deviceUUID == device->UUID()) {
                found = true;
            }
        }
        
        if(!found) {
            devicesToDelete.push_back(device);
        }
    }
    
    // Critical Swap
    {
        REMusicRack::MutexLocker lock_the_rack_(_rack->Mutex());
        
        std::swap(_playlist, playlist);
        std::swap(_tempoTimeline, tempoTimeline);
        std::swap(_tracks, tracks);
        
        // Restore the Devices in the Rack
        _rack->_devices.clear();
        for(int trackIndex=0; trackIndex < _tracks->size(); ++trackIndex) {
            _rack->_devices.push_back(_tracks->at(trackIndex)->_device);
        }
    }
    
    // Delete old stuff
    delete playlist;
    delete tempoTimeline;
    if(tracks) {
        BOOST_FOREACH(RESequencerTrack* track, *tracks) {delete track;}
        delete tracks;
    }
    
    // Delete unused devices
    BOOST_FOREACH(REMusicDevice* device, devicesToDelete) {
        delete device;
    }
}

void RESequencer::SongControllerWillModifySong(const RESongController* controller, const RESong* song)
{
    // Nothing to do
}

void RESequencer::SongControllerDidModifySong(const RESongController* controller, const RESong* song, bool successfully)
{
    _RebuildSequencer(song);
}

void RESequencer::SongControllerDidModifyPhrase(const RESongController* controller, const REPhrase* phrase, bool successfully)
{
	SongControllerDidModifySong(controller, phrase->Voice()->Track()->Song(), successfully);
}

const REPlaylistBar* RESequencer::FirstOccurenceOfBarInPlaylist(int barIndex) const
{
    if(!_playlist) return NULL;
    
    for(unsigned int i=0; i<_playlist->size(); ++i) 
    {
        const REPlaylistBar* pbar = &_playlist->at(i);
        if(pbar->IndexInSong() == barIndex) {
            return pbar;
        }
    }
    return NULL;
}

unsigned long RESequencer::PlaylistDurationInTicks() const
{
    if(_playlist == NULL || _playlist->empty()) {
        return 0;
    }
    
    const REPlaylistBar& pbar = _playlist->back();
    return pbar._tick + pbar._duration;
}

bool RESequencer::IsInitialized() const
{
    return _rack != NULL;
}

void RESequencer::Build(const RESong *song) 
{
    RESequencer::Build(song, REAudioEngine::Instance());
}

void RESequencer::Build(const RESong *song, REAudioEngine* audioEngine)
{
    if(IsInitialized()) return;
    
    _audioEngine = audioEngine;
    double sampleRate = (_audioEngine ? _audioEngine->SampleRate() : 44100.0);
    
    // Create the Rack
    _song = song;
    _rack = new REMusicRack;
    _rack->SetSampleRate(sampleRate);
    _rack->SetDelegate(this);
    
    // Create the rack device for playing metronome ticks
    RESynthMusicDevice* metronomeDevice = _rack->CreateMetronomeDevice();
    metronomeDevice->SetSoundFont(_audioEngine ? _audioEngine->SoundFont() : RESoundFontManager::Instance().DefaultSoundFont());
    metronomeDevice->SetMidiProgramOfAllChannels(0, 128);
    
	// Rebuild the Sequencer from song
    _RebuildSequencer(song);
    
    
    const RETempoItem* tempoItem = (_tempoTimeline ? _tempoTimeline->ItemAt(0, RETimeDiv(0)) : NULL);
    _d->sampleRate = _rack->SampleRate();
    _d->channelCount = 2;
    _d->bpm = (tempoItem != NULL ? tempoItem->BeatsPerMinute() : 90.0);
    _d->playbackRate = 1.0;
    _d->playbackTime = 0.0;
    _d->currentBarIndexInPlaylist = 0;
    _d->currentBarIndexInSong = 0;
    _d->currentTickInBar = 0;
    _d->currentTickInPlaylist = 0;
    
    //std::cout << "[RESequencer::Build] Format: " << _d->sampleRate << ", " << _d->channelCount << " channels" << std::endl;
    
    if(_audioEngine) _audioEngine->AddRack(_rack);
}

void RESequencer::Shutdown() 
{
    if(!IsInitialized()) return;
    
    //std::cout << "[RESequencer::Shutdown]" << std::endl;
    
    // Destroy the Rack
    if(_audioEngine) {
        _audioEngine->RemoveRack(_rack);
    }
    delete _rack;
    _rack = NULL;
    
    _d->running = false;

    for(RESequencerListener* listener : _d->_listeners) {
        listener->OnSequencerUpdateRT(this);
    }
}

void RESequencer::StartPlayback() 
{
    //std::cout << "[RESequencer::StartPlayback]" << std::endl;
    _rack->SetRenderingEnabled(true);
    _d->framesSinceLastUpdateRender = 0;
    _d->running = true;
}

void RESequencer::StopPlayback() 
{
    //std::cout << "[RESequencer::StopPlayback]" << std::endl;
    if(_rack) _rack->SetRenderingEnabled(false);
}

void RESequencer::SetPlaybackRate(double playbackRate)
{
    _d->playbackRate = playbackRate;
}

double RESequencer::PlaybackRate() const
{
    return _d->playbackRate;
}

void RESequencer::JumpTo(int barIndex, int tickInBar)
{
    // CRITICAL SECTION: Do not jump while the Rack is processing
    REMusicRack::MutexLocker lock_the_rack_(_rack->Mutex());
    
    const REAudioSettings& audioSettings = (_audioEngine ? _audioEngine->AudioSettings() : REAudioSettings::DefaultAudioSettings());
        
    const REPlaylistBar* pbar = FirstOccurenceOfBarInPlaylist(barIndex);
    if(pbar == NULL) return;
    
    _d->playbackTime = (double)(pbar->Tick() + tickInBar) / REFLOW_PULSES_PER_QUARTER;
    _d->currentBarIndexInPlaylist = pbar->IndexInPlaylist();
    _d->currentBarIndexInSong = pbar->IndexInSong();
    _d->currentTickInBar = (unsigned long)(tickInBar * (double)REFLOW_PULSES_PER_QUARTER);

    if(_tempoTimeline)
    {
        RETimeDiv timeDiv = Reflow::TicksToTimeDiv(_d->currentTickInBar);
        int itemIdx = _tempoTimeline->IndexOfItemAt(_d->currentBarIndexInSong, timeDiv, NULL);
        if(itemIdx != -1) {
            _d->bpm = _d->newBpm = (double)(_tempoTimeline->Item(itemIdx)->tempo);
        }
    }
    
    // Preclick
    _d->preclickBarCount = audioSettings.PreclickBarCount();
    if(_d->preclickBarCount > 0)
    {
        _d->preclickTime = 0;
        _d->preclickTimeSignature = pbar->TimeSignature();
        _d->preclickDuration = (4.0 * (double)_d->preclickTimeSignature.numerator) / (double)_d->preclickTimeSignature.denominator;
        _d->preclickDuration *= (double)_d->preclickBarCount;
    }
}

void RESequencer::SetLoopPlaybackEnabled(bool loopPlayback)
{
    _d->loopPlayback = loopPlayback;
}
bool RESequencer::IsLoopPlaybackEnabled() const
{
    return _d->loopPlayback;
}
void RESequencer::SetLoopStartIndicator(int barIndex, int tickInBar)
{
    const REPlaylistBar* pbar = FirstOccurenceOfBarInPlaylist(barIndex);
    if(pbar == NULL) return;
    
    _d->loopStartTimeInPlaylist = (double)(pbar->Tick() + tickInBar) / REFLOW_PULSES_PER_QUARTER;
    _d->loopStartBarIndex = barIndex;
    _d->loopStartTickInBar = tickInBar;
}
void RESequencer::SetLoopEndIndicator(int barIndex, int tickInBar)
{
    const REPlaylistBar* pbar = FirstOccurenceOfBarInPlaylist(barIndex);
    if(pbar == NULL) return;
    
    _d->loopEndTimeInPlaylist = (double)(pbar->Tick() + tickInBar) / REFLOW_PULSES_PER_QUARTER;    
    _d->loopEndBarIndex = barIndex;
    _d->loopEndTickInBar = tickInBar;
}

int RESequencer::BarOfLoopStartIndicator() const
{
    return _d->loopStartBarIndex;
}
int RESequencer::TickInBarOfLoopStartIndicator() const
{
    return _d->loopStartTickInBar;
}
int RESequencer::BarOfLoopEndIndicator() const
{
    return _d->loopEndBarIndex;
}
int RESequencer::TickInBarOfLoopEndIndicator() const
{
    return _d->loopEndTickInBar;
}

bool RESequencer::IsPlaybackFinished() const
{
    if(IsLoopPlaybackEnabled()) return false;
    
    return (_d->currentTickInPlaylist >= _song->PlaylistDurationInTicks());
}

const REMidiClip* RESequencerTrack::Clip(int barIndex) const
{
    if(barIndex >= 0 && barIndex < _clips.size())
    {
        return _clips[barIndex];
    }
    return NULL;
}

void RESequencer::_RenderMetronomeSubclicks(double ratio, double volume, double t0, double t1, REMusicDevice *metronomeDevice, REIntSet& clickDelays, int sampleDelay)
{
    const int clickMidi = 33;
    
    double i0 = (t0 * 480.0 * ratio);
    double i1 = (t1 * 480.0 * ratio);
    int x0 = (int)i0/480;
    int x1 = (int)i1/480;
    if(t0 <= 0.0) --x0;
    
    if(x0 < x1)
    {
        for(int x=(x0+1); x<=x1; ++x)
        {
            double t = (double)x1 / ratio;
            uint32_t delay = (uint32_t)((_d->sampleRate * (t - t0) * 60.0) / _d->bpm);
            if(clickDelays.find(delay) == clickDelays.end())
            {
                // Note On
                if(metronomeDevice) {
                    metronomeDevice->MidiEvent(0x90 | 10, clickMidi, (int)(volume * 127.0), delay + sampleDelay);
                    
                    uint32_t delayOff = delay + (uint32_t)((_d->sampleRate * (0.20) * 60.0) / _d->bpm);
                    metronomeDevice->MidiEvent(0x80 | 10, clickMidi, 0, delayOff + sampleDelay);
                }
                
                REPrintf("Click [Ratio: %1.2f] (x0: %d) (x1: %d) (t0: %f) (t1: %f) (t: %f) (delay: %d)\n", ratio, x0, x1, t0, t1, t, delay);
                clickDelays.insert(delay);
            }
        }
    }
}

// t0 and t1 are in bar range ([0 .. 4[ for a 4:4 bar)
void RESequencer::_RenderMetronomeClicks(double t0, double t1, const RETimeSignature& ts, REMusicDevice* metronomeDevice, int sampleDelay)
{
    REIntSet clickDelays;
    
    const REAudioSettings& audioSettings = (_audioEngine ? _audioEngine->AudioSettings() : REAudioSettings::DefaultAudioSettings());
    
    double barClickVolume = audioSettings.MetronomeBarClickVolume();
    double quarterClickVolume = audioSettings.MetronomeQuarterClickVolume();
    double eighthClickVolume = audioSettings.MetronomeEighthClickVolume();
    double tripletClickVolume = audioSettings.MetronomeTripletClickVolume();
    double sixteenthClickVolume = audioSettings.MetronomeSixteenthClickVolume();
    
    const double volumeEpsilon = 0.01;
    bool processBarClick = barClickVolume > volumeEpsilon;
    bool processQuarterClick = quarterClickVolume > volumeEpsilon;
    bool processEighthClick = eighthClickVolume > volumeEpsilon;
    bool processTripletClick = tripletClickVolume > volumeEpsilon;
    bool processSixteenthClick = sixteenthClickVolume > volumeEpsilon;
    
    int barClickMidi = 56;
    
    if(processBarClick)
    {
        if(t0 <= 0.0)
        {
            // Bar Click
            double t = 0.0;
            uint32_t delay = (uint32_t)((_d->sampleRate * (t - t0) * 60.0) / _d->bpm);
            uint32_t delayOff = delay + (uint32_t)((_d->sampleRate * (0.20) * 60.0) / _d->bpm);
            
            // Note On and off
            if(metronomeDevice) {
                metronomeDevice->MidiEvent(0x90 | 10, barClickMidi, (int)(barClickVolume * 127.0), delay + sampleDelay);
                metronomeDevice->MidiEvent(0x80 | 10, barClickMidi, 0, delayOff + sampleDelay);
            }
            
            REPrintf("Bar Click (Offset: %f) ## t0 in bar: %f ## t1 in bar: %f ## delay: %d\n", t, t0, t1, delay);
            clickDelays.insert(delay);
        }
    }
    
    // Quarter notes
    if(processQuarterClick) {
        _RenderMetronomeSubclicks(1.0, quarterClickVolume, t0, t1, metronomeDevice, clickDelays, sampleDelay);
    }
    
    // Eighth notes
    if(processEighthClick) {
        _RenderMetronomeSubclicks(2.0, eighthClickVolume, t0, t1, metronomeDevice, clickDelays, sampleDelay);
    }
    
    // Triplet notes
    if(processTripletClick) {
        _RenderMetronomeSubclicks(3.0, tripletClickVolume, t0, t1, metronomeDevice, clickDelays, sampleDelay);
    }
    
    // Sixteenth notes
    if(processSixteenthClick) {
        _RenderMetronomeSubclicks(4.0, sixteenthClickVolume, t0, t1, metronomeDevice, clickDelays, sampleDelay);
    }
}

void RESequencer::_RenderTickRange(double t0, double t1, int sampleDelay)
{
    if(!_playlist) return;
    
    const REPlaylistBarVector& playlist = *_playlist;
    unsigned int playlistSize = playlist.size();
    for(unsigned int barIndexInPlaylist=0; barIndexInPlaylist<playlistSize; ++barIndexInPlaylist)
    {
        const REPlaylistBar& pbar = playlist[barIndexInPlaylist];
        bool extended = false;
        if(pbar.IntersectsTickRange(t0, t1, &extended))
        {
            double offset = (double)pbar.Tick() / (double)REFLOW_PULSES_PER_QUARTER;
            if(!extended)
            {
                _d->currentBarIndexInPlaylist = pbar.IndexInPlaylist();
                _d->currentBarIndexInSong = pbar.IndexInSong();
                _d->currentTickInBar = (unsigned long)((t1-offset) * (double)REFLOW_PULSES_PER_QUARTER);
            }
            
            // Render every track
            for(unsigned int trackIndex=0; trackIndex<_tracks->size(); ++trackIndex) 
            {
                const RESequencerTrack* track = (*_tracks)[trackIndex];
                REMusicDevice* device = track->_device;
                if(!device) continue;    
                
                const REMidiClip* clip = track->Clip(pbar.IndexInSong());
                if(clip == NULL) continue;
                
                int dpitch = track->_capo;
                
                // Events
                unsigned int nbEvents = clip->EventCount();
                for(unsigned int i=0; i<nbEvents; ++i)
                {
                    const REMidiEvent& evt = clip->Event(i);
                    double t = offset + (double)evt.tick / (double)REFLOW_PULSES_PER_QUARTER;
                    if(t0 <= t && t < t1) 
                    {
                        uint32_t delay = (uint32_t)((_d->sampleRate * (t - t0) * 60.0) / _d->bpm);
                        device->MidiEvent(evt.data[0],
                                          evt.data[1],
                                          evt.data[2],
                                          delay + sampleDelay);
                        
                    }
                }

                // Note Events
                unsigned int nbNoteEvents = clip->NoteEventCount();
                for(unsigned int i=0; i<nbNoteEvents; ++i)
                {
                    const REMidiNoteEvent& evt = clip->NoteEvent(i);
                    double t = offset + (double)evt.tick / (double)REFLOW_PULSES_PER_QUARTER;
                    if(t0 <= t && t < t1) 
                    {
                        double tlast = offset + (double)(evt.tick + evt.duration) / (double)REFLOW_PULSES_PER_QUARTER;
                        
                        // Note On
                        uint32_t delay = (uint32_t)((_d->sampleRate * (t - t0) * 60.0) / _d->bpm);
                        device->MidiEvent(0x90 | evt.channel, 
                                          evt.pitch + dpitch, 
                                          evt.velocity, 
                                          delay + sampleDelay);
                        
                        // Note Off
                        delay = (uint32_t)((_d->sampleRate * (tlast - t0) * 60.0) / _d->bpm);
                        if(evt.flags & REMidiNoteEvent::UseSoundOff)
                        {
                            device->MidiEvent(0xB0 | evt.channel,
                                              120,
                                              0,
                                              delay + sampleDelay);
                        }
                        else {
                            device->MidiEvent(0x80 | evt.channel,
                                              evt.pitch + dpitch,
                                              0,
                                              delay + sampleDelay);
                        }
                    }
                }
            }
            
            // Render Metronome
            {
                REMusicDevice* metronomeDevice = Rack()->MetronomeDevice();
                _RenderMetronomeClicks(t0-offset, t1-offset, pbar.TimeSignature(), metronomeDevice, sampleDelay);
            }
            
            
            // Look for a Tempo marker in this tick range
            if(_tempoTimeline)
            {
                RETimeDiv timeDiv = Reflow::TicksToTimeDiv(_d->currentTickInBar);
                int itemIdx = _tempoTimeline->IndexOfItemAt(_d->currentBarIndexInSong, timeDiv, NULL);
                if(itemIdx != -1)
                {
                    const RETempoItem* tempoItem = _tempoTimeline->Item(itemIdx);
                    _d->newBpm = tempoItem->BeatsPerMinute();
                }
            }
        }
    }
}

double RESequencer::_ApplyPreclickDelay(unsigned int nbFrames)
{
    if(_d->preclickBarCount == 0 || (_d->preclickTime >= _d->preclickDuration)) {
        return 0.0;
    }
    
    double ticksToRender = _d->TicksFromSamples(nbFrames);
    double remainingTicksInPreclick = _d->preclickDuration - _d->preclickTime;
    double ticksInPreclick = std::min<double>(ticksToRender, remainingTicksInPreclick);
    
    double oneBarDuration = _d->preclickDuration / (double)_d->preclickBarCount;
    double t0 = _d->preclickTime;
    double t1 = t0 + ticksInPreclick;
    if(t1 > oneBarDuration && t0 <= oneBarDuration) {
        _d->preclickTime = t1;
        t0 -= oneBarDuration;
        t1 -= oneBarDuration;
    }
    else
    {
        _d->preclickTime = t1;
    }
    
    // Play Metronome
    {
        REMusicDevice* metronomeDevice = Rack()->MetronomeDevice();
        _RenderMetronomeClicks(t0, t1, _d->preclickTimeSignature, metronomeDevice, 0);
    }
    
       // Is preclick finished ?
    if(remainingTicksInPreclick <= ticksToRender)
    {
        _d->preclickTime = _d->preclickDuration;
    }
       
    return ticksInPreclick;
}

void RESequencer::WillRenderRack (REMusicRack* rack, unsigned int nbFrames, float* workBufferL, float* workBufferR)
{
    _d->newBpm = _d->bpm;
    double ticksToRender = _d->TicksFromSamples(nbFrames);
    double ticksInPreclick = _ApplyPreclickDelay(nbFrames);
    int sampleDelay = (ticksInPreclick > 0 ? _d->SamplesFromTicks(ticksInPreclick) : 0);
    ticksToRender -= ticksInPreclick;
    double t0 = _d->playbackTime;
    double t1 = _d->playbackTime + ticksToRender;
    double e = _d->loopEndTimeInPlaylist;
    double s = _d->loopStartTimeInPlaylist;
    const REPlaylistBarVector& playlist = *_playlist;
    unsigned int playlistSize = playlist.size();
    
    // CRITICAL: Lock the Sequencer
    //  Operations on Main Thread that can be performed while playback is running (add notes, delete notes, ...)
    //  are likely to modify the MidiClip contained in the Song
    //
    // Note: Operations that change playlist, track or bar configuration are not allowed.
    //      Only phrase-level modifications can be performed.
    if(ticksToRender > 0.0)
    {
        std::lock_guard<std::mutex> prevent_midi_clips_modifications_while_rendering_(RenderingMutex());
        
        if(_d->loopPlayback && t0 <= e && e <= t1) { 
            _RenderTickRange(t0, e, sampleDelay);
            t1 = s + (t1-e);
            _RenderTickRange(s, t1, sampleDelay);
        }
        else {
            _RenderTickRange(t0, t1, sampleDelay);
        }
    }
    
    _d->bpm = _d->newBpm;
    _d->playbackTime = t1;
    _d->currentTickInPlaylist = (unsigned long)(t1 * (double)REFLOW_PULSES_PER_QUARTER);
    
    static int dumpCounter = 0;
    if(0 == dumpCounter++ % 128) {
        REPrintf("44100 samples => %1.3f Ticks\n", _d->TicksFromSamples(44100));
        REPrintf("nbFrames: %d\n", nbFrames);
        REPrintf("ticks to render: %1.3f\n", ticksToRender);
        REPrintf("playback time: %1.3f\n", _d->playbackTime);
        REPrintf("current bar: %d\n", _d->currentBarIndexInSong);
        REPrintf("current tick: %lu\n", _d->currentTickInBar);
        
        double completion = (double)_d->currentTickInPlaylist / (double)_song->PlaylistDurationInTicks();
        REPrintf("song complete ratio: %1.2f%%\n", completion*100.0);
    }
    
    // Stop playback if finished
    /*if(_d->currentTickInPlaylist >= _song->PlaylistDurationInTicks() && !_d->loopPlayback) {
        _d->running = false;
    }*/
    
    unsigned long nbFramesPerRefresh = _d->sampleRate / 30.0;
    _d->framesSinceLastUpdateRender += nbFrames;
    if(_d->framesSinceLastUpdateRender > nbFramesPerRefresh) {
        _d->framesSinceLastUpdateRender = 0;
        BOOST_FOREACH(RESequencerListener* listener, _d->_listeners) {
            listener->OnSequencerUpdateRT(this);
        }
    }
    
    
}

void RESequencer::DidRenderRack (REMusicRack* rack, unsigned int nbFrames, float* workBufferL, float* workBufferR)
{
    // Nothing to do
}

void RESequencer::WillRenderDevice (REMusicRack* rack, REMusicDevice* device, unsigned int nbFrames, float* workBufferL, float* workBufferR)
{
    if(_tracks == NULL) return;
    
    // Is there a solo track ?
    bool anySoloTrack = false;
    for(unsigned int i=0; i<_tracks->size(); ++i) {
        RESequencerTrack* track = _tracks->at(i);
        if(track->_solo) {
            anySoloTrack = true; 
            break;
        }
    }
    
    // Adjust Volume and Pan
    int index = rack->IndexOfDevice(device);
    RESequencerTrack* track = (index >= 0 && index < _tracks->size() ? _tracks->at(index) : NULL);
    if(track)
    {
        float volume = track->_volume;
        float pan = track->_pan;
        if(anySoloTrack && !track->_solo) {
            volume = 0.0;
        }
        if(track->_mute) {
            volume = 0.0;
        }
        
        device->SetVolume(volume);
        device->SetPan(pan);
        
        // MIDI program change requested
        if(track->_midiProgramChangeRequested)
        {
            for(int channel = 0; channel < 16; ++channel) {
                track->_device->ProcessProgramChangeEvent(channel, track->_midiProgram);
            }
            
            // Hot change every MIDI Program Change event in the uploaded clips
            for(int clipIndex=0; clipIndex < track->_clips.size(); ++clipIndex) {
                REMidiClip* clip = track->_clips[clipIndex];
                unsigned int eventCount = clip->EventCount();
                for(unsigned int eventIndex=0; eventIndex < eventCount; ++eventIndex) {
                    REMidiEvent& evt = clip->_events[eventIndex];
                    if(evt.Type() == 0xC && evt.data[1] == track->_initialMidiProgram) {
                        evt.data[1] = track->_midiProgram;
                    }
                }
            }
            
            track->_initialMidiProgram = track->_midiProgram;
            track->_midiProgramChangeRequested = false;
        }
    }
}
void RESequencer::DidRenderDevice (REMusicRack* rack, REMusicDevice* device, unsigned int nbFrames, float* workBufferL, float* workBufferR)
{
    // Nothing to do
}

REMusicDevice* RESequencer::DeviceForTrack(const RETrack* track)
{
    return _rack->Device(track->Index());
}

bool RESequencer::IsRunning() const
{
    return _d ? _d->running : false;
}

void RESequencer::AddListener(RESequencerListener* listener)
{
    _d->_listeners.push_back(listener);
}

void RESequencer::RemoveListener(RESequencerListener* listener)
{
    _d->_listeners.erase(std::remove(_d->_listeners.begin(), _d->_listeners.end(), listener),
                         _d->_listeners.end());
}

int RESequencer::BarIndexThatsCurrentlyPlaying() const
{
    if(_playlist == NULL) return -1;
    
    int indexInPlaylist = _d->currentBarIndexInPlaylist;
    if(indexInPlaylist >= 0 && indexInPlaylist < _playlist->size()) {
        return _playlist->at(indexInPlaylist).IndexInSong();
    }
    return -1;
}
int RESequencer::NextBarIndexThatShouldBePlaying() const
{
    int indexInPlaylist = _d->currentBarIndexInPlaylist + 1;
    if(indexInPlaylist >= 0 && indexInPlaylist < _playlist->size()) {
        return _playlist->at(indexInPlaylist).IndexInSong();
    }
    return -1;
}
unsigned long RESequencer::TickInBarPlaying() const
{
    return _d->currentTickInBar;
}

unsigned long RESequencer::TickInPlaylist() const
{
    return _d->currentTickInPlaylist;
}

class REMidiPacket
{
public:
    int32_t tick;
    std::string data;
    
public:
    REMidiPacket() : tick(0), data() {}
    REMidiPacket(const REMidiPacket& rhs) {*this=rhs;}
    
    explicit REMidiPacket(const REMidiEvent& midiEvent)
    {
        tick = midiEvent.tick;
        
        data += (char)midiEvent.data[0];
        data += (char)midiEvent.data[1];
        if(midiEvent.Type() != 0xC && midiEvent.Type() != 0xD) {
            data += (char)(midiEvent.Velocity());
        }
    }
    
    REMidiPacket(const REMidiEvent& midiEvent, bool overrideChannel)
    {
        tick = midiEvent.tick;
        
        if(overrideChannel) {
            data += (char)midiEvent.data[0] & 0xF0;
        }
        else {
            data += (char)midiEvent.data[0];
        }
        data += (char)midiEvent.data[1];
        if(midiEvent.Type() != 0xC && midiEvent.Type() != 0xD) {
            data += (char)(midiEvent.Velocity());
        }
    }
    
    REMidiPacket& operator=(const REMidiPacket& rhs) {
        tick = rhs.tick;
        data = rhs.data;
        return *this;
    }
    
    bool operator<(const REMidiPacket& rhs) const {
        if (tick < rhs.tick) {
			return true;
		}
		else if (tick > rhs.tick) {
			return false;
		}
		else 
        {   
            //tick == rhs.time
			int priority1 = 100;
			int priority2 = 100;
            int type1 = EventType();
            int type2 = rhs.EventType();
			if(type1 == 0xC /*Program Change*/) {priority1 = 0;}
			if(type1 == 0x8 /*Note Off*/) {priority1 = 10;}
			if(type1 == 0x9 /*Note On*/) {priority1 = 20;}
			if(type2 == 0xC /*Program Change*/) {priority2 = 0;}
			if(type2 == 0x8 /*Note Off*/) {priority2 = 10;}
			if(type2 == 0x9 /*Note On*/) {priority2 = 20;}
			
			return priority1 < priority2;
		}
    }
    
    int8_t EventType() const {
        return (data[0] & 0xF0) >> 4;
    }
    
    int8_t Channel() const {
        return (data[0] & 0x0F);
    }
};

void RESequencer::GenerateMidiTempoData(REOutputStream& data) const
{
    // Track name
	unsigned char trackNameBytes[] = {0xFF, 0x03};
    data.WriteVLV(0);
    data.Write((const char*)trackNameBytes, 2);
	const char name[] = "Tempo";
    data.WriteVLV(strlen(name));
    data.Write(name, strlen(name));
	
    int32_t currentTick = 0;
    RETimeSignature lastTimeSignature(0,0);
    for(int pbarIndex=0; pbarIndex < _playlist->size(); ++pbarIndex)
    {
        const REPlaylistBar& pbar = _playlist->at(pbarIndex);
        int barIndex = pbar.IndexInSong();
        int32_t pbarTick = pbar.Tick();
        
        // Time signature change
        if(pbar.TimeSignature() != lastTimeSignature)
        {
            int32_t deltaTicks = std::max<int32_t>(0, pbarTick - currentTick);
            currentTick = pbarTick;
            data.WriteVLV(deltaTicks);
            
            unsigned int num = pbar.TimeSignature().numerator;
            unsigned int den = 2;
            switch (pbar.TimeSignature().denominator) {
                case 1: den = 0; break;
                case 2: den = 1; break;
                case 4: den = 2; break;
                case 8: den = 3; break;
                case 16:den = 4; break;
                case 32:den = 5; break;
                default:den = 2; break;
            }
            
            unsigned char bytes[] = {
                0xFF, 0x58, 4,
                static_cast<unsigned char>(num),
                static_cast<unsigned char>(den),
                24, 8};
            data.Write((const char*)bytes, sizeof(bytes));
            
            lastTimeSignature = pbar.TimeSignature();
        }
        
        // Tempo changes in the bar
        int firstItemIndex = 0;
        int lastItemIndex = 0;
        if(_tempoTimeline->FindItemsInBarRange(barIndex, barIndex, &firstItemIndex, &lastItemIndex)) 
        {
            for(int itemIndex=firstItemIndex; itemIndex <= lastItemIndex; ++itemIndex)
            {
                const RETempoItem* item = _tempoTimeline->Item(itemIndex);
                int32_t itemTickInPlaylist = pbarTick + Reflow::TimeDivToTicks(item->beat);
                
                int32_t deltaTicks = std::max<int32_t>(0, itemTickInPlaylist - currentTick);
                currentTick = pbarTick;
                data.WriteVLV(deltaTicks);
                
                unsigned long midiTempo = 60000000 / (int)(item->BeatsPerMinute());
                const char bytes[] = {static_cast<char>(0xFF), 0x51, 0x03};		// dt + cmd
                data.Write(bytes, 3);
                data.WriteUInt24(midiTempo);
            }
        }
    }
    
    // Add Track finished data
    const REPlaylistBar& lastPlaylistBar = _playlist->at(_playlist->size()-1);
    int32_t lastTick = lastPlaylistBar.Tick() + lastPlaylistBar.Duration();
    {
        int32_t deltaTicks = std::max<int32_t>(0, lastTick - currentTick);
        currentTick = lastTick;
        data.WriteVLV(deltaTicks);
        
        const char bytes[] = {static_cast<char>(0xFF), 0x2F, 0x00};
        data.Write(bytes, 3);
    }

}

void RESequencer::SetMergeChannelsOnExport(bool merge)
{
    _mergeChannelsOnExport = merge;
}

void RESequencer::GenerateMidiTrackData(REOutputStream& data, const RESequencerTrack* track) const
{
    // Track name
	unsigned char trackNameBytes[] = {0xFF, 0x03};
    data.WriteVLV(0);
    data.Write((const char*)trackNameBytes, 2);
    std::string name = track->_trackName;
	data.WriteVLV(name.length());
    data.Write(name.data(), name.length());
    
    // Aggregate Packets of all bars
    std::vector<REMidiPacket> packets;
    RETimeSignature lastTimeSignature(0,0);
    for(int pbarIndex=0; pbarIndex < _playlist->size(); ++pbarIndex)
    {
        const REPlaylistBar& pbar = _playlist->at(pbarIndex);
        int pbarTick = pbar.Tick();
        int barIndex = pbar.IndexInSong();
        const REMidiClip* clip = track->Clip(barIndex);
        if(clip == NULL) continue;
        
        // Note Events
        for(int i=0; i<clip->NoteEventCount(); ++i)
        {
            const REMidiNoteEvent& midiNoteEvent = clip->NoteEvent(i);
            REMidiEventPair events = midiNoteEvent.SplitIntoNoteOnAndNoteOffEvents(_mergeChannelsOnExport && midiNoteEvent.channel != 0x09);
        
            // Note On
            REMidiPacket noteOn (events.first);
            noteOn.tick += pbarTick;
            packets.push_back(noteOn);
            
            // Note Off
            REMidiPacket noteOff (events.second);
            noteOff.tick += pbarTick;
            packets.push_back(noteOff);
        }
        
        // All other Events
        for(int i=0; i<clip->EventCount(); ++i)
        {
            const REMidiEvent &midiEvent = clip->Event(i);
            REMidiPacket event (midiEvent, _mergeChannelsOnExport && midiEvent.Channel() != 0x09);
            event.tick += pbarTick;
            if(event.Channel())
            packets.push_back(event);
        }
    }
    
    // Sort Midi packet events
    std::sort(packets.begin(), packets.end());
    
    // Write Packets as raw Midi Data
    int32_t currentTick = 0;
    BOOST_FOREACH(const REMidiPacket& packet, packets)
    {
        int32_t deltaTicks = (packet.tick - currentTick);
        currentTick = packet.tick;
        data.WriteVLV(deltaTicks);
        
        data.Write(packet.data.data(), packet.data.size());
    }
    
    // Add Track finished data
    const REPlaylistBar& lastPlaylistBar = _playlist->at(_playlist->size()-1);
    int32_t lastTick = lastPlaylistBar.Tick() + lastPlaylistBar.Duration();
    {
        int32_t deltaTicks = std::max<int32_t>(0, lastTick - currentTick);
        currentTick = lastTick;
        data.WriteVLV(deltaTicks);
        
        const char bytes[] = {static_cast<char>(0xFF), 0x2F, 0x00};
        data.Write(bytes, 3);
    }
}

void RESequencer::ExportMidiToFile(const std::string& filename) const
{
    // Write the MIDI File data to a temporary buffer first
    REBufferOutputStream data;
    data.SetEndianness(Reflow::BigEndian);
    
    char magic[] = "MThd";
	unsigned int headerSize = 6;
	unsigned short int headerFormat = 1; // Multiple synchronous tracks
	unsigned int headerDivisions = 480; // 480 ticks per quarter note
	unsigned int nbTracks = 1 + _tracks->size();
	
    data.Write(magic, 4);
    data.WriteInt32(headerSize);
    data.WriteInt16(headerFormat);
    data.WriteInt16(nbTracks);
    data.WriteInt16(headerDivisions);
    
    // Tempo track
	{
        REBufferOutputStream midiData;
        midiData.SetEndianness(Reflow::BigEndian);
        GenerateMidiTempoData(midiData);
		
		char magic[] = "MTrk";
		data.Write(magic, 4);
        data.WriteInt32(midiData.Size());
        data.Write(midiData.Data(), midiData.Size());
	}
	
	// Append content for each track
    BOOST_FOREACH(const RESequencerTrack* track, *_tracks)
    {
        REBufferOutputStream midiData;
        midiData.SetEndianness(Reflow::BigEndian);
        GenerateMidiTrackData(midiData, track);
		
		char magic[] = "MTrk";
		data.Write(magic, 4);
        data.WriteInt32(midiData.Size());
        data.Write(midiData.Data(), midiData.Size());
	}
    
    // Write to a File
    FILE* file = fopen(filename.c_str(), "wb");
    if(file != NULL){
        fwrite(data.Data(), data.Size(), 1, file);
        fclose(file);
    }
}

