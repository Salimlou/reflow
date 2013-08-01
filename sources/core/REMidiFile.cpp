//
//  REMidiFile.cpp
//  Reflow
//
//  Created by Sebastien Bourgeois on 13/02/13.
//
//

#include <sstream>

#include "REMidiFile.h"

#include "REInputStream.h"
#include "RESong.h"
#include "RETrack.h"
#include "REBar.h"
#include "REVoice.h"
#include "REPhrase.h"
#include "REChord.h"
#include "RENote.h"

REMidiFile::REMidiFile()
: _error("Not Loaded"), _verbose(false)
{
}

REMidiFile::~REMidiFile()
{
    Clear();
}

const REMidiFileTrack* REMidiFile::Track(int idx) const
{
    return idx >= 0 && idx < _tracks.size() ? _tracks[idx] : NULL;
}

void REMidiFile::Clear()
{
    for(REMidiFileTrack* track : _tracks) {
        delete track;
    }
    _tracks.clear();
}

void REMidiFile::Load(const std::string& filename, const REMidiFileLoadOptions& options)
{
    REFileInputStream stream;
    if(stream.Open(filename))
    {
        Load(stream, options);
    }
    else {
        std::ostringstream oss; oss << "Can't open " << filename << " for reading";
        _error = oss.str();
    }
}

void REMidiFile::Load(REInputStream& stream, const REMidiFileLoadOptions& options)
{
    _options = options;
    stream.SetEndianness(Reflow::BigEndian);
    
    unsigned int nbTracks = 0;
    unsigned int headerSize = 6;
    unsigned short int headerFormat = 1;	// Multiple synchronous tracks
    unsigned int headerDivisions = 480;     // 480 ticks per quarter note
    
    std::string headerMagic = stream.ReadBytes(4);
    if(headerMagic != "MThd") {
        _error = "Bad MIDI File Header";
        return;
    }
    
    headerSize = stream.ReadInt32();
    headerFormat = stream.ReadInt16();
    nbTracks = stream.ReadInt16();
    headerDivisions = stream.ReadInt16();
    _quarterDiv = headerDivisions;
    _lastTick = 0;
    
    for(int i=0; i<nbTracks; ++i)
    {
        std::string chunkHeader = stream.ReadBytes(4);
        unsigned int chunkSize = stream.ReadInt32();
        std::string bytes = stream.ReadBytes(chunkSize);
        
        if(chunkHeader == "MTrk")
        {
            REMidiFileTrack* track = new REMidiFileTrack;
            track->_index = i;
            _tracks.push_back(track);
            if(!track->ReadEvents(bytes.data(), chunkSize)) {
                _error = "Failed to read events from MIDI Track";
                return;
            }
            _lastTick = std::max<uint32_t>(_lastTick, track->_lastTick);
        }
    }
    
    if(_tracks.empty()) {
        _error = "This MIDI file does not contain any track";
        return;
    }
    
    // Import Bars
    REMidiFileTrack* tempoTrack = _tracks[0];
    _lastTick = tempoTrack->_ComputeBars(_quarterDiv, _lastTick);
    
    // Import Notes
    for(REMidiFileTrack* track : _tracks)
    {
        if(options.mergeChannels) {
            track->_ImportNotesMerged(*tempoTrack, _quarterDiv, _lastTick);
        }
        else
        {
            for(int channelIndex=0; channelIndex<16; ++channelIndex)
            {
                if(track->NoteCountOfChannel(channelIndex))
                {
                    track->_ImportNotes(*tempoTrack, _quarterDiv, _lastTick, channelIndex);
                }
            }
        }
    }
    
    _error = "";
}

REMidiFileTrack::REMidiFileTrack()
: _lastTick(0), _tempo(0)
{
    
}

bool REMidiFileTrack::ReadEvents(const char* bytes, int length)
{
    REConstBufferInputStream stream(bytes, length);
    stream.SetEndianness(Reflow::BigEndian);
    
    for(int channelIndex=0; channelIndex<16; ++channelIndex)
    {
        _noteCountPerChannel[channelIndex] = 0;
    }
    
    uint32_t tick = 0;
    uint8_t runningStatus = 0x90;
    while(stream.Pos() < stream.Size())
    {
        // Read Delta Time
        int dt = stream.ReadVLV();
        tick += dt;
        
        // Read MIDI Event
        uint8_t b0 = stream.ReadUInt8();
        if(b0 == 0xFF)
        {
            REMidiFileMetaEvent meta;
            meta._tick = tick;
            meta._type = stream.ReadUInt8();
            int dataLen = stream.ReadVLV();
            meta._data = stream.ReadBytes(dataLen);
            _metaEvents.push_back(meta);
            
            if(meta._type == 0x03) {
                _name = meta._data;
            }
            else if(meta._type == 0x2F) {
                _lastTick = std::max<uint32_t> (meta._tick, _lastTick);
            }
            else if(meta._type == 0x51 && _index == 0) {
                uint32_t b0 = meta._data[0];
                uint32_t b1 = meta._data[1];
                uint32_t b2 = meta._data[2];
                _tempo = 60000000 / (((b0<<16) & 0x00FF0000) | ((b1<<8) & 0x0000FF00) | (b2 & 0xFF));
            }

        }
        else if(b0 == 0xF0 || b0 == 0xF7)
        {
            // SYSEX
            int dataLen = stream.ReadVLV();
            std::string bytes = stream.ReadBytes(dataLen);
        }
        else if(b0 & 0x80)
        {
            // Reset the running status
            runningStatus = b0;
            
            REMidiEvent midi;
            midi.tick = tick;
            midi.data[0] = b0;
            midi.data[1] = stream.ReadUInt8();
            if ((b0 & 0xF0) == 0xC0) {
                midi.data[2] = 0;
            }
            else if ((b0 & 0xF0) == 0xD0) {
                midi.data[2] = 0;
            }
            else {
                midi.data[2] = stream.ReadUInt8();
            }
            
            _midiEventsPerChannel[b0 & 0x0F].push_back(midi);
            if((b0 & 0xF0) == 0x90) {
                _noteCountPerChannel[b0 & 0x0F] = 1 + _noteCountPerChannel[b0 & 0x0F];
            }
        }
        else
        {
            // Use the running status
            REMidiEvent midi;
            midi.tick = tick;
            midi.data[0] = runningStatus;
            midi.data[1] = b0;
            if ((runningStatus & 0xF0) == 0xC0) {
                midi.data[2] = 0;
            }
            else if ((runningStatus & 0xF0) == 0xD0) {
                midi.data[2] = 0;
            }
            else {
                midi.data[2] = stream.ReadUInt8();
            }
            
            _midiEventsPerChannel[runningStatus & 0x0F].push_back(midi);
            if(midi.Type() == 0x9) {
                _noteCountPerChannel[runningStatus & 0x0F] = 1 + _noteCountPerChannel[runningStatus & 0x0F];
            }
        }
    }
    
    return true;
}

const REMidiFileBar* REMidiFileTrack::Bar(int idx) const
{
    return idx >= 0 && idx < _bars.size() ? &_bars[idx] : nullptr;
}

const REMidiFileMetaEvent* REMidiFileTrack::TimeSignatureEventAtTick(int tick) const
{
    for(const REMidiFileMetaEvent& meta : _metaEvents)
    {
        if(meta.Tick() == tick && meta.Type() == 0x58) return &meta;
        if(meta.Tick() > tick) break;
    }
    return NULL;
}

const REMidiEvent* REMidiFileTrack::ProgramChangeEventAtTick(int tick, int channel) const
{
    for(const REMidiEvent& midi : _midiEventsPerChannel[channel])
    {
        if(midi.tick == tick && midi.Type() == 0xC) return &midi;
        if(midi.tick > tick) break;
    }
    return NULL;
}

int REMidiFileTrack::_ComputeBars(int ppqn, int maxTick)
{
    unsigned int currentTick = 0;
    unsigned int currentTSNum = 4;
    unsigned int currentTSDen = 4;
    unsigned int currentBar = 0;
    
    maxTick = std::max<unsigned int>(maxTick, 4 * ppqn);
    while(currentTick < maxTick)
    {
        const REMidiFileMetaEvent* meta = TimeSignatureEventAtTick(currentTick);
        if(meta) {
            currentTSNum = (unsigned int)meta->Data()[0];
            currentTSDen = 1 << ((unsigned int)(meta->Data()[1]));
        }
        
        REMidiFileBar bar;
        bar._tick = currentTick;
        bar._duration = ((ppqn * currentTSNum * 4) / currentTSDen);
        bar._timeSignature = RETimeSignature(currentTSNum, currentTSDen);
        _bars.push_back(bar);
        
        currentTick += bar.Duration();
        ++currentBar;
    }
    return currentTick;
}

void REMidiFileTrack::_ImportNotesMerged(const REMidiFileTrack& tempoTrack, int ppqn, int maxTick)
{
    double tickRatio = (double)REFLOW_PULSES_PER_QUARTER / (double)ppqn;
    REBeat remainingBeat = REBeat(maxTick * tickRatio);
    
    for(int channelIndex = 0; channelIndex < 16; ++channelIndex)
    {
        for(int i=0; i<_midiEventsPerChannel[channelIndex].size(); ++i)
        {
            const REMidiEvent& midi = _midiEventsPerChannel[channelIndex][i];
            if((midi.data[0] & 0xF0) == 0x90 && midi.data[2] > 0)
            {
                unsigned int pitch = midi.data[1];
                for(unsigned int j=i; j<_midiEventsPerChannel[channelIndex].size(); ++j)
                {
                    const REMidiEvent& midi2 = _midiEventsPerChannel[channelIndex][j];
                    if( ((midi2.data[0] & 0xF0) == 0x90 && midi2.data[2] == 0) ||
                       (midi2.data[0] & 0xF0) == 0x80)
                    {
                        if(pitch == midi2.data[1])
                        {
                            RXNote n = RXNote(midi.tick * tickRatio, (midi2.tick - midi.tick) * tickRatio, pitch, midi.data[2]);
                            remainingBeat << n;
                            break;
                        }
                    }
                }
            }
        }
    }
    
    //std::cout << remainingBeat.to_pretty_s() << std::endl;
    
    // Calculate Bar Beats
    REBeatVector beats;
    for(int barIndex=0; barIndex<tempoTrack._bars.size(); ++barIndex)
    {
        const REMidiFileBar& bar = tempoTrack._bars[barIndex];
        REBeat::pair_type cb = remainingBeat.cut(bar.Duration() * tickRatio);
        
        //std::cout << cb.first.to_pretty_s() << std::endl;
        
        int treshold = REFLOW_PULSES_PER_QUARTER / 16;
        REBeat beat = cb.first.DividedBeatWithTreshold(bar.TimeSignature().numerator, treshold).first;
        //std::cout << beat.to_pretty_s() << std::endl;
        
        REBeatVector qbeats;
        for(int i=0; i<beat.SubBeatCount(); ++i)
        {
            REBeat qbeat = beat.SubBeat(i).BestDividedBeat(1<<1 | 1<<2 | 1<<3 | 1<<4 | 1<<6 | 1<<8);
            //std::cout << qbeat.to_pretty_s() << std::endl;
            qbeats.push_back(qbeat);
        }
        
        beat = REBeat::BeatGroup(qbeats);
        //std::cout << beat.to_pretty_s() << std::endl;
        beats.push_back(beat);
        remainingBeat = cb.second;
    }
    /*if(remainingBeat.duration() > 0)
    {
        beats.push_back(remainingBeat);
    }*/
    
    _mainBeat = REBeat::BeatGroup(beats);
    /*std::cout << "#### DUMPING TRACK ####" << std::endl;
    for(int i=0; i<_mainBeat.SubBeatCount(); ++i)
    {
        std::cout << "  Dumping Bar " << i << std::endl;
        const REBeat& beat = _mainBeat.SubBeat(i);
        std::cout << beat.to_pretty_s() << std::endl;
        std::cout << std::endl << std::endl;
    }*/
}

void REMidiFileTrack::_ImportNotes(const REMidiFileTrack& tempoTrack, int ppqn, int maxTick, int channelIndex)
{
    double tickRatio = (double)REFLOW_PULSES_PER_QUARTER / (double)ppqn;
    REBeat remainingBeat = REBeat(maxTick * tickRatio);
    
    for(int i=0; i<_midiEventsPerChannel[channelIndex].size(); ++i)
    {
        const REMidiEvent& midi = _midiEventsPerChannel[channelIndex][i];
        if((midi.data[0] & 0xF0) == 0x90 && midi.data[2] > 0)
        {
            unsigned int pitch = midi.data[1];
            for(unsigned int j=i; j<_midiEventsPerChannel[channelIndex].size(); ++j)
            {
                const REMidiEvent& midi2 = _midiEventsPerChannel[channelIndex][j];
                if( ((midi2.data[0] & 0xF0) == 0x90 && midi2.data[2] == 0) ||
                   (midi2.data[0] & 0xF0) == 0x80)
                {
                    if(pitch == midi2.data[1])
                    {
                        RXNote n = RXNote(midi.tick * tickRatio, (midi2.tick - midi.tick) * tickRatio, pitch, midi.data[2]);
                        remainingBeat << n;
                        break;
                    }
                }
            }
        }
    }
    
    //std::cout << remainingBeat.to_pretty_s() << std::endl;
    
    // Calculate Bar Beats
    REBeatVector beats;
    for(int barIndex=0; barIndex<tempoTrack._bars.size(); ++barIndex)
    {
        const REMidiFileBar& bar = tempoTrack._bars[barIndex];
        REBeat::pair_type cb = remainingBeat.cut(bar.Duration() * tickRatio);
        
        //std::cout << cb.first.to_pretty_s() << std::endl;
        
        int treshold = REFLOW_PULSES_PER_QUARTER / 16;
        REBeat beat = cb.first.DividedBeatWithTreshold(bar.TimeSignature().numerator, treshold).first;
        //std::cout << beat.to_pretty_s() << std::endl;
        
        REBeatVector qbeats;
        for(int i=0; i<beat.SubBeatCount(); ++i)
        {
            REBeat qbeat = beat.SubBeat(i).BestDividedBeat(1<<1 | 1<<2 | 1<<3 | 1<<4 | 1<<6 | 1<<8);
            //std::cout << qbeat.to_pretty_s() << std::endl;
            qbeats.push_back(qbeat);
        }
        
        beat = REBeat::BeatGroup(qbeats);
        //std::cout << beat.to_pretty_s() << std::endl;
        beats.push_back(beat);
        remainingBeat = cb.second;
    }
    /*if(remainingBeat.duration() > 0)
     {
     beats.push_back(remainingBeat);
     }*/
    
    _mainBeatPerChannel[channelIndex] = REBeat::BeatGroup(beats);
    /*std::cout << "#### DUMPING TRACK ####" << std::endl;
     for(int i=0; i<_mainBeat.SubBeatCount(); ++i)
     {
     std::cout << "  Dumping Bar " << i << std::endl;
     const REBeat& beat = _mainBeat.SubBeat(i);
     std::cout << beat.to_pretty_s() << std::endl;
     std::cout << std::endl << std::endl;
     }*/
}


REPhrase* REMidiFileTrack::ImportPhrase(const RETrack* track, REBeat barBeat) const
{
    REPhrase* phrase = new REPhrase;
    
    barBeat.each([&](const REBeat& b)
    {
        REBeat beat = b;
        beat.each([&](const REBeat& b2)
        {
            REChord* chord = new REChord;
            switch(b2.duration()) {
                case 480: chord->SetNoteValue(Reflow::QuarterNote); break;
                case 240: chord->SetNoteValue(Reflow::EighthNote); break;
                case 160: {
                    chord->SetNoteValue(Reflow::EighthNote);
                    chord->SetTuplet(RETuplet(3, 2));
                    break;
                }
                case 120: chord->SetNoteValue(Reflow::SixteenthNote); break;
                case 80: {
                    chord->SetNoteValue(Reflow::SixteenthNote);
                    chord->SetTuplet(RETuplet(6, 4));
                    break;
                }
                case 60: chord->SetNoteValue(Reflow::ThirtySecondNote); break;
                case 30: chord->SetNoteValue(Reflow::SixtyFourthNote); break;
                default: assert(false);
            }
            
            b2.each_note([&](const RXNote& midiNote)
            {
                RENote* note = new RENote;
                note->SetPitchFromMIDI(midiNote.pitch());
                if(NULL == chord->NoteWithMidi(midiNote.pitch())) {
                    chord->InsertNote(note, chord->NoteCount());
                }
            });
            
            phrase->AddChord(chord);
        });
    });
    
    return phrase;
}

RETrack* REMidiFileTrack::ImportTrackMerged(int nbBars) const
{
    bool drums = (NoteCountOfChannel(9) > 0);
    
    RETrack* track = new RETrack(drums ? Reflow::DrumsTrack : Reflow::StandardTrack);
    track->SetName(_name);
    
    int bestChannelIndex = ChannelIndexWithMostNotes();
    if(bestChannelIndex != -1)
    {
        const REMidiEvent* pc = ProgramChangeEventAtTick(0, bestChannelIndex);
        if(pc) track->SetMIDIProgram(pc->data[1]);
    }
    
    _ImportTrackFromBeat(track, nbBars, _mainBeat);
    
    return track;
}

RETrack* REMidiFileTrack::ImportTrack(int nbBars, int channel) const
{
    bool drums = (channel == 9);
    
    RETrack* track = new RETrack(drums ? Reflow::DrumsTrack : Reflow::StandardTrack);
    track->SetName(_name);
    
    const REMidiEvent* pc = ProgramChangeEventAtTick(0, channel);
    if(pc) track->SetMIDIProgram(pc->data[1]);
    
    _ImportTrackFromBeat(track, nbBars, _mainBeatPerChannel[channel]);
    
    return track;
}

void REMidiFileTrack::_ImportTrackFromBeat(RETrack* track, int nbBars, const REBeat& mainBeat) const
{
    for(unsigned int i=0; i<REFLOW_MAX_VOICES; ++i)
    {
        REVoice* voice = new REVoice;
        track->InsertVoice(voice, i);
        for(unsigned int j=0; j<nbBars; ++j)
        {
            if(i == 0)
            {
                const REBeat& beat = mainBeat.SubBeat(j);
                auto phrase = ImportPhrase(track, beat);
                voice->InsertPhrase(phrase, j);
            }
            else {
                voice->InsertPhrase(new REPhrase, j);
            }
        }
    }
}

int REMidiFileTrack::NoteCountOfChannel(int channel) const
{
    return _noteCountPerChannel[channel];
}

int REMidiFileTrack::ChannelIndexWithMostNotes() const
{
    int channelIndex = -1;
    int nbNotes = 0;
    
    for(int i=0; i<16; ++i)
    {
        int noteCount = _noteCountPerChannel[i];
        if(noteCount > nbNotes) {
            channelIndex = i;
            nbNotes = noteCount;
        }
    }
    return channelIndex;
}

RESong* REMidiFile::ImportSong() const
{
    RESong* song = new RESong;
    song->SetTitle("No Title");
    song->SetArtist("No Artist");
    
    const REMidiFileTrack* tempoTrack = _tracks[0];
    for(int barIndex=0; barIndex<tempoTrack->BarCount(); ++barIndex)
    {
        const REMidiFileBar* midiBar = tempoTrack->Bar(barIndex);
        REBar* bar = new REBar;
        bar->SetTimeSignature(midiBar->TimeSignature());
        song->InsertBar(bar, barIndex);
    }
    
    // Tempo
    RETempoTimeline& tempoTimeline = song->TempoTimeline();
    RETempoItem tempoItem(0, RETimeDiv(0), tempoTrack->_tempo);
    tempoTimeline.InsertItem(tempoItem);
    
    // Create Tracks
    for(int midiTrackIndex=0; midiTrackIndex<_tracks.size(); ++midiTrackIndex)
    {
        const REMidiFileTrack* midiTrack = _tracks[midiTrackIndex];
        if(_options.mergeChannels)
        {
            RETrack* track = midiTrack->ImportTrackMerged(song->BarCount());
            song->InsertTrack(track, song->TrackCount());
        }
        else
        {
            for(int channelIndex=0; channelIndex<16; ++channelIndex)
            {
                if(midiTrack->NoteCountOfChannel(channelIndex))
                {
                    RETrack* track = midiTrack->ImportTrack(song->BarCount(), channelIndex);
                    song->InsertTrack(track, song->TrackCount());
                }
            }
        }
    }
    
    song->Refresh(true);
    song->CreatePart(0);
    REScoreSettings* part = song->Score(0);
    
    return song;
}
