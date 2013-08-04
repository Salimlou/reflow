#ifndef REMIXERROWWIDGET_H
#define REMIXERROWWIDGET_H

#include <QWidget>

class REMixerWidget;
class REDocumentView;

namespace Ui {
class REMixerRowWidget;
}

class REMixerRowWidget : public QWidget
{
    Q_OBJECT
    
public:
    explicit REMixerRowWidget(REMixerWidget* parent, int trackIndex);
    ~REMixerRowWidget();

    REDocumentView* DocumentView();

protected slots:
    void on_viewButton_toggled(bool);
    void on_soloButton_toggled(bool);
    void on_muteButton_toggled(bool);
    void on_volumeSlider_valueChanged(int);
    void on_panSlider_valueChanged(int);
    
protected:
    void mouseDoubleClickEvent(QMouseEvent *);

private:
    Ui::REMixerRowWidget *ui;
    int _trackIndex;
};

#endif // REMIXERROWWIDGET_H
