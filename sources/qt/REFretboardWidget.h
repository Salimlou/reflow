#ifndef REFRETBOARDWIDGET_H
#define REFRETBOARDWIDGET_H

#include <QWidget>

class REDocumentView;

class REFretboardWidget : public QWidget
{
    Q_OBJECT
public:
    explicit REFretboardWidget(QWidget *parent = 0);

public slots:
    void RefreshDisplay();

public:
    void ConnectToDocument(REDocumentView* doc);
    void DisconnectFromDocument();
    
    int stringWithYOffset(float y) const;
    float yOffsetOfString(int string) const;
    int fretWithXOffset(float x) const;
    float xOffsetOfFret(int fret) const;
    float xOffsetOfCenterOfFret(int fret) const;

protected:
    void paintEvent(QPaintEvent *);
    void mousePressEvent(QMouseEvent *);

    void grayOut(QPainter& painter);

private:
    REDocumentView* _documentView;
    int _stringCount;
};

#endif // REFRETBOARDWIDGET_H
