#ifndef RETRANSPORTWIDGET_H
#define RETRANSPORTWIDGET_H

#include <QWidget>

namespace Ui {
class RETransportWidget;
}

class RETransportWidget : public QWidget
{
    Q_OBJECT
    
public:
    explicit RETransportWidget(QWidget *parent = 0);
    ~RETransportWidget();
    
protected:
    void paintEvent(QPaintEvent *);

private:
    Ui::RETransportWidget *ui;
};

#endif // RETRANSPORTWIDGET_H
