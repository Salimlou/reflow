//
//  RESong.h
//  ReflowMac
//
//  Created by Sebastien on 12/04/11.
//  Copyright 2011 Gargant Studios. All rights reserved.
//

#ifndef _RESong_H_
#define _RESong_H_

#include "RETypes.h"
#include "RETimeline.h"

class RESong
{
    friend class REArchive;
    
public:
    RESong();
    ~RESong();
    
public:
    const RETrackVector& Tracks() const {return _tracks;}
    RETrackVector& Tracks() {return _tracks;}
    
    const REBarVector& Bars() const {return _bars;}
    REBarVector& Bars() {return _bars;}
        
    unsigned int TrackCount() const {return (unsigned int)_tracks.size();}
    unsigned int BarCount() const {return (unsigned int)_bars.size();}
    unsigned int ScoreCount() const {return (unsigned int)_scores.size();}
    
    const RETrack* Track(int idx) const;
	RETrack* Track(int idx);
	
	const REBar* Bar(int idx) const;
	REBar* Bar(int idx);
    
    const REBar& GetBarOrThrow(int idx) const;
    REBar& GetBarOrThrow(int idx);
    
    const REScoreSettings* Score(int idx) const;
    REScoreSettings* Score(int idx);
    
    const std::string& Title() const {return _title;}
    const std::string& Artist() const {return _artist;}
    const std::string& SubTitle() const {return _subtitle;}
    const std::string& Album() const {return _album;}    
    const std::string& MusicBy() const {return _musicBy;}
    const std::string& LyricsBy() const {return _lyricsBy;}
    const std::string& Transcriber() const {return _transcriber;}
    const std::string& Copyright() const {return _copyright;}
    const std::string& Notice() const {return _notice;}
    
    void SetArtist(const std::string& artist) {_artist = artist;}
    void SetTitle(const std::string& title) {_title = title;}
    void SetSubTitle(const std::string& subtitle) {_subtitle = subtitle;}
    void SetAlbum(const std::string& album) {_album = album;}
    void SetMusicBy(const std::string& musicBy) {_musicBy = musicBy;}
    void SetLyricsBy(const std::string& lyricsBy) {_lyricsBy = lyricsBy;}
    void SetTranscriber(const std::string& transcriber) {_transcriber = transcriber;}
    void SetCopyright(const std::string& copyright) {_copyright = copyright;}
    void SetNotice(const std::string& notice) {_notice = notice;}
    
	void Clear();
    void Refresh(bool refreshTracksToo=false);
	
	void InsertTrack(RETrack* track, int idx);
	void RemoveTrack(int idx);
    void MoveTrack(int fromIndex, int toIndex);
    void DuplicateTrack(int fromIndex, int toIndex);
	
	void InsertBar(REBar* bar, int idx);
	void RemoveBar(int idx);
    void RemoveBarsInRange(int idx, int count);
    void CreateBar(int idx);
    
    int RehearsalSignCount() const;
    void FindRehearsalBarIndices(REIntVector& barIndices) const;
    int BarIndexOfRehearsalAtIndex(int rehearsalIndex) const;
    RERange BarRangeOfRehearsalAtIndex(int rehearsalIndex) const;
    const REBar* BarOfRehearsalAtIndex(int rehearsalIndex) const;
    
    void InsertScore(REScoreSettings* score, int idx);
    void RemoveScore(int idx);
    void MoveScore(int fromIndex, int toIndex);
    void DuplicateScore(int fromIndex, int toIndex);
    
    void SetDefaultTempo(int tempo) {_defaultTempo = tempo;}
    int DefaultTempo() const {return _defaultTempo;}
    const RETempoTimeline& TempoTimeline() const {return _tempoTimeline;}
    RETempoTimeline& TempoTimeline() {return _tempoTimeline;}
    int TempoAt(int barIndex, const RETimeDiv& timeDiv) const;
	
	void EncodeTo(REOutputStream& coder) const;
	void DecodeFrom(REInputStream& decoder);
    
    RESong* Clone() const;
    
    RESong* IsolateNewSongFromBarRange(int firstBar, int lastBar, const RETrackSet& trackSet, unsigned long flags=0) const;
    
    void CalculateTickSet(unsigned int barIndex, const RETrackSet& tracks, RETickSet& outTicks) const;
    
    bool IsBarCollapsibleWithNextSibling(int barIndex) const;
    
    const REPlaylistBarVector& Playlist() const {return _playlist;}
    void RefreshPlaylist();
    std::string PlaylistAsString() const;
    unsigned long PlaylistDurationInTicks() const;
    const REPlaylistBar* FirstOccurenceOfBarInPlaylist(int barIndex) const;
    REPlaylistBarVector* ClonePlaylist() const;
    
    const REBar* FindBarAtTick(int tick) const;
    
    unsigned long TotalDurationInTicks() const;
    
    REPhrase* PhraseAtLocator(const RELocator& phraseLocator);
    REChord* ChordAtLocator(const RELocator& chordLocator);
    RENote* NoteAtLocator(const RELocator& noteLocator);
    
    REScoreSettings* CreateFullScore();
    REScoreSettings* CreatePart(unsigned int trackIndex);
    REScoreSettings* CreateEmptyScoreAtEnd();
    
    int UnselectAllNotes(REIntSet* invalidatedBars);
    int FindSelectedNotes(REConstNoteSet* noteSet, REIntSet* affectedBars=NULL) const;
    int FindSelectedNotes(RENoteSet* noteSet, REIntSet* affectedBars=NULL);
    
public:
    void WriteJson(REJsonWriter& writer, uint32_t version) const;
    void ReadJson(const REJsonDocument& document, uint32_t version);
    
public:
    static RESong* CreateDefaultSong();
    
public:
    bool operator==(const RESong& rhs) const;
    bool operator!=(const RESong& rhs) const {return !(*this == rhs);}
	
private:
	void _UpdateIndices();
    void _ClearPlaylist();
	
private:
    RETrackVector _tracks;
    REBarVector _bars;
    REScoreSettingsVector _scores;
    REPlaylistBarVector _playlist;
    RETempoTimeline _tempoTimeline;
    std::string _title;
    std::string _subtitle;
    std::string _artist;
    std::string _album;
    std::string _musicBy;
    std::string _lyricsBy;
    std::string _transcriber;
    std::string _copyright;
    std::string _notice;
    int _defaultTempo;
    
    
    unsigned long _totalDurationInTicks;
};

#endif
