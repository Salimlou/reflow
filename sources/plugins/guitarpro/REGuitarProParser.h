//
//  REGuitarProParser.h
//  Reflow
//
//  Created by Sebastien on 20/04/11.
//  Copyright 2011 Gargant Studios. All rights reserved.
//

// Note:
// Check out http://dguitar.sourceforge.net/GP4format.html for more info

#ifndef _REGUITARPROPARSER_H_
#define _REGUITARPROPARSER_H_

#include "RETypes.h"

class REGuitarProParser
{
public:
    REGuitarProParser();
    ~REGuitarProParser();
    
public:
    bool Parse(REInputStream* decoder, RESong* song=0);
    
    static bool IsHeaderValid(const char*data, int length);
    
public:
    const std::string& Error() const {return _error;}
    const RESong* const Song() {return _song;}
    unsigned int TrackCount() const {return _trackCount;}
    unsigned int BarCount() const {return _barCount;}
    
private:
    void ParseProperties();
    void ParseLyrics();
    void ParseDirections();
    void ParseBar();
    void ParseTrack();
    void ParsePhrase();
    void ParseChord();
    void ParseChordEffects();
    void ParseNotes();
    void ParseAutomations();
    void ParseDiagram();
    void ParseNoteEffects(RENote* note);
    void ParseBend(RENote* note);
    void ParseSlideV5(RENote* note);
    void ParseSlideV4(RENote* note);
    void ParseDiagramV3();
    void ParseDiagramV4With6Strings();
    void ParseDiagramV4With7Strings();
    
    void DispatchDirections();
    
private:
    std::string ReadGPString();
    std::string ReadGPStringFixed(unsigned long maxLength);
    std::string ReadGPStringAlt();
    
private:
    REInputStream* _decoder;
    RESong* _song;
    std::string _error;
    bool _ownSong;
    unsigned int _version;
    
    unsigned int _trackCount;
    unsigned int _barCount;
    unsigned int _stringCount;
    
    unsigned int _currentBarIndex;
    unsigned int _currentTrackIndex;
    unsigned int _currentVoiceIndex;
    
    RETrack* _currentTrack;
    REBar* _currentBar;
    REVoice* _currentVoice;
    REPhrase* _currentPhrase;
    REChord* _currentChord;
    
    int _midiProgram[4][16];
    int _midiVolume[4][16];
    int _midiPan[4][16];
    
    int16_t _coda;
    int16_t _doubleCoda;
    int16_t _segno;
    int16_t _segnoSegno;
    int16_t _fine;
    int16_t _daCapo;
    int16_t _daCapoAlCoda;
    int16_t _daCapoAlDoubleCoda;
    int16_t _daCapoAlFine;
    int16_t _dalSegno;
    int16_t _dalSegnoAlCoda;
    int16_t _dalSegnoAlDoubleCoda;
    int16_t _dalSegnoAlFine;
    int16_t _dalSegnoSegno;
    int16_t _dalSegnoSegnoAlCoda;
    int16_t _dalSegnoSegnoAlDoubleCoda;
    int16_t _dalSegnoSegnoAlFine;
    int16_t _toCoda;
    int16_t _toDoubleCoda;
};


#endif
