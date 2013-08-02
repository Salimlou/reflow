#ifndef REPIANOWIDGET_H
#define REPIANOWIDGET_H

#include "RETypes.h"

#include <QWidget>
#include <QImage>

class REDocumentView;

class REPianoWidget : public QWidget
{
    Q_OBJECT
public:
    explicit REPianoWidget(QWidget *parent = 0);
    
public slots:
    void RefreshDisplay();

public:
    void ConnectToDocument(REDocumentView* doc);
    void DisconnectFromDocument();

    int StepWithXOffset(float x) const;
    int OctaveWithXOffset(float x) const;
    int AlterationWithXYOffset(float x, float y) const;

    float XOffsetOfCenterOfPitch(unsigned int pitch) const;
    float YOffsetOfRoundOfPitch(unsigned int pitch) const;

    RERect RectForPitch(unsigned int pitch) const;

    float KeySpacing() const;
    float BlackKeyHeight() const;

protected:
    void paintEvent(QPaintEvent *);

private:
    REDocumentView* _documentView;
    QImage _backgroundImage;
    float _scaleX;
    float _offsetX;
};

#endif // REPIANOWIDGET_H
