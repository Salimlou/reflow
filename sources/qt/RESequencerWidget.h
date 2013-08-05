#ifndef RESEQUENCERWIDGET_H
#define RESEQUENCERWIDGET_H

#include <QWidget>
#include <QScrollArea>
#include <QGraphicsScene>
#include <QGraphicsView>

class REDocumentView;
class REMixerHeaderWidget;
class REMixerWidget;
class RENavigatorHeaderWidget;
class RENavigatorScene;

class RESequencerWidget : public QWidget
{
    Q_OBJECT
public:
    explicit RESequencerWidget(QWidget *parent = 0);
    
    virtual QSize sizeHint() const;

signals:
    
public:
    void ConnectToDocument(REDocumentView*);
    void DisconnectFromDocument();

    void Refresh();

protected:
    virtual void resizeEvent(QResizeEvent * event);
    
protected:
    void updateLayout();

protected:
    REDocumentView* _documentView;

    REMixerHeaderWidget* _mixerHeader;

    QScrollArea* _mixerScrollArea;
    REMixerWidget* _mixerWidget;

    QScrollArea* _navigatorHeaderScrollArea;
    RENavigatorHeaderWidget* _navigatorHeader;

    RENavigatorScene* _navigatorScene;
    QGraphicsView* _navigatorView;

    int _headerHeight;
    int _mixerWidth;
};

#endif // RESEQUENCERWIDGET_H
