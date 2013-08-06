#include "REScoreScene.h"
#include "REDocumentView.h"

#include "REScoreController.h"

#include <QGraphicsSceneMouseEvent>
#include <QKeyEvent>

#include <QDebug>

REScoreScene::REScoreScene(REDocumentView* documentView, QObject *parent) :
    QGraphicsScene(parent), _documentView(documentView)
{
}

void REScoreScene::mousePressEvent (QGraphicsSceneMouseEvent * mouseEvent)
{
    REScoreController* scoreController = _documentView->ScoreController();
    REPoint scenePos = REPoint(mouseEvent->scenePos());
    unsigned long flags = 0;
    if(mouseEvent->modifiers() & Qt::ShiftModifier) flags |= REScoreController::CursorShiftDown;

    scoreController->MouseDown(scenePos, flags);
}

void REScoreScene::mouseReleaseEvent (QGraphicsSceneMouseEvent * mouseEvent)
{
    REScoreController* scoreController = _documentView->ScoreController();
    REPoint scenePos = REPoint(mouseEvent->scenePos());
    unsigned long flags = 0;
    if(mouseEvent->modifiers() & Qt::ShiftModifier) flags |= REScoreController::CursorShiftDown;

    scoreController->MouseUp(scenePos, flags);
}

void REScoreScene::mouseMoveEvent(QGraphicsSceneMouseEvent *mouseEvent)
{
    REScoreController* scoreController = _documentView->ScoreController();
    REPoint scenePos = REPoint(mouseEvent->scenePos());
    unsigned long flags = 0;
    if(mouseEvent->modifiers() & Qt::ShiftModifier) flags |= REScoreController::CursorShiftDown;

    if(mouseEvent->buttons() == Qt::NoButton)
    {
    }
    else
    {
        scoreController->MouseDragged(scenePos, flags);
    }
}

void REScoreScene::keyPressEvent(QKeyEvent *event)
{
    REScoreController* scoreController = _documentView->ScoreController();

    int keyCode = event->key();
    bool shiftDown = (0 != (event->modifiers() & Qt::ShiftModifier));
    bool cmdDown = (0 != (event->modifiers() & Qt::ControlModifier));
    bool altDown = (0 != (event->modifiers() & Qt::AltModifier));
    qDebug() << "keyDown: " << (shiftDown ? "shift + " : "") << (int)keyCode;

    bool consumed = false;

    QString characters = event->text();

    if     (keyCode == Qt::Key_1 || characters.startsWith("1")) {_documentView->ActionTypeKeypad(1, altDown); consumed=true;}
    else if(keyCode == Qt::Key_2 || characters.startsWith("2")) {_documentView->ActionTypeKeypad(2, altDown); consumed=true;}
    else if(keyCode == Qt::Key_3 || characters.startsWith("3")) {_documentView->ActionTypeKeypad(3, altDown); consumed=true;}
    else if(keyCode == Qt::Key_4 || characters.startsWith("4")) {_documentView->ActionTypeKeypad(4, altDown); consumed=true;}
    else if(keyCode == Qt::Key_5 || characters.startsWith("5")) {_documentView->ActionTypeKeypad(5, altDown); consumed=true;}
    else if(keyCode == Qt::Key_6 || characters.startsWith("6")) {_documentView->ActionTypeKeypad(6, altDown); consumed=true;}
    else if(keyCode == Qt::Key_7 || characters.startsWith("7")) {_documentView->ActionTypeKeypad(7, altDown); consumed=true;}
    else if(keyCode == Qt::Key_8 || characters.startsWith("8")) {_documentView->ActionTypeKeypad(8, altDown); consumed=true;}
    else if(keyCode == Qt::Key_9 || characters.startsWith("9")) {_documentView->ActionTypeKeypad(9, altDown); consumed=true;}
    else if(keyCode == Qt::Key_0 || characters.startsWith("0")) {_documentView->ActionTypeKeypad(0, altDown); consumed=true;}

    if(!altDown)
    {
        if(characters.startsWith("+")) {_documentView->ActionDecreaseDuration(); consumed = true;}
        if(characters.startsWith("-")) {_documentView->ActionIncreaseDuration(); consumed = true;}
    }

    unsigned long moveFlags = 0;
    if(shiftDown) moveFlags |= REScoreController::CursorShiftDown;
    if(cmdDown) moveFlags |= REScoreController::CursorCmdDown;
    switch (keyCode)
    {
        case Qt::Key_twosuperior:
        {
            _documentView->SetEditLowVoice(!_documentView->IsEditingLowVoice());
            consumed = true;
            break;
        }

        case Qt::Key_Space: {
			_documentView->TogglePlayback();
            consumed=true;
            break;
        }

        case Qt::Key_Left: {
            scoreController->MoveCursorLeft(moveFlags);
            consumed=true;
            break;
        }
        case Qt::Key_Right: {
            scoreController->MoveCursorRight(moveFlags);
            consumed=true;
            break;
        }
        case Qt::Key_Down: {
            scoreController->MoveCursorDown(moveFlags);
            consumed=true;
            break;
        }
        case Qt::Key_Up: {
            scoreController->MoveCursorUp(moveFlags);
            consumed=true;
            break;
        }
        case Qt::Key_Backspace:
        case Qt::Key_Delete: {
            _documentView->ActionDelete();
            consumed=true;
            break;
        }

        case Qt::Key_Slash: {
            _documentView->ActionSplitChord();
            consumed=true;
            break;
        }

        case Qt::Key_Period: {
            _documentView->ActionDottedNote();
            consumed = true;
            break;
        }
    }

    if(consumed) event->accept();
}
