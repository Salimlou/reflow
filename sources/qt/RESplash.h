#ifndef RESPLASH_H
#define RESPLASH_H

#include <QDialog>

namespace Ui {
class RESplash;
}

class RESplash : public QDialog
{
    Q_OBJECT
    
public:
    explicit RESplash(QWidget *parent = 0);
    ~RESplash();
    
protected:
    void paintEvent(QPaintEvent *);
    void mousePressEvent(QMouseEvent *);
    void keyPressEvent(QKeyEvent *);
    void timerEvent(QTimerEvent *);

protected slots:
    void on_forumsButton_pressed();
    void on_wikiButton_pressed();
    void on_blogButton_pressed();
    void on_websiteButton_pressed();

    void on_facebookButton_pressed();
    void on_twitterButton_pressed();
    void on_githubButton_pressed();

private:
    QImage _image;
    Ui::RESplash *ui;
};

#endif // RESPLASH_H
