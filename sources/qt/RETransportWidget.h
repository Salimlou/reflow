#ifndef RETRANSPORTWIDGET_H
#define RETRANSPORTWIDGET_H

#include <QWidget>

class REDocumentView;

namespace Ui {
class RETransportWidget;
}

class RETransportWidget : public QWidget
{
    Q_OBJECT
    
public:
    explicit RETransportWidget(QWidget *parent = 0);
    ~RETransportWidget();
    
public:
    void ConnectToDocument(REDocumentView*);
    void DisconnectFromDocument();

protected:
    void paintEvent(QPaintEvent *);

protected slots:
    void OnPlaybackStarted();
    void OnPlaybackStopped();

    void on_playButton_toggled(bool);
    void on_loopButton_toggled(bool);
    void on_metronomeButton_toggled(bool);
    void on_preclickButton_toggled(bool);

    void on_loopStartButton_pressed();
    void on_loopEndButton_pressed();

private:
    Ui::RETransportWidget *ui;
    REDocumentView* _documentView;
};

#endif // RETRANSPORTWIDGET_H
