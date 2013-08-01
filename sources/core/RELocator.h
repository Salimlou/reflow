#ifndef _RELOCATOR_H_
#define _RELOCATOR_H_

#include "RETypes.h"

class RELocator
{
public:
	RELocator();
	RELocator(const RESong* song);
	RELocator(const RESong* song, int barIndex, int trackIndex);
	RELocator(const RESong* song, int barIndex, int trackIndex, int voiceIndex);

	~RELocator();

	const RESong* Song() const {return _song;}
	const REBar* Bar() const ;
	const RETrack* Track() const;
	const REVoice* Voice() const;
	const REPhrase* Phrase() const;
	const REChord* Chord() const;
	const RENote* Note() const;

	int BarIndex() const {return _barIndex;}
	int TrackIndex() const {return _trackIndex;}
	int VoiceIndex() const {return _voiceIndex;}
	int ChordIndex() const {return _chordIndex;}
	int NoteIndex() const {return _noteIndex;}

	void SetBarIndex(int idx) {_barIndex = idx;}
	void SetTrackIndex(int idx) {_trackIndex = idx;}
	void SetVoiceIndex(int idx) {_voiceIndex = idx;}
	void SetChordIndex(int idx) {_chordIndex = idx;}
	void SetNoteIndex(int idx) {_noteIndex = idx;}

	const REPhrase* NextPhrase();
	const REChord* NextChord();
	const RENote* NextNote();

private:
	const RESong* _song;
	int16_t _barIndex;
	int16_t _chordIndex;
	int16_t _noteIndex;
	int8_t _trackIndex;
	int8_t _voiceIndex;
};

#endif
