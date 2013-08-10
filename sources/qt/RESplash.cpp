#include "RESplash.h"
#include "ui_RESplash.h"

#include <QPainter>
#include <QDesktopServices>
#include <QUrl>
#include <QTimer>

#include "RETypes.h"

const char* _labelStylesheet = "QLabel{font: bold 9px; text-align: left; color: #888888;}";
const char* _linkStylesheet = "QPushButton{font: bold 9px; text-align: left; color: #dadada;}";

RESplash::RESplash(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::RESplash)
{
    _image = QImage(":/splash.png");

    ui->setupUi(this);

    ui->forumsButton->setStyleSheet(_linkStylesheet);
    ui->wikiButton->setStyleSheet(_linkStylesheet);
    ui->websiteButton->setStyleSheet(_linkStylesheet);
    ui->blogButton->setStyleSheet(_linkStylesheet);

    ui->linksLabel->setStyleSheet(_labelStylesheet);
    ui->licenseLabel->setStyleSheet(_labelStylesheet);
    ui->copyrightLabel->setStyleSheet(_labelStylesheet);

    setMinimumSize(_image.size());
    setMaximumSize(_image.size());
    setWindowFlags(Qt::FramelessWindowHint|Qt::WindowStaysOnTopHint);

    QTimer *timer = new QTimer(this);
    connect(timer, SIGNAL(timeout()), this, SLOT(accept()));
    timer->setSingleShot(true);
    //timer->start(8000);
}

RESplash::~RESplash()
{
    delete ui;
}

void RESplash::paintEvent(QPaintEvent *)
{
    QPainter painter(this);

    painter.drawImage(QPoint(0,0), _image);
}

void RESplash::mousePressEvent(QMouseEvent *)
{
    done(QDialog::Accepted);
}

void RESplash::keyPressEvent(QKeyEvent *)
{
    done(QDialog::Accepted);
}

void RESplash::timerEvent(QTimerEvent *)
{
    done(0);
}

void RESplash::on_forumsButton_pressed()
{
    QDesktopServices::openUrl(QUrl(REFLOW_URL_REFLOW_FORUMS));
    done(0);
}

void RESplash::on_wikiButton_pressed()
{
    QDesktopServices::openUrl(QUrl(REFLOW_URL_REFLOW_WIKI));
    done(0);
}

void RESplash::on_blogButton_pressed()
{
    QDesktopServices::openUrl(QUrl(REFLOW_URL_REFLOW_DEVBLOG));
    done(0);
}

void RESplash::on_websiteButton_pressed()
{
    QDesktopServices::openUrl(QUrl(REFLOW_URL_REFLOW_WEBSITE));
    done(0);
}

void RESplash::on_facebookButton_pressed()
{
    QDesktopServices::openUrl(QUrl(REFLOW_URL_REFLOW_FACEBOOK));
    done(0);
}

void RESplash::on_twitterButton_pressed()
{
    QDesktopServices::openUrl(QUrl(REFLOW_URL_REFLOW_TWITTER));
    done(0);
}

void RESplash::on_githubButton_pressed()
{
    QDesktopServices::openUrl(QUrl(REFLOW_URL_REFLOW_GITHUB));
    done(0);
}
