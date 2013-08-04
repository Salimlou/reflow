#ifndef REDOCUMENTVIEW_H
#define REDOCUMENTVIEW_H

#include <QWidget>
#include <QUndoStack>
#include <QTimer>

#include <RETypes.h>
#include <RESongController.h>
#include <REScoreController.h>
#include <RESequencer.h>

class REScoreScene;
class REScoreSceneView;
class REQtViewport;
class REPartListModel;
class RESectionListModel;
class RETrackListModel;

class REDocumentView :
        public QWidget,
        public REScoreControllerDelegate,
        public RESongControllerDelegate
{
    Q_OBJECT

    friend class REQtViewport;
    friend class REMainWindow;

public:
    explicit REDocumentView(QWidget *parent = 0);
    
public:
    REScoreController* ScoreController() {return _scoreController;}
    RESongController* SongController() {return _songController;}

    const RESong* Song() const {return _song;}
    RESong* Song() {return _song;}

    RESequencer* Sequencer() {return _songController->Sequencer();}

    bool IsTrackingEnabled() const {return _trackingEnabled;}
    bool IsEditingLowVoice() const;

	const QUndoStack* UndoStack() const {return _undoStack;}
	QUndoStack* UndoStack() {return _undoStack;}

    REPartListModel* PartListModel() {return _partListModel;}
    const REPartListModel* PartListModel() const {return _partListModel;}

    RETrackListModel* TrackListModel() {return _trackListModel;}
    const RETrackListModel* TrackListModel() const {return _trackListModel;}

    RESectionListModel* SectionListModel() {return _sectionListModel;}
    const RESectionListModel* SectionListModel() const {return _sectionListModel;}

signals:
    
public slots:
    void InitializeWithEmptyDocument();
    void InitializeWithFile(const QString& filename);

	void StartPlayback();
	void StopPlayback();
	void TogglePlayback();
    void SetTrackingEnabled(bool);
    void SetEditLowVoice(bool);
    void SetZoomIndex(int);

    void Save();
    void SaveAs();
    void ExportPDF();
    void ExportMIDI();
    void ExportGP5();

    void ActionCut();
    void ActionCopy();
    void ActionPaste();
    void ActionDelete();

    void ActionIncreaseDuration();
    void ActionDecreaseDuration();
    void ActionTypeKeypad(int, bool);

    void ActionAddChord();
    void ActionInsertChord();
    void ActionDuplicateChord();
    void ActionDeleteChord();

    void ActionAddBar();
    void ActionInsertBar();
    void ActionDeleteBar();

    void ShowFilePropertiesDialog();
    void ShowTracksAndPartsDialog();
    void ShowTracksAndPartsDialogSelectingTrack(int trackIndex);
    void ShowTimeSignatureDialog();
    void ShowClefDialog();
    void ShowKeySignatureDialog();
    void ShowRehearsalDialog();
    void ShowChordNameDialog();
    void ShowChordDiagramDialog();
    void ShowTextDialog();

    void ShowRepeatDialog();
    void ActionSystemBreak();
    void ShowTempoMarkerDialog();

    void ActionDoubleFlat();
    void ActionFlat();
    void ActionNatural();
    void ActionSharp();
    void ActionDoubleSharp();
    void ActionSemitoneUp();
    void ActionSemitoneDown();

    void ActionWholeNote();
    void ActionHalfNote();
    void ActionQuarterNote();
    void Action8thNote();
    void Action16thNote();
    void Action32ndNote();
    void Action64thNote();

    void ActionTuplet2();
    void ActionTuplet3();
    void ActionTuplet4();
    void ActionTuplet5();
    void ActionTuplet6();
    void ActionTuplet7();
    void ActionTuplet9();

    void ActionDottedNote();
    void ActionDoubleDottedNote();
    void ActionTiedNote();
    void ActionSplitChord();
    void ActionForceStemUp();
    void ActionForceStemDown();
    void ActionToggleEnharmonicEquivalent();

    void ActionDynamicPPP();
    void ActionDynamicPP();
    void ActionDynamicP();
    void ActionDynamicMP();
    void ActionDynamicMF();
    void ActionDynamicF();
    void ActionDynamicFF();
    void ActionDynamicFFF();

    void ActionGhostNote();
    void ActionAccent();
    void ActionStrongAccent();
    void ActionStaccato();
    void ActionLeftHandSticking();
    void ActionRightHandSticking();
    void ActionBrushUp();
    void ActionBrushDown();
    void ActionArpeggioUp();
    void ActionArpeggioDown();
    void ActionDeadNote();
    void ActionPickstrokeUp();
    void ActionPickstrokeDown();
    void ActionHammerOnPullOff();
    void ActionTap();
    void ActionSlap();
    void ActionPop();
    void ActionVibrato();
    void ActionPalmMute();
    void ActionLetRing();

    void ShowBendDialog();
    void ActionShiftSlide();
    void ActionSlideOutHigh();
    void ActionSlideOutLow();
    void ActionSlideInHigh();
    void ActionSlideInLow();

protected slots:
    void UpdateViewport();
    void ClickedOnPart(QModelIndex idx);

signals:
    void PlaybackStarted();
    void PlaybackStopped();
    void FileStatusChanged();
    void CursorOrSelectionChanged();
    void DataChanged();

public:
	bool IsPlaybackRunning() const;
    QString Filename() const {return _filename;}

    int ZoomIndex() const {return _zoomIndex;}
    float ZoomFactor() const;

public:
    virtual void SongControllerWillModifySong(const RESongController* controller, const RESong* song);
    virtual void SongControllerDidModifySong(const RESongController* controller, const RESong* song, bool successfully);
	virtual void SongControllerDidModifyPhrase(const RESongController* controller, const REPhrase* phrase, bool successfully);
    virtual void SongControllerWantsToBackup(const RESongController* controller, const RESong* song);

public:
    virtual void ScoreControllerSelectionDidChange(const REScoreController* scoreController);
    virtual void ScoreControllerScoreDidChange(const REScoreController* scoreController, const REScore* score);
    virtual void ScoreControllerRefreshPresentation(const REScoreController* scoreController, const REScore* score);
    virtual void OnShouldCenterOnCursor(const REScoreController* scoreController);
    virtual void OnCopyPhraseToPasteboard(const REPhrase* phrase, Reflow::TrackType trackType);
    virtual void OnCopyPartialSongToPasteboard(const RESong* song);
    virtual void ScoreControllerWillRebuildWithSettings(REScoreSettings& settings);
    virtual void ScoreControllerInspectorWillReload(const REScoreController *scoreController);
    virtual void ScoreControllerInspectorDidReload(const REScoreController *scoreController);

protected:
    void CreateControllers();
    void DestroyControllers();
    void LoadGP(RESong& song, QString filename);
	bool LoadFLOW(RESong& song, QString filename);
    bool WriteFLOW(QString filename);
    bool WritePDF(QString filename) const;
    bool WriteMIDI(QString filename) const;
    bool WriteGP5(QString filename) const;

    void PlaySelectedChordOnMonitoringDevice();

protected:
    RESong* _song;
    RESongController* _songController;
    REScoreController* _scoreController;
    REScoreScene* _scene;
    REScoreSceneView* _scoreView;
    REQtViewport* _viewport;
    QString _filename;
	QUndoStack* _undoStack;
    QTimer* _viewportUpdateTimer;
    REPartListModel* _partListModel;
    RETrackListModel* _trackListModel;
    RESectionListModel* _sectionListModel;
    bool _trackingEnabled;
    int _zoomIndex;
};

#endif // REDOCUMENTVIEW_H
