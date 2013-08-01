#ifndef REQTPALETTE_H
#define REQTPALETTE_H

#include <QWidget>
#include <QHash>
#include <QPushButton>

class REQtPalette : public QWidget
{
    Q_OBJECT
    
    typedef QHash<QString, QPushButton*> ButtonMap;

public:
    explicit REQtPalette(QWidget *parent = 0);
    ~REQtPalette();

    QPushButton* ButtonForIdentifier(const QString& id);

protected:
    void CreateButtons();

    void paintEvent(QPaintEvent *);

protected:
    ButtonMap _buttons;
};

#endif // REQTPALETTE_H
