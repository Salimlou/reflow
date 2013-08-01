#ifndef RESCORESCENEVIEW_H
#define RESCORESCENEVIEW_H

#include <QGraphicsView>

class REScoreScene;

class REScoreSceneView : public QGraphicsView
{
    Q_OBJECT
public:
    explicit REScoreSceneView(REScoreScene* scene, QWidget *parent = 0);
    
signals:
    
public slots:
    
};

#endif // RESCORESCENEVIEW_H
