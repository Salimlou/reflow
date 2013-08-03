//
//  REScoreController.h
//  ReflowTools
//
//  Created by Sebastien Bourgeois on 13/02/12.
//  Copyright (c) 2012 Gargant Studios. All rights reserved.
//

#ifndef ReflowTools_REScoreController_h
#define ReflowTools_REScoreController_h

#include "RETypes.h"
#include "RESongController.h"
#include "RECursor.h"
#include "REScore.h"
#include "RETool.h"

/** REPasteboard class.
 */
class REPasteboard
{
public:
};


class RESongPasteboard : public REPasteboard
{
public:
};


class REPhrasePasteboard : public REPasteboard
{
public:
};


/** REScoreControllerAction class.
 */
class REScoreControllerAction
{
public:
    REScoreControllerAction(const std::string& identifier) : _identifier(identifier), _enabled(false), _checked(false)
    {}
    
    void UpdateUI(const REScoreController* scoreController);
    
private:
    std::string _identifier;
    bool _enabled;
    bool _checked;
};
typedef std::map<std::string, REScoreControllerAction*> REScoreControllerActionMap;



/** REScoreControllerDelegate protocol.
 */
class REScoreControllerDelegate
{
public:
    virtual ~REScoreControllerDelegate() {}
    
    virtual void ScoreControllerSelectionDidChange(const REScoreController* scoreController) = 0;
    virtual void ScoreControllerScoreDidChange(const REScoreController* scoreController, const REScore* score) = 0;
    virtual void ScoreControllerRefreshPresentation(const REScoreController* scoreController, const REScore* score) = 0;
    
    virtual void OnCopyPhraseToPasteboard(const REPhrase* phrase, Reflow::TrackType trackType){}
    virtual void OnCopyPartialSongToPasteboard(const RESong* song){}
    virtual void OnShouldCenterOnCursor(const REScoreController* scoreController) {};

    virtual void ScoreControllerInspectorWillReload(const REScoreController* scoreController) = 0;
    virtual void ScoreControllerInspectorDidReload(const REScoreController* scoreController) = 0;
};


/** REScoreController class.
 */
class REScoreController : public RESongControllerDelegate
{
    friend class RESongController;
    friend class REViewport;
    friend class RETool;
    friend class RESelectTool;
    friend class RETablatureTool;
    
public:
    enum SelectionKindType
    {
        CursorSelection                     = 0,
        RectangleCursorSelection            = 1,
        TickRangeSelection            = 2,
        BarRangeSelection                   = 3,
        SingleTrackBarRangeSelection        = 4
    };
    
    enum CursorMoveFlags {
        CursorShiftDown = 0x01,     // Extend selection
        CursorCmdDown   = 0x02,     // Bar wise move
        CursorAltDown   = 0x04      // Square selection
    };
    
public:
    REScoreController(RESongController* songController);
    virtual ~REScoreController();
    
public:    
    void CreateDefaultViewport();
    
public:
    void TypeKeypadNumber(int num, bool alt=false, bool dead=false);
    void TypeOnVisualFretboard(int string, int fret);
    void TypeDeadNote();
    void TypeGraceNote();
    void InsertNotesWithPitches(const REIntVector& pitches);
    
    void Cut(bool allTracks);
    void Copy(bool allTracks);
    void PastePartialSong(const RESong* song, bool pasteOver=false, bool includeBarInfo=true);
    void PastePhrase(const REPhrase* phrase, Reflow::TrackType trackType);
    void PasteEncodedPartialSong(std::string encodedPartialSong);
    void PasteEncodedPhrase(std::string encodedPhrase);
        
    void MoveSingleTrackBarRangeSelectionTo(int insertBarIndex, int trackIndex, bool pasteOver);
    void CopySingleTrackBarRangeSelectionTo(int insertBarIndex, int trackIndex, bool pasteOver);
    
    void MoveBarRangeSelectionTo(int insertBarIndex, bool pasteOver);
    void CopyBarRangeSelectionTo(int insertBarIndex, bool pasteOver);
    
    RESong* CutBarRangeSelectionToMemory();
    RESong* CutSingleTrackBarRangeSelectionToMemory();
    
    void SetNoteValueOnSelection(Reflow::NoteValue noteValue);
    void IncreaseNoteValueOnSelection();
    void DecreaseNoteValueOnSelection();
    void DeleteSelection();
    void DeleteSelectedChords();
    void InsertChordBeforeCursor();
    void AddChordAfterCursor();
    void DuplicateSelectedChordsAtEnd();
    void AddBarAfterSelection();
    void InsertBarBeforeSelection();
    void DuplicateSelectedBars();
    void DeleteSelectedBars();
    void SplitSelectedChords();
    
    void SetTimeSignatureOfSelectedBars(const RETimeSignature& ts);
    void SetTimeSignatureAndBeamingPatternOfSelectedBars(const RETimeSignature& ts, const REBeamingPattern& pattern);
    void SetKeySignatureOfSelectedBars(const REKeySignature& ks);
    void SetClefOfSelectedBars(Reflow::ClefType clef, Reflow::OttaviaType ottavia);
    void SetRepeatStartOnSelectedBar(bool repeat);
    void SetRepeatEndOnSelectedBar(bool repeat);
    void SetRepeatCountOnSelectedBar(int repeatCount);
    void SetRehearsalSignOnSelectedBar(const std::string& text);
    void UnsetRehearsalSignOnSelectedBar();
    
    void SetLegatoOnSelection(bool legato);
    void SetPalmMuteOnSelection(bool pm);
    void SetLetRingOnSelection(bool pm);
    void SetVibratoOnSelection(bool set);
    void SetAccentOnSelection(bool set);
    void SetStrongAccentOnSelection(bool set);
    void SetStaccatoOnSelection(bool set);
    void SetForceStemUpOnSelection(bool set);
    void SetForceStemDownOnSelection(bool set);
    void SetTapOnSelection(bool set);
    void SetSlapOnSelection(bool set);
    void SetPopOnSelection(bool set);
    void SetArpeggioUpOnSelection(bool set);
    void SetArpeggioDownOnSelection(bool set);
    void SetBrushUpOnSelection(bool set);
    void SetBrushDownOnSelection(bool set);
    void SetPickstrokeUpOnSelection(bool set);
    void SetPickstrokeDownOnSelection(bool set);
    void SetSlideInOnSelection(Reflow::SlideInType slideIn);
    void SetSlideOutOnSelection(Reflow::SlideOutType slideOut);
    void SetAlterationOnSelection(int alteration);
    void SetTiedNoteOnSelection(bool set);
    void SetTupletOnSelection(int tuplet);
    void SetDottedNoteOnSelection(bool set);
    void SetDoubleDottedNoteOnSelection(bool set);
    void SetSystemBreakOnSelection(bool set);
    void SetDynamicsOnSelection(Reflow::DynamicsType dynamics);
    void SetBendOnSelection(REBend bend);
    
    void ToggleLegatoOnSelection();
    void TogglePalmMuteOnSelection();
    void ToggleLetRingOnSelection();
    void ToggleVibratoOnSelection();
    void ToggleAccentOnSelection();
    void ToggleStrongAccentOnSelection();
    void ToggleStaccatoOnSelection();
    void ToggleForceStemUpOnSelection();
    void ToggleForceStemDownOnSelection();
    void ToggleTapOnSelection();
    void ToggleSlapOnSelection();
    void TogglePopOnSelection();
    void ToggleArpeggioUpOnSelection();
    void ToggleArpeggioDownOnSelection();
    void ToggleBrushUpOnSelection();
    void ToggleBrushDownOnSelection();
    void TogglePickstrokeUpOnSelection();
    void TogglePickstrokeDownOnSelection();
    void ToggleSlideInOnSelection(Reflow::SlideInType slideIn);
    void ToggleSlideOutOnSelection(Reflow::SlideOutType slideOut);
    void ToggleAlterationOnSelection(int alteration);
    void ToggleTiedNoteOnSelection();
    void ToggleTupletOnSelection(int tuplet);
    void ToggleDottedNoteOnSelection();
    void ToggleDoubleDottedNoteOnSelection();
    void ToggleSystemBreakOnSelection();
    void ToggleBendOnSelection(REBend bend);
    void ToggleSlurOnSelection();
    
    void SetSlurStartBeat(int trackIndex, Reflow::StaffIdentifier staffId, int slurIndex, const REGlobalTimeDiv& div, const REPoint& offset);
    void SetSlurEndBeat(int trackIndex, Reflow::StaffIdentifier staffId, int slurIndex, const REGlobalTimeDiv& div, const REPoint& offset);
    void SetSlurStartOffset(int trackIndex, Reflow::StaffIdentifier staffId, int slurIndex, const REPoint& offset);
    void SetSlurEndOffset(int trackIndex, Reflow::StaffIdentifier staffId, int slurIndex, const REPoint& offset);
    void SetSlurStartControlPointOffset(int trackIndex, Reflow::StaffIdentifier staffId, int slurIndex, const REPoint& controlPointOffset);
    void SetSlurEndControlPointOffset(int trackIndex, Reflow::StaffIdentifier staffId, int slurIndex, const REPoint& controlPointOffset);
    
    void SetTextOnSelection(const std::string& text, Reflow::TextPositioning textPositioning);
    void RemoveTextOnSelection();
    
    void SetNoteFlagOnSelectedNotes(unsigned int flag);
    void UnsetNoteFlagOnSelectedNotes(unsigned int flag);
    void ToggleNoteFlagOnSelectedNotes(unsigned int flag);
    
    void PitchUpOnSelection();
    void PitchDownOnSelection();
    void ToggleEnharmonicOnSelection();    
    
    void PerformOperationsOnSelectedBars(const REBarOperationVector& operationsOnFirst, const REBarOperationVector& operationsOnLast, const std::string& batchName);
    
    void CreateTrack(const RECreateTrackOptions& opts);
    void RemoveTrack(int idx);
    void SetNameOfTrack(int trackIndex, const std::string& name);
    void SetShortNameOfTrack(int trackIndex, const std::string& name);
    void SetTuningOfTrack(int trackIndex, const int* tuningArray, int stringCount);

    void SetTrackPresentInCurrentScore(int trackIndex, bool set);

public:
    void MoveCursorRight(unsigned long flags);
    void MoveCursorLeft(unsigned long flags);
    void MoveCursorUp(unsigned long flags);
    void MoveCursorDown(unsigned long flags);
    void MoveCursorBy(int delta, unsigned long flags);
    void MoveCursorTo(int staffIndex, int barIndex, int tick, int lineIndex, unsigned long flags);
    void MoveCursorToBar(int barIndex);
    
    void SelectBarsInRange(const RERange& range);
    
public:
    void MouseDown(const REPoint& point, unsigned long flags);
    void MouseUp(const REPoint& point, unsigned long flags);
    void MouseDragged(const REPoint& point, unsigned long flags);
    void MouseMoved(const REPoint& point, unsigned long flags);
    void MouseDoubleClicked(const REPoint& point, unsigned long flags);
    void MouseWheel(const REPoint& point, const REPoint& delta, unsigned long flags);
    
public:
    void SetScoreIndex(int scoreIndex);
    void SetScoreIndex(int scoreIndex, Reflow::ScoreLayoutType, Reflow::PageLayoutType);
    void ResetScore();
    
    int ScoreIndex() const;
    const REScore* Score() const;
    REScore* Score();
    
    const RESongController* SongController() const {return _songController;}
    const RESequencer* Sequencer() const {return _songController->Sequencer();}
    
    const REScoreSettings* ScoreSettings() const;
    
    RESong* CopyBarRangeSelection() const;
    RESong* CopySingleTrackBarRangeSelection() const;
    
    const RECursor& Cursor() const {return _currentCursor;}
    const RECursor& OriginCursor() const {return _originCursor;}
    
    REGlobalTimeDiv FirstSelectedBeat() const;
    REGlobalTimeDiv LastSelectedBeat() const;
    
    const REChord* FirstSelectedChord() const;
    const REChord* LastSelectedChord() const;
    REConstChordPair SelectedChordRange() const;
    
    const REPhrase* FirstSelectedPhrase() const;
    const REPhrase* LastSelectedPhrase() const;
    
    const REBar* FirstSelectedBar() const;
    const REBar* LastSelectedBar() const;
    
    const REStaff* FirstSelectedStaff() const;
    const RETrack* FirstSelectedTrack() const;
    
    int FirstSelectedLine() const;
    int LastSelectedLine() const;
    
    int FirstSelectedBarIndex() const;
    int LastSelectedBarIndex() const;
    
    void SelectBarRange(const RERange& barRange);
    
    bool IsTypingSecondDigit() const {return _typingSecondDigit;}
    bool ExtendedSelection() const;
    SelectionKindType InferredSelectionKind() const;
    void SetPreferredSelectionKind(SelectionKindType kind);
    
    void SetEditLowVoice(bool editLowVoice);
    bool IsEditingLowVoice() const;
    
    void SetDelegate(REScoreControllerDelegate* delegate) {_delegate = delegate;}
	REScoreControllerDelegate* Delegate() {return _delegate;}

    void UpdateLayoutVisibleItems();
    
    int SelectedNoteCount() const;
    void FindSelectedNotes(REConstNoteVector* notes) const;
    void FindSelectedChords(REConstChordVector* chords) const;
    
    REConstTrackVector SelectedTracks() const;
    
    bool AtLeastOneSelectedNoteVerifies(RENotePredicate pred) const;
    bool AtLeastOneSelectedChordVerifies(REChordPredicate pred) const;
    bool AllTheSelectedChordsVerify(REChordPredicate pred) const;
    
    void PerformTaskOnSelectedNotes(RENoteOperation op, const std::string& taskName, unsigned long flags=0);
    void PerformTaskOnSelectedChords(REChordOperation op, const std::string& taskName, unsigned long flags=0);
    
    void PerformTaskOnSelectedGraceNote(RENoteOperation op, const std::string& taskName, unsigned long flags=0);
    
    void PerformTaskOnSongController(RESongControllerOperation op);
    void PerformTaskOnSong(RESongOperation op);
    void PerformTasksOnSong(const RESongOperationVector& ops);
    
    const REScoreControllerAction* Action(const std::string& identifier) const;
    
    void SetViewport(REViewport* viewport) {_viewport = viewport;}
    const REViewport* Viewport() const {return _viewport;}
    REViewport* Viewport() {return _viewport;}
    
    Reflow::ScoreLayoutType LayoutType() const {return _layoutType;}
    Reflow::PageLayoutType PageLayoutType() const {return _pageLayoutType;}
    
    void SetTool(Reflow::ToolType);
    Reflow::ToolType CurrentToolType() const;
    RETool* CurrentTool();
    inline const REToolVector& Tools() const {return _tools;}
    inline REToolVector& Tools() {return _tools;}
    
    RETable* InspectorTable() {return _inspector;}
    void RefreshInspectorTable();
    
    RERect SelectionRect() const;
    const REPoint& SelectionStartPoint() const;
    const REPoint& SelectionEndPoint() const;
    bool SelectionRectIsVisible() const;
    
    bool IsEditingGraceNote() const {return _editingGraceNote;}
    int SelectedGraceNoteIndex() const {return _graceNoteIndex;}
    
    inline RETablatureTool& TablatureTool() {return *static_cast<RETablatureTool*>(_tools[Reflow::TablatureTool]);}
    inline RESelectTool& SelectTool() {return *static_cast<RESelectTool*>(_tools[Reflow::SelectTool]);}
    inline RESlurTool& SlurTool() {return *static_cast<RESlurTool*>(_tools[Reflow::SlurTool]);}
    
public:
	void _BackupCommandDataToStream(REOutputStream& stream);
	void _RestoreCommandDataFromStream(REInputStream& stream);
    
    void EncodeTo(REOutputStream& coder) const;
	void DecodeFrom(REInputStream& decoder);
    
protected:
    void _InferSelectionTypeWithCursor();
    REScoreController::SelectionKindType _StartOrContinueExtendedSelection(unsigned long flags);
    
    void _MoveCursorLeft();
    void _MoveCursorRight();
    
    void _PasteAllTracksPartialSongTo(const RESong* song, int barInsertIndex, bool pasteOver=false, bool includeBarInfo=true);
    void _PasteSingleTrackPartialSongTo(const RESong* song, int barInsertIndex, int trackIndex, bool pasteOver=false, bool includeBarInfo=true);
    
    void UpdateActions();
    void ClearViewport();
    void RebuildViewport();
	void RefreshViewportItemAtBarIndex(int barIndex);
    
protected:
    virtual void SongControllerWillModifySong(const RESongController* controller, const RESong* song);
    virtual void SongControllerDidModifySong(const RESongController* controller, const RESong* song, bool successfully);
    virtual void SongControllerDidModifyPhrase(const RESongController* controller, const REPhrase* phrase, bool successfully);

private:
    RESongController* _songController;
    REScoreControllerDelegate* _delegate;
    Reflow::ScoreLayoutType _layoutType;
    Reflow::PageLayoutType _pageLayoutType;
    Reflow::ToolType _tool;
    REToolVector _tools;
    REViewport* _viewport;
    RETable* _inspector;
    int _scoreIndex;
    REScore _score;
    REScoreControllerActionMap _actions;
    RECursor _originCursor;
    RECursor _currentCursor;
    SelectionKindType _preferredSelection;
    SelectionKindType _inferredSelection;
    bool _typingSecondDigit;
    bool _editingGraceNote;
    int _graceNoteIndex;
    
    // Select (Arrow) Tool
    REPoint _selectionStartPoint;
    REPoint _selectionEndPoint;
    bool _selectionRectVisible;
};



#endif
