//
//  REUndoCommand.h
//  Reflow
//
//  Created by Sebastien Bourgeois on 12/12/12.
//
//

#ifndef _REUNDOCOMMAND_H_
#define _REUNDOCOMMAND_H_

#include <RETypes.h>
#include <QUndoCommand>

#include <REOutputStream.h>

class REScoreUndoCommand : public QUndoCommand
{
public:
    REScoreUndoCommand(REScoreController* scoreController, const REScoreControllerOperation& op);

	virtual void redo();
	virtual void undo();

protected:
	REScoreController* _scoreController;
	REScoreControllerOperation _op;
	REBufferOutputStream _buffer;
    REBufferOutputStream _scoreControllerStateBuffer;
};

#endif
