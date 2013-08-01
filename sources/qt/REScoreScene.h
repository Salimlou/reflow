#ifndef RESCORESCENE_H
#define RESCORESCENE_H

#include <QGraphicsScene>

class REDocumentView;

class REScoreScene : public QGraphicsScene
{
    Q_OBJECT
public:
    explicit REScoreScene(REDocumentView* documentView, QObject *parent = 0);

protected:
    virtual void mousePressEvent (QGraphicsSceneMouseEvent * mouseEvent);
    virtual void mouseReleaseEvent (QGraphicsSceneMouseEvent * mouseEvent);
    virtual void mouseMoveEvent(QGraphicsSceneMouseEvent *mouseEvent);

    virtual void keyPressEvent(QKeyEvent *event);

protected:
    REDocumentView* _documentView;
};

#endif // RESCORESCENE_H
