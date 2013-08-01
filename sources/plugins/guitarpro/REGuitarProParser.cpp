//
//  REGuitarProParser.cpp
//  Reflow
//
//  Created by Sebastien on 20/04/11.
//  Copyright 2011 Gargant Studios. All rights reserved.
//

// Note:
// Check out http://dguitar.sourceforge.net/GP4format.html for more info

#include "REGuitarProParser.h"
#include "REInputStream.h"
#include "RESong.h"
#include "REBar.h"
#include "RETrack.h"
#include "REVoice.h"
#include "REPhrase.h"
#include "REChord.h"
#include "RENote.h"
#include "REException.h"

REGuitarProParser::REGuitarProParser()
: _decoder(0), _error(""), _trackCount(0), _barCount(0), _version(0), _currentBarIndex(0), _currentTrackIndex(0), _ownSong(false)
{
    
}

REGuitarProParser::~REGuitarProParser()
{
    if(_ownSong && _song) {
        delete _song;
    }
}

void REGuitarProParser::ParseProperties()
{
    _decoder->Skip(4);
    std::string title = ReadGPString();
    
    _decoder->Skip(4);
    std::string subtitle = ReadGPString();

    _decoder->Skip(4);
    std::string artist = ReadGPString();

    _decoder->Skip(4);
    std::string album = ReadGPString();

    _decoder->Skip(4);
    std::string lyricsBy = ReadGPString();
    std::string musicBy = "";
    if (_version >= 500)  {
        _decoder->Skip(4);
        musicBy = ReadGPString();
    }
    
    _decoder->Skip(4);
    std::string copyright = ReadGPString();
    
    _decoder->Skip(4);
    std::string transcribedBy = ReadGPString();
    
    _song->SetTitle(title);
    _song->SetSubTitle(subtitle);
    _song->SetArtist(artist);
    _song->SetAlbum(album);
    _song->SetLyricsBy(lyricsBy);
    _song->SetMusicBy(musicBy);
    _song->SetCopyright(copyright);
    _song->SetTranscriber(transcribedBy);
    
    // Comments
    _decoder->Skip(4);
    ReadGPString();
    uint32_t commentsCount = _decoder->ReadUInt32();
    for (uint32_t comment=0; comment<commentsCount; comment++) {
        _decoder->Skip(4);
        ReadGPString();
    }
    
    if (_version < 500) {
        _decoder->ReadUInt8();
    }
    
}

void REGuitarProParser::ParseLyrics()
{
    // TODO: Lyrics are skipped at the moment
    _decoder->ReadUInt32();
    for (int line=0; line<5; line++) {
        _decoder->ReadUInt32();
        uint32_t length = _decoder->ReadUInt32();
        if (length > 0) {
            _decoder->Skip(length);
        }
    }
}

void REGuitarProParser::ParseDirections()
{
    // Following values refer to the Bar index +1 where symbol is located
    
    _coda = _decoder->ReadInt16() - 1;
    _doubleCoda = _decoder->ReadInt16() - 1;
    _segno = _decoder->ReadInt16() - 1;
    _segnoSegno = _decoder->ReadInt16() - 1;
    _fine = _decoder->ReadInt16() - 1;
    
    _daCapo = _decoder->ReadInt16() - 1;
    _daCapoAlCoda = _decoder->ReadInt16() - 1;
    _daCapoAlDoubleCoda = _decoder->ReadInt16() - 1;
    _daCapoAlFine = _decoder->ReadInt16() - 1;

    _dalSegno = _decoder->ReadInt16() - 1;
    _dalSegnoAlCoda = _decoder->ReadInt16() - 1;
    _dalSegnoAlDoubleCoda = _decoder->ReadInt16() - 1;
    _dalSegnoAlFine = _decoder->ReadInt16() - 1;
    
    _dalSegnoSegno = _decoder->ReadInt16() - 1;
    _dalSegnoSegnoAlCoda = _decoder->ReadInt16() - 1;
    _dalSegnoSegnoAlDoubleCoda = _decoder->ReadInt16() - 1;
    _dalSegnoSegnoAlFine = _decoder->ReadInt16() - 1;

    _toCoda = _decoder->ReadInt16() - 1;
    _toDoubleCoda = _decoder->ReadInt16() - 1;
}

void REGuitarProParser::DispatchDirections()
{
    if(_coda >= 0) _song->GetBarOrThrow(_coda).SetDirectionTarget(Reflow::Coda);
    if(_doubleCoda >= 0) _song->GetBarOrThrow(_doubleCoda).SetDirectionTarget(Reflow::DoubleCoda);
    if(_segno >= 0) _song->GetBarOrThrow(_segno).SetDirectionTarget(Reflow::Segno);
    if(_segnoSegno >= 0) _song->GetBarOrThrow(_segnoSegno).SetDirectionTarget(Reflow::SegnoSegno);
    if(_fine >= 0) _song->GetBarOrThrow(_fine).SetDirectionTarget(Reflow::Fine);
    
    if(_daCapo >= 0) _song->GetBarOrThrow(_daCapo).SetDirectionJump(Reflow::DaCapo);
    if(_daCapoAlCoda >= 0) _song->GetBarOrThrow(_daCapoAlCoda).SetDirectionJump(Reflow::DaCapo_AlCoda);
    if(_daCapoAlDoubleCoda >= 0) _song->GetBarOrThrow(_daCapoAlDoubleCoda).SetDirectionJump(Reflow::DaCapo_AlDoubleCoda);
    if(_daCapoAlFine >= 0) _song->GetBarOrThrow(_daCapoAlFine).SetDirectionJump(Reflow::DaCapo_AlFine);
    
    if(_dalSegno >= 0) _song->GetBarOrThrow(_dalSegno).SetDirectionJump(Reflow::DalSegno);
    if(_dalSegnoAlCoda >= 0) _song->GetBarOrThrow(_dalSegnoAlCoda).SetDirectionJump(Reflow::DalSegno_AlCoda);
    if(_dalSegnoAlDoubleCoda >= 0) _song->GetBarOrThrow(_dalSegnoAlDoubleCoda).SetDirectionJump(Reflow::DalSegno_AlDoubleCoda);
    if(_dalSegnoAlFine >= 0) _song->GetBarOrThrow(_dalSegnoAlFine).SetDirectionJump(Reflow::DalSegno_AlFine);
    
    if(_dalSegnoSegno >= 0) _song->GetBarOrThrow(_dalSegnoSegno).SetDirectionJump(Reflow::DalSegnoSegno);
    if(_dalSegnoSegnoAlCoda >= 0) _song->GetBarOrThrow(_dalSegnoSegnoAlCoda).SetDirectionJump(Reflow::DalSegnoSegno_AlCoda);
    if(_dalSegnoSegnoAlDoubleCoda >= 0) _song->GetBarOrThrow(_dalSegnoSegnoAlDoubleCoda).SetDirectionJump(Reflow::DalSegnoSegno_AlDoubleCoda);
    if(_dalSegnoSegnoAlFine >= 0) _song->GetBarOrThrow(_dalSegnoSegnoAlFine).SetDirectionJump(Reflow::DalSegnoSegno_AlFine);
    
    if(_toCoda >= 0) _song->GetBarOrThrow(_toCoda).SetDirectionJump(Reflow::ToCoda);
    if(_toDoubleCoda >= 0) _song->GetBarOrThrow(_toDoubleCoda).SetDirectionJump(Reflow::ToDoubleCoda);
}

bool REGuitarProParser::IsHeaderValid(const char* data, int length)
{
    if(length >= 25)
    {
        // Skip first byte
        std::string type(data+1, 24);
        if      (type == "FICHIER GUITAR PRO v3.00") {return true;}
        else if (type == "FICHIER GUITAR PRO v4.00") {return true;}
        else if (type == "FICHIER GUITAR PRO v4.06") {return true;}
        else if (type == "FICHIER GUITAR PRO L4.06") {return true;}
        else if (type == "FICHIER GUITAR PRO v5.00") {return true;}
        else if (type == "FICHIER GUITAR PRO v5.10") {return true;}
    }
    
    return false;
}

bool REGuitarProParser::Parse(REInputStream *decoder, RESong* song)
{
    try 
    {
        if(decoder == 0) {
            REThrow("No Input Stream was provided");
        }
        _decoder = decoder;
        
        if(song == 0) {
            _song = new RESong;
            _ownSong = true;
        }
        else {
            _song = song;
            _ownSong = false;
        }
        
        // Read File Version
        std::string type = ReadGPString();
        if      (type == "FICHIER GUITAR PRO v3.00") {_version = 300;}
        else if (type == "FICHIER GUITAR PRO v4.00") {_version = 400;}
        else if (type == "FICHIER GUITAR PRO v4.06") {_version = 406;}
        else if (type == "FICHIER GUITAR PRO L4.06") {_version = 406;}
        else if (type == "FICHIER GUITAR PRO v5.00") {_version = 500;}
        else if (type == "FICHIER GUITAR PRO v5.10") {_version = 510;}
        else REThrow("Unknown File Header");

        // Skip
        _decoder->SeekTo(31);
        
        // Properties
        ParseProperties();
        
        // Lyrics on V4+
        if(_version >= 400) {
            ParseLyrics();
        }
        
        // Skip
        if (_version >= 510) {
            _decoder->Skip(19);
        }
        
        // Skip
        if (_version >= 500) 
        {
            _decoder->Skip(30);
            for(int i=0; i<11; ++i) 
            {
                ReadGPStringAlt();
            }
        }
        
        // Tempo
        uint32_t tempo = _decoder->ReadUInt32();
        _song->TempoTimeline().InsertItem (RETempoItem (_currentBarIndex, RETimeDiv(0), tempo));
        
        // Skip
        _decoder->Skip(4);
        
        // Skip on V4+
        if (_version >= 400) {
            _decoder->Skip(1);
        }
        
        // Skip one byte in V5.1
        if (_version >= 510) {
            _decoder->Skip(1);
        }
        
        // Midi settings for each Port
        for (int port=0; port<4; port++) 
        {
            for (int channel=0; channel<16; channel++) 
            {
                _midiProgram[port][channel] = _decoder->ReadUInt32();
                _midiVolume[port][channel] = _decoder->ReadUInt8();
                _midiPan[port][channel] = _decoder->ReadUInt8();
                
                // Ignore others
                _decoder->Skip(6);
            }
        }
        
        // Parse Directions
        if (_version >= 500) {
            ParseDirections();
        }
         
        // Skip
        if (_version >= 500) {
            _decoder->ReadUInt32();
        }
        
        // Bar Count
        _barCount = _decoder->ReadUInt32();
        
        // Track Count
        _trackCount = _decoder->ReadUInt32();
            
        // Parse Bar
        for (unsigned int barIndex=0; barIndex<_barCount; barIndex++) {
            _currentBarIndex = barIndex;
            ParseBar();
        }
        
        // Parse Tracks
        for(unsigned int trackIndex=0; trackIndex<_trackCount; ++trackIndex) 
        {
            _currentTrackIndex = trackIndex;
            ParseTrack();
            
            // Create voices and phrases
            for(unsigned int voiceIndex=0; voiceIndex<REFLOW_MAX_VOICES; ++voiceIndex) {
                RETrack* track = _song->Track(trackIndex);
                REVoice* voice = new REVoice;
                track->InsertVoice(voice, voiceIndex);
                
                for (unsigned int barIndex=0; barIndex<_barCount; barIndex++) {
                    REPhrase* phrase = new REPhrase;
                    voice->InsertPhrase(phrase, barIndex);
                }
            }
        }
        
        // Parse Phrases
        for (unsigned int barIndex=0; barIndex<_barCount; barIndex++) {
            _currentBarIndex = barIndex;
            _currentBar = _song->Bar(barIndex);
            for (unsigned int trackIndex=0; trackIndex<_trackCount; trackIndex++) {
                _currentTrackIndex = trackIndex;
                _currentTrack = _song->Track(trackIndex);
                if(_currentTrack->IsDrums()) {
                    _stringCount = 6;
                }
                else {
                    _stringCount = _currentTrack->StringCount();
                }
                
                ParsePhrase();
            }
        }
        
        // Dispatch directions we read before
        if (_version >= 500) {
            DispatchDirections();
        }
        
        // Fix all tied notes
        for(int i=0; i<_song->TrackCount(); ++i)
        {
            RETrack* track = _song->Track(i);
            if(!track->IsTablature()) continue;
            
            for(int voiceIndex=0; voiceIndex < track->VoiceCount(); ++voiceIndex)
            {
                REVoice* voice = track->Voice(voiceIndex);
                for(int phraseIndex=0; phraseIndex < voice->PhraseCount(); ++ phraseIndex)
                {
                    REPhrase* phrase = voice->Phrase(phraseIndex);
                    int nbChords = phrase->ChordCount();
                    for(int chordIndex=0; chordIndex < nbChords; ++chordIndex)
                    {
                        REChord* chord = phrase->Chord(chordIndex);
                        int nbNotes = chord->NoteCount();
                        for(int noteIndex=0; noteIndex < nbNotes; ++noteIndex)
                        {
                            RENote* note = chord->Note(noteIndex);
                            if(note->HasFlag(RENote::TieOrigin))
                            {
                                RENote* tied = note->FindDestinationOfTiedNote();
                                if(tied) {
                                    tied->SetFret(note->Fret());
                                }
                            }
                        }
                    }
                    
                    phrase->Refresh();
                }
            }
        }
        
        _song->Refresh(true);
    }
    catch(REException& e)
    {
        printf("Guitar Pro Parser Error: %s\n", e.what());
        _error = e.Message();
        return false;
    }
    catch(std::exception& e)
    {
        printf("Guitar Pro Parser Error: %s\n", e.what());
        _error = e.what();
        return false;
    }
    
    return true;
}


void REGuitarProParser::ParseTrack()
{
    RETrack* track = new RETrack;
    _song->InsertTrack(track, _currentTrackIndex);
        
    uint8_t flag = _decoder->ReadUInt8();
	bool isDrums = ((flag & 0x01) != 0);
    
    track->SetName(ReadGPStringFixed(40));
    
	// Create instrument
    track->_type = (isDrums ? Reflow::DrumsTrack : Reflow::TablatureTrack);
    
    // String count
    _stringCount = _decoder->ReadUInt32();
    track->SetStringCount(_stringCount);
    
    if(isDrums) {
        track->ClefTimeline(false).InsertItem(REClefItem(0, RETimeDiv(0), Reflow::NeutralClef));
    }
    else {
        track->ClefTimeline(false).InsertItem(REClefItem(0, RETimeDiv(0), 
                                                    _stringCount >= 6 ? Reflow::TrebleClef : Reflow::BassClef, 
                                                    Reflow::Ottavia_8vb));
    }

    // Tuning (always 7 strings)
    for (uint32_t stringIndex=0; stringIndex<7; stringIndex++) {
        uint32_t note = _decoder->ReadUInt32();
        if (stringIndex >= _stringCount) continue;
        
        track->SetTuningForString(stringIndex, note);
    }
    
    // MIDI Port and channel 
    int port = _decoder->ReadUInt32();
    int channel = _decoder->ReadUInt32();
    _decoder->Skip(8);
    
    // Retrieve the MIDI settings from Port/Channel
    int midiInstrument = _midiProgram[port - 1][channel - 1];
    int midiVolume = _midiVolume[port-1][channel-1];
    int midiPan = _midiPan[port-1][channel-1];
    track->SetMIDIProgram(midiInstrument);
    track->SetVolume((float)midiVolume / 16.0f);
    track->SetPan((float)midiPan / 16.0f);
    
    // Capo
    uint32_t capo = _decoder->ReadUInt32();
	
    // Skip
    _decoder->Skip(4);
    
    // Skip V5
    if (_version >= 500) 
    {
        _decoder->Skip(45);
    }
    
    // Skip V5.1 (seems to be RSE values) 
    if (_version >= 510) 
    {
        _decoder->Skip(4);
        
        ReadGPStringAlt();
        ReadGPStringAlt();
    }
}

void REGuitarProParser::ParseBar()
{
    // Create a new bar
    REBar* bar = new REBar;
    _song->InsertBar(bar, _currentBarIndex);
    
    // Retrieve previous bar
    REBar* previousBar = bar->PreviousBar();

    // Flag
    uint8_t flag = _decoder->ReadUInt8();

    // Time Signature
	bool timeSignatureChange = false;
	RETimeSignature timeSignature(4,4);
    {
        
        // Time signature numerator
        if (flag & 0x01) {
            timeSignature.numerator = _decoder->ReadUInt8();
            timeSignatureChange = true;
        }
        else {
            if(previousBar) {
                timeSignature.numerator = previousBar->TimeSignature().numerator;
            }
        }
        
        // Time signature denominator
        if (flag & 0x02) {
            timeSignature.denominator = _decoder->ReadUInt8();
            timeSignatureChange = true;
        }
        else {
            if(previousBar) {
                timeSignature.denominator = previousBar->TimeSignature().denominator;
            }
        }

        // Set Time Signature
        bar->SetTimeSignature(timeSignature);
    }
	
    // Repeat Start
    if(flag & 0x04) {
        bar->SetFlag(REBar::RepeatStart);
    }
    
    // Repeat End
    if(flag & 0x08) {
        bar->SetFlag(REBar::RepeatEnd);
    }
    
    // Repeat Count
    if (flag & 0x08) {
        bar->SetRepeatCount(_decoder->ReadUInt8());
        if (_version < 500) {
            bar->SetRepeatCount(bar->RepeatCount() + 1);
        }
    }
    
    // Alternate Endings on V3-4
    if ((flag & 0x10) && _version < 500) {
        _decoder->Skip(1);
    }
    
    // Rehearsal Sign
    if (flag & 0x20) {
        std::string rehearsalName = ReadGPStringAlt();
        if(rehearsalName.size() > 0) {
            bar->SetRehearsalSignText(rehearsalName);
            bar->SetFlag(REBar::RehearsalSign);
        }
        
        _decoder->Skip(4);
    }
    
    // Key Signature
    if (flag & 0x40) {
        int8_t sharpCount = _decoder->ReadInt8() + 7;
        bool minor = (_decoder->ReadInt8() == 1);
        
        REKeySignature ks(sharpCount, minor);
        bar->SetKeySignature(ks);
    }
    else {
        if(previousBar) 
            bar->SetKeySignature(previousBar->KeySignature());
    }
    
    // Beaming on V5
    if(_version >= 500)
    {
        if(timeSignatureChange) {
            _decoder->Skip(4);
        }
    }
     
    // Skip
    if(_version >= 500) {
        _decoder->Skip(3);
    }
}

void REGuitarProParser::ParsePhrase()
{
    unsigned int voiceReadCount = 1;
    
    // Guitar Pro 5 has multi voice support
    if (_version >= 500) {
        voiceReadCount = 2;
    }
    
    // Skip
    if (_version >= 500) {
        _decoder->Skip(1);
    }
    
    for (unsigned int voiceIndex=0; voiceIndex<voiceReadCount; voiceIndex++) 
    {
		_currentVoiceIndex = voiceIndex;
        _currentVoice = _currentTrack->Voice(voiceIndex);
        _currentPhrase = _currentVoice->Phrase(_currentBarIndex);
        
        unsigned int beatCount = _decoder->ReadUInt32();
        for (unsigned int beatIndex=0; beatIndex<beatCount; beatIndex++) 
		{
            _currentChord = new REChord;
            _currentPhrase->InsertChord(_currentChord, beatIndex);
            ParseChord();
        }
    }
}

void REGuitarProParser::ParseDiagramV3()
{
    uint8_t flag = _decoder->ReadUInt8();
    if (flag == 0) 
    {    
        // Chord Name
        std::string chordName = ReadGPStringAlt();
        
        int32_t firstFret = _decoder->ReadInt32();
        if (firstFret > 0) {
            _decoder->Skip(24);
        }
    }
    else {
        _decoder->Skip(124);
    }
}

void REGuitarProParser::ParseDiagramV4With6Strings()
{
    uint8_t flag = _decoder->ReadUInt8();
    if (flag == 0) 
    {    
        // Chord Name
        std::string chordName = ReadGPStringAlt();
        
        int32_t firstFret = _decoder->ReadInt32();
        if (firstFret != 0) 
        {
            _decoder->Skip(24);     // 6 strings
        }
    }
    else 
    {
        _decoder->Skip(106);
    }
}

void REGuitarProParser::ParseDiagramV4With7Strings()
{
    uint8_t flag = _decoder->ReadUInt8();
    if (flag == 0) 
    {    
        // Chord Name
        std::string chordName = ReadGPStringAlt();
        
        int32_t firstFret = _decoder->ReadInt32();
        if (firstFret != 0) 
        {
            _decoder->Skip(28);     // 7 strings
        }
    }
    else 
    {
        _decoder->Skip(106);
    }
}

void REGuitarProParser::ParseDiagram()
{
    if(_version == 300) {
        ParseDiagramV3();
    }
    else if(_version >= 406) {
        ParseDiagramV4With7Strings();
    }
    else {
        ParseDiagramV4With6Strings();
    }
}

void REGuitarProParser::ParseAutomations()
{
    _decoder->Skip(1);
    
    if (_version >= 500) {
        _decoder->Skip(16);
    }
    
    int volume = _decoder->ReadInt8();
    int pan = _decoder->ReadInt8();
    int chorus = _decoder->ReadInt8();
    int reverb = _decoder->ReadInt8();
    int phaser = _decoder->ReadInt8();
    int tremolo = _decoder->ReadInt8();
    
    if (_version >= 500) {
        ReadGPStringAlt();
    }
    
    // Tempo
    int tempo = _decoder->ReadInt32();
    
    if (volume >= 0) {
        _decoder->Skip(sizeof(uint8_t));
    }
    if (pan >= 0) {
        _decoder->Skip(sizeof(uint8_t));
    }
    
    // Chorus
    if (chorus >= 0) {
        _decoder->Skip(sizeof(uint8_t));
    }
    
    // Reverb
    if (reverb >= 0) {
        _decoder->Skip(sizeof(uint8_t));
    }
    
    // Phaser
    if (phaser >= 0) {
        _decoder->Skip(sizeof(uint8_t));
    }
    
    // Tremolo
    if (tremolo >= 0) {
        _decoder->Skip(sizeof(uint8_t));
    }
    
    // Tempo
    if (tempo >= 0) 
    {
        _decoder->ReadUInt8();
        
        RETempoItem tempoItem(_currentBarIndex, _currentChord->Offset(), tempo);
        _song->TempoTimeline().InsertItem(tempoItem);
        
        if (_version >= 510) {
            _decoder->Skip(sizeof(uint8_t));
        }
    }
    
    if (_version >= 400) {
        _decoder->Skip(sizeof(uint8_t));
    }
    
    if (_version >= 500) {
        _decoder->Skip(sizeof(uint8_t));
    }
    
    if (_version >= 510) {
        ReadGPStringAlt();
        ReadGPStringAlt();
    }
}

void REGuitarProParser::ParseSlideV4(RENote* note)
{
    int8_t slideType = _decoder->ReadInt8();
    if(note) {
        switch(slideType) 
        {  
            case -1: note->SetSlideIn(Reflow::SlideInFromBelow); break;
            case -2: note->SetSlideIn(Reflow::SlideInFromAbove); break;
            case  1: note->SetSlideOut(Reflow::ShiftSlide); break;
            case  2: note->SetSlideOut(Reflow::ShiftSlide); break;      // TODO: This should be a legato slide
            case  3: note->SetSlideOut(Reflow::SlideOutLow); break;
            case  4: note->SetSlideOut(Reflow::SlideOutHigh); break;
        }
    }
}

void REGuitarProParser::ParseSlideV5(RENote* note)
{
    uint8_t slideType = _decoder->ReadUInt8();
    if(note)
    {
        if(slideType & 0x01) note->SetSlideOut(Reflow::ShiftSlide);
        if(slideType & 0x02) note->SetSlideOut(Reflow::ShiftSlide);     // TODO: This should be a Legato slide
        if(slideType & 0x04) note->SetSlideOut(Reflow::SlideOutLow);
        if(slideType & 0x08) note->SetSlideOut(Reflow::SlideOutHigh);
        if(slideType & 0x10) note->SetSlideIn(Reflow::SlideInFromBelow);
        if(slideType & 0x20) note->SetSlideIn(Reflow::SlideInFromAbove);
    }
}

void REGuitarProParser::ParseBend(RENote* note)
{
    uint8_t bendType = _decoder->ReadUInt8();
    uint32_t bendValue = _decoder->ReadUInt32();
    
    if(note)
    {
        int quarterTones = bendValue/25;
        REBend bend(Reflow::Bend);
        bend.SetBentPitch(quarterTones);
        
        switch (bendType) 
        {
            case 1:	// Bend
                bend.SetType(Reflow::Bend);
                break;
                
            case 2: // Bend and Release
                bend.SetType(Reflow::BendAndRelease);
                break;
                
            case 3: // Bend and Release and Bend -> Treated as Bend and Release
                bend.SetType(Reflow::BendAndRelease);
                break;
                
            case 4: // Prebend
                bend.SetType(Reflow::PreBend);
                break;
                
            case 5:	// Prebend and release
                bend.SetType(Reflow::PreBendAndRelease);
                break;
        }
        
        note->SetBend(bend);
    }
    
    // Skip Bend points
    uint32_t nbPoints = _decoder->ReadUInt32();
    for (int i=0; i<nbPoints; i++) {
        _decoder->Skip(9);
    }
}

void REGuitarProParser::ParseNoteEffects(RENote* note)
{
    uint8_t flag = _decoder->ReadUInt8();
    uint8_t flag2 = (_version >= 400 ? _decoder->ReadUInt8() : 0);
    
    bool hasBend = (flag & 0x01);
    bool hasLegato = (flag & 0x02);
    bool hasLetRing = (flag & 0x08);
    
    bool hasStaccato = (flag2 & 0x01);
    bool hasPalmMute = (flag2 & 0x02);
    bool hasTremoloPicking = (flag2 & 0x04);
    bool hasSlide = (flag2 & 0x08);
    bool hasHarmonic = (flag2 & 0x10);
    bool hasTrill = (flag2 & 0x20);
    bool hasVibrato = (flag2 & 0x40);
    
    // Staccato
    if(hasStaccato) {
        _currentChord->SetFlag(REChord::Staccato);
    }
    
    // Palm Mute
    if(hasPalmMute) {
        _currentChord->SetFlag(REChord::PalmMute);
    }
    
    // Vibrato
    if(hasVibrato) {
        _currentChord->SetFlag(REChord::Vibrato);
    }

    // Bend
    if (hasBend) 
    {
        ParseBend(note);
    }
    
    if (flag & 0x10) {
        _decoder->Skip(4);
        if(_version >= 500) _decoder->Skip(1);
    }
    
    // All the following effects are specific to GP4-5
    if (_version >= 400) 
    {
        // Tremolo Picking
        if (hasTremoloPicking) {
            _decoder->Skip(sizeof(uint8_t));
        }
        
        // Slide
        if (hasSlide) 
        {
            if(_version >= 500) {
                ParseSlideV5(note);
            }
            else {
                ParseSlideV4(note);
            }
        }
        
        // Harmonic
        if (hasHarmonic) 
        {
            uint8_t harmonicType = _decoder->ReadUInt8();
            if (_version >= 500) 
            {
                if (harmonicType == 2) _decoder->Skip(3);
                else if (harmonicType == 3) _decoder->Skip(1);
            }
        }
        
        // Trill
        if (hasTrill) {
            _decoder->Skip(2);
        }
    }
}

void REGuitarProParser::ParseNotes()
{
    uint8_t noteMask = _decoder->ReadUInt8();
    for (int stringIndex=6; stringIndex>=0; stringIndex--) 
	{
        int index = (7 - _stringCount + stringIndex);
        if ((noteMask & (1 << index)) == 0) continue;
		
		int string = (_stringCount - stringIndex - 1);
        RENote* note = NULL;
		
        uint8_t flag = _decoder->ReadUInt8();
        bool customDuration = (0 != (flag & 0x01));
        
        if (flag & 0x20) 
        {
            uint8_t type = _decoder->ReadUInt8();
            
            // No Note on this string
            if(type != 0)
            {
                note = new RENote;
                note->SetString(string);
            }
            
            // Dead Note
            if (type == 3) 
            {
                note->SetFlag(RENote::DeadNote);
                note->SetFret(0);
            }
            
            // Tied Note
			else if (type == 2) 
            {
                note->SetFlag(RENote::TieDestination);
			}
        }
        
        // Custom duration on GP3-4
        if (customDuration && _version < 500) {
            _decoder->Skip(2);
        }
        
		uint8_t dynamic = 6;
        if (flag & 0x10) {
			dynamic = _decoder->ReadUInt8(); // dynamic
        }
		static uint8_t velocityTable[] = {0, 13, 26, 39, 52, 65, 78, 91, 104, 127};
        //TODO: if(note) note->SetVelocity(velocityTable[dynamic]);
        
        if (flag & 0x20) 
        {
            int8_t fret = _decoder->ReadInt8();
            if(!_currentTrack->IsDrums()) {
                if(note) note->SetFret(note->HasFlag(RENote::DeadNote) ? 0 : fret);
            }
            else {
                if(note) note->SetPitchFromMIDI(fret);
            }
        }
        
        // Skip
        if (flag & 0x80) {
            _decoder->Skip(2);
        }
        
        // Custom Duration
        if (_version >= 500) {
            if (customDuration) {
                _decoder->Skip(8);
            }
            _decoder->Skip(sizeof(uint8_t));
        }
		
        // Read note effect
        if (flag & 0x08) 
        {
            ParseNoteEffects(note);
        }
        
        // Insert Note
        if(note != NULL) 
        {
            _currentChord->InsertNote(note, _currentChord->NoteCount());
        }
    }
}

void REGuitarProParser::ParseChordEffects()
{
    uint8_t flag = _decoder->ReadUInt8();
    uint8_t flag2 = (_version >= 400 ? _decoder->ReadUInt8() : 0);
    
    bool hasWideVibrato = (flag & 0x02);
    bool hasVibrato = (flag & 0x01);
    
    bool hasPickStroke = (flag2 & 0x02);
    bool hasTremoloBar = (flag2 & 0x04);
    
    if(flag & 0x20)
    {
        uint8_t effect = _decoder->ReadUInt8();
        if(effect == 1) _currentChord->SetFlag(REChord::Tap);
        if(effect == 2) _currentChord->SetFlag(REChord::Slap);
        if(effect == 3) _currentChord->SetFlag(REChord::Pop);
        
        // Tremolo Bar on V3 ignored
        if(_version == 300) {
            _decoder->Skip(4);
        }
    }
    
    // Tremolo Bar on V4+
    if (hasTremoloBar) 
    {
        _decoder->Skip(5);
        uint32_t nb = _decoder->ReadUInt32();
        for (int i=0; i<nb; i++) {
            _decoder->Skip(9);
        }
    }

    // Brush
    if (flag & 0x40) 
    {
        int8_t brushUp, brushDown;
        if (_version >= 500) {
            brushUp = _decoder->ReadInt8();
            brushDown = _decoder->ReadInt8();
        }
        else {
            brushDown = _decoder->ReadInt8();
            brushUp = _decoder->ReadInt8();
        }
        
        if(brushDown > 0) {
            _currentChord->SetFlag(REChord::Brush);
        }
        else if(brushUp > 0) {
            _currentChord->SetFlag(REChord::Brush);
            _currentChord->SetFlag(REChord::StrumUpwards);
        }
    }
    
    // Pickstroke
    if (hasPickStroke) {
        int8_t dir = _decoder->ReadInt8();
        _currentChord->SetFlag(REChord::PickStroke);
        if(dir == 1) _currentChord->SetFlag(REChord::StrumUpwards);
    }
}

void REGuitarProParser::ParseChord()
{
    uint8_t flag = _decoder->ReadUInt8();
    
    uint8_t beatState = 1;
    if (flag & 0x40) {
        beatState = _decoder->ReadUInt8();
    }
    
    Reflow::NoteValue rhythm = (Reflow::NoteValue) (_decoder->ReadInt8() + 2);
    _currentChord->SetNoteValue(rhythm);

    if (flag & 0x01) {
        _currentChord->SetDots(1);
    }
    
	int tuplet = 0;
	int tupletFor = 0;
    
    if (flag & 0x20) {
		tuplet = _decoder->ReadUInt32();
		switch(tuplet)
		{
            case  1: tupletFor = 1; break;
            case  3: tupletFor = 2; break;
            case  5: tupletFor = 4; break;
            case  6: tupletFor = 4; break;
            case  7: tupletFor = 4; break;
            case  9: tupletFor = 8; break;
            case 10: tupletFor = 8; break;
            case 11: tupletFor = 8; break;
            case 12: tupletFor = 8; break;
            case 13: tupletFor = 8; break;
            default:
                tuplet = tupletFor = 1;
        }
        _currentChord->SetTuplet(RETuplet(tuplet, tupletFor));
    }
    
    if (flag & 0x02) {
        ParseDiagram();
    }
    
    // Text on beat
    if (flag & 0x04) {
        _currentChord->SetTextAttached(ReadGPStringAlt());
    }
    
    // Chord Effects
    if (flag & 0x08) 
    {
        ParseChordEffects();
    }
    
    // Automations
    if (flag & 0x10) {
        _currentPhrase->Refresh();
        ParseAutomations();
    }
	
    // Parse Notes
	ParseNotes();
	
	
    if (_version >= 500) 
    {
        _decoder->Skip(1);
        uint8_t flag2 = _decoder->ReadUInt8();
        if (flag2 & 0x08) _decoder->Skip(1);
    }
}


std::string REGuitarProParser::ReadGPString()
{
    uint8_t count = _decoder->ReadUInt8();
    return _decoder->ReadBytes(count);
}

std::string REGuitarProParser::ReadGPStringFixed(unsigned long fixedLength)
{
    uint8_t count = _decoder->ReadUInt8();
    
    std::string str = _decoder->ReadBytes(count);
    _decoder->Skip(fixedLength - count);
    return str;
}

std::string REGuitarProParser::ReadGPStringAlt()
{
    uint32_t length = _decoder->ReadUInt32();
    _decoder->Skip(1);
    return _decoder->ReadBytes(length-1);
}



