//
//  RESongController.h
//  ReflowTools
//
//  Created by Sebastien Bourgeois on 13/02/12.
//  Copyright (c) 2012 Gargant Studios. All rights reserved.
//

#ifndef _RESongController_h
#define _RESongController_h

#include "RETypes.h"
#include "REScore.h"

#include <mutex>

/** RECreateTrackOptions
 */
struct RECreateTrackOptions
{
public: 
    RECreateTrackOptions();
    RECreateTrackOptions(const RECreateTrackOptions& opts) {*this = opts;}
    
    RECreateTrackOptions& operator=(const RECreateTrackOptions& opts);
    
    Reflow::TrackType type;
    Reflow::TablatureInstrumentType subType;
    
    std::string name;
    std::string shortName;
    
    bool createPart;
    bool addToFullScore;
    bool useGrandStaff;
    bool droneString;
    
    Reflow::ClefType firstClef;
    Reflow::ClefType secondClef;
    Reflow::OttaviaType firstOttavia;
    Reflow::OttaviaType secondOttavia;

    int midiProgram;
    int midiBank;
    
    std::vector<int> tuning;
    int capo;
};
typedef std::vector<RECreateTrackOptions> RECreateTrackOptionsVector;


/** RESongControllerDelegate interface.
 */
class RESongControllerDelegate
{
public:
    virtual void SongControllerWillModifySong(const RESongController* controller, const RESong* song) = 0;
    virtual void SongControllerDidModifySong(const RESongController* controller, const RESong* song, bool successfully) = 0;
	virtual void SongControllerDidModifyPhrase(const RESongController* controller, const REPhrase* phrase, bool successfully) = 0;
    virtual void SongControllerWantsToBackup(const RESongController* controller, const RESong* song) {};
};

typedef std::vector<RESongControllerDelegate*> RESongControllerDelegateVector;



/** RESongControllerTask class.
 */
class RESongControllerTask
{
public:
    RESongControllerTask(const std::string& name, unsigned long flags)
    : _name(name), _flags(flags), _finished(false)
    {}
    
    void Finish() {_finished = true;}
    bool IsFinished() const {return _finished;}
    
protected:  
    std::string _name;
    unsigned long _flags;
    bool _finished;
};


typedef std::deque<RESongControllerTask*> RESongControllerTaskStack;



/** RESongController class.
 */
class RESongController : public RESequencerListener
{
    friend class RESongControllerTask;
    friend class RELockSongControllerForTask;
    friend class REScoreController;
    
public:
    typedef std::recursive_mutex MutexType;
    typedef std::lock_guard<MutexType> MutexLockType;
    
public:
    virtual void OnSequencerUpdateRT(const RESequencer* sequencer);
    
public:
    RESongController(RESong* song);
    virtual ~RESongController();
    
    bool IsEditable(unsigned long flags) const;
    int StackedTaskCount() const {return _tasks.size();}
    bool IsInBatch() const {return StackedTaskCount() > 0;}
    
    REScoreController* ActiveScoreController();
    void RemoveAllScoreControllers();
    
    void AddDelegate(RESongControllerDelegate* delegate);
    
    const RESong* Song() const {return _song;}
    const RESequencer* Sequencer() const {return _sequencer;}
    
    RESequencer* Sequencer() {return _sequencer;}
    
    MutexType& DataMutex() {return _dataMutex;}
    
public:
    void UnselectAllNotes(REIntSet* affectedBars=NULL);
    void SelectNotes(const RENoteSet& notes);
    
public:
    void CreateTrack(const RECreateTrackOptions& opts);
    void RemoveTrack(int trackIndex);
    void MoveTrack(int fromIndex, int toIndex);
    void DuplicateTrack(int fromIndex, int toIndex);
    void SetTuningOfTrack(int trackIndex, const int* tuningArray, int stringCount);
    void SetNameOfTrack(int trackIndex, const std::string& name);
    void SetShortNameOfTrack(int trackIndex, const std::string& name);
    
    void CreateEmptyBarAtIndex(int barIndex);
    void DeleteBarAtIndex(int barIndex);
    void DeleteBarsInRange(int firstBarIndex, int lastBarIndex);
    void DuplicateBarAtIndex(int barIndex);
    void DuplicateBarsInRange(int firstBarIndex, int lastBarIndex);
    void DuplicateBar(int barIndex, int toIndex);
    void DuplicateBars(const RERange& range, int toIndex);
    void MoveBars(const RERange& range, int toIndex);
    
    void SetTimeSignatureOfAllBars(const RETimeSignature& ts);
    void SetTimeSignatureOfBarRange(int firstBar, int lastBar, const RETimeSignature& ts);
    void SetTimeSignatureAtBarIndex(int barIndex, const RETimeSignature& ts);

    void SetBeamingPatternOfBarRange(int firstBar, int lastBar, const REBeamingPattern& pattern);
    void SetBeamingPatternAtBarIndex(int barIndex, const REBeamingPattern& pattern);
    
    void SetKeySignatureOfAllBars(const REKeySignature& ks);
    void SetKeySignatureOfBarRange(int firstBar, int lastBar, const REKeySignature& ks);
    void SetKeySignatureAtBarIndex(int barIndex, const REKeySignature& ks);

    void SetTrackClefOfBarRange(int trackIndex, int firstBar, int lastBar, Reflow::ClefType clef, Reflow::OttaviaType ottavia, bool leftHand);
    void SetTrackClefAtBarIndex(int trackIndex, int barIndex, Reflow::ClefType clef, Reflow::OttaviaType ottavia, bool leftHand);
    
    void SetRepeatStartOnBarAtIndex(int barIndex, bool repeat);
    void SetRepeatEndOnBarAtIndex(int barIndex, bool repeat);
    void SetRepeatCountOnBarAtIndex(int barIndex, int repeatCount);
    
    void SetRehearsalOnBarAtIndex(int barIndex, const std::string& rehearsal);
    void UnsetRehearsalOnBarAtIndex(int barIndex);

    void InsertChordName(const REChordName& chordName, int barIndex, int tick);
    void RemoveChordNamesOfBar(int barIndex);

    void InsertTrackChordDiagramAtBarIndex(const REChordDiagram& chordDiagram, int trackIndex, int barIndex, int tick);
    void RemoveTrackChordDiagramsAtBarIndex(int trackIndex, int barIndex);
    
    void InsertTempoMarker(int barIndex, const RETimeDiv& timeDiv, int tempo, Reflow::TempoUnitType tempoType);
    void DeleteTempoMarkersInBarRange(int firstBar, int lastBar);
    
    void MoveScore(int fromIndex, int toIndex);
    void DuplicateScore(int fromIndex, int toIndex);
    void RemoveScore(int scoreIndex);
    void CreateEmptyScoreAtEnd();
    void SetNameOfScore(int scoreIndex, const std::string& scoreName);
    
    void SetScoreFlag(int scoreIndex, REScoreSettings::ScoreFlag flag, bool set);
    void SetTrackInScore(int scoreIndex, int trackIndex, bool set);
    void SetScoreSettings(int scoreIndex, const REScoreSettings& settings);
    
    void PerformOperationsOnSong(const RESongOperationVector& operations);
    
	void BackupToStream(REOutputStream& stream);
    void RestoreFromStream(REInputStream& stream);
    
    void RestoreFromSongDataStream(REInputStream& stream);
    
public:
	void _BackupCommandDataToStream(REOutputStream& stream);
	void _RestoreCommandDataFromStream(REInputStream& stream);

protected:
    RESongControllerTask* PushTask(const std::string& taskName, unsigned long flags);
    RESongControllerTask* PopTask();
    void CommitTask(RESongControllerTask* task);
    void SongWillUpdate();
    void SongWasUpdated(bool success);
	void PhraseWasUpdated(REPhrase* phrase, bool success);
    
    void BackupSongStateToStream(REOutputStream& stream);
    void RestoreSongStateFromStream(REInputStream& stream);

    void DoSomethingWithReflowError(REException& err);
    void DoSomethingWithError(std::exception& err);
    void DoSomethingWithUnhandledError();
    
protected:
    RESong* _song;
    RESequencer* _sequencer;
    RESongControllerTaskStack _tasks;
    REScoreControllerVector _scoreControllers;
    RESongControllerDelegateVector _delegates;
	bool _updateSinglePhrase;
	REPhrase* _updatedPhrase;
    MutexType _dataMutex;
};




/** RELockSongControllerForTask class.
 */
class RELockSongControllerForTask
{
public:
    RELockSongControllerForTask(RESongController* controller, const std::string& name, unsigned long flags);
    ~RELockSongControllerForTask();
    
    void Commit();
    
    RESong* LockSong();
    REScoreSettings* LockScore(const REScoreSettings*);
    REBar* LockBar(const REBar*);
    RETrack* LockTrack(const RETrack*);
    REPhrase* LockPhrase(const REPhrase*);
    REChord* LockChord(const REChord*);
    RENote* LockNote(const RENote*);
    
private:
    RESongController* _controller;
    RESongControllerTask* _task;
};

#endif
