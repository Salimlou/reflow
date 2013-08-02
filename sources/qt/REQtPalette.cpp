#include "REQtPalette.h"

#include <QPushButton>
#include <QPainter>

REQtPalette::REQtPalette(QWidget *parent) :
    QWidget(parent)
{
    CreateButtons();

    setMinimumWidth(200);
    setFocusPolicy(Qt::NoFocus);
}

REQtPalette::~REQtPalette()
{
}

void REQtPalette::paintEvent(QPaintEvent *)
{
    QPainter painter(this);

    painter.fillRect(this->rect(), QColor::fromRgb(0x33, 0x33, 0x33));
}

void REQtPalette::CreateButtons()
{
    QStringList ids;
    ids << "addchord" << "inschord" << "dupchord" << "delchord" << "addbar" << "insbar" << "delbar";
    ids << "time_signature" << "clef" << "key_signature" << "rehearsal" << "chord_name" << "diagram" << "text";
    ids << "end_repeat" << "systembreak" << "tempomarker" << "" << "" << "" << "";
    ids << "" << "" << "" << "" << "" << "" << "";

    ids << "2flat" << "flat" << "natural" << "sharp" << "2sharp" << "semitone_up" << "semitone_down";
    ids << "whole" << "half" << "quarter" << "8th" << "16th" << "32nd" << "64th";
    ids << "tuplet_2" << "tuplet_3" << "tuplet_4" << "tuplet_5" << "tuplet_6" << "tuplet_7" << "tuplet_9";
    ids << "dot" << "2dot" << "tied" << "splitchord" << "stem_up" << "stem_down" << "enharmonic";
    ids << "" << "" << "" << "" << "" << "" << "";

    ids << "ppp" << "pp" << "p" << "mp" << "ghost" << "accent" << "strong_accent";
    ids << "mf" << "f" << "ff" << "fff" << "staccato" << "left" << "right";
    ids << "brush_down" << "brush_up" << "arpeggio_down" << "arpeggio_up" << "dead" << "pick_up" << "pick_down";
    ids << "hopo" << "tap" << "slap" << "pop" << "vibrato" << "palm_mute" << "let_ring";
    ids << "bend" << "slide" << "slide_out_high" << "slide_out_low" << "slide_in_high" << "slide_in_low" << "";

    float dw = 24.0;

    for(int i=0; i<ids.count(); ++i)
    {
        int row = i/7;
        int col = i%7;
        float x = 4.0 + dw * col;
        float y = (4.0 + dw * row);
        QString ident = ids[i];
        if(ident.isEmpty()) continue;

        QString iconName = QString(":/palette_%1.png").arg(ident);
        QPushButton* btn = new QPushButton(QIcon(iconName), "", this);
        btn->setStyleSheet(QString("margin: 0px; padding: 0px; border: none;"));
        btn->setGeometry(x, y, 24, 24);
        btn->setFlat(true);
        btn->setFocusPolicy(Qt::NoFocus);
        _buttons[ident] = btn;
    }
}

QPushButton* REQtPalette::ButtonForIdentifier(const QString& id)
{
    if(_buttons.contains(id)) {
        return _buttons[id];
    }
    return NULL;
}
