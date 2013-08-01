#include "RESequencerWidget.h"
#include "REMixerHeaderWidget.h"
#include "REMixerWidget.h"
#include "RENavigatorHeaderWidget.h"
#include "RENavigatorScene.h"
#include "REDocumentView.h"

#include <QDebug>
#include <QLabel>
#include <QScrollArea>
#include <QScrollBar>

RESequencerWidget::RESequencerWidget(QWidget *parent) :
    QWidget(parent), _headerHeight(32), _mixerWidth(350), _documentView(nullptr)
{
    _mixerHeader = new REMixerHeaderWidget(this);
    _mixerHeader->setStyleSheet("QLabel { color: #dadada; }");

    _mixerScrollArea = new QScrollArea(this);
    _mixerScrollArea->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    _mixerScrollArea->setStyleSheet( "QScrollArea { border-style: none; background: #fefefe; }" );
    _mixerWidget = new REMixerWidget;
    _mixerWidget->setGeometry(0, 0, _mixerWidth, 400);
    _mixerScrollArea->setWidget(_mixerWidget);

    _navigatorHeaderScrollArea = new QScrollArea(this);
    _navigatorHeaderScrollArea->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    _navigatorHeaderScrollArea->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    _navigatorHeaderScrollArea->setStyleSheet( "QScrollArea { border-style: none; }" );
    _navigatorHeader = new RENavigatorHeaderWidget;
    _navigatorHeader->setGeometry(0, 0, 1200, _headerHeight);
    _navigatorHeaderScrollArea->setWidget(_navigatorHeader);

    _navigatorScene = new RENavigatorScene(this);
    _navigatorScene->setSceneRect(0, 0, 1200, 400);
    _navigatorScene->addRect(QRectF(0, 0, 150, 20), QPen(Qt::black), QBrush(Qt::gray));
    _navigatorView = new QGraphicsView(_navigatorScene, this);
    _navigatorView->setStyleSheet( "QGraphicsView { border-style: none; background: #333333; }" );
    _navigatorView->setAlignment(Qt::AlignTop|Qt::AlignLeft);

    QObject::connect(_navigatorHeaderScrollArea->horizontalScrollBar(), SIGNAL(valueChanged(int)), _navigatorView->horizontalScrollBar(), SLOT(setValue(int)));
    QObject::connect(_navigatorView->horizontalScrollBar(), SIGNAL(valueChanged(int)), _navigatorHeaderScrollArea->horizontalScrollBar(), SLOT(setValue(int)));

    QObject::connect(_mixerScrollArea->verticalScrollBar(), SIGNAL(valueChanged(int)), _navigatorView->verticalScrollBar(), SLOT(setValue(int)));
    QObject::connect(_navigatorView->verticalScrollBar(), SIGNAL(valueChanged(int)), _mixerScrollArea->verticalScrollBar(), SLOT(setValue(int)));
}

QSize RESequencerWidget::sizeHint() const
{
    return QSize(1280, 62);
}

void RESequencerWidget::resizeEvent(QResizeEvent * event)
{
    QWidget::resizeEvent(event);

    qDebug() << "resizeEvent to " << geometry();
    updateLayout();
}

void RESequencerWidget::updateLayout()
{
    _mixerHeader->setVisible(_documentView != nullptr);
    _mixerScrollArea->setVisible(_documentView != nullptr);
    _navigatorHeaderScrollArea->setVisible(_documentView != nullptr);
    _navigatorView->setVisible(_documentView != nullptr);

    if(!_documentView) return;

    QRect rc = geometry();

    int navigatorWidth = _navigatorHeader->TotalWidth();
    int navigatorHeight = _navigatorScene->TotalHeight();

    _mixerHeader->setGeometry(0, 0, _mixerWidth, _headerHeight);
    _mixerScrollArea->setGeometry(0, _headerHeight, _mixerWidth, rc.height() - _headerHeight);
    _mixerWidget->resize(_mixerWidth, navigatorHeight);

    _navigatorHeaderScrollArea->setGeometry(_mixerWidth, 0, rc.width() - _mixerWidth, _headerHeight);
    _navigatorHeader->resize(qMax(navigatorWidth, rc.width() - _mixerWidth), _headerHeight);

    _navigatorView->setGeometry(_mixerWidth, _headerHeight, rc.width() - _mixerWidth, rc.height()-_headerHeight);
    _navigatorScene->setSceneRect(0, 0, navigatorWidth, navigatorHeight);
}

void RESequencerWidget::ConnectToDocument(REDocumentView* doc)
{
    _documentView = doc;

    _navigatorHeader->SetDocumentView(doc);
    _navigatorScene->SetDocumentView(doc);
    _mixerWidget->SetDocumentView(doc);

    updateLayout();
}

void RESequencerWidget::DisconnectFromDocument()
{
    _documentView = nullptr;

    _navigatorHeader->SetDocumentView(nullptr);
    _navigatorScene->SetDocumentView(nullptr);
    _mixerWidget->SetDocumentView(nullptr);

    updateLayout();
}
