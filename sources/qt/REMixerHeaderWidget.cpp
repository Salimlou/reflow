#include "REMixerHeaderWidget.h"
#include "ui_REMixerHeaderWidget.h"

#include <QPainter>

REMixerHeaderWidget::REMixerHeaderWidget(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::REMixerHeaderWidget)
{
    ui->setupUi(this);
}

REMixerHeaderWidget::~REMixerHeaderWidget()
{
    delete ui;
}

void REMixerHeaderWidget::paintEvent(QPaintEvent *)
{
    QPainter p(this);

    QLinearGradient linearGrad(QPointF(0, 0), QPointF(0, 30));
    linearGrad.setColorAt(0, QColor::fromRgb(90, 90, 90));
    linearGrad.setColorAt(1, QColor::fromRgb(0, 0, 0));

    p.fillRect(rect(), QBrush(linearGrad));
}
