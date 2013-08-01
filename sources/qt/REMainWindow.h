#ifndef REMAINWINDOW_H
#define REMAINWINDOW_H

#include <QMainWindow>
#include <QListView>

#include <RETypes.h>

class REScoreScene;
class REQtPalette;
class REDocumentView;
class RESequencerWidget;

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

protected slots:
    void OnCurrentTabChanged(int newIndex);
    void OnCurrentDocumentPlaybackStarted();
    void OnCurrentDocumentPlaybackStopped();

protected:
    void ConnectToDocument();
    void DisconnectFromDocument();

private:
    Ui::REMainWindow *ui;
    REDocumentView* _currentDocument;
    REQtPalette* _palette;
    RESequencerWidget* _sequencerWidget;
    QListView* _partListView;
    QListView* _sectionListView;
	QAction* _undoAction;
	QAction* _redoAction;
    QAction* _playAction;
};

#endif // REMAINWINDOW_H