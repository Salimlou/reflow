//
//  REScoreController.cpp
//  ReflowTools
//
//  Created by Sebastien Bourgeois on 13/02/12.
//  Copyright (c) 2012 Gargant Studios. All rights reserved.
//

#include "REScoreController.h"
#include "RESongController.h"
#include "RESong.h"
#include "REScore.h"
#include "REException.h"
#include "REPhrase.h"
#include "REChord.h"
#include "RENote.h"
#include "REStaff.h"
#include "RETrack.h"
#include "REVoice.h"
#include "REBar.h"
#include "REFunctions.h"
#include "REScore.h"
#include "REViewport.h"
#include "RESystem.h"
#include "RESlice.h"
#include "REBarMetrics.h"
#include "RETable.h"


#include <sstream>
#include <cmath>


using namespace std;

REScoreController::REScoreController(RESongController* songController)
: _songController(songController), _scoreIndex(-1), _score(songController->Song()), _delegate(NULL), _viewport(NULL),_typingSecondDigit(false), _preferredSelection(REScoreController::CursorSelection), _inferredSelection(REScoreController::CursorSelection), _layoutType(Reflow::PageScoreLayout), _pageLayoutType(Reflow::VerticalPageLayout), _tool(Reflow::TablatureTool), _selectionEndPoint(REPoint(0,0)), _selectionStartPoint(REPoint(0,0)), _selectionRectVisible(false), _inspector(nullptr)
{
    _songController->_scoreControllers.push_back(this);
    
    _tools.insert(_tools.end(), Reflow::ToolCount, nullptr);
#ifdef REFLOW_2
    _tools[Reflow::NoTool] = nullptr;
    _tools[Reflow::HandTool] = new REHandTool(this);
    _tools[Reflow::ZoomTool] = new REZoomTool(this);
    _tools[Reflow::SelectTool] = new RESelectTool(this);
    _tools[Reflow::BarSelectTool] = new REBarSelectTool(this);
    _tools[Reflow::NoteTool] = new RENoteTool(this);
    _tools[Reflow::EraserTool] = new REEraserTool(this);
    _tools[Reflow::DesignTool] = new REDesignTool(this);
    _tools[Reflow::SymbolTool] = new RESymbolTool(this);
    _tools[Reflow::LineTool] = new RELineTool(this);
    _tools[Reflow::DiagramTool] = new REDiagramTool(this);
    _tools[Reflow::ClefTool] = new REClefTool(this);
    _tools[Reflow::KeyTool] = new REKeyTool(this);
    _tools[Reflow::TimeTool] = new RETimeTool(this);
    _tools[Reflow::BarlineTool] = new REBarlineTool(this);
    _tools[Reflow::RepeatTool] = new RERepeatTool(this);
    _tools[Reflow::TempoTool] = new RETempoTool(this);
    _tools[Reflow::SectionTool] = new RESectionTool(this);
    _tools[Reflow::LyricsTool] = new RELyricsTool(this);
    _tools[Reflow::TextTool] = new RETextTool(this);
    _tools[Reflow::LiveVelocityTool] = new RELiveVelocityTool(this);
    _tools[Reflow::LiveDurationTool] = new RELiveDurationTool(this);
    _tools[Reflow::LiveOffsetTool] = new RELiveOffsetTool(this);
    _tools[Reflow::SpacerTool] = new RESpacerTool(this);
    _tools[Reflow::MemoTool] = new REMemoTool(this);
#endif
    _tools[Reflow::TablatureTool] = new RETablatureTool(this);
    _tools[Reflow::SlurTool] = new RESlurTool(this);
    _inspector = new RETable();
}

REScoreController::~REScoreController()
{
    ClearViewport();
    
    for(RETool* tool : _tools) delete tool;
}

void REScoreController::_BackupCommandDataToStream(REOutputStream& stream)
{
	_songController->BackupToStream(stream);
}

void REScoreController::_RestoreCommandDataFromStream(REInputStream& stream)
{
	_songController->RestoreFromStream(stream);
}

void REScoreController::SongControllerWillModifySong(const RESongController* controller, const RESong* song)
{
    // Nothing to do
}

void REScoreController::SongControllerDidModifySong(const RESongController* controller, const RESong* song, bool successfully)
{
    _currentCursor.ForceValidPosition();
    _originCursor.ForceValidPosition();
    
    // Inform delegate that it should refresh its contents
    if(_delegate) {
        _delegate->ScoreControllerRefreshPresentation(this, Score());
    }
}

void REScoreController::SongControllerDidModifyPhrase(const RESongController* controller, const REPhrase* phrase, bool successfully)
{
	SongControllerDidModifySong(controller, phrase->Voice()->Track()->Song(), successfully);
}

const REScoreControllerAction* REScoreController::Action(const std::string& identifier) const
{
    REScoreControllerActionMap::const_iterator it = _actions.find(identifier);
    return it != _actions.end() ? it->second : NULL;
}

int REScoreController::SelectedNoteCount() const
{
    int count = 0;
    if(_inferredSelection == REScoreController::CursorSelection)
    {
        const RENote* note = _currentCursor.Note();
        if(note) ++count;
    }
    else if(_inferredSelection == REScoreController::TickRangeSelection)
    {
        REConstChordPair chords = SelectedChordRange();
        if(chords.first && chords.second)
        {
            const REChord* chord = chords.first;
            while(chord) 
            {
                count += chord->NoteCount();
                
                if(chord == chords.second) break;
                else chord = chord->NextSiblingOverMultipleBarlines();
            }
        }
    }
    else if(_inferredSelection == REScoreController::RectangleCursorSelection)
    {
        REConstChordPair chords = SelectedChordRange();
        if(chords.first && chords.second)
        {
            const REStaff* staff = _currentCursor.Staff();
            
            int firstLine = FirstSelectedLine();
            int lastLine = LastSelectedLine();            
            const REChord* chord = chords.first;
            while(chord) 
            {
                if(staff->Type() == Reflow::TablatureStaff) {
                    count += chord->NoteCountInStringRange(firstLine, lastLine);
                }
                else {
                    count += chord->NoteCountInLineRange(firstLine, lastLine, Score()->IsTransposing());
                }
                
                if(chord == chords.second) break;
                else chord = chord->NextSiblingOverMultipleBarlines(); 
            }
        }
    }
    else if(_inferredSelection == REScoreController::BarRangeSelection ||
            _inferredSelection == REScoreController::SingleTrackBarRangeSelection)
    {
        int firstBarIndex = FirstSelectedBarIndex();
        int lastBarIndex = LastSelectedBarIndex();
        
        REConstTrackVector tracks = SelectedTracks();
        for(const RETrack* track : tracks)
        {
            for(int barIndex = firstBarIndex; barIndex <= lastBarIndex; ++barIndex)
            {
                for(int voiceIndex = 0; voiceIndex < track->VoiceCount(); ++voiceIndex)
                {
                    const REPhrase* phrase = track->Voice(voiceIndex)->Phrase(barIndex);
                    for(const REChord* chord : phrase->Chords())
                    {
                        count += chord->NoteCount();
                    }
                }
            }
        }
    }
    return count;
}

void REScoreController::FindSelectedNotes(REConstNoteVector* notes) const
{
    if(notes == NULL) return;
    
    if(_inferredSelection == REScoreController::CursorSelection)
    {
        const RENote* note = _currentCursor.Note();
        if(note) notes->push_back(note);
    }
    else if(_inferredSelection == REScoreController::TickRangeSelection)
    {
        REConstChordPair chords = SelectedChordRange();
        if(chords.first && chords.second)
        {
            const REChord* chord = chords.first;
            while(chord) 
            {
                for(const RENote* note : chord->Notes()) {
                    notes->push_back(note);
                }
                
                if(chord == chords.second) break;
                else chord = chord->NextSiblingOverMultipleBarlines();
            }
        }
    }
    else if(_inferredSelection == REScoreController::RectangleCursorSelection)
    {
        REConstChordPair chords = SelectedChordRange();
        if(chords.first && chords.second)
        {
            const REStaff* staff = _currentCursor.Staff();
            
            int firstLine = FirstSelectedLine();
            int lastLine = LastSelectedLine();            
            const REChord* chord = chords.first;
            while(chord) 
            {
                if(staff->Type() == Reflow::TablatureStaff) {
                    chord->FindNotesInStringRange(notes, firstLine, lastLine);
                }
                else {
                    chord->FindNotesInLineRange(notes, firstLine, lastLine, _score.IsTransposing());
                }
                
                if(chord == chords.second) break;
                else chord = chord->NextSiblingOverMultipleBarlines(); 
            }
        }
    }
    else if(_inferredSelection == REScoreController::BarRangeSelection ||
            _inferredSelection == REScoreController::SingleTrackBarRangeSelection)
    {
        int firstBarIndex = FirstSelectedBarIndex();
        int lastBarIndex = LastSelectedBarIndex();
        
        REConstTrackVector tracks = SelectedTracks();
        for(const RETrack* track : tracks)
        {
            for(int barIndex = firstBarIndex; barIndex <= lastBarIndex; ++barIndex)
            {
                for(int voiceIndex = 0; voiceIndex < track->VoiceCount(); ++voiceIndex)
                {
                    const REPhrase* phrase = track->Voice(voiceIndex)->Phrase(barIndex);
                    for(const REChord* chord : phrase->Chords())
                    {
                        for(const RENote* note : chord->Notes())
                        {
                            notes->push_back(note);
                        }
                    }
                }
            }
        }
    }
}

void REScoreController::FindSelectedChords(REConstChordVector* chords) const
{
    if(_inferredSelection == REScoreController::CursorSelection)
    {
        const REChord* chord = _currentCursor.Chord();
        if(chord) chords->push_back(chord);
    }
    else if(_inferredSelection == REScoreController::TickRangeSelection || 
            _inferredSelection == REScoreController::RectangleCursorSelection)
    {
        REConstChordPair chordPair = SelectedChordRange();
        if(chordPair.first && chordPair.second)
        {
            const REChord* chord = chordPair.first;
            while(chord) {
                chords->push_back(chord);
                
                if(chord == chordPair.second) break;
                else chord = chord->NextSiblingOverMultipleBarlines();
            }
        }
    }
    else if(_inferredSelection == REScoreController::BarRangeSelection ||
            _inferredSelection == REScoreController::SingleTrackBarRangeSelection)
    {
        int firstBarIndex = FirstSelectedBarIndex();
        int lastBarIndex = LastSelectedBarIndex();
        
        REConstTrackVector tracks = SelectedTracks();
        for(const RETrack* track : tracks)
        {
            for(int barIndex = firstBarIndex; barIndex <= lastBarIndex; ++barIndex)
            {
                for(int voiceIndex = 0; voiceIndex < track->VoiceCount(); ++voiceIndex)
                {
                    const REPhrase* phrase = track->Voice(voiceIndex)->Phrase(barIndex);
                    for(const REChord* chord : phrase->Chords())
                    {
                        chords->push_back(chord);
                    }
                }
            }
        }
    }
}

REConstTrackVector REScoreController::SelectedTracks() const
{
    REConstTrackVector tracks;
    if(_inferredSelection == REScoreController::BarRangeSelection)
    {
        const REScore* score = Score();
        if(score)
        {
            const RESong* song = score->Song();
            for(int i=0; i<song->TrackCount(); ++i) {
                const RETrack* track = song->Track(i);
                tracks.push_back(track);
            }
        }
    }
    else
    {
        const RETrack* track = _currentCursor.Track();
        if(track) tracks.push_back(track);
    }
    return tracks;
}

bool REScoreController::AtLeastOneSelectedNoteVerifies(RENotePredicate pred) const
{
    if(_inferredSelection == REScoreController::CursorSelection)
    {
        const RENote* note = _currentCursor.Note();
        return note && pred(note);
    }
    else if(_inferredSelection == REScoreController::TickRangeSelection)
    {
        REConstChordPair chords = SelectedChordRange();
        if(chords.first && chords.second)
        {
            const REChord* chord = chords.first;
            while(chord) {
                if(chord->AtLeastOneNoteVerifies(pred)) return true;
                
                if(chord == chords.second) break;
                else chord = chord->NextSiblingOverMultipleBarlines();
            }
        }
    }
    else if(_inferredSelection == REScoreController::RectangleCursorSelection)
    {
        REConstChordPair chords = SelectedChordRange();
        if(chords.first && chords.second)
        {
            const REStaff* staff = _currentCursor.Staff();
            
            int firstLine = FirstSelectedLine();
            int lastLine = LastSelectedLine();            
            const REChord* chord = chords.first;
            while(chord) {
                if(staff->Type() == Reflow::TablatureStaff) {
                    if(chord->AtLeastOneNoteInStringRangeVerifies(pred, firstLine, lastLine)) return true;
                }
                else {
                    if(chord->AtLeastOneNoteInLineRangeVerifies(pred, firstLine, lastLine, Score()->IsTransposing())) return true;
                }
                
                if(chord == chords.second) break;
                else chord = chord->NextSiblingOverMultipleBarlines(); 
            }
        }
    }
    else if(_inferredSelection == REScoreController::BarRangeSelection ||
            _inferredSelection == REScoreController::SingleTrackBarRangeSelection)
    {
        return AtLeastOneSelectedChordVerifies(std::bind(&REChord::AtLeastOneNoteVerifies, std::placeholders::_1, pred));
    }
    return false;
}

bool REScoreController::AtLeastOneSelectedChordVerifies(REChordPredicate pred) const
{
    if(_inferredSelection == REScoreController::CursorSelection)
    {
        const REChord* chord = _currentCursor.Chord();
        return chord && pred(chord);
    }
    else if(_inferredSelection == REScoreController::TickRangeSelection || 
            _inferredSelection == REScoreController::RectangleCursorSelection)
    {
        REConstChordPair chords = SelectedChordRange();
        if(chords.first && chords.second)
        {
            const REChord* chord = chords.first;
            while(chord) {
                if(pred(chord)) return true;
                
                if(chord == chords.second) break;
                else chord = chord->NextSiblingOverMultipleBarlines();
            }
        }
    }
    else if(_inferredSelection == REScoreController::BarRangeSelection ||
            _inferredSelection == REScoreController::SingleTrackBarRangeSelection)
    {
        int firstBarIndex = FirstSelectedBarIndex();
        int lastBarIndex = LastSelectedBarIndex();
        
        REConstTrackVector tracks = SelectedTracks();
        for(const RETrack* track : tracks)
        {
            for(int barIndex = firstBarIndex; barIndex <= lastBarIndex; ++barIndex)
            {
                for(int voiceIndex = 0; voiceIndex < track->VoiceCount(); ++voiceIndex)
                {
                    const REPhrase* phrase = track->Voice(voiceIndex)->Phrase(barIndex);
                    for(const REChord* chord : phrase->Chords()) {
                        if(pred(chord)) return true;
                    }
                }
            }
        }
    }
    return false;
}

bool REScoreController::AllTheSelectedChordsVerify(REChordPredicate pred) const
{
    if(_inferredSelection == REScoreController::CursorSelection)
    {
        const REChord* chord = _currentCursor.Chord();
        return chord && pred(chord);
    }
    else if(_inferredSelection == REScoreController::TickRangeSelection || 
            _inferredSelection == REScoreController::RectangleCursorSelection)
    {
        REConstChordPair chords = SelectedChordRange();
        if(chords.first && chords.second)
        {
            const REChord* chord = chords.first;
            while(chord) {
                if(!pred(chord)) return false;
                
                if(chord == chords.second) break;
                else chord = chord->NextSiblingOverMultipleBarlines();
            }
        }
    }
    else if(_inferredSelection == REScoreController::BarRangeSelection ||
            _inferredSelection == REScoreController::SingleTrackBarRangeSelection)
    {
        int firstBarIndex = FirstSelectedBarIndex();
        int lastBarIndex = LastSelectedBarIndex();
        
        REConstTrackVector tracks = SelectedTracks();
        for(const RETrack* track : tracks)
        {
            for(int barIndex = firstBarIndex; barIndex <= lastBarIndex; ++barIndex)
            {
                for(int voiceIndex = 0; voiceIndex < track->VoiceCount(); ++voiceIndex)
                {
                    const REPhrase* phrase = track->Voice(voiceIndex)->Phrase(barIndex);
                    for(const REChord* chord : phrase->Chords()) {
                        if(!pred(chord)) return false;
                    }
                }
            }
        }
    }
    return true;
}

void REScoreController::PerformTaskOnSelectedGraceNote(RENoteOperation op, const std::string& taskName, unsigned long flags)
{
    assert(_editingGraceNote);
    if(_inferredSelection == REScoreController::CursorSelection)
    {
        RELockSongControllerForTask task(_songController, taskName, flags);
        
        const RENote* note = _currentCursor.Note();
        if(note)
        {
            RENote* lockedNote = task.LockNote(note);
            REGraceNote* graceNote = lockedNote->GraceNote(_graceNoteIndex);
            if(graceNote)
            {
                op(graceNote);
            }
        }
        
        task.Commit();
    }
}

void REScoreController::PerformTaskOnSelectedNotes(RENoteOperation op, const std::string& taskName, unsigned long flags)
{
    if(_inferredSelection == REScoreController::CursorSelection)
    {
        RELockSongControllerForTask task(_songController, taskName, flags);
        
        const RENote* note = _currentCursor.Note();
        if(note) 
        {
            RENote* lockedNote = task.LockNote(note);
            op(lockedNote);
        }
        
        task.Commit();
    }
    else if(_inferredSelection == REScoreController::RectangleCursorSelection)
    {
        REConstChordPair chords = SelectedChordRange();
        if(chords.first && chords.second)
        {
            const REStaff* staff = _currentCursor.Staff();
            RELockSongControllerForTask task(_songController, taskName, flags);
            
            int firstLine = FirstSelectedLine();
            int lastLine = LastSelectedLine();            
            const REChord* chord = chords.first;
            while(chord) {
                REChord* lockedChord = task.LockChord(chord);
                if(staff->Type() == Reflow::TablatureStaff) {
                    lockedChord->PerformOperationOnNotesInStringRange(op, firstLine, lastLine);
                }
                else {
                    lockedChord->PerformOperationOnNotesInLineRange(op, firstLine, lastLine, _score.IsTransposing());
                }
                
                if(chord == chords.second) break;
                else chord = chord->NextSiblingOverMultipleBarlines();
            }
            
            task.Commit();
        }
    }
    else if(_inferredSelection == REScoreController::TickRangeSelection)
    {
        PerformTaskOnSelectedChords(std::bind(&REChord::PerformOperationOnAllNotes, std::placeholders::_1, op), taskName);
    }
    else if(_inferredSelection == REScoreController::BarRangeSelection ||
            _inferredSelection == REScoreController::SingleTrackBarRangeSelection)
    {
        PerformTaskOnSelectedChords(std::bind(&REChord::PerformOperationOnAllNotes, std::placeholders::_1, op), taskName);
    }
}

void REScoreController::PerformTaskOnSelectedChords(REChordOperation op, const std::string& taskName, unsigned long flags)
{
    if(_inferredSelection == REScoreController::CursorSelection)
    {
        RELockSongControllerForTask task(_songController, taskName, flags);
        
        const REChord* chord = _currentCursor.Chord();
        if(chord) {
            op(task.LockChord(chord));
        }
        
        task.Commit();
    }
    else if(_inferredSelection == REScoreController::TickRangeSelection ||
            _inferredSelection == REScoreController::RectangleCursorSelection)
    {
        REConstChordPair chords = SelectedChordRange();
        if(chords.first && chords.second)
        {
            REGlobalTimeDiv t0 = _currentCursor.Beat();
            REGlobalTimeDiv t1 = _originCursor.Beat();
            REGlobalTimeDiv lastBeat = (t0 <= t1 ? t1 : t0);
            
            RELockSongControllerForTask task(_songController, taskName, flags);
            
            const REChord* chord = chords.first;
            while(chord) {
                op(task.LockChord(chord));
                
                if(chord == chords.second) break;
                else chord = chord->NextSiblingOverMultipleBarlines();
            }
            
            task.Commit();
            
            task.LockPhrase(chords.second->Phrase())->Refresh();
            
            // Adjust selection
            if(t0 <= t1) {
                t1.timeDiv = chords.second->Offset();
                _originCursor.SetBeat(t1);
            }
            else {
                t0.timeDiv = chords.second->Offset();
                _currentCursor.SetBeat(t0);
            }            
        }
    }
    else if(_inferredSelection == REScoreController::BarRangeSelection ||
            _inferredSelection == REScoreController::SingleTrackBarRangeSelection)
    {
        RELockSongControllerForTask task(_songController, taskName, flags);
        
        int firstBarIndex = FirstSelectedBarIndex();
        int lastBarIndex = LastSelectedBarIndex();
        
        REConstTrackVector tracks = SelectedTracks();
        for(const RETrack* track : tracks)
        {
            RETrack* lockedTrack = task.LockTrack(track);
            
            for(int barIndex = firstBarIndex; barIndex <= lastBarIndex; ++barIndex)
            {
                for(int voiceIndex = 0; voiceIndex < track->VoiceCount(); ++voiceIndex)
                {
                    REPhrase* phrase = lockedTrack->Voice(voiceIndex)->Phrase(barIndex);
                    std::for_each(phrase->Chords().begin(), phrase->Chords().end(), op);
                }
            }
        }
        
        task.Commit();
    }
}

const REStaff* REScoreController::FirstSelectedStaff() const
{
    return _currentCursor.Staff();
}

const RETrack* REScoreController::FirstSelectedTrack() const
{
    return _currentCursor.Track();
}

const REChord* REScoreController::FirstSelectedChord() const
{
    if(_inferredSelection == REScoreController::CursorSelection) 
    {
        return _currentCursor.Chord();
    }
    else if(_inferredSelection == REScoreController::RectangleCursorSelection ||
            _inferredSelection == REScoreController::TickRangeSelection)
    {
        REGlobalTimeDiv t0 = _currentCursor.Beat();
        REGlobalTimeDiv t1 = _originCursor.Beat();
        return (t0 <= t1 ? _currentCursor.Chord() : _originCursor.Chord());
    }    
    return NULL;
}

const REChord* REScoreController::LastSelectedChord() const
{
    if(_inferredSelection == REScoreController::CursorSelection) 
    {
        return _currentCursor.Chord();
    }
    else if(_inferredSelection == REScoreController::RectangleCursorSelection ||
            _inferredSelection == REScoreController::TickRangeSelection)
    {
        REGlobalTimeDiv t0 = _currentCursor.Beat();
        REGlobalTimeDiv t1 = _originCursor.Beat();
        return (t0 <= t1 ? _originCursor.Chord() : _currentCursor.Chord());
    }    
    return NULL;
}

REConstChordPair REScoreController::SelectedChordRange() const
{
    return REConstChordPair(FirstSelectedChord(), LastSelectedChord());
}

REGlobalTimeDiv REScoreController::FirstSelectedBeat() const
{
    if(_inferredSelection == REScoreController::CursorSelection) 
    {
        return _currentCursor.Beat();
    }
    else 
    {
        REGlobalTimeDiv t0 = _currentCursor.Beat();
        REGlobalTimeDiv t1 = _originCursor.Beat();
        return (t0 <= t1 ? t0 : t1);
    }    
}

REGlobalTimeDiv REScoreController::LastSelectedBeat() const
{
    if(_inferredSelection == REScoreController::CursorSelection) 
    {
        return _currentCursor.Beat();
    }
    else 
    {
        REGlobalTimeDiv t0 = _currentCursor.Beat();
        REGlobalTimeDiv t1 = _originCursor.Beat();
        return (t0 <= t1 ? t1 : t0);
    }        
}

int REScoreController::FirstSelectedLine() const
{
    if(_inferredSelection == REScoreController::CursorSelection) {
        return _currentCursor.LineIndex();
    }
    int a = _currentCursor.LineIndex();
    int b = _originCursor.LineIndex();
    return std::min<int>(a,b);
}

int REScoreController::LastSelectedLine() const
{
    if(_inferredSelection == REScoreController::CursorSelection) {
        return _currentCursor.LineIndex();
    }
    int a = _currentCursor.LineIndex();
    int b = _originCursor.LineIndex();
    return std::max<int>(a,b);    
}

const REBar* REScoreController::FirstSelectedBar() const
{
    return _songController->Song()->Bar(FirstSelectedBarIndex());
}

const REBar* REScoreController::LastSelectedBar() const
{
    return _songController->Song()->Bar(LastSelectedBarIndex());    
}

int REScoreController::FirstSelectedBarIndex() const
{
    if(_inferredSelection == REScoreController::CursorSelection) {
        return _currentCursor.BarIndex();
    }
    int a = _currentCursor.BarIndex();
    int b = _originCursor.BarIndex();
    return std::min<int>(a,b);
}

int REScoreController::LastSelectedBarIndex() const
{
    if(_inferredSelection == REScoreController::CursorSelection) {
        return _currentCursor.BarIndex();
    }
    int a = _currentCursor.BarIndex();
    int b = _originCursor.BarIndex();
    return std::max<int>(a,b);
}

const REPhrase* REScoreController::FirstSelectedPhrase() const
{
    if(_inferredSelection == REScoreController::BarRangeSelection ||
       _inferredSelection == REScoreController::SingleTrackBarRangeSelection) return NULL;
    
    int firstBarIndex = FirstSelectedBarIndex();
    const REVoice* voice = _currentCursor.Voice();
    return (voice != NULL ? voice->Phrase(firstBarIndex) : NULL);
}

const REPhrase* REScoreController::LastSelectedPhrase() const
{
    if(_inferredSelection == REScoreController::BarRangeSelection ||
       _inferredSelection == REScoreController::SingleTrackBarRangeSelection) return NULL;
    
    int lastBarIndex = LastSelectedBarIndex();
    const REVoice* voice = _currentCursor.Voice();
    return (voice != NULL ? voice->Phrase(lastBarIndex) : NULL);    
}

void REScoreController::SetEditLowVoice(bool editLowVoice)
{
    _currentCursor.SetVoiceSelection(editLowVoice ? RECursor::LowVoiceSelection : RECursor::HighVoiceSelection);
    _originCursor.SetVoiceSelection(_currentCursor.VoiceSelection());
    
    _currentCursor.ForceValidPosition(true);
    if(ExtendedSelection()) _originCursor.ForceValidPosition(true);
    
    UpdateActions();
    if(_delegate) _delegate->ScoreControllerScoreDidChange(this, Score());
}

bool REScoreController::IsEditingLowVoice() const
{
    return _currentCursor.VoiceSelection() == RECursor::LowVoiceSelection;
}

void REScoreController::ResetScore()
{
    SetScoreIndex(_scoreIndex);
}

void REScoreController::SetScoreIndex(int scoreIndex)
{
    SetScoreIndex(scoreIndex, _layoutType, _pageLayoutType);
}

void REScoreController::SetScoreIndex(int scoreIndex, Reflow::ScoreLayoutType lt, Reflow::PageLayoutType plt)
{
    int scoreCount = _songController->Song()->ScoreCount();
    
    _scoreIndex = scoreIndex;
    if(_scoreIndex < 0) _scoreIndex = 0;
    if(_scoreIndex >= scoreCount) _scoreIndex = scoreCount - 1;
    const REScoreSettings* scoreSettings = _songController->_song->Score(_scoreIndex);
    
    // Rebuild score with new settings
	ClearViewport();
    _layoutType = lt;
    _pageLayoutType = plt;
    _score.SetLayoutType(_layoutType);
    _score.SetPageLayoutType(_pageLayoutType);
    
    _score.Rebuild(*scoreSettings);
    
    _currentCursor.SetScore(&_score);
    _originCursor.SetScore(&_score);
    
    _currentCursor.ForceValidPosition();
    _originCursor.ForceValidPosition();
    
    // Rebuild viewport
    RebuildViewport();
    
    // Inform the delegate that the score did change
    UpdateActions();
    if(_delegate) {
        _delegate->ScoreControllerScoreDidChange(this, &_score);
    }
}

void REScoreController::SetTool(Reflow::ToolType tool)
{
    _tool = tool;
    ResetScore();
}

RETool* REScoreController::CurrentTool()
{
    return _tools[(int)_tool];
}

Reflow::ToolType REScoreController::CurrentToolType() const
{
    return _tool;
}

int REScoreController::ScoreIndex() const
{
    return _scoreIndex;
}

const REScore* REScoreController::Score() const
{
    //return _songController->Song()->Score(_scoreIndex);
    return &_score;
}

REScore* REScoreController::Score()
{
    //return _songController->Song()->Score(_scoreIndex);
    return &_score;
}

const REScoreSettings* REScoreController::ScoreSettings() const
{
    return _songController->Song()->Score(_scoreIndex);
}

bool REScoreController::ExtendedSelection() const
{
    return InferredSelectionKind() != CursorSelection;
}

REScoreController::SelectionKindType REScoreController::InferredSelectionKind() const
{
    return _inferredSelection;
}

void REScoreController::SetPreferredSelectionKind(REScoreController::SelectionKindType kind)
{
    _preferredSelection = kind;
    if(_preferredSelection == REScoreController::CursorSelection) {
        _originCursor = _currentCursor;
    }
    _InferSelectionTypeWithCursor();
    
    // Inform our delegate that the selection has just changed
    UpdateActions();
    if(_viewport) {
        _viewport->RepositionTabCursor();
        _viewport->RebuildManipulators();
    }
}

void REScoreController::_InferSelectionTypeWithCursor()
{
    // Basic single cursor selection (not extended)
    if(_preferredSelection == REScoreController::CursorSelection)
    {
        _inferredSelection = REScoreController::CursorSelection;
    }
    
    // Rectangle selection
    else if(_preferredSelection == REScoreController::RectangleCursorSelection) 
    {
        bool sameBar = (_currentCursor.BarIndex() == _originCursor.BarIndex());
        
        if(_currentCursor.Staff() == _originCursor.Staff())
        {
            if(sameBar) {
                _inferredSelection = REScoreController::RectangleCursorSelection;
            }
            else {
                _inferredSelection = REScoreController::SingleTrackBarRangeSelection;
            }
        }
        else if(_currentCursor.Track() == _originCursor.Track())
        {
            if(sameBar) {
                _inferredSelection = REScoreController::TickRangeSelection;
            }
            else {
                _inferredSelection = REScoreController::SingleTrackBarRangeSelection;
            }
        }
        else {
            _inferredSelection = REScoreController::BarRangeSelection;
        }
    }
    
    // We want a Tick Range selection
    else if(_preferredSelection == REScoreController::TickRangeSelection)
    {
        bool sameBar = (_currentCursor.BarIndex() == _originCursor.BarIndex());
        
        if(_currentCursor.Track() != _originCursor.Track())
        {
            _inferredSelection = REScoreController::BarRangeSelection;
        }
        else
        {
            if(sameBar) {
                _inferredSelection = REScoreController::TickRangeSelection;
            }
            else {
                _inferredSelection = REScoreController::SingleTrackBarRangeSelection;
            }
        }
    }
    
    // Single Track Bar Range selection
    else if(_preferredSelection == REScoreController::SingleTrackBarRangeSelection)
    {
        if(_currentCursor.Track() == _originCursor.Track())
        {
            _inferredSelection = REScoreController::SingleTrackBarRangeSelection;
        }
        else {
            _inferredSelection = REScoreController::BarRangeSelection;
        }
    }
    
    // Bar Range
    else {
        _inferredSelection = REScoreController::BarRangeSelection;
    }
}


REScoreController::SelectionKindType REScoreController::_StartOrContinueExtendedSelection(unsigned long flags)
{
    bool shiftDown = flags & REScoreController::CursorShiftDown;
    bool cmdDown = flags & REScoreController::CursorCmdDown;
    bool altDown = flags & REScoreController::CursorAltDown;
    
    // We extend our selection
    REScoreController::SelectionKindType newSelection = CursorSelection;
    if(shiftDown) 
    {
        // Start an extended selection
        if(!ExtendedSelection()) {
            _originCursor = _currentCursor;
        }
        
        // Preferred selection type varies with flags
        if(cmdDown) {
            newSelection = SingleTrackBarRangeSelection;
        }
        else if(altDown) {
            newSelection = RectangleCursorSelection;
        }
        else {
            newSelection = TickRangeSelection;
        }
    }
    return newSelection;
}

void REScoreController::_MoveCursorLeft()
{
    const REPhrase* phrase = _currentCursor.Phrase();
    if(phrase == NULL) return;
    
    int barIndex = phrase->Index();
    int chordIndex = _currentCursor.ChordIndex();
    if(chordIndex <= 0) 
    {
        const REPhrase* prevPhrase = phrase->PreviousSibling();
        if(prevPhrase)
        {
            _currentCursor.SetBarIndex(barIndex-1);
            if(prevPhrase->ChordCount() == 0) {
                _currentCursor.SetTick(0);
            }
            else {
                const REChord* lastChord = prevPhrase->LastChord();
                _currentCursor.SetTimeDiv(lastChord ? lastChord->Offset() : 0);
            }
        }
    }
    else {
        const REChord* chord = _currentCursor.Chord();
        const REChord* prevChord = (chord ? chord->PreviousSibling() : NULL);
        _currentCursor.SetTimeDiv(prevChord ? prevChord->Offset() : 0);
    }
    
    _currentCursor.ForceValidPosition(true);
    _typingSecondDigit = false;
    _editingGraceNote = false;
}

void REScoreController::_MoveCursorRight()
{
    const REPhrase* phrase = _currentCursor.Phrase();
    if(phrase == NULL) return;
    
    int barIndex = phrase->Index();
    int chordIndex = _currentCursor.ChordIndex();
    int chordCount = phrase->ChordCount();
    if(chordCount == 0)
    {
        const REPhrase* nextPhrase = phrase->NextSibling();
        if(nextPhrase == NULL)
        {
            if(_songController->IsEditable(0)) {
                AddBarAfterSelection();
            }
        }
        
        _currentCursor.SetBeat(REGlobalTimeDiv(barIndex + 1, RETimeDiv(0)));
    }
    else if(chordIndex >= chordCount-1) 
    {
        if(chordIndex >= chordCount) {
            chordIndex = chordCount-1;
        }
        const REChord* chord = phrase->Chord(chordIndex);
        if(phrase->IsBarCompleteOrExceeded())
        {
            const REPhrase* nextPhrase = phrase->NextSibling();
            if(nextPhrase == NULL && _songController->IsEditable(0)) {
                AddBarAfterSelection();
            }
            _currentCursor.SetBeat(REGlobalTimeDiv(barIndex + 1, RETimeDiv(0)));            
        }
        else {
            unsigned long newTick = chord->OffsetInTicks() + chord->DurationInTicks();
            
            // Create a new chord just on the right
            AddChordAfterCursor();
            _currentCursor.SetTick(newTick);
        }
    }
    else {
        const REChord* chord = _currentCursor.Chord();
        const REChord* nextChord = (chord ? chord->NextSibling() : NULL);
        _currentCursor.SetTimeDiv(nextChord ? nextChord->Offset() : 0);
    }
    
    _currentCursor.ForceValidPosition(true);
    _typingSecondDigit = false;
    _editingGraceNote = false;
}

void REScoreController::UpdateActions()
{
    RefreshInspectorTable();
}

void REScoreController::RefreshInspectorTable()
{
    if(_delegate) {
        _delegate->ScoreControllerInspectorWillReload(this);
    }
    
    _inspector->Clear();
    {
        // Selected Notes
        if(_tool == Reflow::TablatureTool)
        {
            REConstNoteVector notes;
            FindSelectedNotes(&notes);
            if(notes.size() == 1)
            {
                const RENote* note = notes.front();
                
                RETableSection* noteSection = new RETableSection("Note");
                _inspector->AddSection(noteSection);
                
                RETableRow* pitchRow = new RETableRow();
                {
                    std::ostringstream oss; oss << "Pitch " << (int)note->Pitch().midi;
                    pitchRow->SetTitle(oss.str());
                    noteSection->AddRow(pitchRow);
                }
                
                RETableRow* velocityRow = new RETableRow();
                {
                    std::ostringstream oss; oss << "Live Velocity " << (int)note->LiveVelocity();
                    velocityRow->SetTitle(oss.str());
                    noteSection->AddRow(velocityRow);
                }
            }
        }
        else
        {
            REConstNoteSet notes;
            if(_score.Song()->FindSelectedNotes(&notes))
            {
                std::ostringstream oss; oss << notes.size() << " note(s)";
                RETableSection* noteSection = new RETableSection(oss.str());
                _inspector->AddSection(noteSection);            


                RETableRow* pitchRow = new RETableRow();
                pitchRow->SetTitle("Pitch");
                noteSection->AddRow(pitchRow);
                
                RETableRow* velocityRow = new RETableRow();
                velocityRow->SetTitle("Live Velocity");
                noteSection->AddRow(velocityRow);
                
            }
        }
    }
    _inspector->UpdateIndices();
    
    // This will refresh the inspector presentation
    if(_delegate) {
        _delegate->ScoreControllerInspectorDidReload(this);
    }
}

void REScoreController::MoveCursorRight(unsigned long flags)
{
    REScoreController::SelectionKindType newSelection = _StartOrContinueExtendedSelection(flags);
    
    // Move cursor
    _MoveCursorRight();
    
    // New selection kind (will refresh the score)
    SetPreferredSelectionKind(newSelection);
    
    // Center on cursor if needed
    if(_delegate && _inferredSelection == REScoreController::CursorSelection) {
        _delegate->OnShouldCenterOnCursor(this);
    }
}

void REScoreController::MoveCursorLeft(unsigned long flags)
{
    REScoreController::SelectionKindType newSelection = _StartOrContinueExtendedSelection(flags);
    
    // Move cursor
    _MoveCursorLeft();
    
    // New selection kind (will refresh the score)
    SetPreferredSelectionKind(newSelection);

    // Center on cursor if needed
    if(_delegate && _inferredSelection == REScoreController::CursorSelection) {
        _delegate->OnShouldCenterOnCursor(this);
    }
}

void REScoreController::MoveCursorUp(unsigned long flags)
{
    REScoreController::SelectionKindType newSelection = _StartOrContinueExtendedSelection(flags);
    
    // Move Cursor
    _currentCursor.SetLineIndex(_currentCursor.LineIndex() - 1);
    _currentCursor.ForceValidPosition(true);
    _typingSecondDigit = false;
    _editingGraceNote = false;
    
    // New selection kind (will refresh the score)
    SetPreferredSelectionKind(newSelection);
    
    // Center on cursor if needed
    if(_delegate && _inferredSelection == REScoreController::CursorSelection) {
        _delegate->OnShouldCenterOnCursor(this);
    }
}

void REScoreController::MoveCursorDown(unsigned long flags)
{
    REScoreController::SelectionKindType newSelection = _StartOrContinueExtendedSelection(flags);
    
    // Move Cursor
    _currentCursor.SetLineIndex(_currentCursor.LineIndex() + 1);
    _currentCursor.ForceValidPosition(true);
    _typingSecondDigit = false;
    _editingGraceNote = false;
    
    // New selection kind (will refresh the score)
    SetPreferredSelectionKind(newSelection);
    
    // Center on cursor if needed
    if(_delegate && _inferredSelection == REScoreController::CursorSelection) {
        _delegate->OnShouldCenterOnCursor(this);
    }
}

void REScoreController::MoveCursorBy(int delta, unsigned long flags)
{
    REScoreController::SelectionKindType newSelection = _StartOrContinueExtendedSelection(flags);
    
    // Move Cursor
    _currentCursor.SetLineIndex(_currentCursor.LineIndex() + delta);
    _currentCursor.ForceValidPosition(true);
    _typingSecondDigit = false;
    _editingGraceNote = false;
    
    // New selection kind (will refresh the score)
    SetPreferredSelectionKind(newSelection);
    
    // Center on cursor if needed
    if(_delegate && _inferredSelection == REScoreController::CursorSelection) {
        _delegate->OnShouldCenterOnCursor(this);
    }
}

void REScoreController::MoveCursorTo(int staffIndex, int barIndex, int tick, int lineIndex, unsigned long flags)
{
    REScoreController::SelectionKindType newSelection = _StartOrContinueExtendedSelection(flags);
    
    // Move Cursor
    _currentCursor.SetStaffIndex(staffIndex);
    _currentCursor.SetBarIndex(barIndex);
    _currentCursor.SetTick(tick);
    _currentCursor.SetLineIndex(lineIndex);
    _currentCursor.ForceValidPosition(true);
    _typingSecondDigit = false;
    _editingGraceNote = false;
    _graceNoteIndex = 0;
    
    // New selection kind (will refresh the score)
    SetPreferredSelectionKind(newSelection);
    
    // Center on cursor if needed
    if(_delegate && _inferredSelection == REScoreController::CursorSelection) {
        _delegate->OnShouldCenterOnCursor(this);
    }
}

void REScoreController::MoveCursorToBar(int barIndex)
{
    // Move Cursor
    _currentCursor.SetBarIndex(barIndex);
    _currentCursor.SetTick(0);
    _currentCursor.ForceValidPosition(false);
    _typingSecondDigit = false;
    _editingGraceNote = false;
    _graceNoteIndex = 0;
    
    // New selection kind (will refresh the score)
    SetPreferredSelectionKind(REScoreController::CursorSelection);
    
    // Center on cursor if needed
    if(_delegate && _inferredSelection == REScoreController::CursorSelection) {
        _delegate->OnShouldCenterOnCursor(this);
    }
}

void REScoreController::SelectBarsInRange(const RERange& range)
{
    _typingSecondDigit = false;
    _editingGraceNote = false;
    
    _originCursor = _currentCursor;
    _originCursor.SetBarIndex(range.FirstIndex());
    _originCursor.SetTick(0);
    _currentCursor.SetBarIndex(range.LastIndex());
    _currentCursor.SetTick(0);

    _currentCursor.ForceValidPosition(false);
    _originCursor.ForceValidPosition(false);
    
    // New selection kind (will refresh the score)
    SetPreferredSelectionKind(REScoreController::BarRangeSelection);
    
    // Center on cursor if needed
    if(_delegate) {
        _delegate->OnShouldCenterOnCursor(this);
    }
}


RERect REScoreController::SelectionRect() const
{
    return RERect::FromPoints(_selectionStartPoint, _selectionEndPoint);
}

const REPoint& REScoreController::SelectionStartPoint() const
{
    return _selectionStartPoint;
}
const REPoint& REScoreController::SelectionEndPoint() const
{
    return _selectionEndPoint;
}
bool REScoreController::SelectionRectIsVisible() const
{
    return _selectionRectVisible;
}






#define REFLOW_SCORE_OPERATION_BEGIN(opName)      try{RELockSongControllerForTask lock_(_songController, opName, 0);
#define REFLOW_SCORE_OPERATION_END                lock_.Commit();} catch(REException& ex) {_songController->DoSomethingWithReflowError(ex);} catch(std::exception& ex) {_songController->DoSomethingWithError(ex);} catch(...) {_songController->DoSomethingWithUnhandledError();}
#define REFLOW_SCORE_OPERATION_CATCH_EXCEPTIONS   catch(REException& ex) {_songController->DoSomethingWithReflowError(ex);} catch(std::exception& ex) {_songController->DoSomethingWithError(ex);} catch(...) {_songController->DoSomethingWithUnhandledError();}

// Type Keypad Number
// --------------------------------------------------------------------------------------------------------
#pragma mark Type keypad number (TAB)
static int _AlterationFromKeypadNumber(int num)
{
    static int _AlterationArray[] = {0, -1, -2, 0, 1, 2};
    if(num >= 0 && num <= 5) {
        return _AlterationArray[num];
    }
    return 0;
}


void REScoreController::PerformTaskOnSong(RESongOperation op)
{
    REFLOW_SCORE_OPERATION_BEGIN("Unknown Operation")
    {
        op(lock_.LockSong());
    }
    REFLOW_SCORE_OPERATION_END;
}

void REScoreController::PerformTasksOnSong(const RESongOperationVector& ops)
{
    REFLOW_SCORE_OPERATION_BEGIN("Unknown Operation")
    {
        RESong* song = lock_.LockSong();
        for(auto op : ops) {
            op(song);
        }
    }
    REFLOW_SCORE_OPERATION_END;
}

void REScoreController::TypeOnVisualFretboard(int string, int fret)
{
    // Only for Cursor selection
    if(_inferredSelection != REScoreController::CursorSelection) {
        return;
    }
    
    const REStaff* staff = _currentCursor.Staff();
    if(!staff) return;
    
    const RETrack* track = staff->Track();
    if(track == NULL || !track->IsTablature()) {
        return;
    }
    
    int capo = track->Capo();
    if(capo) {
        fret -= capo;
        if(fret < 0) fret = 0;
    }
    
    REFLOW_SCORE_OPERATION_BEGIN("Enter Note")
    {
        const REPhrase* phrase = FirstSelectedPhrase();
        const REChord* chord = FirstSelectedChord();
        const RENote* note = (chord ? chord->NoteOnString(string) : NULL);
        
        if(note)
        {
            if(note->Fret() == fret) 
            {
                REChord* lockedChord = lock_.LockChord(chord);
                REPhrase* lockedPhrase = lock_.LockPhrase(phrase);
                
                lockedChord->RemoveNote(note->Index());
                lockedPhrase->Refresh();
            }
            else 
            {
                RENote* lockedNote = lock_.LockNote(note);
                REPhrase* lockedPhrase = lock_.LockPhrase(phrase);
                
                lockedNote->SetFret(fret);
                lockedPhrase->Refresh();
            }
        }
        else 
        {
            REPhrase* lockedPhrase = lock_.LockPhrase(phrase);
            
            if(chord == NULL) 
            {
                REChord* newChord = new REChord;
                chord = newChord;
                lockedPhrase->AddChord(newChord);
            }
            
            // Insert Note
            {
                REChord* lockedChord = lock_.LockChord(chord);                
                
                RENote* note = new RENote;
                note->SetFret(fret);
                note->SetString(string);
                
                lockedChord->InsertNote(note, lockedChord->NoteCount());
            }
            lockedPhrase->Refresh();
        }
    }
    REFLOW_SCORE_OPERATION_END;
    
}

void REScoreController::TypeDeadNote()
{
    if(_inferredSelection == REScoreController::CursorSelection && _currentCursor.Note() == NULL)
    {
        TypeKeypadNumber(0, false, true);
    }
    else
    {
        RENotePredicate f = std::bind(&RENote::HasFlag, std::placeholders::_1, RENote::DeadNote);
        if(!_editingGraceNote && AtLeastOneSelectedNoteVerifies(f)) {
            UnsetNoteFlagOnSelectedNotes(RENote::DeadNote);
        }
        else SetNoteFlagOnSelectedNotes(RENote::DeadNote);
    }
}

void REScoreController::TypeGraceNote()
{
    if(_inferredSelection == REScoreController::CursorSelection)
    {
        if(_editingGraceNote)
        {
            _editingGraceNote = false;
        }
        else
        {
            const RENote* note = _currentCursor.Note();
            if(note)
            {
                if(note->GraceNoteCount() == 0)
                {
                    REFLOW_SCORE_OPERATION_BEGIN("Enter Grace Note")
                    {
                        REGraceNote* graceNote = new REGraceNote;
                        graceNote->SetPitch(note->Pitch());
                        graceNote->SetString(note->String());
                        graceNote->SetFret(note->Fret());

                        lock_.LockNote(note)->AddGraceNote(graceNote);
                    }
                    REFLOW_SCORE_OPERATION_END;
                }
                
                _editingGraceNote = true;
                _graceNoteIndex = 0;
            }
        }
        
        
        // New selection kind (will refresh the score)
        SetPreferredSelectionKind(REScoreController::CursorSelection);
        
        // Center on cursor if needed
        if(_delegate && _inferredSelection == REScoreController::CursorSelection) {
            _delegate->OnShouldCenterOnCursor(this);
        }
    }
}

void REScoreController::TypeKeypadNumber(int num, bool alt, bool dead)
{
    int lineIndex = _currentCursor.LineIndex();
    const REStaff* staff = _currentCursor.Staff();
    if(!staff) return;
    
    const RETrack* track = staff->Track();
    
    if(staff->Type() == Reflow::StandardStaff && track->Type() != Reflow::DrumsTrack)
    {
        switch(num) {
            case 6: ToggleForceStemDownOnSelection(); return;
            case 7: PitchDownOnSelection(); return;
            case 8: PitchUpOnSelection(); return;
            case 9: ToggleForceStemUpOnSelection(); return;
        }
    }
    
    if(_inferredSelection == REScoreController::CursorSelection)
    {
        const REPhrase* phrase = _currentCursor.Phrase();
        if(phrase == NULL) return;
        
        REFLOW_SCORE_OPERATION_BEGIN("Enter Note")
        {
            REPhrase* lockedPhrase = lock_.LockPhrase(phrase);
            
            // Create the Chord if needed
            const REChord* chordUnder = _currentCursor.Chord();
            REChord* chord = NULL;
            if(chordUnder == NULL) {
                chord = new REChord;
                lockedPhrase->InsertChord(chord, lockedPhrase->ChordCount());
                lockedPhrase->Refresh();
            }
            else {
                chord = lock_.LockChord(chordUnder);
            }
            
            // Note In Tablature
            if(staff->Type() == Reflow::TablatureStaff)
            {
                const RENote* noteUnder = _currentCursor.Note();
                if(noteUnder)
                {
                    RENote* note = lock_.LockNote(noteUnder);
                    if(_editingGraceNote) {
                        REGraceNote* gn = note->GraceNote(_graceNoteIndex);
                        if(gn) note = gn;
                    }
                    
                    if(_typingSecondDigit) {
                        note->SetFret(noteUnder->Fret() * 10 + num);
                        _typingSecondDigit = false;
                    }
                    else {
                        note->SetFret(num);
                        if(num <= 3) _typingSecondDigit = true;
                    }
                }
                else {
                    RENote* note = new RENote;
                    note->SetString(_currentCursor.LineIndex());
                    if(!dead) {
                        note->SetFret(num);
                    }
                    else {
                        note->SetFret(0);
                        note->SetFlag(RENote::DeadNote);
                    }
                    chord->InsertNote(note, chord->NoteCount());
                    
                    if(num <= 3) _typingSecondDigit = true;
                }
            }
            
            // Note in Standard Staff
            else if(staff->Type() == Reflow::StandardStaff)
            {
                // Drums
                if(track->Type() == Reflow::DrumsTrack)
                {
                    int midi = 0;
                    switch(num)
                    {
                        case 0: midi = 36;  break;  // Bass Drum
                        case 1: midi = 42;  break;  // HH close
                        case 2: midi = 38;  break;  // Snare
                        case 3: midi = 49;  break;  // Crash
                        case 4: midi = (alt ? 44 : 46);  break;  // Hi hat pedal / HH open
                        case 5: midi = 37;  break;  // side stick (rim shot)
                        case 6: midi = (alt ? 53 : 51);  break;  // Ride bell / Ride
                        case 7: midi = (alt ? 43 : 41);  break;  // low tom 1 - low tom 2
                        case 8: midi = (alt ? 47 : 45);  break;  // mid tom 1 - mid tom 2
                        case 9: midi = (alt ? 50 : 48);  break;  // high tom 1 - high tom 2
                    }
                    
                    if(_editingGraceNote)
                    {
                        const RENote* noteUnder = _currentCursor.Note();
                        if(noteUnder)
                        {
                            RENote* note = lock_.LockNote(noteUnder);
                            REGraceNote* gn = note->GraceNote(_graceNoteIndex);
                            
                            if(gn)
                            {
                                if(gn->Pitch().midi == midi) {
                                    note->RemoveGraceNote(_graceNoteIndex);
                                }
                                else {
                                    gn->SetPitchFromMIDI(midi);
                                }
                            }
                        }
                    }
                    else
                    {
                    
                        const RENote* noteUnder = chord->NoteWithMidi(midi);
                        if(noteUnder)
                        {
                            // Remove Note
                            chord->RemoveNote(noteUnder->Index());
                        }
                        else if(midi != 0)
                        {
                            RENote* note = new RENote;
                            note->SetPitchFromMIDI(midi);
                            chord->InsertNote(note, chord->NoteCount());
                            
                            const REDrumMapping& drumMapping = Reflow::StandardDrumMapping(midi);
                            _currentCursor.SetLineIndex(drumMapping.line);
                            
                            // Remove mutual exclusive notes
                            {
                                int notesToRemove[3] = {0,0,0};
                                int nbNotesToRemove = 0;
                                if(midi == 42) {
                                    notesToRemove[nbNotesToRemove++] = 44;
                                    notesToRemove[nbNotesToRemove++] = 46;
                                }
                                if(midi == 44) {
                                    notesToRemove[nbNotesToRemove++] = 42;
                                    notesToRemove[nbNotesToRemove++] = 46;
                                }
                                if(midi == 46) {
                                    notesToRemove[nbNotesToRemove++] = 42;
                                    notesToRemove[nbNotesToRemove++] = 44;
                                }
                                if(midi == 51) {notesToRemove[nbNotesToRemove++] = 53;}
                                if(midi == 53) {notesToRemove[nbNotesToRemove++] = 51;}
                                
                                for(int i=0; i<nbNotesToRemove; ++i) {
                                    int midiToRemove = notesToRemove[i];
                                    const RENote* noteToRemove = chord->NoteWithMidi(midiToRemove);
                                    if(noteToRemove) {
                                        chord->RemoveNote(noteToRemove->Index());
                                    }
                                }
                            }
                        }
                    } // _editingGraceNote
                }
                
                // Standard
                else 
                {
                    RENote* noteUnder = chord->NoteOnStaffLine(lineIndex, Score()->IsTransposing());
                    if(noteUnder)
                    {
                        RENotePitch pitch = noteUnder->Pitch();
                        pitch.alter = _AlterationFromKeypadNumber(num);
                        pitch.midi = Reflow::MidiFromPitch(pitch.step, pitch.octave, pitch.alter);
                        
                        lock_.LockNote(noteUnder)->SetPitch(pitch);
                    }
                    else {
                        RENotePitch pitch = phrase->PitchFromStd(chord->Index(), lineIndex, Score()->IsTransposing());
                        pitch.alter = _AlterationFromKeypadNumber(num);
                        pitch.midi = Reflow::MidiFromPitch(pitch.step, pitch.octave, pitch.alter);
                        
                        if(track->Type() == Reflow::StandardTrack)
                        {
                            RENote* note = new RENote;
                            note->SetPitch(pitch);
                            note->SetString(0);
                            note->SetFret(0);
                            chord->InsertNote(note, chord->NoteCount());
                        }
                        else if(track->Type() == Reflow::TablatureTrack)
                        {
                            int fret;
                            int string = chord->FindUnusedStringForMidi(pitch.midi, &fret);
                            if(string != -1)
                            {
                                RENote* note = new RENote;
                                note->SetString(string);
                                if(!dead) {
                                    note->SetFret(fret);
                                }
                                else {
                                    note->SetFret(0);
                                    note->SetFlag(RENote::DeadNote);
                                }
                                chord->InsertNote(note, chord->NoteCount());
                            }
                        }
                    }
                }
            }
            
            lockedPhrase->Refresh();
        }
        REFLOW_SCORE_OPERATION_END;
    }
}

void REScoreController::InsertNotesWithPitches(const REIntVector& pitches)
{
    if(_inferredSelection == REScoreController::BarRangeSelection ||
       _inferredSelection == REScoreController::SingleTrackBarRangeSelection) {
        return;
    }
    
    const REChord* cchord = FirstSelectedChord();
    const REPhrase* cphrase = FirstSelectedPhrase();
    const RETrack* ctrack = (cphrase ? cphrase->Track() : NULL);

    if(ctrack == NULL) {
        return;
    }
    
    REFLOW_SCORE_OPERATION_BEGIN("Insert Midi Notes")
    {
        REChord* chord = (cchord ? lock_.LockChord(cchord) : NULL);
        REPhrase* phrase = (cphrase ? lock_.LockPhrase(cphrase) : NULL);
        
        for(int i=0; i<pitches.size(); ++i)
        {
            int pitch = pitches[i];
            
            // Modify in standard notation
            RENote* note = (chord ? chord->NoteWithMidi(pitch) : NULL);
            if(note)
            {
                chord->RemoveNote(note->Index());
                phrase->Refresh();
                
            }
            else 
            {
                if(ctrack->IsTablature())
                {
                    int fret = 0;
                    int string = -1;
                    
                    // Find string/fret to insert note
                    if(chord == NULL)
                    {
                        REStringFretPairVector stringFrets = Reflow::FindStringFretPairsForMidi(pitch, ctrack->TuningArray(), ctrack->StringCount(), ctrack->MaxFret());
                        if(!stringFrets.empty()) {
                            string = stringFrets[0].string;
                            fret = stringFrets[0].fret;
                        }
                    }
                    else {
                        string = chord->FindUnusedStringForMidi(pitch, &fret);
                    }
                    
                    // Create Note (and chord if needed)
                    if(string != -1)
                    {
                        if(chord == NULL) {
                            chord = new REChord;
                            phrase->AddChord(chord);
                        }
                        RENote* note = new RENote;
                        note->SetFret(fret);
                        note->SetString(string);
                        chord->InsertNote(note, chord->NoteCount());
                        phrase->Refresh();
                    }
                }
                else
                {
                    if(chord == NULL) {
                        chord = new REChord;
                        phrase->AddChord(chord);
                    }
                    RENote* note = new RENote;
                    note->SetPitchFromMIDI(pitch);
                    chord->InsertNote(note, chord->NoteCount());
                    phrase->Refresh();
                }
            }
        }
    }
    REFLOW_SCORE_OPERATION_END;
}

void REScoreController::PerformOperationsOnSelectedBars(const REBarOperationVector& operationsOnFirst, const REBarOperationVector& operationsOnLast, const std::string& batchName)
{
    const REBar* firstBar = FirstSelectedBar();
    const REBar* lastBar = LastSelectedBar();
    if(firstBar == NULL || lastBar == NULL) return;
    
    REFLOW_SCORE_OPERATION_BEGIN(batchName)
    {
        REBar* firstBarLocked = lock_.LockBar(firstBar);
        for(REBarOperation op : operationsOnFirst) {
            op(firstBarLocked);
        }
        
        REBar* lastBarLocked = lock_.LockBar(lastBar);
        for(REBarOperation op : operationsOnLast) {
            op(lastBarLocked);
        }
    }
    REFLOW_SCORE_OPERATION_END;
}

void REScoreController::CreateTrack(const RECreateTrackOptions &opts)
{
    _songController->CreateTrack(opts);
}

void REScoreController::RemoveTrack(int idx)
{
    _songController->RemoveTrack(idx);
}

void REScoreController::SetNameOfTrack(int trackIndex, const std::string& name)
{
    _songController->SetNameOfTrack(trackIndex, name);
}

void REScoreController::SetShortNameOfTrack(int trackIndex, const std::string& name)
{
    _songController->SetShortNameOfTrack(trackIndex, name);
}

void REScoreController::SetTuningOfTrack(int trackIndex, const int* tuningArray, int stringCount)
{
    _songController->SetTuningOfTrack(trackIndex, tuningArray, stringCount);
}

void REScoreController::SetTrackPresentInCurrentScore(int trackIndex, bool set)
{
    _songController->SetTrackInScore(_scoreIndex, trackIndex, set);
}

// Delete Selection
// --------------------------------------------------------------------------------------------------------
#pragma mark Delete Selection
void REScoreController::DeleteSelection()
{
    if(_inferredSelection == REScoreController::CursorSelection && _editingGraceNote)
    {
        const RENote* note = _currentCursor.Note();
        const REGraceNote* graceNote = (note ? note->GraceNote(_graceNoteIndex) : nullptr);
        if(graceNote)
        {
            REFLOW_SCORE_OPERATION_BEGIN("Enter Grace Note")
            {
                lock_.LockNote(note)->RemoveGraceNote(_graceNoteIndex);
                _editingGraceNote = false;
            }
            REFLOW_SCORE_OPERATION_END;
        }
    }
    else
    {
        int noteCount = SelectedNoteCount();
        if(noteCount == 0) 
        {
            bool allChordsEmpty = AllTheSelectedChordsVerify(bind(&REChord::IsRest, std::placeholders::_1));
            if(allChordsEmpty) {
                DeleteSelectedChords();
            }
        }
        else 
        {
            // Delete Notes in selection
            REConstNoteVector notesToDelete;
            FindSelectedNotes(&notesToDelete);
            
            REFLOW_SCORE_OPERATION_BEGIN("Delete Selected Notes")
            {
                for(const RENote* note : notesToDelete)
                {
                    REChord* chord = lock_.LockChord(note->Chord());
                    chord->RemoveNote(note->Index());
                }
            }
            REFLOW_SCORE_OPERATION_END;
        }
    }
}

void REScoreController::DeleteSelectedChords()
{
    REConstChordVector chordsToDelete;
    FindSelectedChords(&chordsToDelete);
    
    REFLOW_SCORE_OPERATION_BEGIN("Delete Selected Chords")
    {
        for(const REChord* chord : chordsToDelete)
        {
            REPhrase* phrase = lock_.LockPhrase(chord->Phrase());
            phrase->RemoveChord(chord->Index());
        }
    }
    REFLOW_SCORE_OPERATION_END;
}


// Note Value
// --------------------------------------------------------------------------------------------------------
#pragma mark Note Value
void REScoreController::SetNoteValueOnSelection(Reflow::NoteValue noteValue)
{
    try 
    {
        PerformTaskOnSelectedChords(std::bind(&REChord::SetNoteValue, std::placeholders::_1, noteValue), "Set Note Value");
    }
    REFLOW_SCORE_OPERATION_CATCH_EXCEPTIONS;
}

void REScoreController::IncreaseNoteValueOnSelection()
{
    try 
    {
        PerformTaskOnSelectedChords(std::bind(&REChord::IncreaseNoteValue, std::placeholders::_1), "Increase Note Value");
    }
    REFLOW_SCORE_OPERATION_CATCH_EXCEPTIONS;
}
void REScoreController::DecreaseNoteValueOnSelection()
{
    try 
    {
        PerformTaskOnSelectedChords(std::bind(&REChord::DecreaseNoteValue, std::placeholders::_1), "Decrease Note Value");
    }
    REFLOW_SCORE_OPERATION_CATCH_EXCEPTIONS;
}


// Legato (Hammer on / Pull off)
// --------------------------------------------------------------------------------------------------------
#pragma mark Legato
void REScoreController::SetLegatoOnSelection(bool legato)
{
    try {
        PerformTaskOnSelectedNotes([=](RENote* note) {note->SetFlag(RENote::Legato, legato);}, "Set Legato");
    }
    REFLOW_SCORE_OPERATION_CATCH_EXCEPTIONS;
}


void REScoreController::ToggleLegatoOnSelection()
{
    bool flagIsSet = AtLeastOneSelectedNoteVerifies(std::bind(&RENote::HasFlag, std::placeholders::_1, RENote::Legato));
    SetLegatoOnSelection(!flagIsSet);
    
    ToggleSlurOnSelection();
}



// Arpeggio / Brush / Pickstroke
// --------------------------------------------------------------------------------------------------------
#pragma mark Arpeggio / Brush / Pickstroke
void REScoreController::SetArpeggioUpOnSelection(bool set)
{
    if(set) 
    {
        REFLOW_SCORE_OPERATION_BEGIN("Set Arpeggio Upwards")
        {
            PerformTaskOnSelectedChords(std::bind(&REChord::SetFlag, std::placeholders::_1, REChord::Arpeggio), "Set Arpeggio");
            PerformTaskOnSelectedChords(std::bind(&REChord::SetFlag, std::placeholders::_1, REChord::StrumUpwards), "Strum Upwards");
        }
        REFLOW_SCORE_OPERATION_END;
    }
    else {
        try {
            PerformTaskOnSelectedChords(std::bind(&REChord::UnsetFlag, std::placeholders::_1, REChord::Arpeggio), "Unset Arpeggio");
        }
        REFLOW_SCORE_OPERATION_CATCH_EXCEPTIONS;
    }
}

void REScoreController::SetArpeggioDownOnSelection(bool set)
{
    if(set) 
    {
        REFLOW_SCORE_OPERATION_BEGIN("Set Arpeggio Downwards")
        {
            PerformTaskOnSelectedChords(std::bind(&REChord::SetFlag, std::placeholders::_1, REChord::Arpeggio), "Set Arpeggio");
            PerformTaskOnSelectedChords(std::bind(&REChord::UnsetFlag, std::placeholders::_1, REChord::StrumUpwards), "Strum Downwards");
        }
        REFLOW_SCORE_OPERATION_END;
    }
    else {
        try {
            PerformTaskOnSelectedChords(std::bind(&REChord::UnsetFlag, std::placeholders::_1, REChord::Arpeggio), "Unset Arpeggio");
        }
        REFLOW_SCORE_OPERATION_CATCH_EXCEPTIONS;
    }
}

void REScoreController::SetPickstrokeUpOnSelection(bool set)
{
    if(set) 
    {
        REFLOW_SCORE_OPERATION_BEGIN("Set Pickstroke Upwards")
        {
            PerformTaskOnSelectedChords(std::bind(&REChord::SetFlag, std::placeholders::_1, REChord::PickStroke), "Set Pickstroke");
            PerformTaskOnSelectedChords(std::bind(&REChord::SetFlag, std::placeholders::_1, REChord::StrumUpwards), "Strum Upwards");
        }
        REFLOW_SCORE_OPERATION_END;
    }
    else {
        try {
            PerformTaskOnSelectedChords(std::bind(&REChord::UnsetFlag, std::placeholders::_1, REChord::PickStroke), "Unset Pickstroke");
        }
        REFLOW_SCORE_OPERATION_CATCH_EXCEPTIONS;
    }
}

void REScoreController::SetPickstrokeDownOnSelection(bool set)
{
    if(set) 
    {
        REFLOW_SCORE_OPERATION_BEGIN("Set Pickstroke Downwards")
        {
            PerformTaskOnSelectedChords(std::bind(&REChord::SetFlag, std::placeholders::_1, REChord::PickStroke), "Set Pickstroke");
            PerformTaskOnSelectedChords(std::bind(&REChord::UnsetFlag, std::placeholders::_1, REChord::StrumUpwards), "Strum Downwards");
        }
        REFLOW_SCORE_OPERATION_END;
    }
    else {
        try {
            PerformTaskOnSelectedChords(std::bind(&REChord::UnsetFlag, std::placeholders::_1, REChord::PickStroke), "Unset Pickstroke");
        }
        REFLOW_SCORE_OPERATION_CATCH_EXCEPTIONS;
    }
}


void REScoreController::SetBrushUpOnSelection(bool set)
{
    if(set) 
    {
        REFLOW_SCORE_OPERATION_BEGIN("Set Brush Upwards")
        {
            PerformTaskOnSelectedChords(std::bind(&REChord::SetFlag, std::placeholders::_1, REChord::Brush), "Set Brush");
            PerformTaskOnSelectedChords(std::bind(&REChord::SetFlag, std::placeholders::_1, REChord::StrumUpwards), "Strum Upwards");
        }
        REFLOW_SCORE_OPERATION_END;
    }
    else {
        try {
            PerformTaskOnSelectedChords(std::bind(&REChord::UnsetFlag, std::placeholders::_1, REChord::Brush), "Unset Brush");
        }
        REFLOW_SCORE_OPERATION_CATCH_EXCEPTIONS;
    }
}

void REScoreController::SetBrushDownOnSelection(bool set)
{
    if(set) 
    {
        REFLOW_SCORE_OPERATION_BEGIN("Set Brush Downwards")
        {
            PerformTaskOnSelectedChords(std::bind(&REChord::SetFlag, std::placeholders::_1, REChord::Brush), "Set Brush");
            PerformTaskOnSelectedChords(std::bind(&REChord::UnsetFlag, std::placeholders::_1, REChord::StrumUpwards), "Strum Downwards");
        }
        REFLOW_SCORE_OPERATION_END;
    }
    else {
        try {
            PerformTaskOnSelectedChords(std::bind(&REChord::UnsetFlag, std::placeholders::_1, REChord::Brush), "Unset Brush");
        }
        REFLOW_SCORE_OPERATION_CATCH_EXCEPTIONS;
    }
}

void REScoreController::ToggleArpeggioUpOnSelection()
{
    bool isSet = AtLeastOneSelectedChordVerifies(std::bind(&REChord::HasFlag, std::placeholders::_1, REChord::Arpeggio)) &&
                 AtLeastOneSelectedChordVerifies(std::bind(&REChord::HasFlag, std::placeholders::_1, REChord::StrumUpwards));
    SetArpeggioUpOnSelection(!isSet);
}
void REScoreController::ToggleArpeggioDownOnSelection()
{
    bool isSet = AtLeastOneSelectedChordVerifies(std::bind(&REChord::HasFlag, std::placeholders::_1, REChord::Arpeggio));
    SetArpeggioDownOnSelection(!isSet);    
}
void REScoreController::ToggleBrushUpOnSelection()
{
    bool isSet = AtLeastOneSelectedChordVerifies(std::bind(&REChord::HasFlag, std::placeholders::_1, REChord::Brush)) &&
                 AtLeastOneSelectedChordVerifies(std::bind(&REChord::HasFlag, std::placeholders::_1, REChord::StrumUpwards));
    SetBrushUpOnSelection(!isSet);
}
void REScoreController::ToggleBrushDownOnSelection()
{
    bool isSet = AtLeastOneSelectedChordVerifies(std::bind(&REChord::HasFlag, std::placeholders::_1, REChord::Brush));
    SetBrushDownOnSelection(!isSet);
}
void REScoreController::TogglePickstrokeUpOnSelection()
{
    bool isSet = AtLeastOneSelectedChordVerifies(std::bind(&REChord::HasFlag, std::placeholders::_1, REChord::PickStroke)) &&
    AtLeastOneSelectedChordVerifies(std::bind(&REChord::HasFlag, std::placeholders::_1, REChord::StrumUpwards));
    SetPickstrokeUpOnSelection(!isSet);
}
void REScoreController::TogglePickstrokeDownOnSelection()
{
    bool isSet = AtLeastOneSelectedChordVerifies(std::bind(&REChord::HasFlag, std::placeholders::_1, REChord::PickStroke));
    SetPickstrokeDownOnSelection(!isSet);
}


// Force Stem Direction
// --------------------------------------------------------------------------------------------------------
#pragma mark Force Stem Direction

void REScoreController::SetForceStemUpOnSelection(bool set)
{
    if(set)
    {
        REFLOW_SCORE_OPERATION_BEGIN("Force Stem Up")
        {
            PerformTaskOnSelectedChords(std::bind(&REChord::UnsetFlag, std::placeholders::_1, REChord::ForceStemDown), "Stop Force Stem Down");
            PerformTaskOnSelectedChords(std::bind(&REChord::SetFlag, std::placeholders::_1, REChord::ForceStemUp), "Force Stem Up");
        }
        REFLOW_SCORE_OPERATION_END;
    }
    else {
        try {
            PerformTaskOnSelectedChords(std::bind(&REChord::UnsetFlag, std::placeholders::_1, REChord::ForceStemUp), "Stop Force Stem Up");
        }
        REFLOW_SCORE_OPERATION_CATCH_EXCEPTIONS;
    }
}


void REScoreController::ToggleForceStemUpOnSelection()
{
    bool isSet = AtLeastOneSelectedChordVerifies(std::bind(&REChord::HasFlag, std::placeholders::_1, REChord::ForceStemUp));
    SetForceStemUpOnSelection(!isSet);
}


void REScoreController::SetForceStemDownOnSelection(bool set)
{
    if(set)
    {
        REFLOW_SCORE_OPERATION_BEGIN("Force Stem Down")
        {
            PerformTaskOnSelectedChords(std::bind(&REChord::UnsetFlag, std::placeholders::_1, REChord::ForceStemUp), "Stop Force Stem Up");
            PerformTaskOnSelectedChords(std::bind(&REChord::SetFlag, std::placeholders::_1, REChord::ForceStemDown), "Force Stem Down");
        }
        REFLOW_SCORE_OPERATION_END;
    }
    else {
        try {
            PerformTaskOnSelectedChords(std::bind(&REChord::UnsetFlag, std::placeholders::_1, REChord::ForceStemDown), "Stop Force Stem Down");
        }
        REFLOW_SCORE_OPERATION_CATCH_EXCEPTIONS;
    }
}


void REScoreController::ToggleForceStemDownOnSelection()
{
    bool isSet = AtLeastOneSelectedChordVerifies(std::bind(&REChord::HasFlag, std::placeholders::_1, REChord::ForceStemDown));
    SetForceStemDownOnSelection(!isSet);
}


// Palm Mute / Let Ring
// --------------------------------------------------------------------------------------------------------
#pragma mark Palm Mute / Let Ring
void REScoreController::SetPalmMuteOnSelection(bool pm)
{
    if(pm) {
        REFLOW_SCORE_OPERATION_BEGIN("Set Palm Mute")
        {
            PerformTaskOnSelectedChords(std::bind(&REChord::SetFlag, std::placeholders::_1, REChord::PalmMute), "Set Palm Mute");
            PerformTaskOnSelectedChords(std::bind(&REChord::UnsetFlag, std::placeholders::_1, REChord::LetRing), "Unset Let Ring");
        }
        REFLOW_SCORE_OPERATION_END;
    }
    else {
        try {
            PerformTaskOnSelectedChords(std::bind(&REChord::UnsetFlag, std::placeholders::_1, REChord::PalmMute), "Unset Palm Mute");
        }
        REFLOW_SCORE_OPERATION_CATCH_EXCEPTIONS;
    }
}

void REScoreController::TogglePalmMuteOnSelection()
{
    bool isSet = AtLeastOneSelectedChordVerifies(std::bind(&REChord::HasFlag, std::placeholders::_1, REChord::PalmMute));
    SetPalmMuteOnSelection(!isSet);
}

void REScoreController::SetLetRingOnSelection(bool pm)
{
    if(pm) {
        REFLOW_SCORE_OPERATION_BEGIN("Set Let Ring")
        {
            PerformTaskOnSelectedChords(std::bind(&REChord::SetFlag, std::placeholders::_1, REChord::LetRing), "Set Let Ring");
            PerformTaskOnSelectedChords(std::bind(&REChord::UnsetFlag, std::placeholders::_1, REChord::PalmMute), "Unset Palm Mute");
        }
        REFLOW_SCORE_OPERATION_END;
    }
    else {
        try {
            PerformTaskOnSelectedChords(std::bind(&REChord::UnsetFlag, std::placeholders::_1, REChord::LetRing), "Unset Let Ring");
        }
        REFLOW_SCORE_OPERATION_CATCH_EXCEPTIONS;
    }
}

void REScoreController::ToggleLetRingOnSelection()
{
    bool isSet = AtLeastOneSelectedChordVerifies(std::bind(&REChord::HasFlag, std::placeholders::_1, REChord::LetRing));
    SetLetRingOnSelection(!isSet);
}


// Tap / Slap / Pop
// --------------------------------------------------------------------------------------------------------
#pragma mark Tap / Slap / Pop

void REScoreController::SetTapOnSelection(bool set)
{
    if(set) {
        REFLOW_SCORE_OPERATION_BEGIN("Set Tapping")
        {
            PerformTaskOnSelectedChords(std::bind(&REChord::SetFlag, std::placeholders::_1, REChord::Tap), "Set Tapping");
            PerformTaskOnSelectedChords(std::bind(&REChord::UnsetFlag, std::placeholders::_1, REChord::Slap), "Unset Slapping");
            PerformTaskOnSelectedChords(std::bind(&REChord::UnsetFlag, std::placeholders::_1, REChord::Pop), "Unset Popping");
        }
        REFLOW_SCORE_OPERATION_END;
    }
    else {
        try {
            PerformTaskOnSelectedChords(std::bind(&REChord::UnsetFlag, std::placeholders::_1, REChord::Tap), "Unset Tapping");
        }
        REFLOW_SCORE_OPERATION_CATCH_EXCEPTIONS;
    }
}

void REScoreController::SetSlapOnSelection(bool set)
{
    if(set) {
        REFLOW_SCORE_OPERATION_BEGIN("Set Slapping")
        {
            PerformTaskOnSelectedChords(std::bind(&REChord::UnsetFlag, std::placeholders::_1, REChord::Tap), "Unset Tapping");
            PerformTaskOnSelectedChords(bind(&REChord::SetFlag, std::placeholders::_1, REChord::Slap), "Set Slapping");
            PerformTaskOnSelectedChords(bind(&REChord::UnsetFlag, std::placeholders::_1, REChord::Pop), "Unset Popping");
        }
        REFLOW_SCORE_OPERATION_END;
    }
    else {
        try {
            PerformTaskOnSelectedChords(bind(&REChord::UnsetFlag, std::placeholders::_1, REChord::Slap), "Unset Slapping");
        }
        REFLOW_SCORE_OPERATION_CATCH_EXCEPTIONS;
    }
}

void REScoreController::SetPopOnSelection(bool set)
{
    if(set) {
        REFLOW_SCORE_OPERATION_BEGIN("Set Popping")
        {
            PerformTaskOnSelectedChords(bind(&REChord::UnsetFlag, std::placeholders::_1, REChord::Tap), "Unset Tapping");
            PerformTaskOnSelectedChords(bind(&REChord::UnsetFlag, std::placeholders::_1, REChord::Slap), "Unset Slapping");
            PerformTaskOnSelectedChords(bind(&REChord::SetFlag, std::placeholders::_1, REChord::Pop), "Set Popping");
        }
        REFLOW_SCORE_OPERATION_END;
    }
    else {
        try {
            PerformTaskOnSelectedChords(bind(&REChord::UnsetFlag, std::placeholders::_1, REChord::Pop), "Unset Popping");
        }
        REFLOW_SCORE_OPERATION_CATCH_EXCEPTIONS;
    }
}

void REScoreController::ToggleTapOnSelection()
{
    bool isSet = AtLeastOneSelectedChordVerifies(bind(&REChord::HasFlag, std::placeholders::_1, REChord::Tap));
    SetTapOnSelection(!isSet);
}
void REScoreController::ToggleSlapOnSelection()
{
    bool isSet = AtLeastOneSelectedChordVerifies(bind(&REChord::HasFlag, std::placeholders::_1, REChord::Slap));
    SetSlapOnSelection(!isSet);
}
void REScoreController::TogglePopOnSelection()
{
    bool isSet = AtLeastOneSelectedChordVerifies(bind(&REChord::HasFlag, std::placeholders::_1, REChord::Pop));
    SetPopOnSelection(!isSet);
}


// Vibrato 
// --------------------------------------------------------------------------------------------------------
#pragma mark Vibrato
void REScoreController::SetVibratoOnSelection(bool set) 
{
    try {
        PerformTaskOnSelectedChords(bind(&REChord::SetFlag, std::placeholders::_1, REChord::Vibrato), set ? "Set Vibrato" : "Unset Vibrato");
    }
    REFLOW_SCORE_OPERATION_CATCH_EXCEPTIONS;
}

void REScoreController::ToggleVibratoOnSelection() 
{
    bool isSet = AtLeastOneSelectedChordVerifies(bind(&REChord::HasFlag, std::placeholders::_1, REChord::Vibrato));
    SetVibratoOnSelection(!isSet);
}



// Accent / Strong Accent 
// --------------------------------------------------------------------------------------------------------
#pragma mark Accent / Strong Accent
void REScoreController::SetAccentOnSelection(bool set)
{
    if(set) {
        REFLOW_SCORE_OPERATION_BEGIN("Set Accent")
        {
            PerformTaskOnSelectedChords(bind(&REChord::SetFlag, std::placeholders::_1, REChord::Accent), "Set Accent");
            PerformTaskOnSelectedChords(bind(&REChord::UnsetFlag, std::placeholders::_1, REChord::StrongAccent), "Unset Strong Accent");
        }
        REFLOW_SCORE_OPERATION_END;
    }
    else {
        PerformTaskOnSelectedChords(bind(&REChord::UnsetFlag, std::placeholders::_1, REChord::Accent), "Unset Accent");
    }
}
void REScoreController::SetStrongAccentOnSelection(bool set)
{
    if(set) {
        REFLOW_SCORE_OPERATION_BEGIN("Set Strong Accent")
        {
            PerformTaskOnSelectedChords(bind(&REChord::SetFlag, std::placeholders::_1, REChord::StrongAccent), "Set Strong Accent");
            PerformTaskOnSelectedChords(bind(&REChord::UnsetFlag, std::placeholders::_1, REChord::Accent), "Unset Accent");
        }
        REFLOW_SCORE_OPERATION_END;
    }
    else {
        PerformTaskOnSelectedChords(bind(&REChord::UnsetFlag, std::placeholders::_1, REChord::StrongAccent), "Unset Strong Accent");
    }
}
void REScoreController::ToggleAccentOnSelection()
{
    bool isSet = AtLeastOneSelectedChordVerifies(bind(&REChord::HasFlag, std::placeholders::_1, REChord::Accent));
    SetAccentOnSelection(!isSet);
}
void REScoreController::ToggleStrongAccentOnSelection()
{
    bool isSet = AtLeastOneSelectedChordVerifies(bind(&REChord::HasFlag, std::placeholders::_1, REChord::StrongAccent));
    SetStrongAccentOnSelection(!isSet);
}


// Staccato 
// --------------------------------------------------------------------------------------------------------
#pragma mark Staccato
void REScoreController::SetStaccatoOnSelection(bool set) 
{
    try {
        PerformTaskOnSelectedChords(bind(&REChord::SetFlag, std::placeholders::_1, REChord::Staccato), set ? "Set Staccato" : "Unset Staccato");
    }
    REFLOW_SCORE_OPERATION_CATCH_EXCEPTIONS;
}

void REScoreController::ToggleStaccatoOnSelection() 
{
    bool isSet = AtLeastOneSelectedChordVerifies(bind(&REChord::HasFlag, std::placeholders::_1, REChord::Staccato));
    SetStaccatoOnSelection(!isSet);
}


// Slide In / Slide Out
// --------------------------------------------------------------------------------------------------------
#pragma mark Slide In / Slide Out
void REScoreController::SetSlideInOnSelection(Reflow::SlideInType slideIn)
{
    try {
        PerformTaskOnSelectedNotes(bind(&RENote::SetSlideIn, std::placeholders::_1, slideIn), (slideIn != Reflow::NoSlideIn ? "Set Slide In" : "Unset Slide In"));
    }
    REFLOW_SCORE_OPERATION_CATCH_EXCEPTIONS;
}

void REScoreController::SetSlideOutOnSelection(Reflow::SlideOutType slideOut)
{
    try {
        PerformTaskOnSelectedNotes(bind(&RENote::SetSlideOut, std::placeholders::_1, slideOut), (slideOut != Reflow::NoSlideOut ? "Set Slide Out" : "Unset Slide Out"));
    }
    REFLOW_SCORE_OPERATION_CATCH_EXCEPTIONS;
}

void REScoreController::ToggleSlideInOnSelection(Reflow::SlideInType slideIn)
{
    bool isSet = AtLeastOneSelectedNoteVerifies([=](const RENote* n){return n->SlideIn() == slideIn;});
    SetSlideInOnSelection(isSet ? Reflow::NoSlideIn : slideIn);
}
void REScoreController::ToggleSlideOutOnSelection(Reflow::SlideOutType slideOut)
{
    bool isSet = AtLeastOneSelectedNoteVerifies([=](const RENote* n){return n->SlideOut() == slideOut;});
    SetSlideOutOnSelection(isSet ? Reflow::NoSlideOut : slideOut);
}




// Bend
// --------------------------------------------------------------------------------------------------------
#pragma mark Bend
void REScoreController::SetBendOnSelection(REBend bend)
{
    try {
        PerformTaskOnSelectedNotes(bind(&RENote::SetBend, std::placeholders::_1, bend), (bend.Type() != Reflow::NoBend ? "Set Bend" : "Clear Bend"));
    }
    REFLOW_SCORE_OPERATION_CATCH_EXCEPTIONS;
}

void REScoreController::ToggleBendOnSelection(REBend bend)
{
    bool isSet = AtLeastOneSelectedNoteVerifies(bind(&RENote::HasBend, std::placeholders::_1));
    SetBendOnSelection(isSet ? REBend() : bend);
}


// Text attached
// --------------------------------------------------------------------------------------------------------
#pragma mark Text
void REScoreController::SetTextOnSelection(const std::string& text, Reflow::TextPositioning textPositioning)
{
    if(text.empty())
    {
        RemoveTextOnSelection();
    }
    else 
    {
        REFLOW_SCORE_OPERATION_BEGIN("Set Text")
        {
            PerformTaskOnSelectedChords(std::bind(&REChord::SetTextAttached, std::placeholders::_1, text), "Set Text");
            PerformTaskOnSelectedChords(std::bind(&REChord::SetTextPositioning, std::placeholders::_1, textPositioning), "Set Text Positioning");
        }
        REFLOW_SCORE_OPERATION_END;
    }
}

void REScoreController::RemoveTextOnSelection()
{
    try {
        PerformTaskOnSelectedChords(std::bind(&REChord::SetTextAttached, std::placeholders::_1, std::string("")), "Unset Text");
    }
    REFLOW_SCORE_OPERATION_CATCH_EXCEPTIONS;
}


// Tied Note
// --------------------------------------------------------------------------------------------------------
#pragma mark Tied Note

static void _SetTiedNote(RENote* note)
{
    RENote* tied = note->FindOriginOfTiedNote();
    if(tied) {
        note->SetFlag(RENote::TieDestination);
        note->SetFret(tied->Fret());
    }
}

void REScoreController::SetTiedNoteOnSelection(bool set)
{
    try {
        if(set) {
            PerformTaskOnSelectedNotes(std::bind(_SetTiedNote, std::placeholders::_1), "Set Tied Note");
        }
        else {
            PerformTaskOnSelectedNotes([=](RENote* note) {note->UnsetFlag(RENote::TieDestination);}, "Unset Tied Note");
        }
    }
    REFLOW_SCORE_OPERATION_CATCH_EXCEPTIONS;

}

void REScoreController::ToggleTiedNoteOnSelection()
{
    bool isSet = AtLeastOneSelectedNoteVerifies(std::bind(&RENote::HasFlag, std::placeholders::_1, RENote::TieDestination));
    SetTiedNoteOnSelection(!isSet);
}


// Tuplet
// --------------------------------------------------------------------------------------------------------
#pragma mark Tuplet
void REScoreController::SetTupletOnSelection(int tuplet)
{
    unsigned int denominator = 0;
    switch(tuplet) 
    {
        case 2: denominator = 3; break;
        case 3: denominator = 2; break;
        case 4: denominator = 6; break;
        case 5: denominator = 4; break;
        case 6: denominator = 4; break;
        case 7: denominator = 4; break;
        case 9: denominator = 8; break;
        default:denominator = 0; break;
    }
    
    try {
        RETuplet tup(tuplet, denominator);
        PerformTaskOnSelectedChords(std::bind(&REChord::SetTuplet, std::placeholders::_1, tup), (tuplet != 0 ? "Set Tuplet" : "Unset Tuplet"));
    }
    REFLOW_SCORE_OPERATION_CATCH_EXCEPTIONS;
}

void REScoreController::ToggleTupletOnSelection(int tuplet)
{
    bool isSet = AtLeastOneSelectedChordVerifies(std::bind(&RETuplet::IsValid, std::bind(&REChord::Tuplet, std::placeholders::_1)));
    SetTupletOnSelection(isSet ? 0 : tuplet);
}


// Dotted Note / Double Dotted Note
// --------------------------------------------------------------------------------------------------------
#pragma mark Dotted / Double Dotted

void REScoreController::SetDottedNoteOnSelection(bool set)
{
    try {
        if(set) {
            PerformTaskOnSelectedChords(bind(&REChord::SetDots, std::placeholders::_1, 1), "Set Dotted Note");
        }
        else {
            PerformTaskOnSelectedChords(bind(&REChord::SetDots, std::placeholders::_1, 0), "Unset Dotted Note");
        }
    }
    REFLOW_SCORE_OPERATION_CATCH_EXCEPTIONS;
}

void REScoreController::SetDoubleDottedNoteOnSelection(bool set)
{
    try {
        if(set) {
            PerformTaskOnSelectedChords(bind(&REChord::SetDots, std::placeholders::_1, 2), "Set Double Dotted Note");
        }
        else {
            PerformTaskOnSelectedChords(bind(&REChord::SetDots, std::placeholders::_1, 0), "Unset Double Dotted Note");
        }
    }
    REFLOW_SCORE_OPERATION_CATCH_EXCEPTIONS;    
}

void REScoreController::ToggleDottedNoteOnSelection()
{
    bool foundDot = AtLeastOneSelectedChordVerifies([=](const REChord* chord) {return chord->Dots() >= 1;});
    SetDottedNoteOnSelection(!foundDot);
}

void REScoreController::ToggleDoubleDottedNoteOnSelection()
{
    bool foundDoubleDot = AtLeastOneSelectedChordVerifies([=](const REChord* chord) {return chord->Dots() == 1;});
    SetDoubleDottedNoteOnSelection(!foundDoubleDot);
}


// Alteration
// --------------------------------------------------------------------------------------------------------
#pragma mark Alteration

static bool _HasAlteration(const RENote* note, int alteration) {
    return (note ? note->Pitch().alter : 0);
}

static void _SetNoteAlteration(RENote* note, int alteration) {
    RENotePitch pitch = note->Pitch();
    pitch.alter = alteration;
    pitch.midi = Reflow::MidiFromPitch(pitch.step, pitch.octave, pitch.alter);
    note->SetPitch(pitch);
}

void REScoreController::SetAlterationOnSelection(int alteration)
{
    try {
        PerformTaskOnSelectedNotes(bind(_SetNoteAlteration, std::placeholders::_1, alteration), "Set Note Alteration");
    }
    REFLOW_SCORE_OPERATION_CATCH_EXCEPTIONS;
}

void REScoreController::ToggleAlterationOnSelection(int alteration)
{
    bool isSet = AtLeastOneSelectedNoteVerifies(bind(_HasAlteration, std::placeholders::_1, alteration));
    SetAlterationOnSelection(isSet ? 0 : alteration);
}

// Note Flag
// --------------------------------------------------------------------------------------------------------
#pragma mark Note Flags
void REScoreController::SetNoteFlagOnSelectedNotes(unsigned int flag)
{
    try {
        if(!_editingGraceNote) {
            PerformTaskOnSelectedNotes([=](RENote* note){note->SetFlag((RENote::NoteFlag)flag);}, "Set Note Attributes");
        }
        else {
            PerformTaskOnSelectedGraceNote([=](RENote* note){note->SetFlag((RENote::NoteFlag)flag);}, "Set Grace Note Attribute");
        }
        
    }
    REFLOW_SCORE_OPERATION_CATCH_EXCEPTIONS;    
}
void REScoreController::UnsetNoteFlagOnSelectedNotes(unsigned int flag)
{
    try {
        if(!_editingGraceNote) {
            PerformTaskOnSelectedNotes([=](RENote* note){note->UnsetFlag((RENote::NoteFlag)flag);}, "Unset Note Attributes");
        }
        else {
            PerformTaskOnSelectedGraceNote([=](RENote* note){note->UnsetFlag((RENote::NoteFlag)flag);}, "Unset Grace Note Attributes");
        }
    }
    REFLOW_SCORE_OPERATION_CATCH_EXCEPTIONS;
}
void REScoreController::ToggleNoteFlagOnSelectedNotes(unsigned int flag)
{
    bool isSet = AtLeastOneSelectedNoteVerifies(bind(&RENote::HasFlag, std::placeholders::_1, (RENote::NoteFlag)flag));
    if(isSet) {
        UnsetNoteFlagOnSelectedNotes(flag);
    }
    else {
        SetNoteFlagOnSelectedNotes(flag);
    }
}

void REScoreController::PitchUpOnSelection()
{
    if(!_editingGraceNote)
    {
        // Should we move the cursor after having modified its pitch ?
        bool moveCursorAfterOperation = false;
        if(_inferredSelection == REScoreController::CursorSelection)
        {
            const RENote* note = _currentCursor.Note();
            if(note && _currentCursor.StandardStaff() != NULL) 
            {
                RENotePitch currentPitch = note->Pitch();
                RENotePitch newPitch = note->DetermineNewPitch(note->Pitch().midi + 1);
                moveCursorAfterOperation = (currentPitch.step != newPitch.step);
            }
        }
        
        // Perform operation
        try 
        {
            PerformTaskOnSelectedNotes(bind(&RENote::IncrementPitch, std::placeholders::_1), "Increment Pitch");
        }
        REFLOW_SCORE_OPERATION_CATCH_EXCEPTIONS;
        
        if(moveCursorAfterOperation) {
            MoveCursorUp(0);
        }
    }
    else
    {
        // Perform operation
        try
        {
            const RENote* baseNote = _currentCursor.Note();
            PerformTaskOnSelectedGraceNote(bind(&RENote::IncrementNotePitch, std::placeholders::_1, baseNote), "Increment Pitch");
        }
        REFLOW_SCORE_OPERATION_CATCH_EXCEPTIONS;
    }
}

void REScoreController::PitchDownOnSelection()
{
    if(!_editingGraceNote)
    {
        // Should we move the cursor after having modified its pitch ?
        bool moveCursorAfterOperation = false;
        if(_inferredSelection == REScoreController::CursorSelection)
        {
            const RENote* note = _currentCursor.Note();
            if(note && _currentCursor.StandardStaff() != NULL) 
            {
                RENotePitch currentPitch = note->Pitch();
                RENotePitch newPitch = note->DetermineNewPitch(note->Pitch().midi - 1);
                moveCursorAfterOperation = (currentPitch.step != newPitch.step);
            }
        }
        
        // Perform operation
        try {
            PerformTaskOnSelectedNotes(bind(&RENote::DecrementPitch, std::placeholders::_1), "Decrement Pitch");
        }
        REFLOW_SCORE_OPERATION_CATCH_EXCEPTIONS;  
        
        if(moveCursorAfterOperation) {
            MoveCursorDown(0);
        }
    }
    else
    {
        // Perform operation
        try
        {
            const RENote* baseNote = _currentCursor.Note();
            PerformTaskOnSelectedGraceNote(bind(&RENote::DecrementNotePitch, std::placeholders::_1, baseNote), "Decrement Pitch");
        }
        REFLOW_SCORE_OPERATION_CATCH_EXCEPTIONS;
    }
}

void REScoreController::ToggleEnharmonicOnSelection()
{
    if(!_editingGraceNote)
    {
        // Should we move the cursor after having modified its pitch ?
        int moveCursorAfterOperation = 0;
        if(_inferredSelection == REScoreController::CursorSelection)
        {
            const RENote* note = _currentCursor.Note();
            if(note && _currentCursor.StandardStaff() != NULL) 
            {
                RENotePitch currentPitch = note->Pitch();
                RENotePitch newPitch = note->DetermineNewPitch(note->Pitch().midi, note->NextEnharmonicHints());
                
                int deltaStep = newPitch.step - currentPitch.step;
                int deltaOctave = newPitch.octave - currentPitch.octave;
                
                moveCursorAfterOperation = (deltaStep + deltaOctave*7);
            }
        }
        
        // Perform operation
        try {
            PerformTaskOnSelectedNotes(bind(&RENote::ToggleEnharmonicHints, std::placeholders::_1), "Toggle Enharmonic Equivalent");
        }
        REFLOW_SCORE_OPERATION_CATCH_EXCEPTIONS;  
        
        if(moveCursorAfterOperation) {
            MoveCursorBy(-moveCursorAfterOperation, 0);
        }
    }
    else
    {
        try {
            PerformTaskOnSelectedGraceNote(bind(&RENote::ToggleEnharmonicHints, std::placeholders::_1), "Toggle Enharmonic Equivalent");
        }
        REFLOW_SCORE_OPERATION_CATCH_EXCEPTIONS;
    }
}



// --------------------------------------------------------------------------------------------------------
void REScoreController::InsertChordBeforeCursor()
{
    if(_inferredSelection != REScoreController::BarRangeSelection &&
       _inferredSelection != REScoreController::SingleTrackBarRangeSelection)
    {
        REFLOW_SCORE_OPERATION_BEGIN("Insert Chord")
        {
            const REChord* chord = FirstSelectedChord();
            const REPhrase* phrase = FirstSelectedPhrase();
            if(chord) 
            {
                REPhrase* lockedPhrase = lock_.LockPhrase(chord->Phrase());
                REChord* newChord = new REChord;
                newChord->CopyRhythmFrom(*chord);
                lockedPhrase->InsertChord(newChord, chord->Index());
            }
            else if(phrase)
            {
                REPhrase* lockedPhrase = lock_.LockPhrase(phrase);
                REChord* newChord = new REChord;
                lockedPhrase->InsertChord(newChord, 0);
            }
        }
        REFLOW_SCORE_OPERATION_END;
    }
}

void REScoreController::AddChordAfterCursor()
{
    if(_inferredSelection != REScoreController::BarRangeSelection &&
       _inferredSelection != REScoreController::SingleTrackBarRangeSelection)
    {
        REFLOW_SCORE_OPERATION_BEGIN("Add Chord")
        {
            const REChord* chord = LastSelectedChord();
            const REPhrase* phrase = LastSelectedPhrase();
            if(chord) 
            {
                REPhrase* lockedPhrase = lock_.LockPhrase(chord->Phrase());
                REChord* newChord = new REChord;
                newChord->CopyRhythmFrom(*chord);
                lockedPhrase->InsertChord(newChord, chord->Index()+1);
            }
            else if(phrase)
            {
                REPhrase* lockedPhrase = lock_.LockPhrase(phrase);
                REChord* newChord = new REChord;
                lockedPhrase->InsertChord(newChord, 0);
            }
        }
        REFLOW_SCORE_OPERATION_END;
    }
}

void REScoreController::DuplicateSelectedChordsAtEnd()
{
    if(_inferredSelection == REScoreController::BarRangeSelection ||
       _inferredSelection == REScoreController::SingleTrackBarRangeSelection) return;
    
    if(FirstSelectedBarIndex() != LastSelectedBarIndex()) return;

    const REPhrase* phrase = FirstSelectedPhrase();
    if(phrase == NULL) return;
    
    REFLOW_SCORE_OPERATION_BEGIN("Duplicate Chords")
    {
        REConstChordVector chordsToDuplicate;
        FindSelectedChords(&chordsToDuplicate);
        
        // Clone Chords
        REChordVector clonedChords;
        for(const REChord* chord : chordsToDuplicate) {
            clonedChords.push_back(chord->Clone());
        }
        
        // Reinsert duplicated chords
        REPhrase* lockedPhrase = lock_.LockPhrase(phrase);
        for(REChord* chord : clonedChords) {
            lockedPhrase->InsertChord(chord, lockedPhrase->ChordCount());
        }
    }
    REFLOW_SCORE_OPERATION_END;
}



// --------------------------------------------------------------------------------------------------------
void REScoreController::SetSystemBreakOnSelection(bool set)
{
    REFLOW_SCORE_OPERATION_BEGIN(set ? "Set System Break" : "Unset System Break")
    {
        REScoreSettings* score = lock_.LockScore(ScoreSettings());
        score->SetSystemBreakAtBarIndex(FirstSelectedBarIndex(), set);
    }
    REFLOW_SCORE_OPERATION_END;
}

void REScoreController::ToggleSystemBreakOnSelection()
{
    const REScoreSettings* score = ScoreSettings();
    bool isSet = score->HasSystemBreakAtBarIndex(FirstSelectedBarIndex());
    SetSystemBreakOnSelection(!isSet);
}

// --------------------------------------------------------------------------------------------------------
void REScoreController::SetRehearsalSignOnSelectedBar(const std::string& text)
{
    _songController->SetRehearsalOnBarAtIndex(FirstSelectedBarIndex(), text);
}

void REScoreController::UnsetRehearsalSignOnSelectedBar()
{
    _songController->UnsetRehearsalOnBarAtIndex(FirstSelectedBarIndex());
}

// --------------------------------------------------------------------------------------------------------
void REScoreController::AddBarAfterSelection()
{
    _songController->CreateEmptyBarAtIndex(LastSelectedBarIndex()+1);
}

void REScoreController::InsertBarBeforeSelection()
{
    _songController->CreateEmptyBarAtIndex(FirstSelectedBarIndex());    
}

void REScoreController::DuplicateSelectedBars()
{
    int firstBarIndex = FirstSelectedBarIndex();
    int lastBarIndex = LastSelectedBarIndex();
    _songController->DuplicateBarsInRange(firstBarIndex, lastBarIndex);
}

void REScoreController::DeleteSelectedBars()
{
    int firstBarIndex = FirstSelectedBarIndex();
    int lastBarIndex = LastSelectedBarIndex();
    _songController->DeleteBarsInRange(firstBarIndex, lastBarIndex);
}


// --------------------------------------------------------------------------------------------------------
void REScoreController::MoveSingleTrackBarRangeSelectionTo(int insertBarIndex, int trackIndex, bool pasteOver)
{
    REFLOW_SCORE_OPERATION_BEGIN("Move Bars")
    {
        RESong* partial = CutSingleTrackBarRangeSelectionToMemory();
        _PasteSingleTrackPartialSongTo(partial, insertBarIndex, trackIndex, pasteOver, false);
        delete partial;
    }
    REFLOW_SCORE_OPERATION_END;
}
void REScoreController::CopySingleTrackBarRangeSelectionTo(int insertBarIndex, int trackIndex, bool pasteOver)
{
    REFLOW_SCORE_OPERATION_BEGIN("Copy Bars")
    {
        RESong* partial = CopySingleTrackBarRangeSelection();
        _PasteSingleTrackPartialSongTo(partial, insertBarIndex, trackIndex, pasteOver, false);
        delete partial;
    }
    REFLOW_SCORE_OPERATION_END;
}

void REScoreController::MoveBarRangeSelectionTo(int insertBarIndex, bool pasteOver)
{
    int firstIdx = FirstSelectedBarIndex();
    int lastIdx = LastSelectedBarIndex();
    int barCount = (lastIdx - firstIdx + 1);
    
    if(insertBarIndex >= firstIdx && insertBarIndex <= lastIdx) {
        return;
    }
    
    int insertBarIndexAfterCut = insertBarIndex;
    if(insertBarIndex > lastIdx) {
        insertBarIndexAfterCut -= barCount;
    }
    
    REFLOW_SCORE_OPERATION_BEGIN("Move Bar Range")
    {
        RESong* partial = CutBarRangeSelectionToMemory();
        _PasteAllTracksPartialSongTo(partial, insertBarIndexAfterCut, pasteOver, true);
        delete partial;
    }
    REFLOW_SCORE_OPERATION_END;
}
void REScoreController::CopyBarRangeSelectionTo(int insertBarIndex, bool pasteOver)
{
    REFLOW_SCORE_OPERATION_BEGIN("Copy Bar Range")
    {
        RESong* partial = CopyBarRangeSelection();
        _PasteAllTracksPartialSongTo(partial, insertBarIndex, pasteOver, true);
        delete partial;
    }
    REFLOW_SCORE_OPERATION_END;
}

void REScoreController::_PasteAllTracksPartialSongTo(const RESong* decodedSong, int insertBarIndex, bool pasteOver, bool includeBarInfo)
{
    assert(Score()->Song()->TrackCount() == decodedSong->TrackCount());
    
    REFLOW_SCORE_OPERATION_BEGIN("Paste")
    {
        const REScore* score = Score();
        RESong* mySong = lock_.LockSong();
        
        // Paste bars
        for(unsigned int i=0; i<decodedSong->BarCount(); ++i)
        {
            bool newBarWasInserted = false;
            if(pasteOver)
            {
                REBar* bar = mySong->Bar(insertBarIndex);
                if(bar == NULL)
                {
                    REBar* clonedBar = decodedSong->Bar(i)->Clone();
                    mySong->InsertBar(clonedBar, insertBarIndex);
                    newBarWasInserted = true;
                }
                else
                {
                    if(includeBarInfo) {
                        const REBar* decodedBar = decodedSong->Bar(i);
                        bar->CopyFrom(*decodedBar);
                    }
                    newBarWasInserted = false;
                }
            }
            else
            {
                REBar* clonedBar = decodedSong->Bar(i)->Clone();
                mySong->InsertBar(clonedBar, insertBarIndex);
                newBarWasInserted = true;
            }
            
            
            // For each track
            for(unsigned int trackIndex=0; trackIndex < mySong->TrackCount(); ++trackIndex)
            {
                RETrack* track = mySong->Track(trackIndex);
                const RETrack* trackToPaste = trackToPaste = decodedSong->Track(trackIndex);
                
                /*else if(decodedSong->TrackCount() == 1 && track == FirstSelectedTrack())
                {
                    trackToPaste = decodedSong->Track(0);
                }*/
                
                // Check incompatible track types
                // TODO: Convert ...
                if(trackToPaste && trackToPaste->Type() != track->Type()) {
                    trackToPaste = NULL;
                }
                
                if(trackToPaste == NULL)
                {
                    if(newBarWasInserted)
                    {
                        for(unsigned int voiceIndex=0; voiceIndex < REFLOW_MAX_VOICES; ++voiceIndex) {
                            REVoice* voice = track->Voice(voiceIndex);
                            voice->InsertPhrase(new REPhrase(), insertBarIndex);
                        }
                    }
                }
                else
                {
                    for(unsigned int voiceIndex=0; voiceIndex < REFLOW_MAX_VOICES; ++voiceIndex)
                    {
                        const REVoice* decodedVoice = trackToPaste->Voice(voiceIndex);
                        REVoice* voice = track->Voice(voiceIndex);
                        if(newBarWasInserted)
                        {
                            REPhrase* phraseToPaste = decodedVoice->Phrase(i)->Clone();
                            voice->InsertPhrase(phraseToPaste, insertBarIndex);
                        }
                        else
                        {
                            const REPhrase* phraseToPaste = decodedVoice->Phrase(i);
                            REPhrase* phraseToPasteOver = voice->Phrase(insertBarIndex);
                            phraseToPasteOver->CopyFrom(*phraseToPaste);
                        }
                    }
                }
            }
            
            ++insertBarIndex;
        }
    }
    REFLOW_SCORE_OPERATION_END

}

void REScoreController::_PasteSingleTrackPartialSongTo(const RESong* decodedSong, int insertBarIndex, int trackIndex, bool pasteOver, bool includeBarInfo)
{
    assert(1 == decodedSong->TrackCount());
    
    REFLOW_SCORE_OPERATION_BEGIN("Paste")
    {
        RESong* mySong = lock_.LockSong();
        
        // Paste bars
        for(unsigned int i=0; i<decodedSong->BarCount(); ++i)
        {
            bool newBarWasInserted = false;
            if(pasteOver)
            {
                REBar* bar = mySong->Bar(insertBarIndex);
                if(bar == NULL)
                {
                    REBar* clonedBar = decodedSong->Bar(i)->Clone();
                    mySong->InsertBar(clonedBar, insertBarIndex);
                    newBarWasInserted = true;
                }
                else
                {
                    if(includeBarInfo) {
                        const REBar* decodedBar = decodedSong->Bar(i);
                        bar->CopyFrom(*decodedBar);
                    }
                    newBarWasInserted = false;
                }
            }
            else
            {
                REBar* clonedBar = decodedSong->Bar(i)->Clone();
                mySong->InsertBar(clonedBar, insertBarIndex);
                newBarWasInserted = true;
            }
            
            
            // For each track
            for(unsigned int theTrackIndex=0; theTrackIndex < mySong->TrackCount(); ++theTrackIndex)
            {
                RETrack* track = mySong->Track(theTrackIndex);
                const RETrack* trackToPaste = (theTrackIndex == trackIndex ? decodedSong->Track(0) : NULL);
                
                // Check incompatible track types
                // TODO: Convert ...
                if(trackToPaste && trackToPaste->Type() != track->Type()) {
                    trackToPaste = NULL;
                }
                
                if(trackToPaste == NULL)
                {
                    if(newBarWasInserted)
                    {
                        for(unsigned int voiceIndex=0; voiceIndex < REFLOW_MAX_VOICES; ++voiceIndex) {
                            REVoice* voice = track->Voice(voiceIndex);
                            voice->InsertPhrase(new REPhrase(), insertBarIndex);
                        }
                    }
                }
                else
                {
                    for(unsigned int voiceIndex=0; voiceIndex < REFLOW_MAX_VOICES; ++voiceIndex)
                    {
                        const REVoice* decodedVoice = trackToPaste->Voice(voiceIndex);
                        REVoice* voice = track->Voice(voiceIndex);
                        if(newBarWasInserted)
                        {
                            REPhrase* phraseToPaste = decodedVoice->Phrase(i)->Clone();
                            voice->InsertPhrase(phraseToPaste, insertBarIndex);
                        }
                        else
                        {
                            const REPhrase* phraseToPaste = decodedVoice->Phrase(i);
                            REPhrase* phraseToPasteOver = voice->Phrase(insertBarIndex);
                            phraseToPasteOver->CopyFrom(*phraseToPaste);
                        }
                    }
                }
            }
            
            ++insertBarIndex;
        }
    }
    REFLOW_SCORE_OPERATION_END
}

RESong* REScoreController::CutBarRangeSelectionToMemory()
{
    RESong* partial = CopyBarRangeSelection();
    REFLOW_SCORE_OPERATION_BEGIN("Cut Bars") {
        DeleteSelectedBars();
    }
    REFLOW_SCORE_OPERATION_END;
    return partial;
}
RESong* REScoreController::CutSingleTrackBarRangeSelectionToMemory()
{
    RESong* partial = CopySingleTrackBarRangeSelection();
    REFLOW_SCORE_OPERATION_BEGIN("Cut Bars") {
        DeleteSelectedChords();
    }
    REFLOW_SCORE_OPERATION_END;
    return partial;
}

void REScoreController::Cut(bool allTracks)
{
    if(_inferredSelection == REScoreController::BarRangeSelection || allTracks)
    {
        Copy(allTracks);
        REFLOW_SCORE_OPERATION_BEGIN("Cut Bars") {
            DeleteSelectedBars();
        }
        REFLOW_SCORE_OPERATION_END;
    }
    else
    {
        Copy(allTracks);
        REFLOW_SCORE_OPERATION_BEGIN("Cut Chords") {
            DeleteSelectedChords();
        }
        REFLOW_SCORE_OPERATION_END;
    }
}

RESong* REScoreController::CopyBarRangeSelection() const
{
    const REScore* score = Score();
    const RESong* song = score->Song();
    
    RESong* isolatedSong = NULL;
    RETrackSet trackSet;
    trackSet.SetAll();
    
    int firstBarIndex = FirstSelectedBarIndex();
    int lastBarIndex = LastSelectedBarIndex();
    isolatedSong = song->IsolateNewSongFromBarRange(firstBarIndex, lastBarIndex, trackSet);
    
    REPrintf("isolated %d tracks and %d bars of song\n", isolatedSong->TrackCount(), isolatedSong->BarCount());
    return isolatedSong;
}

RESong* REScoreController::CopySingleTrackBarRangeSelection() const
{
    const REScore* score = Score();
    const RESong* song = score->Song();
    
    RESong* isolatedSong = NULL;
    int trackIndex = FirstSelectedTrack()->Index();
    RETrackSet trackSet;
    trackSet.Set(trackIndex);
    
    int firstBarIndex = FirstSelectedBarIndex();
    int lastBarIndex = LastSelectedBarIndex();
    isolatedSong = song->IsolateNewSongFromBarRange(firstBarIndex, lastBarIndex, trackSet);
    
    REPrintf("isolated track %d and %d bars of song\n", trackIndex, isolatedSong->BarCount());
    return isolatedSong;
}

void REScoreController::Copy(bool allTracks)
{
    if(_inferredSelection == REScoreController::BarRangeSelection || allTracks)
    {
        RESong* isolatedSong = CopyBarRangeSelection();
        if(_delegate) _delegate->OnCopyPartialSongToPasteboard(isolatedSong);
        delete isolatedSong;
    }
    else if(_inferredSelection == REScoreController::SingleTrackBarRangeSelection)
    {
        RESong* isolatedSong = CopySingleTrackBarRangeSelection();
        if(_delegate) _delegate->OnCopyPartialSongToPasteboard(isolatedSong);
        delete isolatedSong;
    }
    else
    {
        const REStaff* staff = _currentCursor.Staff();
        if(staff == NULL) return;
        
        const REVoice* voice = _currentCursor.Voice();
        if(voice == NULL) return;
        
        const REScore* score = Score();
        const REChord* firstChord = FirstSelectedChord();
        const REChord* lastChord = LastSelectedChord();
        
        REPhrase* isolatedPhrase = NULL;
        if(_inferredSelection == REScoreController::TickRangeSelection || 
           _inferredSelection == REScoreController::CursorSelection)
        {
            isolatedPhrase = voice->IsolateNewPhraseFromSelection(firstChord, lastChord);
        }
        else 
        {
            if(staff->Type() == Reflow::TablatureStaff)
            {
                int firstString = FirstSelectedLine();
                int lastString = LastSelectedLine();
                isolatedPhrase = voice->IsolateNewPhraseFromSelectionInTablature(firstChord, lastChord, firstString, lastString);
            }
            else if(staff->Type() == Reflow::StandardStaff)
            {
                int firstLine = FirstSelectedLine();
                int lastLine = LastSelectedLine();
                isolatedPhrase = voice->IsolateNewPhraseFromSelectionInStaff(firstChord, lastChord, firstLine, lastLine, score->IsTransposing());
            }
        }
        
        REPrintf("isolated phrase, chord count: %d\n", isolatedPhrase->ChordCount());
        
        if(_delegate) _delegate->OnCopyPhraseToPasteboard(isolatedPhrase, staff->Track()->Type());
        delete isolatedPhrase;
    }
}

void REScoreController::PasteEncodedPartialSong(std::string encodedPartialSong)
{
    REConstBufferInputStream input(encodedPartialSong.data(), encodedPartialSong.size());

    unsigned int barCount = input.ReadUInt32();
    unsigned int trackCount = input.ReadUInt32();
    RESong* song = new RESong;
    song->DecodeFrom(input);

    bool pasteOver = song->TrackCount() == 1;
    bool includeBarInfo = false;
    PastePartialSong(song, pasteOver, includeBarInfo);

    delete song;
}

void REScoreController::PastePartialSong(const RESong* decodedSong, bool pasteOver, bool includeBarInfo)
{
    REFLOW_SCORE_OPERATION_BEGIN("Paste")
    {
        const REScore* score = Score();
        RESong* mySong = lock_.LockSong();
        
        // Paste bars
        int insertBarIndex = FirstSelectedBarIndex();
        for(unsigned int i=0; i<decodedSong->BarCount(); ++i)
        {
            bool newBarWasInserted = false;
            if(pasteOver)
            {
                REBar* bar = mySong->Bar(insertBarIndex);
                if(bar == NULL)
                {
                    REBar* clonedBar = decodedSong->Bar(i)->Clone();
                    mySong->InsertBar(clonedBar, insertBarIndex);
                    newBarWasInserted = true;
                }
                else
                {
                    if(includeBarInfo) {
                        const REBar* decodedBar = decodedSong->Bar(i);
                        bar->CopyFrom(*decodedBar);
                    }
                    newBarWasInserted = false;
                }
            }
            else
            {
                REBar* clonedBar = decodedSong->Bar(i)->Clone();
                mySong->InsertBar(clonedBar, insertBarIndex);
                newBarWasInserted = true;
            }
            
            
            // For each track
            for(unsigned int trackIndex=0; trackIndex < mySong->TrackCount(); ++trackIndex)
            {
                RETrack* track = mySong->Track(trackIndex);
                const RETrack* trackToPaste = NULL;
                
                // Same number of tracks in the pasteboard as we have in our song
                if(mySong->TrackCount() == decodedSong->TrackCount()) {
                    trackToPaste = decodedSong->Track(trackIndex);
                }
                
                else if(decodedSong->TrackCount() == 1 && track == FirstSelectedTrack())
                {
                    trackToPaste = decodedSong->Track(0);
                }
                
                // Check incompatible track types
                // TODO: Convert ...
                if(trackToPaste && trackToPaste->Type() != track->Type()) {
                    trackToPaste = NULL;
                }
                
                if(trackToPaste == NULL)
                {
                    if(newBarWasInserted)
                    {
                        for(unsigned int voiceIndex=0; voiceIndex < REFLOW_MAX_VOICES; ++voiceIndex) {
                            REVoice* voice = track->Voice(voiceIndex);
                            voice->InsertPhrase(new REPhrase(), insertBarIndex);
                        }
                    }
                }
                else 
                {
                    for(unsigned int voiceIndex=0; voiceIndex < REFLOW_MAX_VOICES; ++voiceIndex) 
                    {
                        const REVoice* decodedVoice = trackToPaste->Voice(voiceIndex);
                        REVoice* voice = track->Voice(voiceIndex);
                        if(newBarWasInserted)
                        {
                            REPhrase* phraseToPaste = decodedVoice->Phrase(i)->Clone();
                            voice->InsertPhrase(phraseToPaste, insertBarIndex);
                        }
                        else
                        {
                            const REPhrase* phraseToPaste = decodedVoice->Phrase(i);
                            REPhrase* phraseToPasteOver = voice->Phrase(insertBarIndex);
                            phraseToPasteOver->CopyFrom(*phraseToPaste);
                        }
                    }
                }
            }
            
            ++insertBarIndex;
        }
    }
    REFLOW_SCORE_OPERATION_END
}

void REScoreController::PasteEncodedPhrase(std::string encodedPhrase)
{
    REConstBufferInputStream input(encodedPhrase.data(), encodedPhrase.size());

    REPhrase* p = new REPhrase;
    Reflow::TrackType trackType = static_cast<Reflow::TrackType>(input.ReadUInt8());
    p->DecodeFrom(input);
    PastePhrase(p, trackType);
    delete p;
}

void REScoreController::PastePhrase(const REPhrase* phraseToInsert, Reflow::TrackType trackType)
{
    if(_inferredSelection == REScoreController::BarRangeSelection ||
       _inferredSelection == REScoreController::SingleTrackBarRangeSelection)
    {
        return;
    }
    
    REFLOW_SCORE_OPERATION_BEGIN("Paste Phrase")
    {
        int currentChordIdx = 0;
        const REPhrase* destPhrase = FirstSelectedPhrase();
        const REChord* chordAfter = FirstSelectedChord();
        while(currentChordIdx < phraseToInsert->ChordCount())
        {
            const REChord* chordToInsert = phraseToInsert->Chord(currentChordIdx);
            
            int insertChordIdx = (chordAfter != NULL ? chordAfter->Index() : destPhrase->ChordCount());
            if(insertChordIdx < 0) insertChordIdx = 0;
            if(insertChordIdx > destPhrase->ChordCount()) insertChordIdx = destPhrase->ChordCount();
            
            REPhrase* lockedPhrase = lock_.LockPhrase(destPhrase);
            {
                lockedPhrase->InsertChord(chordToInsert->Clone(), insertChordIdx);
            }
            
            ++currentChordIdx;
        }
    }
    REFLOW_SCORE_OPERATION_END
}

void REScoreController::SetTimeSignatureOfSelectedBars(const RETimeSignature& ts)
{
    if(_inferredSelection == REScoreController::CursorSelection)
    {
        _songController->SetTimeSignatureAtBarIndex(FirstSelectedBarIndex(), ts);
    }
    else 
    {
        _songController->SetTimeSignatureOfBarRange(FirstSelectedBarIndex(), LastSelectedBarIndex(), ts);
    }
}

void REScoreController::SetTimeSignatureAndBeamingPatternOfSelectedBars(const RETimeSignature& ts, const REBeamingPattern& pattern)
{
    REFLOW_SCORE_OPERATION_BEGIN("Change Time Signature and Beaming Pattern")
    {
        if(_inferredSelection == REScoreController::CursorSelection) {
            _songController->SetTimeSignatureAtBarIndex(FirstSelectedBarIndex(), ts);
            _songController->SetBeamingPatternAtBarIndex(FirstSelectedBarIndex(), pattern);
        }
        else {
            _songController->SetTimeSignatureOfBarRange(FirstSelectedBarIndex(), LastSelectedBarIndex(), ts);
            _songController->SetBeamingPatternOfBarRange(FirstSelectedBarIndex(), LastSelectedBarIndex(), pattern);
        }
    }
    REFLOW_SCORE_OPERATION_END
}

void REScoreController::SetKeySignatureOfSelectedBars(const REKeySignature& ks)
{
    if(_inferredSelection == REScoreController::CursorSelection)
    {
        _songController->SetKeySignatureAtBarIndex(FirstSelectedBarIndex(), ks);
    }
    else 
    {
        _songController->SetKeySignatureOfBarRange(FirstSelectedBarIndex(), LastSelectedBarIndex(), ks);
    }
}

void REScoreController::SetClefOfSelectedBars(Reflow::ClefType clef, Reflow::OttaviaType ottavia)
{
    const REStaff* firstStaff = Cursor().Staff();
    if(firstStaff == NULL) return;
    
    const RETrack* track = firstStaff->Track();
    int barIndex = FirstSelectedBarIndex();
    bool leftHand = (firstStaff->FirstVoiceIndex() != 0);
    
    if(_inferredSelection == REScoreController::CursorSelection)
    {
        _songController->SetTrackClefAtBarIndex(track->Index(), barIndex, clef, ottavia, leftHand);
    }
    else 
    {
        int lastBarIndex = LastSelectedBarIndex();
        _songController->SetTrackClefOfBarRange(track->Index(), barIndex, lastBarIndex, clef, ottavia, leftHand);
    }
}

void REScoreController::SetRepeatStartOnSelectedBar(bool repeat)
{
    _songController->SetRepeatStartOnBarAtIndex(FirstSelectedBarIndex(), repeat);
}
void REScoreController::SetRepeatEndOnSelectedBar(bool repeat)
{
    _songController->SetRepeatEndOnBarAtIndex(FirstSelectedBarIndex(), repeat);    
}
void REScoreController::SetRepeatCountOnSelectedBar(int repeatCount)
{
    _songController->SetRepeatCountOnBarAtIndex(FirstSelectedBarIndex(), repeatCount);
}

void REScoreController::SetDynamicsOnSelection(Reflow::DynamicsType dynamics)
{
    try {
        if(AllTheSelectedChordsVerify([=](const REChord* chord) {return chord->Dynamics() == dynamics;})) {
            PerformTaskOnSelectedChords(bind(&REChord::SetDynamics, std::placeholders::_1, Reflow::DynamicsUndefined), "Unset Dynamics");
        }
        else {
            PerformTaskOnSelectedChords(bind(&REChord::SetDynamics, std::placeholders::_1, dynamics), "Set Dynamics");
        }
    }
    REFLOW_SCORE_OPERATION_CATCH_EXCEPTIONS;
}

void REScoreController::SplitSelectedChords()
{
    REConstChordVector selectedChords;
    FindSelectedChords(&selectedChords);
    
    REFLOW_SCORE_OPERATION_BEGIN("Split Beat")
    {
        for(const REChord* chord : selectedChords)
        {
            REPhrase* phrase = lock_.LockPhrase(chord->Phrase());
            REChord* chord0 = phrase->Chord(chord->Index());
            chord0->IncreaseNoteValue();
            REChord* chord1 = chord0->Clone();
            phrase->InsertChord(chord1, chord0->Index()+1);
        }
    }
    REFLOW_SCORE_OPERATION_END;
}

#pragma mark Symbols
void REScoreController::SetSlurStartBeat(int trackIndex, Reflow::StaffIdentifier staffId, int slurIndex, const REGlobalTimeDiv& div, const REPoint& offset)
{
    REFLOW_SCORE_OPERATION_BEGIN("tweak slur")
    {
        RETrack* track = lock_.LockTrack(_score.Song()->Track(trackIndex));
        RESlur* slur = (track ? track->Slur(staffId, slurIndex) : nullptr);
        if(slur)
        {
            slur->SetStartBeat(div);
            slur->SetStartOffset(offset);
        }
    }
    REFLOW_SCORE_OPERATION_END;
}
void REScoreController::SetSlurEndBeat(int trackIndex, Reflow::StaffIdentifier staffId, int slurIndex, const REGlobalTimeDiv& div, const REPoint& offset)
{
    REFLOW_SCORE_OPERATION_BEGIN("tweak slur")
    {
        RETrack* track = lock_.LockTrack(_score.Song()->Track(trackIndex));
        RESlur* slur = (track ? track->Slur(staffId, slurIndex) : nullptr);
        if(slur)
        {
            slur->SetEndBeat(div);
            slur->SetEndOffset(offset);
        }
    }
    REFLOW_SCORE_OPERATION_END;
}
void REScoreController::SetSlurStartOffset(int trackIndex, Reflow::StaffIdentifier staffId, int slurIndex, const REPoint& offset)
{
    REFLOW_SCORE_OPERATION_BEGIN("tweak slur")
    {
        RETrack* track = lock_.LockTrack(_score.Song()->Track(trackIndex));
        RESlur* slur = (track ? track->Slur(staffId, slurIndex) : nullptr);
        if(slur)
        {
            slur->SetStartOffset(offset);
        }
    }
    REFLOW_SCORE_OPERATION_END;
}
void REScoreController::SetSlurEndOffset(int trackIndex, Reflow::StaffIdentifier staffId, int slurIndex, const REPoint& offset)
{
    REFLOW_SCORE_OPERATION_BEGIN("tweak slur")
    {
        RETrack* track = lock_.LockTrack(_score.Song()->Track(trackIndex));
        RESlur* slur = (track ? track->Slur(staffId, slurIndex) : nullptr);
        if(slur)
        {
            slur->SetEndOffset(offset);
        }
    }
    REFLOW_SCORE_OPERATION_END;
}
void REScoreController::SetSlurStartControlPointOffset(int trackIndex, Reflow::StaffIdentifier staffId, int slurIndex, const REPoint& controlPointOffset)
{
    REFLOW_SCORE_OPERATION_BEGIN("tweak slur")
    {
        RETrack* track = lock_.LockTrack(_score.Song()->Track(trackIndex));
        RESlur* slur = (track ? track->Slur(staffId, slurIndex) : nullptr);
        if(slur)
        {
            slur->SetStartControlPointOffset(controlPointOffset);
        }
    }
    REFLOW_SCORE_OPERATION_END;
}
void REScoreController::SetSlurEndControlPointOffset(int trackIndex, Reflow::StaffIdentifier staffId, int slurIndex, const REPoint& controlPointOffset)
{
    REFLOW_SCORE_OPERATION_BEGIN("tweak slur")
    {
        RETrack* track = lock_.LockTrack(_score.Song()->Track(trackIndex));
        RESlur* slur = (track ? track->Slur(staffId, slurIndex) : nullptr);
        if(slur)
        {
            slur->SetEndControlPointOffset(controlPointOffset);
        }
    }
    REFLOW_SCORE_OPERATION_END;
}

void REScoreController::ToggleSlurOnSelection()
{
    if(_inferredSelection == REScoreController::TickRangeSelection)
    {
        const REChord* firstChord = FirstSelectedChord();
        const REChord* lastChord = LastSelectedChord();
        RELocator firstChordLocator = firstChord->Locator();
        RELocator lastChordLocator = lastChord->Locator();
        PerformTaskOnSong([&](RESong* song) {
            RETrack* track = song->Track(firstChordLocator.TrackIndex());
            
            RESlur slur;
            slur.SetStartBeat(REGlobalTimeDiv(firstChordLocator.BarIndex(), firstChord->Offset()));
            slur.SetEndBeat(REGlobalTimeDiv(lastChordLocator.BarIndex(), lastChord->Offset()));
            
            Reflow::StaffIdentifier staffId = Cursor().Staff()->Identifier();
            track->Slurs(staffId).push_back(slur);
        });
    }
    else if(_inferredSelection == REScoreController::CursorSelection)
    {
        const REStaff* staff = Cursor().Staff();
        if(!staff) return;
        
        const RETrack* track = staff->Track();
        const REChord* firstChord = FirstSelectedChord();
        if(!firstChord) return;
        
        RELocator locator = firstChord->Locator();
        REGlobalTimeDiv beat(locator.BarIndex(), firstChord->Offset());
        
        const RESlurVector& slurs = track->Slurs(staff->Identifier());
        for(const RESlur& slur : slurs) {
            if(slur.StartBeat() == beat || slur.EndBeat() == beat) {
                int idx = track->IndexOfSlur(&slur, staff->Identifier());
                if(idx != -1)
                {
                    PerformTaskOnSong([&](RESong* song)
                    {
                        RETrack* t = song->Track(track->Index());
                        t->RemoveSlurAtIndex(idx, staff->Identifier());
                    });
                    return;
                }
            }
        }
    }
}

#pragma mark Layout
void REScoreController::ClearViewport()
{
    if(_viewport == NULL) return;

    _viewport->Clear();
}

void REScoreController::RebuildViewport()
{
    if(_viewport == NULL) return;
    
    _viewport->Clear();
    _viewport->Build();
}

void REScoreController::RefreshViewportItemAtBarIndex(int barIndex)
{
	if(_viewport) _viewport->RefreshItemAtBarIndex(barIndex);
}

void REScoreController::UpdateLayoutVisibleItems()
{
    if(_viewport) {
        _viewport->UpdateVisibleViews();
    }
}

void REScoreController::EncodeTo(REOutputStream& coder) const
{
    coder.WriteInt32(_scoreIndex);
    coder.WriteInt32((int32_t)_layoutType);
    coder.WriteInt32((int32_t)_pageLayoutType);
    coder.WriteInt32((int32_t)_tool);
    coder.WriteInt32(_preferredSelection);
    coder.WriteInt32(_inferredSelection);
    _currentCursor.EncodeTo(coder);
    _originCursor.EncodeTo(coder);
    coder.WriteInt8(_typingSecondDigit ? 1:0);
}
void REScoreController::DecodeFrom(REInputStream& decoder)
{
    _scoreIndex = decoder.ReadInt32();
    _layoutType = (Reflow::ScoreLayoutType)decoder.ReadInt32();
    _pageLayoutType = (Reflow::PageLayoutType)decoder.ReadInt32();
    _tool = (Reflow::ToolType)decoder.ReadInt32();
    _preferredSelection = (REScoreController::SelectionKindType)decoder.ReadInt32();
    _inferredSelection = (REScoreController::SelectionKindType)decoder.ReadInt32();
    _currentCursor.DecodeFrom(decoder);
    _originCursor.DecodeFrom(decoder);
    _typingSecondDigit = (decoder.ReadInt8() == 1);
}

void REScoreController::MouseDown(const REPoint& pointInScene, unsigned long flags)
{
    REPrintf("## MouseDown at (%1.3f, %1.3f)\n", pointInScene.x, pointInScene.y);
    if(_tool == Reflow::TablatureTool)
    {
        if(SlurTool().MouseDown(pointInScene, flags)) {
            return ;
        }
        
        const RESystem* system = Score()->PickSystem(pointInScene);
        if(system == NULL) return;
        
        REPoint pointInSystem = system->PointFromSceneToLocal(pointInScene);
        float x = 0.0f;
        float y = pointInSystem.y;
        const RESlice* slice = system->SystemBarAtX(pointInSystem.x, &x);
        
        int lineIndex = 0;
        int chordIndex = 0;
        const REStaff* staff = (system ? system->StaffAtY(y, &lineIndex) : NULL);
        if(system && staff && slice)
        {
            REPrintf("  >> Over Line %d of Staff %d\n", lineIndex, staff->Index());
            
            // Phrase under mouse
            int currentVoice = Cursor().VoiceIndex();
            const RETrack* track = staff->Track();
            const REVoice* voice = track->Voice(currentVoice);
            const REPhrase* phrase = voice->Phrase(slice->BarIndex());
            int tick = 0;
            const REBarMetrics& barMetrics = slice->Metrics();
            
            int columnIndex;
            float snapX;
            RESlice::QueryColumnResult result = slice->QueryColumnAtX(x, &columnIndex, &snapX);
            if(result & RESlice::InLeadingSpace) {
                chordIndex = 0;
                tick = 0;
            }
            else if(result & RESlice::InTrailingSpace) {
                chordIndex = phrase->ChordCount()-1;
                if(barMetrics.ColumnCount()) {
                    tick = barMetrics.Column(barMetrics.ColumnCount()-1).tick;
                }
            }
            else if(result & RESlice::OnColumn) {
                float snapDist = 8.0;
                unsigned long tickUnderMouse = slice->Metrics().TickOfColumn(columnIndex);
                tick = tickUnderMouse;
                
                const REChord* chordL = NULL;
                const REChord* chordR = NULL;
                phrase->ChordsSurroundingTick(tickUnderMouse, &chordL, &chordR);
                //RELog(@"      $$Chord Left is %d", (chordL ? chordL->Index() : -1));
                //RELog(@"      $$Chord Right is %d", (chordR ? chordR->Index() : -1));
                
                if(chordL != NULL && chordR == NULL)
                {
                    chordIndex = chordL->Index();
                }
                else if(chordL == NULL && chordR != NULL)
                {
                    chordIndex = chordR->Index();
                }
                else if(chordL != NULL && chordR != NULL)
                {
                    float xL = slice->XOffsetOfTick(chordL->OffsetInTicks());
                    float xR = slice->XOffsetOfTick(chordR->OffsetInTicks());
                    float dt = (x - xL) / (xR-xL);
                    if(dt <= 0.50) {
                        chordIndex = chordL->Index();
                    }
                    else {
                        chordIndex = chordR->Index();
                    }
                }
            }
            else {
                chordIndex = 0;
            }
            
            MoveCursorTo(staff->Index(), slice->BarIndex(), tick, lineIndex, flags);
        }
    }
    else if(_tool == Reflow::DesignTool)
    {
        _selectionStartPoint = pointInScene;
        _selectionRectVisible = false;
    }
    else
    {
        RETool* tool = CurrentTool();
        if(tool) {
            tool->MouseDown(pointInScene, flags);
        }
    }
    
    if(_viewport) _viewport->UpdateSelectionRect();
}

void REScoreController::MouseDoubleClicked(const REPoint &point, unsigned long flags)
{
    RETool* tool = CurrentTool();
    if(tool) {
        tool->MouseDoubleClicked(point, flags);
    }
    
    if(_viewport) _viewport->OnMouseDoubleClicked(point, flags);
}

void REScoreController::MouseUp(const REPoint& pointInScene, unsigned long flags)
{
    _selectionRectVisible = false;
    if(_tool == Reflow::DesignTool)
    {
        _selectionEndPoint = pointInScene;
    }
    else if(_tool == Reflow::TablatureTool) {
        SlurTool().MouseUp(pointInScene, flags);
    }
    else
    {
        RETool* tool = CurrentTool();
        if(tool) {
            tool->MouseUp(pointInScene, flags);
        }
    }
    if(_viewport) _viewport->UpdateSelectionRect();
}
void REScoreController::MouseDragged(const REPoint& absPos, unsigned long flags)
{
    const RESong* song = Score()->Song();
    if(_tool == Reflow::DesignTool)
    {
        _selectionEndPoint = absPos;
        _selectionRectVisible = true;
        if(_viewport) _viewport->UpdateSelectionRect();
        
        RERect selectionRect = SelectionRect();
        
        // Unselect all notes and retrieve affected bars
        REIntSet affectedBars;
        _songController->UnselectAllNotes(&affectedBars);
        
        // Pick notes in selection rectangle
        RENoteSet notes;
        for(const RESystem* system : _score.Systems())
        {
            RERect localRect = system->RectFromSceneToLocal(selectionRect);
            if(RERect::Intersects(system->Bounds(), localRect)) {
                system->PickNotesInRect(localRect, &notes, &affectedBars);
            }
        }
        
        // Select notes
        _songController->SelectNotes(notes);
        
        // Find affected systems given affected bars
        RESystemSet affectedSystems;
        _score.FindSystemsWithBarIndexSet(&affectedSystems, affectedBars);
        for(RESystemSet::const_iterator it = affectedSystems.begin(); it != affectedSystems.end(); ++it)
        {
            REViewportSystemItem* systemItem = static_cast<REViewportSystemItem*>((*it)->ViewportItem());
            if(systemItem) systemItem->SetNeedsDisplay();
        }
    }
    else if(_tool == Reflow::TablatureTool)
    {
        if(SlurTool().MouseDragged(absPos, flags)) {
            return ;
        }
        else {
            MouseDown(absPos, flags | REScoreController::CursorShiftDown);
        }
    }
    else
    {
        RETool* tool = CurrentTool();
        if(tool) {
            tool->MouseDragged(absPos, flags);
        }
    }
}

void REScoreController::MouseMoved(const REPoint& pointInScene, unsigned long flags)
{
    RETool* tool = CurrentTool();
    if(tool) {
        tool->MouseMoved(pointInScene, flags);
    }
}

void REScoreController::MouseWheel(const REPoint& point, const REPoint& delta, unsigned long flags)
{
    if(_viewport) {
        REPoint offset = _viewport->ViewportOffset();
        float ox = roundf(offset.x + delta.x);
        float oy = roundf(offset.y + delta.y);
        _viewport->SetViewportOffset(REPoint(ox, oy));
    }
}

void REScoreController::CreateDefaultViewport()
{
    _viewport = new REViewport(this);
}
