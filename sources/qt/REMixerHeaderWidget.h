#ifndef REMIXERHEADERWIDGET_H
#define REMIXERHEADERWIDGET_H

#include <QWidget>

namespace Ui {
class REMixerHeaderWidget;
}

class REMixerHeaderWidget : public QWidget
{
    Q_OBJECT
    
public:
    explicit REMixerHeaderWidget(QWidget *parent = 0);
    ~REMixerHeaderWidget();
    
protected:
    void paintEvent(QPaintEvent *);

private:
    Ui::REMixerHeaderWidget *ui;
};

#endif // REMIXERHEADERWIDGET_H
