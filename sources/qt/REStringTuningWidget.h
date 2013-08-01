#ifndef RESTRINGTUNINGWIDGET_H
#define RESTRINGTUNINGWIDGET_H

#include <QWidget>

namespace Ui {
class REStringTuningWidget;
}

class REStringTuningWidget : public QWidget
{
    Q_OBJECT
    
public:
    explicit REStringTuningWidget(QWidget *parent = 0);
    ~REStringTuningWidget();
    
    void SetStringIndex(int string) {_string = string;}
    int StringIndex() const {return _string;}

    void SetPitch(int p);
    int Pitch() const {return _pitch;}

signals:
    void stringTuningChanged(int stringIndex, int newPitch);

protected:
    void UpdateUI();

protected slots:
    void on_tuneDownButton_clicked();
    void on_tuneUpButton_clicked();

private:
    Ui::REStringTuningWidget *ui;
    int _string;
    int _pitch;
};

#endif // RESTRINGTUNINGWIDGET_H
