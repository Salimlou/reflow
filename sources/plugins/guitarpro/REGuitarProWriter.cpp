//
//  REGuitarProWriter.cpp
//  Reflow
//
//  Created by Sebastien Bourgeois on 21/02/12.
//  Copyright (c) 2012 Gargant Studios. All rights reserved.
//

#include "REGuitarProWriter.h"
#include "REException.h"
#include "REOutputStream.h"

#include "RESong.h"
#include "RETrack.h"
#include "REVoice.h"
#include "REPhrase.h"
#include "REBar.h"
#include "REChord.h"
#include "RENote.h"
#include "REFunctions.h"

REGuitarProWriter::REGuitarProWriter()
{
}

REGuitarProWriter::~REGuitarProWriter()
{
}

void REGuitarProWriter::WritePageSettings()
{
	// Starts at offset 143
	_data.WriteUInt32(210);
	_data.WriteUInt32(297);
	_data.WriteUInt32(0x0A);
	_data.WriteUInt32(0x0A);
	_data.WriteUInt32(0x0F);
	_data.WriteUInt32(0x0A);
	_data.WriteUInt32(0x64);
	
	_data.WriteUInt8(0xFF);
	_data.WriteUInt8(0x01);
	
	// Starts at offset 173
	WriteGPStringAlt("%TITLE%");
	WriteGPStringAlt("%SUBTITLE%");
	WriteGPStringAlt("%ARTIST%");
	WriteGPStringAlt("%ALBUM%");
	WriteGPStringAlt("Words by %WORDS%");
	WriteGPStringAlt("Music by %MUSIC%");
	WriteGPStringAlt("Words & Music by %WORDSMUSIC%");
	WriteGPStringAlt("Copyright %COPYRIGHT%");
	WriteGPStringAlt("All Rights Reserved - International Copyright Secured");
	WriteGPStringAlt("Page %N%/%P%");
	WriteGPStringAlt("Moderate");
}

void REGuitarProWriter::WriteInformation()
{
	// Starts at offset 31
	WriteInt32AndGPString(_song->Title());
	WriteInt32AndGPString(_song->SubTitle());
    WriteInt32AndGPString(_song->Artist());
	WriteInt32AndGPString(_song->Album());
	WriteInt32AndGPString(_song->LyricsBy());
	WriteInt32AndGPString(_song->MusicBy());
	WriteInt32AndGPString(_song->Copyright());
	WriteInt32AndGPString(_song->Transcriber());
	WriteInt32AndGPString("");
    
	// Starts at offset 76
	_data.WriteUInt32(0);		// 0 notice
	_data.WriteUInt32(0);		// lyric
	for(int i=0; i<5; ++i) {
		// Starts at 84
		_data.Skip(8);
	}
	
	// Starts at offset 124
	_data.WriteUInt32(64);		// master volume (0..15) ?
	_data.WriteUInt32(0);		// master reverb
	for(int i=0; i<10; ++i) {
		_data.WriteUInt8(0);		// master EQ
	}
	_data.WriteUInt8(0);		// master pre
}

void REGuitarProWriter::WriteMeasure(int barIndex)
{
    REPrintf("      Offset %d : Writing Measure %d\n", (int)_data.Pos(), barIndex+1);
	uint8_t flag = 0;
	
	flag |= 0x01;	// Time signature numerator change
	flag |= 0x02;	// Time signature denominator change
	flag |= 0x40;	// Tone change
	_data.WriteUInt8(flag);
	
	if(flag & 0x01) {
		_data.WriteUInt8(4);	// Time signature num
	}
	if(flag & 0x02) {
		_data.WriteUInt8(4);	// Time signature den
	}
	
	if(flag & 0x40) {		// Key signature
		uint8_t keySignature = 7;
        _data.WriteInt8(keySignature-7);    // Sharp count
		_data.Skip(1);
	}
	
	// Beaming
	if(flag & 0x03) {
		static const uint8_t beamingBytes[] = {2,2,2,2};
        _data.Write((const char*)beamingBytes, 4);
	}
	
	// Alternate endings
	_data.WriteUInt8(0);
	
	// Swing
	_data.WriteUInt8(0);
	
	// ??
	_data.WriteUInt8(0x00);
}

void REGuitarProWriter::WriteTrack(int trackIndex)
{
    REPrintf("      Offset %d : Writing Track %d\n", (int)_data.Pos(), trackIndex+1);
	
    const RETrack* track = _song->Track(trackIndex);
    
	uint8_t flag1 = 0x08;		
	
	if(track->IsDrums()) {
		flag1 |= 0x01;
	}
	
	// First Byte
	_data.WriteUInt8(flag1);
	
	// Track name on 40 bytes
    WriteGPStringFixed(track->Name(), 40);
	
	// String count & Tuning
	uint32_t stringCount = (!track->IsTablature() ? 6 : track->StringCount());
	if(stringCount > 7) stringCount = 7;
	if(stringCount < 4) stringCount = 4;
	_data.WriteUInt32(stringCount);
	for(unsigned int stringIndex=0; stringIndex<7; ++stringIndex) 
    {
		if(track->IsDrums()) {
			_data.WriteUInt32(0x00000000);
		}
        else if(track->IsStandard()) {
            _data.WriteUInt32(Reflow::StandardTuningForString(stringIndex));
        }
		else {
			uint32_t note = track->TuningForString(stringIndex);
			_data.WriteUInt32(note);
		}
	}
	
	uint32_t port = 1;
	uint32_t channel = trackIndex+1;
	if(track->IsDrums()) {
		channel = 10;
	}
	uint32_t channel2 = channel;
	uint32_t nbFrets = 24;
	_data.WriteUInt32(port);
	_data.WriteUInt32(channel);
	_data.WriteUInt32(channel2);
	_data.WriteUInt32(nbFrets);
	
	// Capo
	_data.WriteUInt32(0);
	
	// Track color
	_data.WriteUInt32(0x00000000);
	
	uint8_t flag2 = 0x43;		// 0x40 | showNS:0x02 | showTAB:0x01
	_data.WriteUInt8(flag2);
	
	uint8_t flag3 = 0x00;
	_data.WriteUInt8(flag3);
	
	// RSE crap starts here
	_data.Skip(3);
	_data.WriteUInt32(0);			// Clef ?
	_data.WriteUInt32(0);			// unknown
	_data.WriteUInt32(0x00000064);	// unknown
	
	const char skipThat[] = {0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x0A, 0x07, 0x08, 0x09};
    _data.Write(skipThat, 10);
	
	_data.WriteUInt8(0xDF);
	_data.WriteUInt8(0x03);
	
	// RSE banks
	_data.WriteUInt32(0xFFFFFFFF);
	_data.WriteUInt32(0xFFFFFFFF);
	_data.WriteUInt32(0xFFFFFFFF);
	_data.WriteUInt32(0xFFFFFFFF);
	
	// RSE EQ ?
	_data.Skip(4); //(bass/mid/treble/pre)
	
	// RSE preset names
	WriteGPStringAlt("");
	WriteGPStringAlt("");
}

void REGuitarProWriter::WriteBeat(const REChord* chord)
{
    int noteCount = chord->NoteCount();
    
    bool hasVibrato = chord->HasFlag(REChord::Vibrato);
    bool hasPickStroke = chord->HasFlag(REChord::PickStroke);
    bool hasBrush = chord->HasFlag(REChord::Brush);
    bool hasArpeggio = chord->HasFlag(REChord::Arpeggio);
    
    bool hasBeatEffect = hasVibrato /*|| hasPickStroke || hasBrush || hasArpeggio*/;
	
    bool hasTuplet = chord->Tuplet().tuplet >= 2 && chord->Tuplet().tupletFor > 0;
    
	// Flag on one byte
	uint8_t flag1 = 0x00;
	if(chord->Dots() >= 1) flag1 |= 0x01;
	if(chord->IsRest()) flag1 |= 0x40;
	if(hasTuplet) flag1 |= 0x20;
	if(hasBeatEffect) flag1 |= 0x08;
    
	_data.WriteUInt8(flag1);
	
	if(chord->IsRest()) {
		_data.WriteUInt8(2);
	}
	
	// Note value
    _data.WriteInt8(chord->NoteValue() - 2);
	
	// Tuplet
	if(hasTuplet) {
		_data.WriteUInt32(chord->Tuplet().tuplet);
	}
	
	// Beat effects
	if(hasBeatEffect) {
		uint8_t flag = 0x00;
		uint8_t flag2 = 0x00;
		
		if(hasVibrato) {
			flag |= 0x01;
		}
		
		_data.WriteUInt8(flag);
		_data.WriteUInt8(flag2);
	}
	
    const RETrack* track = chord->Phrase()->Voice()->Track();
    if(track->IsDrums())
    {
        uint8_t noteStringMask = 0x00;
        for(int i=1; i<=7; ++i) 
        {
            if(i <= noteCount)
            {
                noteStringMask |= (1 << (7-i)); 
            }
        }
        _data.WriteUInt8(noteStringMask);
        
        // Write Notes
        for(int i=1; i<=7; ++i) 
        {
            if(i <= noteCount)
            {
                const RENote* note = chord->Note(i-1);
                
                uint8_t flag = 0x20;
                _data.WriteUInt8(flag);
                
                if(note->HasFlag(RENote::DeadNote)) {
                    _data.WriteUInt8(3);
                }
                else if(note->HasFlag(RENote::TieDestination)) {
                    _data.WriteUInt8(2);
                }
                else {
                    _data.WriteUInt8(1);	
                }
                
                // Fret 
                if(flag & 0x20) {
                    _data.WriteUInt8(note->Pitch().midi);
                }
                _data.WriteUInt8(0);
            }
        }
    }
    else if(track->IsTablature())
    {
        
        uint8_t noteStringMask = 0x00;
        for(int i=1; i<=7; ++i) 
        {
            if(chord->HasNoteOnString(i-1))
            {
                noteStringMask |= (1 << (7-i)); 
            }
        }
        _data.WriteUInt8(noteStringMask);
        
        // Write Notes
        for(int i=1; i<=7; ++i) 
        {
            const RENote* note = chord->NoteOnString(i-1);
            if(note)
            {
                uint8_t flag = 0x20;
                _data.WriteUInt8(flag);
                                 
                if(note->HasFlag(RENote::DeadNote)) {
                    _data.WriteUInt8(3);
                }
                else if(note->HasFlag(RENote::TieDestination)) {
                    _data.WriteUInt8(2);
                }
                else {
                    _data.WriteUInt8(1);	
                }
                                 
                // Fret 
                if(flag & 0x20) {
                    _data.WriteUInt8(note->Fret());
                }
                _data.WriteUInt8(0);
            }
        }
    }
    else 
    {
        uint8_t noteStringMask = 0x00;
        _data.WriteUInt8(noteStringMask);
    }
    
	
	uint8_t flag = 0x00;
	_data.WriteUInt8(flag);
	
	uint8_t flag2 = 0x00;
	_data.WriteUInt8(flag2);
}

void REGuitarProWriter::WriteMeasureOfTrack(int barIndex, int trackIndex)
{
    REPrintf("      Offset %d : Writing Measure %d of Track %d\n", (int)_data.Pos(), barIndex+1, trackIndex+1);
	
    const RETrack* track = _song->Track(trackIndex);
    
    _data.WriteUInt8(0);    // line break
	
    for(int voiceIndex=0; voiceIndex<2; ++voiceIndex)
    {
        const REVoice* voice = track->Voice(voiceIndex);
        const REPhrase* phrase = voice->Phrase(barIndex);
        
        int nbBeats = phrase->ChordCount();
        _data.WriteUInt32(nbBeats);
        for(int chordIndex=0; chordIndex < nbBeats; ++chordIndex)
        {
            const REChord* chord = phrase->Chord(chordIndex);
            WriteBeat(chord);
        }
    }
}

void REGuitarProWriter::WriteSong()
{
    // Header
    WriteGPStringFixed("FICHIER GUITAR PRO v5.10", 30);
	
	// Information
    WriteInformation();
	
	// Page settings
    WritePageSettings();
	
	// Starts at offset 415
	_data.WriteUInt32(90);  // tempo
	_data.WriteUInt8(0);	// hideTempo
	_data.WriteUInt32(0);	// key signature ?
	_data.WriteUInt8(0);	// ottavia ?
	
	// Starts at offset 425 (last 768 bytes)
	for (int port=0; port<4; port++) 
    {
        for (int channel=0; channel<16; channel++) 
        {
            const RETrack* track = _song->Track(channel);
            _data.WriteUInt32(track != NULL ? track->MIDIProgram() : 0);
            _data.WriteUInt8(13);   // volume
			_data.WriteUInt8(0x08);
            _data.Skip(6 * sizeof(uint8_t));
        }
    }
	
	// Starts at offset 1193 (last 19*2 bytes) -> Directions
	for(int i=0; i<38; ++i) {_data.WriteUInt8(0xFF);}
	
	// Starts at offset 1231
	_data.WriteUInt32(0);
    
	// Starts at offset 1235 -> BAR COUNT & TRACK COUNT
    unsigned int barCount = _song->BarCount();
    unsigned int trackCount = _song->TrackCount();
	_data.WriteUInt32(barCount);
	_data.WriteUInt32(trackCount);
	
	REPrintf("## Starting GP Export : %d Measures - %d Tracks##", barCount+1, trackCount+1);
	
	// Exporting Measures
    for(int barIndex=0; barIndex < barCount; ++barIndex)
    {
        _currentBarIndex = barIndex;
        WriteMeasure(barIndex);
    }
    
	// Exporting Tracks
	for(int i=0; i<trackCount; ++i)
	{
		_currentTrackIndex = i;
        WriteTrack(i);
	}
	
	// Exporting Measures 
    for(int barIndex=0; barIndex < barCount; ++barIndex)
    {
        _currentBarIndex = barIndex;
        for(int trackIndex=0; trackIndex < trackCount; ++trackIndex)
        {
            _currentTrackIndex = trackIndex;
            WriteMeasureOfTrack(barIndex, trackIndex);
        }
    }
}

bool REGuitarProWriter::ExportSongToFile(const RESong* song, const std::string& filename)
{
    if(song == NULL) return false;
    _song = song;
    _filename = filename;
    
    // Write the MIDI File data to a temporary buffer first
    _data.SetEndianness(Reflow::LittleEndian);
    
    try 
    {
        WriteSong();
    } 
    catch (std::exception& ex) {
        REPrintf("Exception caught: %s\n", ex.what());
        return false;
    }
    
    // Write to file
    FILE* file = fopen(filename.c_str(), "wb");
    if(file != NULL){
        fwrite(_data.Data(), _data.Size(), 1, file);
        fclose(file);
    }
    else {
        REPrintf("Can't open [%s] for writing\n", filename.c_str());
        return false;
    }
    
    return true;
}

void REGuitarProWriter::WriteGPString(const std::string& str)
{
    _data.WriteUInt8(str.size());
    _data.Write(str.data(), str.size());
}

void REGuitarProWriter::WriteGPStringFixed(const std::string& str, unsigned long fixedLength)
{
    int len = str.size();
    if(len > fixedLength)
    {
        _data.WriteUInt8(fixedLength);
        _data.Write(str.data(), fixedLength);
    }
    else {
        _data.WriteUInt8(len);
        _data.Write(str.data(), len);
        _data.Skip(fixedLength - len);
    }
}

void REGuitarProWriter::WriteInt32AndGPString(const std::string& str)
{
    _data.WriteUInt32(1+str.size());
    WriteGPString(str);
}

void REGuitarProWriter::WriteGPStringAlt(const std::string& str)
{
    _data.WriteUInt32(1+str.size());
    WriteGPString(str);
}


