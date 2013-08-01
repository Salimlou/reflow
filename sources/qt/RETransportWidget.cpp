#include "RETransportWidget.h"
#include "ui_RETransportWidget.h"

#include <QPainter>

RETransportWidget::RETransportWidget(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::RETransportWidget)
{
    ui->setupUi(this);

    setMinimumHeight(88);
    setMaximumHeight(88);
}

RETransportWidget::~RETransportWidget()
{
    delete ui;
}

void RETransportWidget::paintEvent(QPaintEvent *)
{
    QPainter p(this);
    p.fillRect(rect(), QBrush(QColor::fromRgb(0x33, 0x33, 0x33)));
}
