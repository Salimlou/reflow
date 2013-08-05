#include "REMainWindow.h"
#include "REDocumentView.h"
#include "REScoreScene.h"
#include "ui_REMainWindow.h"
#include "REQtPalette.h"
#include "REPartListModel.h"
#include "RESectionListModel.h"
#include "RETrackListModel.h"
#include "RETransportWidget.h"
#include "RESequencerWidget.h"
#include "REPianoWidget.h"
#include "REFretboardWidget.h"
#include "REPreferencesDialog.h"

#include <RESong.h>
#include <RESongController.h>

#include <QGraphicsTextItem>
#include <QFileDialog>
#include <QDockWidget>
#include <QSettings>

static const char* _listViewStyleSheet =
    "QListView {\n"
    "   background: #333333;\n"
    "    show-decoration-selected: 1; /* make the selection span the entire width of the view */\n"
    "}\n"
    "\n"
    "QListView::item {\n"
    "    height: 28px;\n"
    "    color: #DADADA;\n"
    "}\n"
    "\n"
    "QListView::item:alternate {\n"
    "    background: #EEEEEE;\n"
    "}\n"
    "\n"
    "QListView::item:selected {\n"
    "    border: 1px solid #6a6ea9;\n"
    "}\n"
    "\n"
    "QListView::item:selected:!active {\n"
    "    background: qlineargradient(x1: 0, y1: 0, x2: 0, y2: 1,\n"
    "                                stop: 0 #ABAFE5, stop: 1 #8588B2);\n"
    "}\n"
    "\n"
    "QListView::item:selected:active {\n"
    "    background: qlineargradient(x1: 0, y1: 0, x2: 0, y2: 1,\n"
    "                                stop: 0 #6a6ea9, stop: 1 #888dd9);\n"
    "}\n"
    "\n"
    "QListView::item:hover {\n"
    "    background: qlineargradient(x1: 0, y1: 0, x2: 0, y2: 1,\n"
    "                                stop: 0 #555555, stop: 1 #343434);\n"
    "}";


REMainWindow::REMainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::REMainWindow),
    _currentDocument(NULL), _palette(NULL), _partListView(nullptr),
	_undoAction(NULL), _redoAction(NULL)
{
    ui->setupUi(this);

    QToolBar* toolbar = ui->mainToolBar;
    _playAction = toolbar->addAction(QIcon(":/toolbar-play.png"), tr("Play"));

    _editLowVoiceButton = new QPushButton("Low");
    _editLowVoiceButton->setCheckable(true);
    _editLowVoiceButton->setMinimumSize(QSize(24, 24));
    _editLowVoiceButton->setMaximumSize(QSize(32, 24));
    _editLowVoiceButton->setFocusPolicy(Qt::NoFocus);
    ui->statusBar->addWidget(_editLowVoiceButton);

    _zoomCombo = new QComboBox;
    QStringList zoomValues;
    zoomValues << "25%" << "50%" << "75%" << "90%" << "100%" << "110%" << "125%" << "150%" << "175%" << "200%" << "250%" << "300%" << "400%";
    _zoomCombo->addItems(zoomValues);
    _zoomCombo->setFocusPolicy(Qt::NoFocus);
    ui->statusBar->addWidget(_zoomCombo);

    setStyleSheet(QString(
                      "QMainWindow::separator {"
                      "  background: black;"
                      "  width: 1px;"
                      "  height: 1px;"
                      "}"
                      "QMainWindow::separator:hover {"
                      "  background: darkgray;"
                      "}"));

    QTabWidget* tab = new QTabWidget;
    tab->setDocumentMode(true);
    tab->setTabsClosable(true);
    tab->setElideMode(Qt::ElideLeft);
    tab->setFocusPolicy(Qt::NoFocus);
    QObject::connect(tab, SIGNAL(tabCloseRequested(int)), this, SLOT(CloseDocumentTab(int)));
    setCentralWidget(tab);

    _palette = new REQtPalette;

    _sequencerWidget = new RESequencerWidget;
    _sequencerWidget->setMinimumHeight(40);

    _transportWidget = new RETransportWidget;

    _pianoWidget = new REPianoWidget;
    _fretboardWidget = new REFretboardWidget;

    _partListView = new QListView;
    _partListView->setFocusPolicy(Qt::NoFocus);
    _partListView->setStyleSheet(QString::fromLatin1(_listViewStyleSheet));

    _sectionListView = new QListView;
    _sectionListView->setFocusPolicy(Qt::NoFocus);
    _sectionListView->setStyleSheet(QString::fromLatin1(_listViewStyleSheet));

    QDockWidget* paletteDock = new QDockWidget(tr("Palette"));
    paletteDock->setObjectName("paletteDock");
    paletteDock->setWidget(_palette);
    addDockWidget(Qt::LeftDockWidgetArea, paletteDock);
    ui->menuView->addAction(paletteDock->toggleViewAction());

    QDockWidget* transportDock = new QDockWidget(tr("Transport"));
    transportDock->setObjectName("transportDock");
    transportDock->setWidget(_transportWidget);
    transportDock->setAllowedAreas(Qt::AllDockWidgetAreas);
    addDockWidget(Qt::RightDockWidgetArea, transportDock);
    ui->menuView->addAction(transportDock->toggleViewAction());

    QDockWidget* sequencerDock = new QDockWidget(tr("Sequencer"));
    sequencerDock->setObjectName("sequencerDock");
    sequencerDock->setWidget(_sequencerWidget);
    sequencerDock->setAllowedAreas(Qt::BottomDockWidgetArea | Qt::TopDockWidgetArea);
    addDockWidget(Qt::BottomDockWidgetArea, sequencerDock);
    ui->menuView->addAction(sequencerDock->toggleViewAction());

    QDockWidget* partListDock = new QDockWidget(tr("Parts"));
    partListDock->setObjectName("partListDock");
    partListDock->setWidget(_partListView);
    addDockWidget(Qt::RightDockWidgetArea, partListDock);
    ui->menuView->addAction(partListDock->toggleViewAction());

    QDockWidget* sectionListDock = new QDockWidget(tr("Sections"));
    sectionListDock->setObjectName("sectionListDock");
    sectionListDock->setWidget(_sectionListView);
    addDockWidget(Qt::RightDockWidgetArea, sectionListDock);
    ui->menuView->addAction(sectionListDock->toggleViewAction());

    QDockWidget* pianoDock = new QDockWidget(tr("Keyboard"));
    pianoDock->setObjectName("pianoDock");
    pianoDock->setWidget(_pianoWidget);
    pianoDock->setAllowedAreas(Qt::BottomDockWidgetArea | Qt::TopDockWidgetArea);
    addDockWidget(Qt::BottomDockWidgetArea, pianoDock);
    ui->menuView->addAction(pianoDock->toggleViewAction());

    QDockWidget* fretboardDock = new QDockWidget(tr("Fretboard"));
    fretboardDock->setObjectName("fretboardDock");
    fretboardDock->setWidget(_fretboardWidget);
    fretboardDock->setAllowedAreas(Qt::BottomDockWidgetArea | Qt::TopDockWidgetArea);
    addDockWidget(Qt::BottomDockWidgetArea, fretboardDock);
    ui->menuView->addAction(fretboardDock->toggleViewAction());

    QObject::connect(tab, SIGNAL(currentChanged(int)), this, SLOT(OnCurrentTabChanged(int)));

    QObject::connect(ui->actionNew, SIGNAL(triggered()), this, SLOT(ActionNew()));
    QObject::connect(ui->actionOpen, SIGNAL(triggered()), this, SLOT(ActionOpen()));
    QObject::connect(ui->actionClose, SIGNAL(triggered()), this, SLOT(ActionClose()));

    QSettings settings;
    restoreGeometry(settings.value("mainWindow/geometry").toByteArray());
    restoreState(settings.value("mainWindow/state").toByteArray());
}

REMainWindow::~REMainWindow()
{
    delete ui;
}

void REMainWindow::on_actionPreferences_triggered()
{
    REPreferencesDialog dlg(this);
    dlg.exec();
}

void REMainWindow::ActionNew()
{
    QTabWidget* tab = qobject_cast<QTabWidget*>(centralWidget());

    REDocumentView *doc = new REDocumentView;
    doc->InitializeWithEmptyDocument();
    tab->addTab(doc, "New File");

    tab->setCurrentIndex(tab->count()-1);
}

void REMainWindow::ActionOpen()
{
    QString filename = QFileDialog::getOpenFileName(this, tr("Open File"), QDir::home().absolutePath(), tr("Reflow Files (*.flow *.gp3 *.gp4 *.gp5)"));
    ActionOpen(filename);
}

void REMainWindow::ActionOpen(QString filename)
{
    if(!filename.isEmpty())
    {
        QTabWidget* tab = qobject_cast<QTabWidget*>(centralWidget());

        REDocumentView *doc = new REDocumentView;
        doc->InitializeWithFile(filename);
        tab->addTab(doc, filename);

        tab->setCurrentIndex(tab->count()-1);
    }
}

void REMainWindow::ActionClose()
{
    QTabWidget* tab = qobject_cast<QTabWidget*>(centralWidget());
    int idx = tab->currentIndex();
    if(idx != -1)
    {
        CloseDocumentTab(idx);
    }
}

void REMainWindow::CloseDocumentTab(int index)
{
    QTabWidget* tab = qobject_cast<QTabWidget*>(centralWidget());
    QWidget* w = tab->widget(index);
    if(w)
    {
        REDocumentView* doc = qobject_cast<REDocumentView*>(w);
        if(doc == _currentDocument) {
            DisconnectFromDocument();
        }

        tab->removeTab(index);

        doc->DestroyControllers();
        delete doc;
    }
}

void REMainWindow::OnCurrentTabChanged(int newIndex)
{
    qDebug("changing tab to %d\n", newIndex);
    QTabWidget* tab = qobject_cast<QTabWidget*>(centralWidget());

    DisconnectFromDocument();
    REDocumentView* doc = qobject_cast<REDocumentView*>(tab->widget(newIndex));
    _currentDocument = doc;
    _currentDocument->setFocus();
    ConnectToDocument();
}

void REMainWindow::OnCurrentDocumentStatusChanged()
{
    QTabWidget* tab = qobject_cast<QTabWidget*>(centralWidget());

    int index = tab->currentIndex();
    QWidget* w = tab->widget(index);
    if(w) {
        REDocumentView* doc = qobject_cast<REDocumentView*>(w);
        tab->setTabText(index, doc->Filename());
        UpdateWindowTitleFromCurrentDocument();
    }
}

void REMainWindow::OnCurrentDocumentPlaybackStarted()
{
    _playAction->setIcon(QIcon(":/toolbar-stop.png"));
    _playAction->setText(tr("Stop"));
}

void REMainWindow::OnCurrentDocumentPlaybackStopped()
{
    _playAction->setIcon(QIcon(":/toolbar-play.png"));
    _playAction->setText(tr("Play"));
}

void REMainWindow::UpdateWindowTitleFromCurrentDocument()
{
    QString title = _currentDocument->Filename();
    if(title.isEmpty()) {
        setWindowTitle("Reflow");
        setWindowFilePath("");
    }
    else {
        setWindowTitle("");
        setWindowFilePath(title);
    }
}

void REMainWindow::RefreshInterfaceFromCurrentDocument()
{
    if(_currentDocument == nullptr) return;

    _editLowVoiceButton->blockSignals(true);
    _editLowVoiceButton->setChecked(_currentDocument->IsEditingLowVoice());
    _editLowVoiceButton->blockSignals(false);
}

void REMainWindow::closeEvent(QCloseEvent* e)
{
    QSettings settings;
    settings.setValue("mainWindow/geometry", saveGeometry());
    settings.setValue("mainWindow/state", saveState());

    QMainWindow::closeEvent(e);
}

void REMainWindow::ConnectToDocument()
{
    if(_currentDocument == NULL) return;

    UpdateWindowTitleFromCurrentDocument();

    const REScoreController* scoreController = _currentDocument->ScoreController();
    const RESong* song = scoreController->Score()->Song();

    // Sequencer
    _sequencerWidget->ConnectToDocument(_currentDocument);
    _transportWidget->ConnectToDocument(_currentDocument);

    _pianoWidget->ConnectToDocument(_currentDocument);
    QObject::connect(_currentDocument, SIGNAL(CursorOrSelectionChanged()), _pianoWidget, SLOT(RefreshDisplay()));
    QObject::connect(_currentDocument, SIGNAL(DataChanged()), _pianoWidget, SLOT(RefreshDisplay()));

    _fretboardWidget->ConnectToDocument(_currentDocument);
    QObject::connect(_currentDocument, SIGNAL(CursorOrSelectionChanged()), _fretboardWidget, SLOT(RefreshDisplay()));
    QObject::connect(_currentDocument, SIGNAL(DataChanged()), _fretboardWidget, SLOT(RefreshDisplay()));

    // Part List
    _partListView->setModel(_currentDocument->PartListModel());
    int scoreIndex = scoreController->ScoreIndex();
    if(scoreIndex >= 0 && scoreIndex < song->ScoreCount()) {
        _partListView->setCurrentIndex(_currentDocument->PartListModel()->index(scoreIndex, 0));
    }
    QObject::connect(_partListView, SIGNAL(clicked(QModelIndex)), _currentDocument, SLOT(ClickedOnPart(QModelIndex)));

    // Section List
    _sectionListView->setModel(_currentDocument->SectionListModel());

	QUndoStack* undoStack = _currentDocument->UndoStack();
	_undoAction = undoStack->createUndoAction(this);
	_redoAction = undoStack->createRedoAction(this);
    _undoAction->setShortcut(QKeySequence::Undo);
    _redoAction->setShortcut(QKeySequence::Redo);
    ui->menuEdit->addAction(_undoAction);
	ui->menuEdit->addAction(_redoAction);

    _editLowVoiceButton->blockSignals(true);
    _editLowVoiceButton->setChecked(_currentDocument->IsEditingLowVoice());
    _editLowVoiceButton->blockSignals(false);
    QObject::connect(_editLowVoiceButton, SIGNAL(toggled(bool)), _currentDocument, SLOT(SetEditLowVoice(bool)));

    _zoomCombo->blockSignals(true);
    _zoomCombo->setCurrentIndex(_currentDocument->ZoomIndex());
    _zoomCombo->blockSignals(false);
    QObject::connect(_zoomCombo, SIGNAL(currentIndexChanged(int)), _currentDocument, SLOT(SetZoomIndex(int)));

    QObject::connect(_currentDocument, SIGNAL(CursorOrSelectionChanged()), this, SLOT(RefreshInterfaceFromCurrentDocument()));

    QObject::connect(_playAction, SIGNAL(triggered()), _currentDocument, SLOT(TogglePlayback()));
    QObject::connect(_currentDocument, SIGNAL(PlaybackStarted()), this, SLOT(OnCurrentDocumentPlaybackStarted()));
    QObject::connect(_currentDocument, SIGNAL(PlaybackStopped()), this, SLOT(OnCurrentDocumentPlaybackStopped()));
    QObject::connect(_currentDocument, SIGNAL(FileStatusChanged()), this, SLOT(OnCurrentDocumentStatusChanged()));

    QObject::connect(ui->actionSave, SIGNAL(triggered()), _currentDocument, SLOT(Save()));
    QObject::connect(ui->actionSaveAs, SIGNAL(triggered()), _currentDocument, SLOT(SaveAs()));
    QObject::connect(ui->actionExportPDF, SIGNAL(triggered()), _currentDocument, SLOT(ExportPDF()));
    QObject::connect(ui->actionExportMIDI, SIGNAL(triggered()), _currentDocument, SLOT(ExportMIDI()));
    QObject::connect(ui->actionExportGuitarPro5, SIGNAL(triggered()), _currentDocument, SLOT(ExportGP5()));

    QObject::connect(ui->actionCut, SIGNAL(triggered()), _currentDocument, SLOT(ActionCut()));
    QObject::connect(ui->actionCopy, SIGNAL(triggered()), _currentDocument, SLOT(ActionCopy()));
    QObject::connect(ui->actionPaste, SIGNAL(triggered()), _currentDocument, SLOT(ActionPaste()));
    QObject::connect(ui->actionDelete, SIGNAL(triggered()), _currentDocument, SLOT(ActionDelete()));

    QObject::connect(ui->actionFileProperties, SIGNAL(triggered()), _currentDocument, SLOT(ShowFilePropertiesDialog()));
    QObject::connect(ui->actionTracksAndParts, SIGNAL(triggered()), _currentDocument, SLOT(ShowTracksAndPartsDialog()));
    QObject::connect(ui->actionTimeSignature, SIGNAL(triggered()), _currentDocument, SLOT(ShowTimeSignatureDialog()));
    QObject::connect(ui->actionKeySignature, SIGNAL(triggered()), _currentDocument, SLOT(ShowKeySignatureDialog()));
    QObject::connect(ui->actionClef, SIGNAL(triggered()), _currentDocument, SLOT(ShowClefDialog()));
    QObject::connect(ui->actionRehearsalSign, SIGNAL(triggered()), _currentDocument, SLOT(ShowRehearsalDialog()));
    QObject::connect(ui->actionRepeats, SIGNAL(triggered()), _currentDocument, SLOT(ShowRepeatDialog()));
    QObject::connect(ui->actionTempoMarker, SIGNAL(triggered()), _currentDocument, SLOT(ShowTempoMarkerDialog()));
    QObject::connect(ui->actionText, SIGNAL(triggered()), _currentDocument, SLOT(ShowTextDialog()));
    QObject::connect(ui->actionChordName, SIGNAL(triggered()), _currentDocument, SLOT(ShowChordNameDialog()));
    QObject::connect(ui->actionChordDiagram, SIGNAL(triggered()), _currentDocument, SLOT(ShowChordDiagramDialog()));

    QObject::connect(_palette->ButtonForIdentifier("addchord"), SIGNAL(clicked()), _currentDocument, SLOT(ActionAddChord()));
    QObject::connect(_palette->ButtonForIdentifier("inschord"), SIGNAL(clicked()), _currentDocument, SLOT(ActionInsertChord()));
    QObject::connect(_palette->ButtonForIdentifier("dupchord"), SIGNAL(clicked()), _currentDocument, SLOT(ActionDuplicateChord()));
    QObject::connect(_palette->ButtonForIdentifier("delchord"), SIGNAL(clicked()), _currentDocument, SLOT(ActionDeleteChord()));
    QObject::connect(_palette->ButtonForIdentifier("addbar"), SIGNAL(clicked()), _currentDocument, SLOT(ActionAddBar()));
    QObject::connect(_palette->ButtonForIdentifier("insbar"), SIGNAL(clicked()), _currentDocument, SLOT(ActionInsertBar()));
    QObject::connect(_palette->ButtonForIdentifier("delbar"), SIGNAL(clicked()), _currentDocument, SLOT(ActionDeleteBar()));

    QObject::connect(_palette->ButtonForIdentifier("time_signature"), SIGNAL(clicked()), _currentDocument, SLOT(ShowTimeSignatureDialog()));
    QObject::connect(_palette->ButtonForIdentifier("clef"), SIGNAL(clicked()), _currentDocument, SLOT(ShowClefDialog()));
    QObject::connect(_palette->ButtonForIdentifier("key_signature"), SIGNAL(clicked()), _currentDocument, SLOT(ShowKeySignatureDialog()));
    QObject::connect(_palette->ButtonForIdentifier("rehearsal"), SIGNAL(clicked()), _currentDocument, SLOT(ShowRehearsalDialog()));
    QObject::connect(_palette->ButtonForIdentifier("chord_name"), SIGNAL(clicked()), _currentDocument, SLOT(ShowChordNameDialog()));
    QObject::connect(_palette->ButtonForIdentifier("diagram"), SIGNAL(clicked()), _currentDocument, SLOT(ShowChordDiagramDialog()));
    QObject::connect(_palette->ButtonForIdentifier("text"), SIGNAL(clicked()), _currentDocument, SLOT(ShowTextDialog()));

    QObject::connect(_palette->ButtonForIdentifier("end_repeat"), SIGNAL(clicked()), _currentDocument, SLOT(ShowRepeatDialog()));
    QObject::connect(_palette->ButtonForIdentifier("systembreak"), SIGNAL(clicked()), _currentDocument, SLOT(ActionSystemBreak()));
    QObject::connect(_palette->ButtonForIdentifier("tempomarker"), SIGNAL(clicked()), _currentDocument, SLOT(ShowTempoMarkerDialog()));

    QObject::connect(_palette->ButtonForIdentifier("2flat"), SIGNAL(clicked()), _currentDocument, SLOT(ActionDoubleFlat()));
    QObject::connect(_palette->ButtonForIdentifier("flat"), SIGNAL(clicked()), _currentDocument, SLOT(ActionFlat()));
    QObject::connect(_palette->ButtonForIdentifier("natural"), SIGNAL(clicked()), _currentDocument, SLOT(ActionNatural()));
    QObject::connect(_palette->ButtonForIdentifier("sharp"), SIGNAL(clicked()), _currentDocument, SLOT(ActionSharp()));
    QObject::connect(_palette->ButtonForIdentifier("2sharp"), SIGNAL(clicked()), _currentDocument, SLOT(ActionDoubleSharp()));
    QObject::connect(_palette->ButtonForIdentifier("semitone_up"), SIGNAL(clicked()), _currentDocument, SLOT(ActionSemitoneUp()));
    QObject::connect(_palette->ButtonForIdentifier("semitone_down"), SIGNAL(clicked()), _currentDocument, SLOT(ActionSemitoneDown()));

    QObject::connect(_palette->ButtonForIdentifier("whole"), SIGNAL(clicked()), _currentDocument, SLOT(ActionWholeNote()));
    QObject::connect(_palette->ButtonForIdentifier("half"), SIGNAL(clicked()), _currentDocument, SLOT(ActionHalfNote()));
    QObject::connect(_palette->ButtonForIdentifier("quarter"), SIGNAL(clicked()), _currentDocument, SLOT(ActionQuarterNote()));
    QObject::connect(_palette->ButtonForIdentifier("8th"), SIGNAL(clicked()), _currentDocument, SLOT(Action8thNote()));
    QObject::connect(_palette->ButtonForIdentifier("16th"), SIGNAL(clicked()), _currentDocument, SLOT(Action16thNote()));
    QObject::connect(_palette->ButtonForIdentifier("32nd"), SIGNAL(clicked()), _currentDocument, SLOT(Action32ndNote()));
    QObject::connect(_palette->ButtonForIdentifier("64th"), SIGNAL(clicked()), _currentDocument, SLOT(Action64thNote()));

    QObject::connect(_palette->ButtonForIdentifier("tuplet_2"), SIGNAL(clicked()), _currentDocument, SLOT(ActionTuplet2()));
    QObject::connect(_palette->ButtonForIdentifier("tuplet_3"), SIGNAL(clicked()), _currentDocument, SLOT(ActionTuplet3()));
    QObject::connect(_palette->ButtonForIdentifier("tuplet_4"), SIGNAL(clicked()), _currentDocument, SLOT(ActionTuplet4()));
    QObject::connect(_palette->ButtonForIdentifier("tuplet_5"), SIGNAL(clicked()), _currentDocument, SLOT(ActionTuplet5()));
    QObject::connect(_palette->ButtonForIdentifier("tuplet_6"), SIGNAL(clicked()), _currentDocument, SLOT(ActionTuplet6()));
    QObject::connect(_palette->ButtonForIdentifier("tuplet_7"), SIGNAL(clicked()), _currentDocument, SLOT(ActionTuplet7()));
    QObject::connect(_palette->ButtonForIdentifier("tuplet_9"), SIGNAL(clicked()), _currentDocument, SLOT(ActionTuplet9()));

    QObject::connect(_palette->ButtonForIdentifier("dot"), SIGNAL(clicked()), _currentDocument, SLOT(ActionDottedNote()));
    QObject::connect(_palette->ButtonForIdentifier("2dot"), SIGNAL(clicked()), _currentDocument, SLOT(ActionDoubleDottedNote()));
    QObject::connect(_palette->ButtonForIdentifier("tied"), SIGNAL(clicked()), _currentDocument, SLOT(ActionTiedNote()));
    QObject::connect(_palette->ButtonForIdentifier("splitchord"), SIGNAL(clicked()), _currentDocument, SLOT(ActionSplitChord()));
    QObject::connect(_palette->ButtonForIdentifier("stem_up"), SIGNAL(clicked()), _currentDocument, SLOT(ActionForceStemUp()));
    QObject::connect(_palette->ButtonForIdentifier("stem_down"), SIGNAL(clicked()), _currentDocument, SLOT(ActionForceStemDown()));
    QObject::connect(_palette->ButtonForIdentifier("enharmonic"), SIGNAL(clicked()), _currentDocument, SLOT(ActionToggleEnharmonicEquivalent()));

    QObject::connect(_palette->ButtonForIdentifier("ppp"), SIGNAL(clicked()), _currentDocument, SLOT(ActionDynamicPPP()));
    QObject::connect(_palette->ButtonForIdentifier("pp"), SIGNAL(clicked()), _currentDocument, SLOT(ActionDynamicPP()));
    QObject::connect(_palette->ButtonForIdentifier("p"), SIGNAL(clicked()), _currentDocument, SLOT(ActionDynamicP()));
    QObject::connect(_palette->ButtonForIdentifier("mp"), SIGNAL(clicked()), _currentDocument, SLOT(ActionDynamicMP()));
    QObject::connect(_palette->ButtonForIdentifier("mf"), SIGNAL(clicked()), _currentDocument, SLOT(ActionDynamicMF()));
    QObject::connect(_palette->ButtonForIdentifier("f"), SIGNAL(clicked()), _currentDocument, SLOT(ActionDynamicF()));
    QObject::connect(_palette->ButtonForIdentifier("ff"), SIGNAL(clicked()), _currentDocument, SLOT(ActionDynamicFF()));
    QObject::connect(_palette->ButtonForIdentifier("fff"), SIGNAL(clicked()), _currentDocument, SLOT(ActionDynamicFFF()));

    QObject::connect(_palette->ButtonForIdentifier("ghost"), SIGNAL(clicked()), _currentDocument, SLOT(ActionGhostNote()));
    QObject::connect(_palette->ButtonForIdentifier("accent"), SIGNAL(clicked()), _currentDocument, SLOT(ActionAccent()));
    QObject::connect(_palette->ButtonForIdentifier("strong_accent"), SIGNAL(clicked()), _currentDocument, SLOT(ActionStrongAccent()));
    QObject::connect(_palette->ButtonForIdentifier("staccato"), SIGNAL(clicked()), _currentDocument, SLOT(ActionStaccato()));
    QObject::connect(_palette->ButtonForIdentifier("left"), SIGNAL(clicked()), _currentDocument, SLOT(ActionLeftHandSticking()));
    QObject::connect(_palette->ButtonForIdentifier("right"), SIGNAL(clicked()), _currentDocument, SLOT(ActionRightHandSticking()));

    QObject::connect(_palette->ButtonForIdentifier("brush_up"), SIGNAL(clicked()), _currentDocument, SLOT(ActionBrushUp()));
    QObject::connect(_palette->ButtonForIdentifier("brush_down"), SIGNAL(clicked()), _currentDocument, SLOT(ActionBrushDown()));
    QObject::connect(_palette->ButtonForIdentifier("arpeggio_up"), SIGNAL(clicked()), _currentDocument, SLOT(ActionArpeggioUp()));
    QObject::connect(_palette->ButtonForIdentifier("arpeggio_down"), SIGNAL(clicked()), _currentDocument, SLOT(ActionArpeggioDown()));
    QObject::connect(_palette->ButtonForIdentifier("dead"), SIGNAL(clicked()), _currentDocument, SLOT(ActionDeadNote()));
    QObject::connect(_palette->ButtonForIdentifier("pick_up"), SIGNAL(clicked()), _currentDocument, SLOT(ActionPickstrokeUp()));
    QObject::connect(_palette->ButtonForIdentifier("pick_down"), SIGNAL(clicked()), _currentDocument, SLOT(ActionPickstrokeDown()));

    QObject::connect(_palette->ButtonForIdentifier("hopo"), SIGNAL(clicked()), _currentDocument, SLOT(ActionHammerOnPullOff()));
    QObject::connect(_palette->ButtonForIdentifier("tap"), SIGNAL(clicked()), _currentDocument, SLOT(ActionTap()));
    QObject::connect(_palette->ButtonForIdentifier("slap"), SIGNAL(clicked()), _currentDocument, SLOT(ActionSlap()));
    QObject::connect(_palette->ButtonForIdentifier("pop"), SIGNAL(clicked()), _currentDocument, SLOT(ActionPop()));
    QObject::connect(_palette->ButtonForIdentifier("vibrato"), SIGNAL(clicked()), _currentDocument, SLOT(ActionVibrato()));
    QObject::connect(_palette->ButtonForIdentifier("palm_mute"), SIGNAL(clicked()), _currentDocument, SLOT(ActionPalmMute()));
    QObject::connect(_palette->ButtonForIdentifier("let_ring"), SIGNAL(clicked()), _currentDocument, SLOT(ActionLetRing()));

    QObject::connect(_palette->ButtonForIdentifier("bend"), SIGNAL(clicked()), _currentDocument, SLOT(ShowBendDialog()));
    QObject::connect(_palette->ButtonForIdentifier("slide"), SIGNAL(clicked()), _currentDocument, SLOT(ActionShiftSlide()));
    QObject::connect(_palette->ButtonForIdentifier("slide_out_high"), SIGNAL(clicked()), _currentDocument, SLOT(ActionSlideOutHigh()));
    QObject::connect(_palette->ButtonForIdentifier("slide_out_low"), SIGNAL(clicked()), _currentDocument, SLOT(ActionSlideOutLow()));
    QObject::connect(_palette->ButtonForIdentifier("slide_in_high"), SIGNAL(clicked()), _currentDocument, SLOT(ActionSlideInHigh()));
    QObject::connect(_palette->ButtonForIdentifier("slide_in_low"), SIGNAL(clicked()), _currentDocument, SLOT(ActionSlideInLow()));
}


void REMainWindow::DisconnectFromDocument()
{
    if(_currentDocument == NULL) return;

    // Sequencer
    _sequencerWidget->DisconnectFromDocument();
    _transportWidget->DisconnectFromDocument();

    _pianoWidget->DisconnectFromDocument();
    QObject::disconnect(_currentDocument, SIGNAL(CursorOrSelectionChanged()), _pianoWidget, SLOT(RefreshDisplay()));
    QObject::disconnect(_currentDocument, SIGNAL(DataChanged()), _pianoWidget, SLOT(RefreshDisplay()));

    _fretboardWidget->DisconnectFromDocument();
    QObject::connect(_currentDocument, SIGNAL(CursorOrSelectionChanged()), _fretboardWidget, SLOT(RefreshDisplay()));
    QObject::connect(_currentDocument, SIGNAL(DataChanged()), _fretboardWidget, SLOT(RefreshDisplay()));

    // Part List
    _partListView->setModel(nullptr);
    QObject::disconnect(_partListView, SIGNAL(clicked(QModelIndex)), _currentDocument, SLOT(ClickedOnPart(QModelIndex)));

    // Section List
    _sectionListView->setModel(nullptr);

	if(_undoAction) {
		ui->menuEdit->removeAction(_undoAction);
		delete _undoAction;
		_undoAction = NULL;
	}
	if(_redoAction) {
		ui->menuEdit->removeAction(_redoAction);
		delete _redoAction;
		_redoAction = NULL;
	}

    _editLowVoiceButton->blockSignals(true);
    _editLowVoiceButton->setChecked(false);
    _editLowVoiceButton->blockSignals(false);
    QObject::disconnect(_editLowVoiceButton, SIGNAL(toggled(bool)), _currentDocument, SLOT(SetEditLowVoice(bool)));

    _zoomCombo->blockSignals(true);
    _zoomCombo->setCurrentIndex(4);
    _zoomCombo->blockSignals(false);
    QObject::disconnect(_zoomCombo, SIGNAL(currentIndexChanged(int)), _currentDocument, SLOT(SetZoomIndex(int)));

    QObject::disconnect(_currentDocument, SIGNAL(CursorOrSelectionChanged()), this, SLOT(RefreshInterfaceFromCurrentDocument()));

    QObject::disconnect(_playAction, SIGNAL(triggered()), _currentDocument, SLOT(TogglePlayback()));
    QObject::disconnect(_currentDocument, SIGNAL(PlaybackStarted()), this, SLOT(OnCurrentDocumentPlaybackStarted()));
    QObject::disconnect(_currentDocument, SIGNAL(PlaybackStopped()), this, SLOT(OnCurrentDocumentPlaybackStopped()));
    QObject::disconnect(_currentDocument, SIGNAL(FileStatusChanged()), this, SLOT(OnCurrentDocumentStatusChanged()));

    QObject::disconnect(ui->actionSave, SIGNAL(triggered()), _currentDocument, SLOT(Save()));
    QObject::disconnect(ui->actionSaveAs, SIGNAL(triggered()), _currentDocument, SLOT(SaveAs()));
    QObject::disconnect(ui->actionExportPDF, SIGNAL(triggered()), _currentDocument, SLOT(ExportPDF()));
    QObject::disconnect(ui->actionExportMIDI, SIGNAL(triggered()), _currentDocument, SLOT(ExportMIDI()));
    QObject::disconnect(ui->actionExportGuitarPro5, SIGNAL(triggered()), _currentDocument, SLOT(ExportGP5()));

    QObject::disconnect(ui->actionCut, SIGNAL(triggered()), _currentDocument, SLOT(ActionCut()));
    QObject::disconnect(ui->actionCopy, SIGNAL(triggered()), _currentDocument, SLOT(ActionCopy()));
    QObject::disconnect(ui->actionPaste, SIGNAL(triggered()), _currentDocument, SLOT(ActionPaste()));
    QObject::disconnect(ui->actionDelete, SIGNAL(triggered()), _currentDocument, SLOT(ActionDelete()));

    QObject::disconnect(ui->actionFileProperties, SIGNAL(triggered()), _currentDocument, SLOT(ShowFilePropertiesDialog()));
    QObject::disconnect(ui->actionTracksAndParts, SIGNAL(triggered()), _currentDocument, SLOT(ShowTracksAndPartsDialog()));
    QObject::disconnect(ui->actionTimeSignature, SIGNAL(triggered()), _currentDocument, SLOT(ShowTimeSignatureDialog()));
    QObject::disconnect(ui->actionKeySignature, SIGNAL(triggered()), _currentDocument, SLOT(ShowKeySignatureDialog()));
    QObject::disconnect(ui->actionClef, SIGNAL(triggered()), _currentDocument, SLOT(ShowClefDialog()));
    QObject::disconnect(ui->actionRehearsalSign, SIGNAL(triggered()), _currentDocument, SLOT(ShowRehearsalDialog()));
    QObject::disconnect(ui->actionRepeats, SIGNAL(triggered()), _currentDocument, SLOT(ShowRepeatDialog()));
    QObject::disconnect(ui->actionTempoMarker, SIGNAL(triggered()), _currentDocument, SLOT(ShowTempoMarkerDialog()));
    QObject::disconnect(ui->actionText, SIGNAL(triggered()), _currentDocument, SLOT(ShowTextDialog()));
    QObject::disconnect(ui->actionChordName, SIGNAL(triggered()), _currentDocument, SLOT(ShowChordNameDialog()));
    QObject::disconnect(ui->actionChordDiagram, SIGNAL(triggered()), _currentDocument, SLOT(ShowChordDiagramDialog()));

    QObject::disconnect(_palette->ButtonForIdentifier("addchord"), SIGNAL(clicked()), _currentDocument, SLOT(ActionAddChord()));
    QObject::disconnect(_palette->ButtonForIdentifier("inschord"), SIGNAL(clicked()), _currentDocument, SLOT(ActionInsertChord()));
    QObject::disconnect(_palette->ButtonForIdentifier("dupchord"), SIGNAL(clicked()), _currentDocument, SLOT(ActionDuplicateChord()));
    QObject::disconnect(_palette->ButtonForIdentifier("delchord"), SIGNAL(clicked()), _currentDocument, SLOT(ActionDeleteChord()));
    QObject::disconnect(_palette->ButtonForIdentifier("addbar"), SIGNAL(clicked()), _currentDocument, SLOT(ActionAddBar()));
    QObject::disconnect(_palette->ButtonForIdentifier("insbar"), SIGNAL(clicked()), _currentDocument, SLOT(ActionInsertBar()));
    QObject::disconnect(_palette->ButtonForIdentifier("delbar"), SIGNAL(clicked()), _currentDocument, SLOT(ActionDeleteBar()));

    QObject::disconnect(_palette->ButtonForIdentifier("time_signature"), SIGNAL(clicked()), _currentDocument, SLOT(ShowTimeSignatureDialog()));
    QObject::disconnect(_palette->ButtonForIdentifier("clef"), SIGNAL(clicked()), _currentDocument, SLOT(ShowClefDialog()));
    QObject::disconnect(_palette->ButtonForIdentifier("key_signature"), SIGNAL(clicked()), _currentDocument, SLOT(ShowKeySignatureDialog()));
    QObject::disconnect(_palette->ButtonForIdentifier("rehearsal"), SIGNAL(clicked()), _currentDocument, SLOT(ShowRehearsalDialog()));
    QObject::disconnect(_palette->ButtonForIdentifier("chord_name"), SIGNAL(clicked()), _currentDocument, SLOT(ShowChordNameDialog()));
    QObject::disconnect(_palette->ButtonForIdentifier("diagram"), SIGNAL(clicked()), _currentDocument, SLOT(ShowChordDiagramDialog()));
    QObject::disconnect(_palette->ButtonForIdentifier("text"), SIGNAL(clicked()), _currentDocument, SLOT(ShowTextDialog()));

    QObject::disconnect(_palette->ButtonForIdentifier("end_repeat"), SIGNAL(clicked()), _currentDocument, SLOT(ShowRepeatDialog()));
    QObject::disconnect(_palette->ButtonForIdentifier("systembreak"), SIGNAL(clicked()), _currentDocument, SLOT(ActionSystemBreak()));
    QObject::disconnect(_palette->ButtonForIdentifier("tempomarker"), SIGNAL(clicked()), _currentDocument, SLOT(ShowTempoMarkerDialog()));

    QObject::disconnect(_palette->ButtonForIdentifier("2flat"), SIGNAL(clicked()), _currentDocument, SLOT(ActionDoubleFlat()));
    QObject::disconnect(_palette->ButtonForIdentifier("flat"), SIGNAL(clicked()), _currentDocument, SLOT(ActionFlat()));
    QObject::disconnect(_palette->ButtonForIdentifier("natural"), SIGNAL(clicked()), _currentDocument, SLOT(ActionNatural()));
    QObject::disconnect(_palette->ButtonForIdentifier("sharp"), SIGNAL(clicked()), _currentDocument, SLOT(ActionSharp()));
    QObject::disconnect(_palette->ButtonForIdentifier("2sharp"), SIGNAL(clicked()), _currentDocument, SLOT(ActionDoubleSharp()));
    QObject::disconnect(_palette->ButtonForIdentifier("semitone_up"), SIGNAL(clicked()), _currentDocument, SLOT(ActionSemitoneUp()));
    QObject::disconnect(_palette->ButtonForIdentifier("semitone_down"), SIGNAL(clicked()), _currentDocument, SLOT(ActionSemitoneDown()));

    QObject::disconnect(_palette->ButtonForIdentifier("whole"), SIGNAL(clicked()), _currentDocument, SLOT(ActionWholeNote()));
    QObject::disconnect(_palette->ButtonForIdentifier("half"), SIGNAL(clicked()), _currentDocument, SLOT(ActionHalfNote()));
    QObject::disconnect(_palette->ButtonForIdentifier("quarter"), SIGNAL(clicked()), _currentDocument, SLOT(ActionQuarterNote()));
    QObject::disconnect(_palette->ButtonForIdentifier("8th"), SIGNAL(clicked()), _currentDocument, SLOT(Action8thNote()));
    QObject::disconnect(_palette->ButtonForIdentifier("16th"), SIGNAL(clicked()), _currentDocument, SLOT(Action16thNote()));
    QObject::disconnect(_palette->ButtonForIdentifier("32nd"), SIGNAL(clicked()), _currentDocument, SLOT(Action32ndNote()));
    QObject::disconnect(_palette->ButtonForIdentifier("64th"), SIGNAL(clicked()), _currentDocument, SLOT(Action64thNote()));

    QObject::disconnect(_palette->ButtonForIdentifier("tuplet_2"), SIGNAL(clicked()), _currentDocument, SLOT(ActionTuplet2()));
    QObject::disconnect(_palette->ButtonForIdentifier("tuplet_3"), SIGNAL(clicked()), _currentDocument, SLOT(ActionTuplet3()));
    QObject::disconnect(_palette->ButtonForIdentifier("tuplet_4"), SIGNAL(clicked()), _currentDocument, SLOT(ActionTuplet4()));
    QObject::disconnect(_palette->ButtonForIdentifier("tuplet_5"), SIGNAL(clicked()), _currentDocument, SLOT(ActionTuplet5()));
    QObject::disconnect(_palette->ButtonForIdentifier("tuplet_6"), SIGNAL(clicked()), _currentDocument, SLOT(ActionTuplet6()));
    QObject::disconnect(_palette->ButtonForIdentifier("tuplet_7"), SIGNAL(clicked()), _currentDocument, SLOT(ActionTuplet7()));
    QObject::disconnect(_palette->ButtonForIdentifier("tuplet_9"), SIGNAL(clicked()), _currentDocument, SLOT(ActionTuplet9()));

    QObject::disconnect(_palette->ButtonForIdentifier("dot"), SIGNAL(clicked()), _currentDocument, SLOT(ActionDottedNote()));
    QObject::disconnect(_palette->ButtonForIdentifier("2dot"), SIGNAL(clicked()), _currentDocument, SLOT(ActionDoubleDottedNote()));
    QObject::disconnect(_palette->ButtonForIdentifier("tied"), SIGNAL(clicked()), _currentDocument, SLOT(ActionTiedNote()));
    QObject::disconnect(_palette->ButtonForIdentifier("splitchord"), SIGNAL(clicked()), _currentDocument, SLOT(ActionSplitChord()));
    QObject::disconnect(_palette->ButtonForIdentifier("stem_up"), SIGNAL(clicked()), _currentDocument, SLOT(ActionForceStemUp()));
    QObject::disconnect(_palette->ButtonForIdentifier("stem_down"), SIGNAL(clicked()), _currentDocument, SLOT(ActionForceStemDown()));
    QObject::disconnect(_palette->ButtonForIdentifier("enharmonic"), SIGNAL(clicked()), _currentDocument, SLOT(ActionToggleEnharmonicEquivalent()));

    QObject::disconnect(_palette->ButtonForIdentifier("ppp"), SIGNAL(clicked()), _currentDocument, SLOT(ActionDynamicPPP()));
    QObject::disconnect(_palette->ButtonForIdentifier("pp"), SIGNAL(clicked()), _currentDocument, SLOT(ActionDynamicPP()));
    QObject::disconnect(_palette->ButtonForIdentifier("p"), SIGNAL(clicked()), _currentDocument, SLOT(ActionDynamicP()));
    QObject::disconnect(_palette->ButtonForIdentifier("mp"), SIGNAL(clicked()), _currentDocument, SLOT(ActionDynamicMP()));
    QObject::disconnect(_palette->ButtonForIdentifier("mf"), SIGNAL(clicked()), _currentDocument, SLOT(ActionDynamicMF()));
    QObject::disconnect(_palette->ButtonForIdentifier("f"), SIGNAL(clicked()), _currentDocument, SLOT(ActionDynamicF()));
    QObject::disconnect(_palette->ButtonForIdentifier("ff"), SIGNAL(clicked()), _currentDocument, SLOT(ActionDynamicFF()));
    QObject::disconnect(_palette->ButtonForIdentifier("fff"), SIGNAL(clicked()), _currentDocument, SLOT(ActionDynamicFFF()));

    QObject::disconnect(_palette->ButtonForIdentifier("ghost"), SIGNAL(clicked()), _currentDocument, SLOT(ActionGhostNote()));
    QObject::disconnect(_palette->ButtonForIdentifier("accent"), SIGNAL(clicked()), _currentDocument, SLOT(ActionAccent()));
    QObject::disconnect(_palette->ButtonForIdentifier("strong_accent"), SIGNAL(clicked()), _currentDocument, SLOT(ActionStrongAccent()));
    QObject::disconnect(_palette->ButtonForIdentifier("staccato"), SIGNAL(clicked()), _currentDocument, SLOT(ActionStaccato()));
    QObject::disconnect(_palette->ButtonForIdentifier("left"), SIGNAL(clicked()), _currentDocument, SLOT(ActionLeftHandSticking()));
    QObject::disconnect(_palette->ButtonForIdentifier("right"), SIGNAL(clicked()), _currentDocument, SLOT(ActionRightHandSticking()));

    QObject::disconnect(_palette->ButtonForIdentifier("brush_up"), SIGNAL(clicked()), _currentDocument, SLOT(ActionBrushUp()));
    QObject::disconnect(_palette->ButtonForIdentifier("brush_down"), SIGNAL(clicked()), _currentDocument, SLOT(ActionBrushDown()));
    QObject::disconnect(_palette->ButtonForIdentifier("arpeggio_up"), SIGNAL(clicked()), _currentDocument, SLOT(ActionArpeggioUp()));
    QObject::disconnect(_palette->ButtonForIdentifier("arpeggio_down"), SIGNAL(clicked()), _currentDocument, SLOT(ActionArpeggioDown()));
    QObject::disconnect(_palette->ButtonForIdentifier("dead"), SIGNAL(clicked()), _currentDocument, SLOT(ActionDeadNote()));
    QObject::disconnect(_palette->ButtonForIdentifier("pick_up"), SIGNAL(clicked()), _currentDocument, SLOT(ActionPickstrokeUp()));
    QObject::disconnect(_palette->ButtonForIdentifier("pick_down"), SIGNAL(clicked()), _currentDocument, SLOT(ActionPickstrokeDown()));

    QObject::disconnect(_palette->ButtonForIdentifier("hopo"), SIGNAL(clicked()), _currentDocument, SLOT(ActionHammerOnPullOff()));
    QObject::disconnect(_palette->ButtonForIdentifier("tap"), SIGNAL(clicked()), _currentDocument, SLOT(ActionTap()));
    QObject::disconnect(_palette->ButtonForIdentifier("slap"), SIGNAL(clicked()), _currentDocument, SLOT(ActionSlap()));
    QObject::disconnect(_palette->ButtonForIdentifier("pop"), SIGNAL(clicked()), _currentDocument, SLOT(ActionPop()));
    QObject::disconnect(_palette->ButtonForIdentifier("vibrato"), SIGNAL(clicked()), _currentDocument, SLOT(ActionVibrato()));
    QObject::disconnect(_palette->ButtonForIdentifier("palm_mute"), SIGNAL(clicked()), _currentDocument, SLOT(ActionPalmMute()));
    QObject::disconnect(_palette->ButtonForIdentifier("let_ring"), SIGNAL(clicked()), _currentDocument, SLOT(ActionLetRing()));

    QObject::disconnect(_palette->ButtonForIdentifier("bend"), SIGNAL(clicked()), _currentDocument, SLOT(ShowBendDialog()));
    QObject::disconnect(_palette->ButtonForIdentifier("slide"), SIGNAL(clicked()), _currentDocument, SLOT(ActionShiftSlide()));
    QObject::disconnect(_palette->ButtonForIdentifier("slide_out_high"), SIGNAL(clicked()), _currentDocument, SLOT(ActionSlideOutHigh()));
    QObject::disconnect(_palette->ButtonForIdentifier("slide_out_low"), SIGNAL(clicked()), _currentDocument, SLOT(ActionSlideOutLow()));
    QObject::disconnect(_palette->ButtonForIdentifier("slide_in_high"), SIGNAL(clicked()), _currentDocument, SLOT(ActionSlideInHigh()));
    QObject::disconnect(_palette->ButtonForIdentifier("slide_in_low"), SIGNAL(clicked()), _currentDocument, SLOT(ActionSlideInLow()));
}
