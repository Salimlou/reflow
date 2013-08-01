//
//  REArchive.cpp
//  Reflow
//
//  Created by Sebastien Bourgeois on 14/04/13.
//
//

#include "REArchive.h"

#include "REOutputStream.h"
#include "RESong.h"
#include "REBar.h"
#include "RETrack.h"
#include "REVoice.h"
#include "REPhrase.h"
#include "REChord.h"
#include "RENote.h"
#include "REChordDiagram.h"
#include "REScoreSettings.h"

void REArchive::ReadSongV15(RESong& song, REInputStream& decoder, uint32_t version)
{
    song.Clear();
    
    song._title = decoder.ReadString();
    song._subtitle = decoder.ReadString();
    song._artist = decoder.ReadString();
    song._album = decoder.ReadString();
    song._musicBy = decoder.ReadString();
    song._lyricsBy = decoder.ReadString();
    song._transcriber = decoder.ReadString();
    song._copyright = decoder.ReadString();
    song._notice = decoder.ReadString();
    
    uint32_t nbBars = decoder.ReadUInt32();
    for(uint32_t i=0; i<nbBars; ++i)
    {
        REBar* bar = new REBar;
        bar->_index = i;
        bar->_parent = &song;
        song._bars.push_back(bar);
        bar->DecodeFrom(decoder);
    }
    
    uint32_t nbTracks = decoder.ReadUInt32();
    for(uint32_t i=0; i<nbTracks; ++i)
    {
        RETrack* track = new RETrack;
        track->_index = i;
        track->_parent = &song;
        song._tracks.push_back(track);
        track->DecodeFrom(decoder);
    }
    
    uint32_t nbScores = decoder.ReadUInt32();
    for(uint32_t i=0; i<nbScores; ++i)
    {
        REScoreSettings* score = new REScoreSettings;
        score->_index = i;
        song._scores.push_back(score);
        score->DecodeFrom(decoder);
    }
    
    song._tempoTimeline.DecodeFrom(decoder);
    if(decoder.Version() >= 150) {
        song._defaultTempo = decoder.ReadInt16();
    }
    else song._defaultTempo = 90;
    song.Refresh();
}
void REArchive::ReadBarV15(REBar& bar, REInputStream& decoder, uint32_t version)
{
    //TODO
}
void REArchive::ReadTrackV15(RETrack& track, REInputStream& decoder, uint32_t version)
{
    //TODO
}
void REArchive::ReadVoiceV15(REVoice& voice, REInputStream& decoder, uint32_t version)
{
    //TODO
}
void REArchive::ReadPhraseV15(REPhrase& phrase, REInputStream& decoder, uint32_t version)
{
    //TODO
}
void REArchive::ReadChordV15(REChord& chord, REInputStream& decoder, uint32_t version)
{
    //TODO
}
void REArchive::ReadChordDiagramV15(REChordDiagram& diagram, REInputStream& decoder, uint32_t version)
{
    //TODO
}
void REArchive::ReadScoreSettingsV15(REScoreSettings& settings, REInputStream& decoder, uint32_t version)
{
    //TODO
}

void REArchive::WriteSongV15(const RESong& song, REOutputStream& coder, uint32_t version)
{
    coder.WriteString(song._title);
    coder.WriteString(song._subtitle);
    coder.WriteString(song._artist);
    coder.WriteString(song._album);
    coder.WriteString(song._musicBy);
    coder.WriteString(song._lyricsBy);
    coder.WriteString(song._transcriber);
    coder.WriteString(song._copyright);
    coder.WriteString(song._notice);
    
    coder.WriteUInt32(song.BarCount());
    for(unsigned int i=0; i<song._bars.size(); ++i) {
        WriteBarV15(*song._bars[i], coder, version);
    }
    
    coder.WriteUInt32(song.TrackCount());
    for(unsigned int i=0; i<song._tracks.size(); ++i) {
        WriteTrackV15(*song._tracks[i], coder, version);
    }
    
    coder.WriteUInt32(song.ScoreCount());
    for(unsigned int i=0; i<song._scores.size(); ++i) {
        WriteScoreSettingsV15(*song._scores[i], coder, version);
    }
    
    song._tempoTimeline.EncodeTo(coder);
    coder.WriteInt16(song._defaultTempo);
}

void REArchive::WriteBarV15(const REBar& bar, REOutputStream& coder, uint32_t version)
{
    //TODO
    bar.EncodeTo(coder);
}

void REArchive::WriteTrackV15(const RETrack& track, REOutputStream& coder, uint32_t version)
{
    coder.WriteUInt8(track._type);
    coder.WriteUInt8(track._tablatureInstrumentType);
    coder.WriteUInt32(track._flags);
    coder.WriteInt32(track._deviceUUID);
    
    // Tuning for Tablature tracks
    if(track._type == Reflow::TablatureTrack)
    {
        coder.WriteUInt8(track._tuning.size());   // string count
        for(uint8_t i=0; i<track._tuning.size(); ++i) {
            coder.WriteUInt8(track._tuning[i]);
        }
    }
    
    // Capo
    coder.WriteInt8(track._capo);
    
    // Name
    coder.WriteString(track._name);
    coder.WriteString(track._shortName);
    
    // MIDI program
    coder.WriteInt8(track._midiProgram);
    coder.WriteFloat(track._volume);
    coder.WriteFloat(track._pan);
    unsigned long flags = 0x00000000;
    if(track._solo) flags |= 0x01;
    if(track._mute) flags |= 0x02;
    coder.WriteUInt32(flags);
    
    // Clef Timeline
    track._clefTimeline.EncodeTo(coder);
    track._clefTimelineLeftHand.EncodeTo(coder);
    
    // Write Voices
    coder.WriteUInt8(track._voices.size());
    for(unsigned int i=0; i<track._voices.size(); ++i) {
        WriteVoiceV15(*track._voices[i], coder, version);
    }
}

void REArchive::WriteVoiceV15(const REVoice& voice, REOutputStream& coder, uint32_t version)
{
    uint16_t barCount = voice._phrases.size();
    coder.WriteUInt16(barCount);
    for(uint16_t i=0; i<barCount; ++i) {
        const REPhrase* phrase = voice._phrases[i];
        WritePhraseV15(*phrase, coder, version);
    }
}

void REArchive::WritePhraseV15(const REPhrase& phrase, REOutputStream& coder, uint32_t version)
{
    coder.WriteUInt32(phrase._flags);
    
    phrase._ottaviaModifier.EncodeTo(coder);
    
    // Chord Diagrams
    coder.WriteInt8(phrase._chordDiagrams.size());
    for(int i=0; i<phrase._chordDiagrams.size(); ++i) {
        const REPhrase::REChordDiagramTickPair& p = phrase._chordDiagrams[i];
        coder.WriteInt32(p.first);
        WriteChordDiagramV15(p.second, coder, version);
    }
    
    uint16_t chordCount = phrase._chords.size();
    coder.WriteUInt16(chordCount);
    for(uint16_t i=0; i<chordCount; ++i) {
        const REChord* chord = phrase._chords[i];
        WriteChordV15(*chord, coder, version);
    }
}

void REArchive::WriteChordV15(const REChord& chord, REOutputStream& coder, uint32_t version)
{
    coder.WriteUInt8(chord._noteValue);
    coder.WriteUInt8(chord._dots);
    coder.WriteUInt8(chord._tuplet.tuplet);
    coder.WriteUInt8(chord._tuplet.tupletFor);
    coder.WriteUInt32(chord._flags);
    coder.WriteInt8(chord._dynamics);
    
    uint8_t noteCount = chord._notes.size();
    coder.WriteUInt8(noteCount);
    for(uint8_t i=0; i<noteCount; ++i) {
        const RENote* note = chord._notes[i];
        note->EncodeTo(coder);
    }
    
    if(chord.HasTextAttached()) {
        chord._text->EncodeTo(coder);
    }
}

void REArchive::WriteChordDiagramV15(const REChordDiagram& diagram, REOutputStream& coder, uint32_t version)
{
    //TODO
    diagram._chord.EncodeTo(coder);
    diagram._grip.EncodeTo(coder);
}

void REArchive::WriteScoreSettingsV15(const REScoreSettings& settings, REOutputStream& coder, uint32_t version)
{
    //TODO
    settings.EncodeTo(coder);
}
