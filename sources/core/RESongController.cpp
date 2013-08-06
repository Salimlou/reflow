//
//  RESongController.cpp
//  ReflowTools
//
//  Created by Sebastien Bourgeois on 13/02/12.
//  Copyright (c) 2012 Gargant Studios. All rights reserved.
//

#include "RESongController.h"
#include "RESong.h"
#include "REException.h"
#include "REFunctions.h"
#include "REInputStream.h"
#include "REOutputStream.h"
#include "REScoreController.h"
#include "RECursor.h"
#include "RENote.h"
#include "REChord.h"
#include "REPhrase.h"
#include "RETrack.h"
#include "RELocator.h"
#include "RESequencer.h"
#include "REScore.h"
#include "REBar.h"
#include "REVoice.h"
#include "RETimer.h"
#include "REViewport.h"

RECreateTrackOptions::RECreateTrackOptions()
{
    type = Reflow::StandardTrack;
    subType = Reflow::GuitarInstrument;
    name = "Track";
    shortName = "Trk.";
    
    createPart = true;
    addToFullScore = true;
    useGrandStaff = true;
    droneString = false;
    
    firstClef = Reflow::TrebleClef;
    firstOttavia = Reflow::NoOttavia;
    secondClef = Reflow::TrebleClef;
    secondOttavia = Reflow::NoOttavia;
    
    midiProgram = 0;
    midiBank = 0;
    
    capo = 0;
}

RECreateTrackOptions& RECreateTrackOptions::operator=(const RECreateTrackOptions& opts)
{
    type = opts.type;
    subType = opts.subType;
    name = opts.name;
    shortName = opts.shortName;

    createPart = opts.createPart;
    addToFullScore = opts.addToFullScore;
    useGrandStaff = opts.useGrandStaff;
    droneString = opts.droneString;

    firstClef = opts.firstClef;
    firstOttavia = opts.firstOttavia;
    secondClef = opts.secondClef;
    secondOttavia = opts.secondOttavia;
    
    midiProgram = opts.midiProgram;
    midiBank = opts.midiBank;
    
    tuning = opts.tuning;
    
    capo = opts.capo;
    
    return *this;
}

RESongController::RESongController(RESong* song) 
	: _song(nullptr), _sequencer(nullptr), _updateSinglePhrase(false), _updatedPhrase(nullptr)
{
    _song = song;
    _sequencer = new RESequencer;
    _sequencer->AddListener(this);
    AddDelegate(_sequencer);
}

RESongController::~RESongController()
{
    delete _sequencer;
    _sequencer = nullptr;
}

void RESongController::OnSequencerUpdateRT(const RESequencer* sequencer)
{
    for(REScoreController* scoreController : _scoreControllers)
    {
        REViewport* viewport = scoreController->Viewport();
        if(viewport) viewport->_UpdatePlaybackRT(sequencer);
    }
}

bool RESongController::IsEditable(unsigned long flags) const
{
    return true;
}

REScoreController* RESongController::ActiveScoreController()
{
    return _scoreControllers.empty() ? NULL : _scoreControllers.front();
}

void RESongController::RemoveAllScoreControllers()
{
    _scoreControllers.clear();
}

void RESongController::AddDelegate(RESongControllerDelegate* delegate)
{
    _delegates.push_back(delegate);
}

RESongControllerTask* RESongController::PushTask(const std::string& taskName, unsigned long flags)
{
    RESongControllerTask* task = new RESongControllerTask(taskName, flags);
    if(_tasks.empty())
    {
        SongWillUpdate();
        
        // This is the first task we push, create a restore point
        for(RESongControllerDelegate* delegate : _delegates)
        {
            delegate->SongControllerWantsToBackup(this, _song);
        }

		_updateSinglePhrase = true;
		_updatedPhrase = NULL;
    }
    _tasks.push_back(task);
    return task;
}

RESongControllerTask* RESongController::PopTask()
{
    RESongControllerTask* task = _tasks.back();
    _tasks.pop_back();
    return task;
}

void RESongController::CommitTask(RESongControllerTask* task)
{
    if(_tasks.empty())
    {   
		if(_updateSinglePhrase && _updatedPhrase != nullptr) {
			PhraseWasUpdated(_updatedPhrase, task->IsFinished());
		}
		else {
			SongWasUpdated(task->IsFinished());
		}
    }
}

void RESongController::BackupToStream(REOutputStream& stream)
{
	_song->EncodeTo(stream);
    
    for(REScoreController* scoreController : _scoreControllers)
    {
        scoreController->EncodeTo(stream);
    }
}

void RESongController::RestoreFromStream(REInputStream& stream)
{
    SongWillUpdate();
    
    _song->DecodeFrom(stream);
    
    for(REScoreController* scoreController : _scoreControllers)
    {
        scoreController->ClearViewport();
        scoreController->DecodeFrom(stream);
    }
    
    SongWasUpdated(true);
}

void RESongController::RestoreFromSongDataStream(REInputStream& stream)
{
    SongWillUpdate();
    
    // Backup score controllers
    REBufferOutputStream backupStream;
    for(REScoreController* scoreController : _scoreControllers) scoreController->EncodeTo(backupStream);
    
    // Decode song
    _song->DecodeFrom(stream);

    // Restore score controllers
    REConstBufferInputStream restoreStream(backupStream.Data(), backupStream.Size());
    for(REScoreController* scoreController : _scoreControllers) scoreController->DecodeFrom(restoreStream);
    
    SongWasUpdated(true);
}

void RESongController::SongWillUpdate()
{
    for(RESongControllerDelegate* delegate : _delegates) {
        delegate->SongControllerWillModifySong(this, _song);
    }
    for(REScoreController* scoreController : _scoreControllers) {
        scoreController->SongControllerWillModifySong(this, _song);
    }
}

void RESongController::SongWasUpdated(bool success)
{
    REPrintf("[runtime] Song was Updated\n");
    RETimer timer;
    timer.Start();
    
    // Refresh song
    _song->Refresh(true);
    double deltaTimeForRefresh = timer.DeltaTimeInMilliseconds();
    /*for(int i=0; i<_song->ScoreCount(); ++i) {
        _song->Score(i)->SetDirty();
    }*/
    
    // There is no more pending task, we can inform our delegates that we are done with modifying the song
    for(RESongControllerDelegate* delegate : _delegates) {
        delegate->SongControllerDidModifySong(this, _song, success);
    }
    
    for(REScoreController* scoreController : _scoreControllers)
    {
        REScore& score = scoreController->_score;
        int scoreIndex = scoreController->ScoreIndex();
        if(scoreIndex >= _song->ScoreCount()) scoreIndex = _song->ScoreCount()-1;
        const REScoreSettings* scoreSettings = _song->Score(scoreIndex);
        RETimer timer2; timer2.Start();
//        score->Refresh();
        
        scoreController->ClearViewport(); // <<---
        score.SetLayoutType(scoreController->LayoutType());
        score.SetPageLayoutType(scoreController->PageLayoutType());
        score.Rebuild(*scoreSettings);

        scoreController->_currentCursor.ForceValidPosition();
        scoreController->_originCursor.ForceValidPosition();
        
        scoreController->RebuildViewport();
        timer2.Stop();
        REPrintf("[runtime] score rebuild: %1.4f\n", timer2.DeltaTimeInMilliseconds());
        
        scoreController->UpdateActions();
        scoreController->SongControllerDidModifySong(this, _song, success);
    }
    
    timer.Stop();
    REPrintf("[runtime] %1.4f ms (song refresh: %1.4f)\n", timer.DeltaTimeInMilliseconds(), deltaTimeForRefresh);
}

void RESongController::PhraseWasUpdated(REPhrase* phrase, bool success)
{
#if 1
    SongWasUpdated(success);
#else
	phrase->Refresh(false);
	bool modifiedPreviousSiblingPhrase = phrase->_FixTieFlags();
	if(modifiedPreviousSiblingPhrase) {
		SongWasUpdated(success);
	}
	else 
	{
		// Inform delegates
		std::for_each(_delegates.begin(), _delegates.end(), 
			boost::bind(&RESongControllerDelegate::SongControllerDidModifyPhrase, _1, this, phrase, success));

		for(REScoreController* scoreController : _scoreControllers) 
		{
			REScore& score = scoreController->_score;
			score.RefreshSingleBar(phrase->Index());
			scoreController->RefreshViewportItemAtBarIndex(phrase->Index());
			
			scoreController->UpdateActions();
			scoreController->SongControllerDidModifyPhrase(this, phrase, success);
		}
	}
#endif
}

void RESongController::BackupSongStateToStream(REOutputStream& stream)
{
    _song->EncodeTo(stream);
}

void RESongController::RestoreSongStateFromStream(REInputStream& stream)
{
    _song->DecodeFrom(stream);
}





RELockSongControllerForTask::RELockSongControllerForTask(RESongController* controller, const std::string& name, unsigned long flags)
: _controller(controller), _task(NULL)
{
    _task = _controller->PushTask(name, flags);
}

void RELockSongControllerForTask::Commit()
{
    if(_task) {
        _task->Finish();
    }
}

RELockSongControllerForTask::~RELockSongControllerForTask()
{
    if(_task) 
    {
        // Pop the Task
        _controller->PopTask();
        
        // If it was gracefully finished, commit the changes
        if(_task->IsFinished()) {
            _controller->CommitTask(_task);
        }
        
        delete _task;
    }
}

RESong* RELockSongControllerForTask::LockSong()
{
	_controller->_updateSinglePhrase = false;
    return _controller->_song;
}

REScoreSettings* RELockSongControllerForTask::LockScore(const REScoreSettings* score)
{
	_controller->_updateSinglePhrase = false;
    return (score ? _controller->_song->Score(score->Index()) : NULL);
}

RETrack* RELockSongControllerForTask::LockTrack(const RETrack* track)
{
	_controller->_updateSinglePhrase = false;
    return (track ? _controller->_song->Track(track->Index()) : NULL);
}

REBar* RELockSongControllerForTask::LockBar(const REBar* bar)
{
	_controller->_updateSinglePhrase = false;
    return bar ? _controller->_song->Bar(bar->Index()) : NULL;
}

REPhrase* RELockSongControllerForTask::LockPhrase(const REPhrase* phrase)
{
	if(phrase == NULL) return NULL;

	// Verify that we are still updating a single phrase in the song
	if(_controller->_updateSinglePhrase) {
		if(_controller->_updatedPhrase == NULL) {
			_controller->_updatedPhrase = _controller->_song->PhraseAtLocator(phrase->Locator());
		}
		else if(_controller->_updatedPhrase != phrase) {
			_controller->_updateSinglePhrase = false;
		}
	}

    return _controller->_song->PhraseAtLocator(phrase->Locator());
}

REChord* RELockSongControllerForTask::LockChord(const REChord* chord)
{
	if(chord == NULL) return NULL;

	// Phrase is always locked when we modify one of its chords
	LockPhrase(chord->Phrase());	

    return _controller->_song->ChordAtLocator(chord->Locator());
}

RENote* RELockSongControllerForTask::LockNote(const RENote* note)
{
	if(note == NULL) return NULL;

	// Chord (then Phrase) is always locked when we modify one of its notes
	LockChord(note->Chord());

    return _controller->_song->NoteAtLocator(note->Locator());
}

void RESongController::DoSomethingWithReflowError(REException& err)
{
    assert(false);
}
void RESongController::DoSomethingWithError(std::exception& err)
{
    assert(false);    
}
void RESongController::DoSomethingWithUnhandledError()
{
    assert(false);    
}

#define REFLOW_OPERATION_BEGIN(opName)      try{RELockSongControllerForTask lock_(this, opName, 0);
#define REFLOW_OPERATION_END                lock_.Commit();} catch(REException& ex) {DoSomethingWithReflowError(ex);} catch(std::exception& ex) {DoSomethingWithError(ex);} catch(...) {DoSomethingWithUnhandledError();}

void RESongController::SetRehearsalOnBarAtIndex(int barIndex, const std::string& rehearsal)
{
    const REBar* bar = _song->Bar(barIndex);
    if(bar == NULL) return;
    
    if(rehearsal.empty()) 
    {
        UnsetRehearsalOnBarAtIndex(barIndex);
    }
    else 
    {
        REFLOW_OPERATION_BEGIN("Set Rehearsal Sign")
        {
            REBar* barLocked = lock_.LockBar(bar);
            barLocked->SetRehearsalSignText(rehearsal);
            barLocked->SetFlag(REBar::RehearsalSign);
        }
        REFLOW_OPERATION_END;
    }
}

void RESongController::UnsetRehearsalOnBarAtIndex(int barIndex)
{
    const REBar* bar = _song->Bar(barIndex);
    if(bar == NULL) return;
    
    REFLOW_OPERATION_BEGIN("Unset Rehearsal Sign")
    {
        REBar* barLocked = lock_.LockBar(bar);
        barLocked->UnsetFlag(REBar::RehearsalSign);
    }
    REFLOW_OPERATION_END;    
}

void RESongController::InsertTempoMarker(int barIndex, const RETimeDiv& timeDiv, int tempo, Reflow::TempoUnitType tempoType)
{
    REFLOW_OPERATION_BEGIN("Insert Tempo Marker")
    {
        RESong* song = lock_.LockSong();
        RETempoTimeline& tempoTimeline = song->TempoTimeline();
        
        RETempoItem item(barIndex, timeDiv, tempo, tempoType);
        tempoTimeline.InsertItem(item);
    }
    REFLOW_OPERATION_END
}

void RESongController::DeleteTempoMarkersInBarRange(int firstBar, int lastBar)
{
    REFLOW_OPERATION_BEGIN("Delete Tempo Markers")
    {
        RESong* song = lock_.LockSong();
        RETempoTimeline& tempoTimeline = song->TempoTimeline();
        tempoTimeline.RemoveItemsInBarRange(firstBar, lastBar);
    }
    REFLOW_OPERATION_END;
}

void RESongController::InsertChordName(const REChordName& chordName, int barIndex, int tick)
{
    const REBar* bar = _song->Bar(barIndex);
    if(bar == NULL) return;
    
    REFLOW_OPERATION_BEGIN("Insert Chord Name")
    {
        REBar* lockedBar = lock_.LockBar(bar);
        lockedBar->InsertChordName(tick, chordName);
    }
    REFLOW_OPERATION_END;
}

void RESongController::RemoveChordNamesOfBar(int barIndex)
{
    const REBar* bar = _song->Bar(barIndex);
    if(bar == NULL) return;
    
    REFLOW_OPERATION_BEGIN("Remove Chord Names")
    {
        REBar* lockedBar = lock_.LockBar(bar);
        lockedBar->RemoveAllChordNames();
    }
    REFLOW_OPERATION_END;    
}

void RESongController::InsertTrackChordDiagramAtBarIndex(const REChordDiagram& chordDiagram, int trackIndex, int barIndex, int tick)
{
    const REPhrase* phrase = RELocator(_song, barIndex, trackIndex, 0).Phrase();
    if(phrase == NULL) return;
    
    REFLOW_OPERATION_BEGIN("Insert Chord Diagram")
    {
        REPhrase* lockedPhrase = lock_.LockPhrase(phrase);
        lockedPhrase->InsertChordDiagram(tick, chordDiagram);
    }
    REFLOW_OPERATION_END;
}

void RESongController::RemoveTrackChordDiagramsAtBarIndex(int trackIndex, int barIndex)
{
    const REPhrase* phrase = RELocator(_song, barIndex, trackIndex, 0).Phrase();
    if(phrase == NULL) return;
    
    REFLOW_OPERATION_BEGIN("Remove Chord Diagrams")
    {
        REPhrase* lockedPhrase = lock_.LockPhrase(phrase);
        lockedPhrase->RemoveAllChordDiagrams();
    }
    REFLOW_OPERATION_END;    
}

void RESongController::CreateEmptyBarAtIndex(int barIndex)
{
    if(barIndex > _song->BarCount()) barIndex = _song->BarCount();
    
    REFLOW_OPERATION_BEGIN("Create Bar")
    {
        RESong* song = lock_.LockSong();
        
        // Add a Bar
        REBar* bar = new REBar;
        song->InsertBar(bar, barIndex);
        song->TempoTimeline().InsertBarAtIndex(barIndex);
        
        // Update Time and key signature
        REBar* previousBar = bar->PreviousBar();
        if(previousBar) {
            bar->SetTimeSignature(previousBar->TimeSignature());
            bar->SetKeySignature(previousBar->KeySignature());
            bar->SetBeamingPattern(previousBar->BeamingPattern());
        }
        
        // Now for each track
        for(unsigned int i=0; i<song->TrackCount(); ++i)
        {
            RETrack* track = song->Track(i);
            track->ClefTimeline(false).InsertBarAtIndex(barIndex);
            track->ClefTimeline(true).InsertBarAtIndex(barIndex);
            
            for(unsigned int voiceIndex=0; voiceIndex<track->VoiceCount(); ++voiceIndex)
            {
                REVoice* voice = track->Voice(voiceIndex);
                REPhrase* phrase = new REPhrase;
                voice->InsertPhrase(phrase, barIndex);
                phrase->Refresh();
            }
        }
    }
    REFLOW_OPERATION_END
}

void RESongController::DeleteBarAtIndex(int barIndex)
{
    DeleteBarsInRange(barIndex, barIndex);
}

void RESongController::DeleteBarsInRange(int firstBarIndex, int lastBarIndex)
{
    REFLOW_OPERATION_BEGIN("Delete Bars")
    {
        RESong* song = lock_.LockSong();        
        int barCount = (lastBarIndex - firstBarIndex + 1);
        
        // Remove the Bar
        song->RemoveBarsInRange(firstBarIndex, barCount);
        song->TempoTimeline().RemoveItemsInBarRange(firstBarIndex, lastBarIndex);
        song->TempoTimeline().RemoveBarsInRange(firstBarIndex, barCount);
        
        // Now for each track
        for(unsigned int i=0; i<song->TrackCount(); ++i)
        {
            RETrack* track = song->Track(i);
            track->ClefTimeline(false).RemoveItemsInBarRange(firstBarIndex, lastBarIndex);
            track->ClefTimeline(false).RemoveBarsInRange(firstBarIndex, barCount);
            track->ClefTimeline(true).RemoveItemsInBarRange(firstBarIndex, lastBarIndex);
            track->ClefTimeline(true).RemoveBarsInRange(firstBarIndex, barCount);
            
            for(unsigned int voiceIndex=0; voiceIndex<track->VoiceCount(); ++voiceIndex)
            {
                REVoice* voice = track->Voice(voiceIndex);
                voice->RemovePhrasesInRange(firstBarIndex, barCount);
            }
        }
        
    }
    REFLOW_OPERATION_END;
}

void RESongController::DuplicateBarAtIndex(int barIndex)
{
    DuplicateBarsInRange(barIndex, barIndex);
}

void RESongController::DuplicateBarsInRange(int firstBarIndex, int lastBarIndex)
{
    REFLOW_OPERATION_BEGIN("Duplicate Bars")
    {
        RESong* song = lock_.LockSong();
        
        int barCount = (lastBarIndex - firstBarIndex + 1);
        for(int barIndex = firstBarIndex; barIndex <= lastBarIndex; ++barIndex)
        {
            int clonedBarDestIndex = barIndex + barCount;
            
            // Duplicate Bar
            REBar* clonedBar = song->Bar(barIndex)->Clone();
            song->InsertBar(clonedBar, clonedBarDestIndex);
            song->TempoTimeline().InsertBarAtIndex(clonedBarDestIndex);
            
            // For each Track
            for(unsigned int i=0; i<song->TrackCount(); ++i)
            {
                RETrack* track = song->Track(i);
                track->ClefTimeline(false).InsertBarAtIndex(clonedBarDestIndex);
                track->ClefTimeline(true).InsertBarAtIndex(clonedBarDestIndex);
                
                for(unsigned int voiceIndex=0; voiceIndex<track->VoiceCount(); ++voiceIndex)
                {
                    REVoice* voice = track->Voice(voiceIndex);
                    REPhrase* clonedPhrase = voice->Phrase(barIndex)->Clone();
                    voice->InsertPhrase(clonedPhrase, clonedBarDestIndex);
                    clonedPhrase->Refresh();
                }
            }
        }
    }
    REFLOW_OPERATION_END
}

void RESongController::DuplicateBar(int barIndex, int toIndex)
{
    REFLOW_OPERATION_BEGIN("Duplicate Bar")
    {
        RESong* song = lock_.LockSong();
        
        int clonedBarDestIndex = toIndex;
        
        // Duplicate Bar
        REBar* clonedBar = song->Bar(barIndex)->Clone();
        song->InsertBar(clonedBar, clonedBarDestIndex);
        song->TempoTimeline().InsertBarAtIndex(clonedBarDestIndex);
        
        // For each Track
        for(unsigned int i=0; i<song->TrackCount(); ++i)
        {
            RETrack* track = song->Track(i);
            track->ClefTimeline(false).InsertBarAtIndex(clonedBarDestIndex);
            track->ClefTimeline(true).InsertBarAtIndex(clonedBarDestIndex);
            
            for(unsigned int voiceIndex=0; voiceIndex<track->VoiceCount(); ++voiceIndex)
            {
                REVoice* voice = track->Voice(voiceIndex);
                REPhrase* clonedPhrase = voice->Phrase(barIndex)->Clone();
                voice->InsertPhrase(clonedPhrase, clonedBarDestIndex);
                clonedPhrase->Refresh();
            }
        }
    }
    REFLOW_OPERATION_END;
}

void RESongController::DuplicateBars(const RERange& range, int toIndex)
{
    REFLOW_OPERATION_BEGIN("Duplicate Bars")
    {
        int count = range.count;
        bool inc = (toIndex <= range.index);
        int firstBarIndex = range.index;
        for(int i=0; i<count; ++i)
        {
            int barIndex = firstBarIndex + i;
            int destIndex = toIndex + i;
            
            DuplicateBar(barIndex, destIndex);
            
            if(inc) ++firstBarIndex;
        }
    }
    REFLOW_OPERATION_END;
}

void RESongController::MoveBars(const RERange& range, int toIndex)
{
    REFLOW_OPERATION_BEGIN("Move Bars")
    {
        DuplicateBars(range, toIndex);
        
        RERange rangeToDelete = range;
        if(toIndex < range.index) {
            rangeToDelete.index += range.count;
        }
        DeleteBarsInRange(rangeToDelete.FirstIndex(), rangeToDelete.LastIndex());
    }
    REFLOW_OPERATION_END;
}

void RESongController::SetTimeSignatureOfAllBars(const RETimeSignature& ts)
{
    REFLOW_OPERATION_BEGIN("Change Time Signature")
    {
        RESong* song = lock_.LockSong();
        std::for_each(song->Bars().begin(), song->Bars().end(), std::bind(&REBar::SetTimeSignature, std::placeholders::_1, ts));
    }
    REFLOW_OPERATION_END;
}
void RESongController::SetTimeSignatureOfBarRange(int firstBar, int lastBar, const RETimeSignature& ts)
{
    REFLOW_OPERATION_BEGIN("Change Time Signature")
    {
        RESong* song = lock_.LockSong();
        for(int i=firstBar; i<=lastBar; ++i) {
            REBar* bar = song->Bar(i);
            if(bar) bar->SetTimeSignature(ts);
        }
    }
    REFLOW_OPERATION_END;
}
void RESongController::SetTimeSignatureAtBarIndex(int barIndex, const RETimeSignature& ts)
{
    REFLOW_OPERATION_BEGIN("Change Time Signature")
    {
        RESong* song = lock_.LockSong();
        
        RETimeSignature originalTimeSignature = song->Bar(barIndex)->TimeSignature();
        while(barIndex < song->BarCount() && song->Bar(barIndex)->TimeSignature() == originalTimeSignature)
        {
            song->Bar(barIndex)->SetTimeSignature(ts);
            ++barIndex;
        }
    }
    REFLOW_OPERATION_END;
}

void RESongController::SetBeamingPatternOfBarRange(int firstBar, int lastBar, const REBeamingPattern& pattern)
{
    REFLOW_OPERATION_BEGIN("Change Beaming Pattern")
    {
        RESong* song = lock_.LockSong();
        for(int i=firstBar; i<=lastBar; ++i) {
            REBar* bar = song->Bar(i);
            if(bar) bar->SetBeamingPattern(pattern);
        }
    }
    REFLOW_OPERATION_END;
}
void RESongController::SetBeamingPatternAtBarIndex(int barIndex, const REBeamingPattern& pattern)
{
    REFLOW_OPERATION_BEGIN("Change Beaming Pattern")
    {
        RESong* song = lock_.LockSong();
        
        RETimeSignature originalTimeSignature = song->Bar(barIndex)->TimeSignature();
        while(barIndex < song->BarCount() && song->Bar(barIndex)->TimeSignature() == originalTimeSignature)
        {
            song->Bar(barIndex)->SetBeamingPattern(pattern);
            ++barIndex;
        }
    }
    REFLOW_OPERATION_END;
}


void RESongController::SetKeySignatureOfAllBars(const REKeySignature& ks)
{
    REFLOW_OPERATION_BEGIN("Change Key Signature")
    {
        RESong* song = lock_.LockSong();
        std::for_each(song->Bars().begin(), song->Bars().end(), std::bind(&REBar::SetKeySignature, std::placeholders::_1, ks));
    }
    REFLOW_OPERATION_END;
}
void RESongController::SetKeySignatureOfBarRange(int firstBar, int lastBar, const REKeySignature& ks)
{
    REFLOW_OPERATION_BEGIN("Change Key Signature")
    {
        RESong* song = lock_.LockSong();
        for(int i=firstBar; i<=lastBar; ++i) {
            REBar* bar = song->Bar(i);
            if(bar) bar->SetKeySignature(ks);
        }
    }
    REFLOW_OPERATION_END;
}
void RESongController::SetKeySignatureAtBarIndex(int barIndex, const REKeySignature& ks)
{
    REFLOW_OPERATION_BEGIN("Change Key Signature")
    {
        RESong* song = lock_.LockSong();
        
        REKeySignature originalKeySignature = song->Bar(barIndex)->KeySignature();
        while(barIndex < song->BarCount() && song->Bar(barIndex)->KeySignature() == originalKeySignature)
        {
            song->Bar(barIndex)->SetKeySignature(ks);
            ++barIndex;
        }
    }
    REFLOW_OPERATION_END;
}

void RESongController::SetTrackClefOfBarRange(int trackIndex, int firstBar, int lastBar, Reflow::ClefType clef, Reflow::OttaviaType ottavia, bool leftHand)
{
    const RETrack* constTrack = _song->Track(trackIndex);
    if(constTrack == NULL) return;
    
    REFLOW_OPERATION_BEGIN("Change Clef")
    {
        RETrack* track = lock_.LockTrack(constTrack);
        REClefTimeline& clefTimeline = track->ClefTimeline(leftHand);
        
        const REClefItem* clefItemAfterPtr = clefTimeline.ItemAt(lastBar+1, RETimeDiv(0));
        REClefItem clefItemAfter; 
        clefItemAfter.clef = (clefItemAfterPtr == NULL ? Reflow::TrebleClef : clefItemAfterPtr->clef);
        clefItemAfter.ottavia = (clefItemAfterPtr == NULL ? Reflow::NoOttavia : clefItemAfterPtr->ottavia);
        clefItemAfter.bar = lastBar+1;
        clefItemAfter.beat = RETimeDiv(0);
        
        // Clean items
        clefTimeline.RemoveItemsInBarRange(firstBar, lastBar);
        
        // Insert first clef item
        REClefItem newClefItem(firstBar, RETimeDiv(0), clef, ottavia);
        clefTimeline.InsertItem(newClefItem);
        
        // Insert last clef item
        int barToInsert = lastBar+1;
        if(barToInsert < _song->BarCount()) {
            clefTimeline.InsertItem(clefItemAfter);
        }
    }
    REFLOW_OPERATION_END;
}

void RESongController::SetTrackClefAtBarIndex(int trackIndex, int barIndex, Reflow::ClefType clef, Reflow::OttaviaType ottavia, bool leftHand)
{
    const RETrack* constTrack = _song->Track(trackIndex);
    if(constTrack == NULL) return;
    
    REFLOW_OPERATION_BEGIN("Change Clef")
    {
        RETrack* track = lock_.LockTrack(constTrack);
        REClefTimeline& clefTimeline = track->ClefTimeline(leftHand);
        
        // Insert clef item
        REClefItem newClefItem(barIndex, RETimeDiv(0), clef, ottavia);
        clefTimeline.InsertItem(newClefItem);
    }
    REFLOW_OPERATION_END;
}

void RESongController::SetNameOfTrack(int trackIndex, const std::string& name)
{
    const RETrack* track = _song->Track(trackIndex);
    if(track == NULL) return;
    
    REFLOW_OPERATION_BEGIN("Rename Track")
    {
        RETrack* lockedTrack = lock_.LockTrack(track);
        lockedTrack->SetName(name);
    }
    REFLOW_OPERATION_END;
}

void RESongController::SetShortNameOfTrack(int trackIndex, const std::string& name)
{
    const RETrack* track = _song->Track(trackIndex);
    if(track == NULL) return;
    
    REFLOW_OPERATION_BEGIN("Change Track Short Name")
    {
        RETrack* lockedTrack = lock_.LockTrack(track);
        lockedTrack->SetShortName(name);
    }    
    REFLOW_OPERATION_END;
}

void RESongController::CreateTrack(const RECreateTrackOptions& opts)
{
    REFLOW_OPERATION_BEGIN("Create Track")
    {
        RESong* song = lock_.LockSong();
        
        // Create and Insert Track
        unsigned int trackIndex = _song->TrackCount();
        {
            RETrack* track = new RETrack(opts.type);
            track->SetName(opts.name);
            track->SetShortName(opts.shortName);
            track->SetTablatureInstrumentType(opts.subType);
            
            // Grand Staff
            if(track->IsStandard() && opts.useGrandStaff)
            {
                track->SetFlag(RETrack::GrandStaff);
            }
            else {
                track->UnsetFlag(RETrack::GrandStaff);
            }
            
            // Banjo drone string
            if(opts.subType == Reflow::BanjoInstrument && opts.droneString) {
                track->SetFlag(RETrack::BanjoDroneString);
            }
            else {
                track->UnsetFlag(RETrack::BanjoDroneString);
            }
            
            // Tuning
            if(track->IsTablature() && !opts.tuning.empty()) {
                const int* tuning = &opts.tuning[0];
                track->SetTuning(tuning, opts.tuning.size());
            }
            if(track->IsTablature()) {
                track->SetCapo(opts.capo);
            }
            
            // Create Phrases
            for(unsigned int i=0; i<REFLOW_MAX_VOICES; ++i) {
                REVoice* voice = new REVoice;
                track->InsertVoice(voice, i);
                for(unsigned int j=0; j<_song->BarCount(); ++j) {
                    voice->InsertPhrase(new REPhrase, j);
                }
            }
            
            track->ClefTimeline(false).InsertItem(REClefItem(0, RETimeDiv(0), opts.firstClef, opts.firstOttavia));
            track->ClefTimeline(true).InsertItem(REClefItem(0, RETimeDiv(0), opts.secondClef, opts.secondOttavia));
            
            // Midi program
            track->SetMIDIProgram(opts.midiProgram);
            //TODO: SetMIDIBank
            
            song->InsertTrack(track, trackIndex);
        }
        
        // Add To full score
        REScoreSettings* fullScore = song->Score(0);
        RETrackSet ts (fullScore->TrackSet());
        if(opts.addToFullScore) {
            ts.Set(trackIndex);
        }
        else {
            ts.Unset(trackIndex);
        }
        fullScore->SetTrackSet(ts);
        
        // Create Part for this track
        if(opts.createPart)
        {
            int scoreIdx = song->ScoreCount();
            REScoreSettings* part = song->CreatePart(trackIndex);
        }
    }
    REFLOW_OPERATION_END;
}

void RESongController::RemoveTrack(int trackIndex)
{
    const RETrack* track = _song->Track(trackIndex);
    if(track == NULL) return;
    
    REFLOW_OPERATION_BEGIN("Remove Track")
    {
        RESong* song = lock_.LockSong();
        
        // Delete the track and remove from the song
        song->RemoveTrack(trackIndex);
        
        // Remove this track for every score that contains it
        // Beware: track is a dangling pointer here, do not dereference it!
        for(unsigned int i=0; i<_song->ScoreCount(); ++i) 
        {
            REScoreSettings* score = song->Score(i);
            if(!score->TrackSet().IsSet(trackIndex)) continue;
            
            RETrackVector tracks = score->Tracks(song);
            tracks.erase(std::find(tracks.begin(), tracks.end(), track), tracks.end());
            score->SetTracks(tracks);
        }
    }
    REFLOW_OPERATION_END;
}

void RESongController::MoveTrack(int fromIndex, int toIndex)
{
    const RETrack* track = _song->Track(fromIndex);
    if(track == NULL) return;
    
    REFLOW_OPERATION_BEGIN("Move Track")
    {
        RESong* song = lock_.LockSong();
        
        // Backup old score config
        std::vector<RETrackVector> tracksPerScore;
        for(unsigned int i=0; i<_song->ScoreCount(); ++i)
        {
            REScoreSettings* score = song->Score(i);
            RETrackVector tracks = score->Tracks(song);
            tracksPerScore.push_back(tracks);
        }
        
        // Delete the track and remove from the song
        song->MoveTrack(fromIndex, toIndex);
        
        // Update Scores because the track indices have changed
        for(unsigned int i=0; i<_song->ScoreCount(); ++i) 
        {
            REScoreSettings* score = song->Score(i);
            const RETrackVector& tracks = tracksPerScore[i];
            score->SetTracks(tracks);
        }
    }
    REFLOW_OPERATION_END;
}

void RESongController::DuplicateTrack(int fromIndex, int toIndex)
{
    const RETrack* track = _song->Track(fromIndex);
    if(track == NULL) return;
    
    REFLOW_OPERATION_BEGIN("Duplicate Track")
    {
        RESong* song = lock_.LockSong();
        
        // Delete the track and remove from the song
        song->DuplicateTrack(fromIndex, toIndex);
        
        // Update Scores because the track indices have changed
        for(unsigned int i=0; i<_song->ScoreCount(); ++i) 
        {
            REScoreSettings* score = song->Score(i);
            RETrackVector tracks = score->Tracks(song);
            score->SetTracks(tracks);
        }
    }
    REFLOW_OPERATION_END;
}

void RESongController::SetTuningOfTrack(int trackIndex, const int* tuningArray, int stringCount)
{
    REFLOW_OPERATION_BEGIN("Change Tuning")
    {
        RETrack* track = lock_.LockTrack(_song->Track(trackIndex));
        track->SetTuning(tuningArray, stringCount);
    }
    REFLOW_OPERATION_END;
}

void RESongController::SetRepeatStartOnBarAtIndex(int barIndex, bool repeat)
{
    const REBar* bar = _song->Bar(barIndex);
    if(bar == NULL) return;
    
    REFLOW_OPERATION_BEGIN(repeat ? "Set Repeat Start" : "Unset Repeat Start")
    {
        if(repeat) lock_.LockBar(bar)->SetFlag(REBar::RepeatStart);
        else        lock_.LockBar(bar)->UnsetFlag(REBar::RepeatStart);
    }
    REFLOW_OPERATION_END;
}
void RESongController::SetRepeatEndOnBarAtIndex(int barIndex, bool repeat)
{
    const REBar* bar = _song->Bar(barIndex);
    if(bar == NULL) return;
    
    REFLOW_OPERATION_BEGIN(repeat ? "Set Repeat End" : "Unset Repeat End")
    {
        if(repeat) lock_.LockBar(bar)->SetFlag(REBar::RepeatEnd);
        else        lock_.LockBar(bar)->UnsetFlag(REBar::RepeatEnd);
    }
    REFLOW_OPERATION_END;    
}
void RESongController::SetRepeatCountOnBarAtIndex(int barIndex, int repeatCount)
{
    const REBar* bar = _song->Bar(barIndex);
    if(bar == NULL) return;
    
    REFLOW_OPERATION_BEGIN("Set Repeat Count") {
        lock_.LockBar(bar)->SetRepeatCount(repeatCount);
    }
    REFLOW_OPERATION_END;
}

void RESongController::MoveScore(int fromIndex, int toIndex)
{
    REFLOW_OPERATION_BEGIN("Move Score") {
        _song->MoveScore(fromIndex, toIndex);
    }
    REFLOW_OPERATION_END;

    // Refresh score controllers pointers
    for(REScoreController* scoreController : _scoreControllers) {
        scoreController->SetScoreIndex(scoreController->ScoreIndex());
    }
}

void RESongController::DuplicateScore(int fromIndex, int toIndex)
{
    REFLOW_OPERATION_BEGIN("Duplicate Score") {
        _song->DuplicateScore(fromIndex, toIndex);
    }
    REFLOW_OPERATION_END;    
    
    // Refresh score controllers pointers
    for(REScoreController* scoreController : _scoreControllers) {
        scoreController->SetScoreIndex(scoreController->ScoreIndex());
    }
}

void RESongController::RemoveScore(int scoreIndex)
{
    REFLOW_OPERATION_BEGIN("Remove Score")
    {
        RESong* song = lock_.LockSong();
        
        song->RemoveScore(scoreIndex);
        
        // Refresh score controllers pointers
        for(REScoreController* scoreController : _scoreControllers) {
            scoreController->SetScoreIndex(scoreController->ScoreIndex());
        }
    }
    REFLOW_OPERATION_END;
}

void RESongController::SetNameOfScore(int scoreIndex, const std::string& scoreName)
{
    const REScoreSettings* score = _song->Score(scoreIndex);
    if(score == NULL) return;
    
    REFLOW_OPERATION_BEGIN("Rename Score")
    {
        REScoreSettings* lockedScore = lock_.LockScore(score);
        lockedScore->SetName(scoreName);
    }
    REFLOW_OPERATION_END;
}

void RESongController::CreateEmptyScoreAtEnd()
{
    REFLOW_OPERATION_BEGIN("Create Score")
    {
        RESong* song = lock_.LockSong();
        
        song->CreateEmptyScoreAtEnd();
    }
    REFLOW_OPERATION_END;
    
    // Refresh score controllers pointers
    for(REScoreController* scoreController : _scoreControllers) {
        scoreController->SetScoreIndex(scoreController->ScoreIndex());
    }
}


void RESongController::SetScoreFlag(int scoreIndex, REScoreSettings::ScoreFlag flag, bool set)
{
    const REScoreSettings* score = _song->Score(scoreIndex);
    if(score == NULL) return;
    
    REFLOW_OPERATION_BEGIN("Change Score Settings")
    {
        REScoreSettings* lockedScore = lock_.LockScore(score);
        if(set) lockedScore->SetFlag(flag); else lockedScore->UnsetFlag(flag);
    }
    REFLOW_OPERATION_END;
}

void RESongController::SetTrackInScore(int scoreIndex, int trackIndex, bool set)
{
    const REScoreSettings* score = _song->Score(scoreIndex);
    if(score == NULL) return;
    
    REFLOW_OPERATION_BEGIN("Set Staff in Score")
    {
        REScoreSettings* lockedScore = lock_.LockScore(score);
        
        RETrackSet trackSet (score->TrackSet());
        
        if(set) trackSet.Set(trackIndex);
        else trackSet.Unset(trackIndex);
        
        lockedScore->SetTrackSet(trackSet);
    }
    REFLOW_OPERATION_END;
}

void RESongController::SetScoreSettings(int scoreIndex, const REScoreSettings& settings)
{
    const REScoreSettings* score = _song->Score(scoreIndex);
    if(score == NULL) return;
    
    REFLOW_OPERATION_BEGIN("Set Score Settings")
    {
        REScoreSettings* lockedScore = lock_.LockScore(score);
        *lockedScore = settings;
    }
    REFLOW_OPERATION_END;
}


void RESongController::PerformOperationsOnSong(const RESongOperationVector& operations)
{
    REFLOW_OPERATION_BEGIN("Modify Song")
    {
        RESong* song = lock_.LockSong();
        
        for(RESongOperation op : operations) {
            op(song);
        }
    }
    REFLOW_OPERATION_END;
}

void RESongController::UnselectAllNotes(REIntSet* affectedBars)
{
    _song->UnselectAllNotes(affectedBars);
}

void RESongController::SelectNotes(const RENoteSet& notes)
{
    RENoteSet::const_iterator it = notes.begin();
    for(; it != notes.end(); ++it) {
        RENote* note = *it;
        note->SetSelected(true);
    }
}
