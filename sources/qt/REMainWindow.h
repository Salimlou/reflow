#ifndef REMAINWINDOW_H
#define REMAINWINDOW_H

#include <QMainWindow>
#include <QListView>
#include <QPushButton>

#include <RETypes.h>

class REScoreScene;
class REQtPalette;
class REDocumentView;
class RESequencerWidget;
class RETransportWidget;
class REPianoWidget;

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

protected slots:
    void OnCurrentTabChanged(int newIndex);
    void OnCurrentDocumentPlaybackStarted();
    void OnCurrentDocumentPlaybackStopped();
    void OnCurrentDocumentStatusChanged();

    void CloseDocumentTab(int index);
    void RefreshInterfaceFromCurrentDocument();

protected:
    void ConnectToDocument();
    void DisconnectFromDocument();

    void UpdateWindowTitleFromCurrentDocument();

private:
    Ui::REMainWindow *ui;
    REDocumentView* _currentDocument;
    REQtPalette* _palette;
    RESequencerWidget* _sequencerWidget;
    RETransportWidget* _transportWidget;
    REPianoWidget* _pianoWidget;
    QListView* _partListView;
    QListView* _sectionListView;
	QAction* _undoAction;
	QAction* _redoAction;
    QAction* _playAction;
    QPushButton* _editLowVoiceButton;
};

#endif // REMAINWINDOW_H
