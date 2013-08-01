//
//  RESong.cpp
//  ReflowMac
//
//  Created by Sebastien on 12/04/11.
//  Copyright 2011 Gargant Studios. All rights reserved.
//


#include "RESong.h"
#include "RETrack.h"
#include "REBar.h"
#include "REVoice.h"
#include "REPhrase.h"
#include "REScore.h"
#include "REOutputStream.h"
#include "REInputStream.h"
#include "RETrackSet.h"
#include "REBarMetrics.h"
#include "REChord.h"
#include "RENote.h"
#include "RELocator.h"
#include "REPlaylistBar.h"
#include "RESongError.h"
#include "REPlaylistCompiler.h"
#include "REFunctions.h"
#include "REException.h"

RESong::RESong()
: _defaultTempo(90)
{
    
}

RESong::~RESong() 
{
    Clear();
}

void RESong::Clear() {
    for(RETrackVector::const_iterator it = _tracks.begin(); it != _tracks.end(); ++it) {
        delete *it;
    }
    _tracks.clear();
    
    for(REBarVector::const_iterator it = _bars.begin(); it != _bars.end(); ++it) {
        delete  *it;
    }
    _bars.clear();
    
    for(REScoreSettingsVector::const_iterator it = _scores.begin(); it != _scores.end(); ++it) {
        delete *it;
    }
    _scores.clear();
    
    _ClearPlaylist();
}

const RETrack* RESong::Track(int idx) const
{
    if(idx >= 0 && idx < _tracks.size()) {
        return _tracks[idx];
    }
    return 0;
}
RETrack* RESong::Track(int idx)
{
    if(idx >= 0 && idx < _tracks.size()) {
        return _tracks[idx];
    }
    return 0;
}

const REBar* RESong::Bar(int idx) const
{
    if(idx >= 0 && idx < _bars.size()) {
        return _bars[idx];
    }
    return 0;
}
REBar* RESong::Bar(int idx)
{
    if(idx >= 0 && idx < _bars.size()) {
        return _bars[idx];
    }
    return 0;    
}

const REBar& RESong::GetBarOrThrow(int idx) const
{
    const REBar* bar = Bar(idx);
    if(bar == NULL) {
        REThrow("Bar Index is out of bounds");
    }
    return *bar;
}

REBar& RESong::GetBarOrThrow(int idx)
{
    REBar* bar = Bar(idx);
    if(bar == NULL) {
        REThrow("Bar Index is out of bounds");
    }
    return *bar;
}

int RESong::TempoAt(int barIndex, const RETimeDiv& timeDiv) const
{
    const RETempoItem* item = _tempoTimeline.ItemAt(barIndex, timeDiv);
    return item ? item->tempo : DefaultTempo();
}

const REScoreSettings* RESong::Score(int idx) const
{
    if(idx >= 0 && idx < _scores.size()) {
        return _scores[idx];
    }
    return 0;
}
REScoreSettings* RESong::Score(int idx)
{
    if(idx >= 0 && idx < _scores.size()) {
        return _scores[idx];
    }
    return 0;
}

void RESong::InsertTrack(RETrack* track, int idx)
{
    _tracks.insert(_tracks.begin() + idx, track);
    track->_parent = this;
    _UpdateIndices();
}

void RESong::RemoveTrack(int idx)
{
    if(idx >= 0 && idx < _tracks.size()) {
        RETrack* track = _tracks[idx];
        delete track;
        _tracks.erase(_tracks.begin() + idx);
        _UpdateIndices();
    }
}

void RESong::CreateBar(int idx)
{
    REBar* bar = new REBar;
    InsertBar(bar, idx);
}

void RESong::InsertBar(REBar *bar, int idx)
{
    _bars.insert(_bars.begin() + idx, bar);
    bar->_parent = this;
    _UpdateIndices();
}

void RESong::RemoveBar(int idx)
{
    if(idx >= 0 && idx < _bars.size()) {
        REBar* bar = _bars[idx];
        delete bar;
        _bars.erase(_bars.begin() + idx);
        _UpdateIndices();
    }
}

static inline void _DeleteBar(REBar* bar) {delete bar;}

void RESong::RemoveBarsInRange(int idx, int count)
{
    std::for_each(_bars.begin()+idx, _bars.begin()+idx+count, std::bind(_DeleteBar, std::placeholders::_1));
    _bars.erase(_bars.begin()+idx, _bars.begin()+idx+count);
    _UpdateIndices();
}

REPhrase* RESong::PhraseAtLocator(const RELocator& phraseLocator)
{
    if(phraseLocator.Song() != this) return NULL;
    
    int trackIndex = phraseLocator.TrackIndex();
    int barIndex = phraseLocator.BarIndex();
    int voiceIndex = phraseLocator.VoiceIndex();
    
    RETrack* track = Track(trackIndex);
    REVoice* voice = (track != NULL ? track->Voice(voiceIndex) : NULL);
    REPhrase* phrase = (voice != NULL ? voice->Phrase(barIndex) : NULL);
    return phrase;
}

REChord* RESong::ChordAtLocator(const RELocator& chordLocator)
{
    if(chordLocator.Song() != this) return NULL;
    
    int chordIndex = chordLocator.ChordIndex();
    
    REPhrase* phrase = PhraseAtLocator(chordLocator);
    return (phrase != NULL ? phrase->Chord(chordIndex) : NULL);
}

RENote* RESong::NoteAtLocator(const RELocator &noteLocator)
{
    if(noteLocator.Song() != this) return NULL;
    
    int noteIndex = noteLocator.NoteIndex();
    
    REChord* chord = ChordAtLocator(noteLocator);
    return (chord ? chord->Note(noteIndex) : NULL);
}

void RESong::InsertScore(REScoreSettings *score, int idx)
{
    _scores.insert(_scores.begin() + idx, score);
    _UpdateIndices();
}

void RESong::RemoveScore(int idx)
{
    if(idx >= 0 && idx < _scores.size()) {
        REScoreSettings* score = _scores[idx];
        delete score;
        _scores.erase(_scores.begin() + idx);
        _UpdateIndices();
    }
}

void RESong::_UpdateIndices() {
    for(unsigned int i=0; i<_tracks.size(); ++i) {
        _tracks[i]->_index = i;
    }
    for(unsigned int i=0; i<_bars.size(); ++i) {
        _bars[i]->_index = i;
    }
    for(unsigned int i=0; i<_scores.size(); ++i) {
        _scores[i]->_index = i;
    }
}

void RESong::Refresh(bool refreshTracksToo)
{
    unsigned int nbBars = _bars.size();
    unsigned long offset = 0;
    for(unsigned int i=0; i<nbBars; ++i) 
    {
        REBar* bar = Bar(i);
        
        bar->_offsetInTicks = offset;
        unsigned long duration = bar->TheoricDurationInTicks();        
        offset += duration;
    }
    _totalDurationInTicks = offset;
    
    if(refreshTracksToo)
    {
        for(unsigned int i=0; i<_tracks.size(); ++i)
        {
            RETrack* track = _tracks[i];
            REBot* bot = track->Bot();
            if(bot)
            {
                for(unsigned int barIndex=0; barIndex<nbBars; ++barIndex) {
                    bot->GenerateBar(track, barIndex);
                }
            }
            track->Refresh();
        }
    }
    
    _tempoTimeline.RemoveIdenticalSiblingItems();
    
    RefreshPlaylist();
}

const REBar* RESong::FindBarAtTick(int tick) const
{
    if(_bars.empty()) return NULL;
    
    for(int i=_bars.size()-1; i>=0; --i) {
        const REBar* bar = Bar(i);
        if(bar->OffsetInTicks() <= tick) {
            return bar;
        }
    }
    return NULL;
}

unsigned long RESong::TotalDurationInTicks() const
{
    return _totalDurationInTicks;
}

int RESong::UnselectAllNotes(std::set<int>* invalidatedBars)
{
    int nbNotesUnselected = 0;
    int nbTracks = TrackCount();
    for(int trackIndex=0; trackIndex<nbTracks; ++trackIndex)
    {
        RETrack* track = Track(trackIndex);
        int nbVoices = track->VoiceCount();
        for(int voiceIndex=0; voiceIndex < nbVoices; ++voiceIndex)
        {
            REVoice* voice = track->Voice(voiceIndex);
            int nbPhrases = voice->PhraseCount();
            for(int phraseIndex=0; phraseIndex < nbPhrases; ++phraseIndex)
            {
                REPhrase* phrase = voice->Phrase(phraseIndex);
                int nbChords = phrase->ChordCount();
                for(int chordIndex=0; chordIndex < nbChords; ++chordIndex)
                {
                    REChord* chord = phrase->Chord(chordIndex);
                    int nbNotes = chord->NoteCount();
                    for(int noteIndex = 0; noteIndex < nbNotes; ++noteIndex)
                    {
                        RENote* note = chord->Note(noteIndex);
                        if(note->IsSelected())
                        {
                            if(invalidatedBars != NULL) invalidatedBars->insert(phraseIndex);
                            
                            note->SetSelected(false);
                            ++nbNotesUnselected;
                        }
                    }
                }
            }
        }
    }
    return nbNotesUnselected;
}

int RESong::FindSelectedNotes(REConstNoteSet* noteSet, REIntSet* affectedBars) const
{
    int nbNotesSelected = 0;
    int nbTracks = TrackCount();
    for(int trackIndex=0; trackIndex<nbTracks; ++trackIndex)
    {
        const RETrack* track = Track(trackIndex);
        int nbVoices = track->VoiceCount();
        for(int voiceIndex=0; voiceIndex < nbVoices; ++voiceIndex)
        {
            const REVoice* voice = track->Voice(voiceIndex);
            int nbPhrases = voice->PhraseCount();
            for(int phraseIndex=0; phraseIndex < nbPhrases; ++phraseIndex)
            {
                const REPhrase* phrase = voice->Phrase(phraseIndex);
                int nbChords = phrase->ChordCount();
                for(int chordIndex=0; chordIndex < nbChords; ++chordIndex)
                {
                    const REChord* chord = phrase->Chord(chordIndex);
                    int nbNotes = chord->NoteCount();
                    for(int noteIndex = 0; noteIndex < nbNotes; ++noteIndex)
                    {
                        const RENote* note = chord->Note(noteIndex);
                        if(note->IsSelected())
                        {
                            if(affectedBars != NULL) affectedBars->insert(phraseIndex);
                                if(noteSet != NULL) noteSet->insert(note);
                                
                            ++nbNotesSelected;
                        }
                    }
                }
            }
        }
    }
    return nbNotesSelected;
}

int RESong::FindSelectedNotes(RENoteSet* noteSet, REIntSet* affectedBars)
{
    int nbNotesSelected = 0;
    int nbTracks = TrackCount();
    for(int trackIndex=0; trackIndex<nbTracks; ++trackIndex)
    {
        RETrack* track = Track(trackIndex);
        int nbVoices = track->VoiceCount();
        for(int voiceIndex=0; voiceIndex < nbVoices; ++voiceIndex)
        {
            REVoice* voice = track->Voice(voiceIndex);
            int nbPhrases = voice->PhraseCount();
            for(int phraseIndex=0; phraseIndex < nbPhrases; ++phraseIndex)
            {
                REPhrase* phrase = voice->Phrase(phraseIndex);
                int nbChords = phrase->ChordCount();
                for(int chordIndex=0; chordIndex < nbChords; ++chordIndex)
                {
                    REChord* chord = phrase->Chord(chordIndex);
                    int nbNotes = chord->NoteCount();
                    for(int noteIndex = 0; noteIndex < nbNotes; ++noteIndex)
                    {
                        RENote* note = chord->Note(noteIndex);
                        if(note->IsSelected())
                        {
                            if(affectedBars != NULL) affectedBars->insert(phraseIndex);
                            if(noteSet != NULL) noteSet->insert(note);
                            
                            ++nbNotesSelected;
                        }
                    }
                }
            }
        }
    }
    return nbNotesSelected;
}

void RESong::WriteJson(REJsonWriter& writer, uint32_t version) const
{
    writer.StartObject();
    
    writer.String("bars");
    writer.StartArray();
    for(const REBar* bar : _bars) {bar->WriteJson(writer, version);}
    writer.EndArray();
    
    writer.String("tracks");
    writer.StartArray();
    for(const RETrack* track : _tracks) {track->WriteJson(writer, version);}
    writer.EndArray();
    
    // Scores
    writer.String("scores");
    writer.StartArray();
    for(const REScoreSettings* scoreSettings : _scores) {scoreSettings->WriteJson(writer, version);}
    writer.EndArray();
    
    writer.String("tempo_changes");
    _tempoTimeline.WriteJson(writer, version);
    
    writer.EndObject();
}

void RESong::ReadJson(const REJsonDocument& document, uint32_t version)
{
    Clear();
    
    const REJsonValue& bars = document["bars"];
    const REJsonValue& tracks = document["tracks"];
    const REJsonValue& scores = document["scores"];
    
    // Bars
    if(bars.IsArray())
    {
        for(auto it = bars.Begin(); it != bars.End(); ++it)
        {
            REBar* bar = new REBar;
            bar->_index = _bars.size();
            bar->_parent = this;
            _bars.push_back(bar);
            bar->ReadJson(*it, version);
        }
    }
    
    // Tracks
    if(tracks.IsArray())
    {
        for(auto it = tracks.Begin(); it != tracks.End(); ++it)
        {
            RETrack* track = new RETrack;
            track->_index = _tracks.size();
            track->_parent = this;
            _tracks.push_back(track);
            track->ReadJson(*it, version);
        }
    }
    
    // Scores
    if(scores.IsArray()) {
        
    }
    
    const REJsonValue& tempos = document["tempo_changes"];
    if(tempos.IsArray()) {
        _tempoTimeline.ReadJson(tempos, version);
    }
}

void RESong::EncodeTo(REOutputStream& coder) const
{
    coder.WriteString(_title);
    coder.WriteString(_subtitle);
    coder.WriteString(_artist);
    coder.WriteString(_album);
    coder.WriteString(_musicBy);
    coder.WriteString(_lyricsBy);
    coder.WriteString(_transcriber);
    coder.WriteString(_copyright);
    coder.WriteString(_notice);
    
    coder.WriteUInt32(BarCount());
    for(unsigned int i=0; i<_bars.size(); ++i) {
        _bars[i]->EncodeTo(coder);
    }
    
    coder.WriteUInt32(TrackCount());    
    for(unsigned int i=0; i<_tracks.size(); ++i) {
        _tracks[i]->EncodeTo(coder);
    }

    coder.WriteUInt32(ScoreCount());
    for(unsigned int i=0; i<_scores.size(); ++i) {
        _scores[i]->EncodeTo(coder);
    }
    
    _tempoTimeline.EncodeTo(coder);
    coder.WriteInt16(_defaultTempo);
}
void RESong::DecodeFrom(REInputStream& decoder)
{
    Clear();
    
    _title = decoder.ReadString();
    _subtitle = decoder.ReadString();
    _artist = decoder.ReadString();
    _album = decoder.ReadString();
    _musicBy = decoder.ReadString();
    _lyricsBy = decoder.ReadString();
    _transcriber = decoder.ReadString();
    _copyright = decoder.ReadString();
    _notice = decoder.ReadString();

    uint32_t nbBars = decoder.ReadUInt32();
    for(uint32_t i=0; i<nbBars; ++i)
    {
        REBar* bar = new REBar;
        bar->_index = i;
        bar->_parent = this;
        _bars.push_back(bar);
        bar->DecodeFrom(decoder);
    }
    
    uint32_t nbTracks = decoder.ReadUInt32();
    for(uint32_t i=0; i<nbTracks; ++i)
    {
        RETrack* track = new RETrack;
        track->_index = i;
        track->_parent = this;
        _tracks.push_back(track);
        track->DecodeFrom(decoder);
    }
    
    uint32_t nbScores = decoder.ReadUInt32();
    for(uint32_t i=0; i<nbScores; ++i)
    {
        REScoreSettings* score = new REScoreSettings;
        score->_index = i;
        _scores.push_back(score);
        score->DecodeFrom(decoder);
    }
    
    _tempoTimeline.DecodeFrom(decoder);
    if(decoder.Version() >= 150) {
        _defaultTempo = decoder.ReadInt16();
    }
    else _defaultTempo = 90;
    Refresh();
}

RESong* RESong::Clone() const
{
    REBufferOutputStream buf;
    EncodeTo(buf);
    
    REConstBufferInputStream input(buf.Data(), buf.Size());
    RESong* song = new RESong;
    song->DecodeFrom(input);
    
    for(RETrack* t : song->_tracks) {
        t->_deviceUUID = -1;
    }
    return song;
}

RESong* RESong::IsolateNewSongFromBarRange(int firstBar, int lastBar, const RETrackSet& trackSet, unsigned long cloneOptions) const
{
    RESong* song = new RESong;
    
    // Clone bars
    for(int barIndex=firstBar; barIndex<=lastBar; ++barIndex)
    {
        int clonedBarIndex = barIndex-firstBar;
        REBar* bar = this->Bar(barIndex)->Clone(cloneOptions);
        song->InsertBar(bar, clonedBarIndex);
    }
    
    // Clone Tracks
    for(unsigned int trackIndex = 0; trackIndex < TrackCount(); ++trackIndex)
    {
        if(trackSet.IsSet(trackIndex))
        {
            RETrack* clonedTrack = Track(trackIndex)->CloneKeepingPhrasesInRange(firstBar, lastBar, cloneOptions);
            song->InsertTrack(clonedTrack, song->TrackCount());
        }
    }
    
    song->SetDefaultTempo(DefaultTempo());
    return song;
}

bool RESong::IsBarCollapsibleWithNextSibling(int barIndex) const
{
    int barCount = BarCount();
    if(barIndex >= (barCount-1)) {
        return false;
    }
    
    const REBar* barA = Bar(barIndex);
    const REBar* barB = Bar(barIndex+1);
    if(barB->HasFlag(REBar::RehearsalSign)) return false;
    if(barB->HasTimeSignatureChange()) return false;
    if(barB->HasKeySignatureChange()) return false;
    if(barA->HasFlag(REBar::RepeatEnd) || barA->HasFlag(REBar::RepeatStart) || barA->HasAnyAlternateEnding()) return false;
    if(barB->HasFlag(REBar::RepeatEnd) || barB->HasFlag(REBar::RepeatStart) || barB->HasAnyAlternateEnding()) return false;
    if(barB->HasTempoMarker()) return false;
    
    return true;
}

bool RESong::operator==(const RESong& rhs) const
{
    if(TrackCount() != rhs.TrackCount() ||
       BarCount() != rhs.BarCount()) {
        return false;
    }
    
    for(unsigned int i=0; i<TrackCount(); ++i) {
        if(*Track(i) != *rhs.Track(i)) {
            return false;
        }
    }
    
    for(unsigned int i=0; i<BarCount(); ++i) {
        if(*Bar(i) != *rhs.Bar(i)) {
            return false;
        }
    }
    
    for(unsigned int i=0; i<ScoreCount(); ++i) {
        if(*Score(i) != *rhs.Score(i)) {
            return false;
        }
    }
    
    return true;
}

void RESong::CalculateTickSet(unsigned int barIndex, const RETrackSet& tracks, RETickSet& outTicks) const
{
    for(unsigned int i=0; i<_tracks.size(); ++i) 
    {
        if(!tracks.IsSet(i)) continue;
        
        const RETrack* track = Track(i);
        for(unsigned int v=0; v<track->VoiceCount(); ++v) 
        {
            const REVoice* voice = track->Voice(v);
            const REPhrase* phrase = voice->Phrase(barIndex);
            for(unsigned int c=0; c<phrase->ChordCount(); ++c)
            {
                const REChord* chord = phrase->Chord(c);
                outTicks.insert(chord->OffsetInTicks());
            }
        }
    }
}
void RESong::_ClearPlaylist()
{
    /*BOOST_FOREACH(REPlaylistBar* pbar, _playlist) {
        delete pbar;
    }*/
    _playlist.clear();
}

REPlaylistBarVector* RESong::ClonePlaylist() const
{
    return new REPlaylistBarVector(_playlist);
}

void RESong::RefreshPlaylist()
{
    _ClearPlaylist();
    
    RESongErrorVector errors;
    REPlaylistCompiler playlistCompiler;
    playlistCompiler.Compile(this, errors);
    
    const std::vector<int>& pl = playlistCompiler.Playlist();
    
    unsigned int nbBars = pl.size();
    unsigned int barIndex = 0;
    uint32_t tick = 0;
    for(int i=0; i<nbBars; ++i)
    {
        unsigned int barIndex = pl[i];
        const REBar* bar = Bar(barIndex);
        REPlaylistBar pbar;
        pbar._indexInSong = barIndex;
        pbar._indexInPlaylist = i;
        pbar._tick = tick;
        pbar._extendedTick = tick;
        pbar._timeSignature = bar->TimeSignature();
        
        uint32_t duration = Reflow::TimeDivToTicks(bar->TheoricDuration());
        pbar._duration = duration;
        pbar._extendedDuration = duration;
        tick += duration;
        
        _playlist.push_back(pbar);
        
        ++barIndex;
    }
}

const REPlaylistBar* RESong::FirstOccurenceOfBarInPlaylist(int barIndex) const
{
    for(unsigned int i=0; i<_playlist.size(); ++i) 
    {
        const REPlaylistBar* pbar = &_playlist[i];
        if(pbar->IndexInSong() == barIndex) {
            return pbar;
        }
    }
    return NULL;
}

unsigned long RESong::PlaylistDurationInTicks() const
{
    if(_playlist.empty()) {
        return 0;
    }

    const REPlaylistBar& pbar = _playlist.back();
    return pbar._tick + pbar._duration;
}

std::string RESong::PlaylistAsString() const
{
    std::ostringstream oss;
    for(unsigned int i=0; i<_playlist.size(); ++i) {
        const REPlaylistBar& pbar = _playlist[i];
        if(i!=0) {oss << ",";}
        oss << (1+pbar.IndexInSong());
    }
    return oss.str();
}

REScoreSettings* RESong::CreateFullScore()
{
    RETrackSet trackSet; trackSet.SetAll();
    
    REScoreSettings* fullScore = new REScoreSettings;
    fullScore->SetName("Full Score");
    InsertScore(fullScore, ScoreCount());
    fullScore->SetTrackSet(trackSet);
    return fullScore;
}

REScoreSettings* RESong::CreatePart(unsigned int trackIndex)
{
    RETrackSet trackSet;
    trackSet.Set(trackIndex);
    
    REScoreSettings* score = new REScoreSettings;
    score->SetName(Track(trackIndex)->Name());
    InsertScore(score, ScoreCount());
    score->SetTrackSet(trackSet);
    return score;
}

REScoreSettings* RESong::CreateEmptyScoreAtEnd()
{
    RETrackSet trackSet;
    
    REScoreSettings* score = new REScoreSettings;
    score->SetName("New Part");
    InsertScore(score, ScoreCount());
    score->SetTrackSet(trackSet);
    return score;
}

int RESong::RehearsalSignCount() const
{
    return std::count_if(_bars.begin(), _bars.end(), std::bind(&REBar::HasFlag, std::placeholders::_1, REBar::RehearsalSign));
}
void RESong::FindRehearsalBarIndices(REIntVector& barIndices) const
{
    for(const REBar* bar : _bars) {
        if(bar->HasFlag(REBar::RehearsalSign)) {
            barIndices.push_back(bar->Index());
        }
    }
}
int RESong::BarIndexOfRehearsalAtIndex(int rehearsalIndex) const
{
    REIntVector barIndices;
    FindRehearsalBarIndices(barIndices);
    if(rehearsalIndex >= 0 && rehearsalIndex < barIndices.size()) {
        return barIndices[rehearsalIndex];
    }
    return -1;
}
RERange RESong::BarRangeOfRehearsalAtIndex(int rehearsalIndex) const
{
    int rbarIndex = BarIndexOfRehearsalAtIndex(rehearsalIndex);
    int nextRBarIndex = BarIndexOfRehearsalAtIndex(rehearsalIndex + 1);
    
    int firstBarIndex = (rbarIndex == -1 ? 0 : rbarIndex);
    int lastBarIndex = (nextRBarIndex == -1 ? BarCount()-1 : nextRBarIndex-1);
    
    return RERange(firstBarIndex, lastBarIndex - firstBarIndex + 1);
}
const REBar* RESong::BarOfRehearsalAtIndex(int rehearsalIndex) const
{
    return Bar(BarIndexOfRehearsalAtIndex(rehearsalIndex));
}

void RESong::MoveScore(int fromIndex, int toIndex)
{
    REScoreSettings* score = Score(fromIndex);
    _scores.erase(_scores.begin() + fromIndex);
//    InsertScore(score, (fromIndex < toIndex ? toIndex - 1 : toIndex));
    InsertScore(score, toIndex);
}
void RESong::DuplicateScore(int fromIndex, int toIndex)
{
    REScoreSettings* score = Score(fromIndex)->Clone();
    InsertScore(score, toIndex);
}

void RESong::MoveTrack(int fromIndex, int toIndex)
{
    RETrack* track = Track(fromIndex);
    _tracks.erase(_tracks.begin() + fromIndex);
//    InsertTrack(track, (fromIndex < toIndex ? toIndex - 1 : toIndex));
    InsertTrack(track, toIndex);
    
    //std::for_each(_scores.begin(), _scores.end(), boost::bind(&REScore::SetDirty, _1));
}

void RESong::DuplicateTrack(int fromIndex, int toIndex)
{
    RETrack* track = Track(fromIndex)->Clone();
    InsertTrack(track, toIndex);

    //std::for_each(_scores.begin(), _scores.end(), boost::bind(&REScore::SetDirty, _1));
}

RESong* RESong::CreateDefaultSong()
{
    const int nbBars = 30;
    
    RESong* song = new RESong;
    for(int i=0; i<nbBars; ++i) {
        REBar* bar = new REBar;
        song->InsertBar(bar, i);
    }
    
    // Create Track
    RETrack* track = new RETrack(Reflow::TablatureTrack);
    song->InsertTrack(track, 0);
    track->SetName("Guitar");
    track->SetShortName("Gtr.");
    track->SetMIDIProgram(25);
    track->ClefTimeline(false).InsertItem(REClefItem(0, RETimeDiv(0), Reflow::TrebleClef, Reflow::Ottavia_8vb));
    for(unsigned int i=0; i<REFLOW_MAX_VOICES; ++i) {
        REVoice* voice = new REVoice;
        track->InsertVoice(voice, i);
        for(unsigned int j=0; j<song->BarCount(); ++j) {
            voice->InsertPhrase(new REPhrase, j);
        }
    }
        
    // Full score & part
    song->Refresh();
    song->CreatePart(0);
    
    return song;
}

