#include "RELocator.h"
#include "RESong.h"
#include "REBar.h"
#include "RETrack.h"
#include "REVoice.h"
#include "REPhrase.h"
#include "REChord.h"
#include "RENote.h"

RELocator::RELocator()
	: _song(0), _trackIndex(0), _barIndex(0), _voiceIndex(0), _chordIndex(0), _noteIndex(0)
{}

RELocator::RELocator(const RESong* song)
	: _song(song), _trackIndex(0), _barIndex(0), _voiceIndex(0), _chordIndex(0), _noteIndex(0)
{}

RELocator::RELocator(const RESong* song, int barIndex, int trackIndex)
	: _song(song), _trackIndex(trackIndex), _barIndex(barIndex), _voiceIndex(0), _chordIndex(0), _noteIndex(0)
{}

RELocator::RELocator(const RESong* song, int barIndex, int trackIndex, int voiceIndex)
	: _song(song), _trackIndex(trackIndex), _barIndex(barIndex), _voiceIndex(voiceIndex), _chordIndex(0), _noteIndex(0)
{}


RELocator::~RELocator()
{
}

const REBar* RELocator::Bar() const 
{
	return _song ? _song->Bar(_barIndex) : 0;
}

const RETrack* RELocator::Track() const
{
	return _song ? _song->Track(_trackIndex) : 0;
}

const REVoice* RELocator::Voice() const
{
	const RETrack* track = Track();
	return track ? track->Voice(_voiceIndex) : 0;
}

const REPhrase* RELocator::Phrase() const
{
	const REVoice* voice = Voice();
	return voice ? voice->Phrase(_barIndex) : 0;
}

const REChord* RELocator::Chord() const
{
	const REPhrase* phrase = Phrase();
	return phrase ? phrase->Chord(_chordIndex) : 0;
}

const RENote* RELocator::Note() const
{
	const REChord* chord = Chord();
	return chord ? chord->Note(_noteIndex) : 0;
}

const REPhrase* RELocator::NextPhrase()
{
	++_barIndex;
    _chordIndex = 0;
    _noteIndex = 0;
    return Phrase();
}

const REChord* RELocator::NextChord()
{
	++_chordIndex;
    _noteIndex = 0;
    return Chord();
}

const RENote* RELocator::NextNote()
{
	++_noteIndex;
    return Note();
}
