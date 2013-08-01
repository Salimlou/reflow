#ifndef REMAINWINDOW_H
#define REMAINWINDOW_H

#include <QMainWindow>
#include <QListView>

#include <RETypes.h>

class REScoreScene;
class REQtPalette;
class REDocumentView;
class RESequencerWidget;
class RETransportWidget;

namespace Ui {
    class REMainWindow;
}

class REMainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit REMainWindow(QWidget *parent = 0);
    ~REMainWindow();

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
    QListView* _partListView;
    QListView* _sectionListView;
	QAction* _undoAction;
	QAction* _redoAction;
    QAction* _playAction;
};

#endif // REMAINWINDOW_H
