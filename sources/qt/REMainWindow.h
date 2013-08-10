#ifndef REMAINWINDOW_H
#define REMAINWINDOW_H

#include <QMainWindow>
#include <QListView>
#include <QComboBox>
#include <QPushButton>
#include <QNetworkReply>

#include <RETypes.h>

class REScoreScene;
class REQtPalette;
class REDocumentView;
class RESequencerWidget;
class RETransportWidget;
class REPianoWidget;
class REFretboardWidget;

namespace Ui {
    class REMainWindow;
}

class REMainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit REMainWindow(QWidget *parent = 0);
    ~REMainWindow();

    REDocumentView* CurrentDocumentView() const {return _currentDocument;}

public slots:
    void ActionNew();
    void ActionOpen();
    void ActionOpen(QString filename);
    void ActionClose();

    void CheckUpdatesInBackground();

protected slots:
    void OnCurrentTabChanged(int newIndex);
    void OnCurrentDocumentPlaybackStarted();
    void OnCurrentDocumentPlaybackStopped();
    void OnCurrentDocumentStatusChanged();

    void CloseDocumentTab(int index);
    void RefreshInterfaceFromCurrentDocument();

    void CheckUpdateFinished(QNetworkReply*);

    void on_actionPreferences_triggered();

protected:
    void ConnectToDocument();
    void DisconnectFromDocument();

    void UpdateWindowTitleFromCurrentDocument();

protected:
    virtual void closeEvent(QCloseEvent *);

private:
    Ui::REMainWindow *ui;
    REDocumentView* _currentDocument;
    REQtPalette* _palette;
    RESequencerWidget* _sequencerWidget;
    RETransportWidget* _transportWidget;
    REPianoWidget* _pianoWidget;
    REFretboardWidget* _fretboardWidget;
    QListView* _partListView;
    QListView* _sectionListView;
	QAction* _undoAction;
	QAction* _redoAction;
    QAction* _playAction;
    QPushButton* _editLowVoiceButton;
    QComboBox* _zoomCombo;
};

#endif // REMAINWINDOW_H
