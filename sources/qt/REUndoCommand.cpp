//
//  REUndoCommand.cpp
//  Reflow
//
//  Created by Sebastien Bourgeois on 12/12/12.
//
//

#include "REUndoCommand.h"

#include <REScoreController.h>
#include <RESongController.h>
#include <REOutputStream.h>
#include <REInputStream.h>

REScoreUndoCommand::REScoreUndoCommand(REScoreController* scoreController, const REScoreControllerOperation& op)
	: _scoreController(scoreController), _op(op)
{
    // Save complete song state for undo
	_scoreController->_BackupCommandDataToStream(_buffer);

    // Save cursor state for redo
    _scoreController->EncodeTo(_scoreControllerStateBuffer);
}

void REScoreUndoCommand::redo()
{
    // Restore cursor state before applying command
    REBufferInputStream input(_scoreControllerStateBuffer.Data(), _scoreControllerStateBuffer.Size());
    _scoreController->DecodeFrom(input);

    // Apply command
	_op(_scoreController);
}

void REScoreUndoCommand::undo()
{
	REBufferInputStream input(_buffer.Data(), _buffer.Size());
	_scoreController->_RestoreCommandDataFromStream(input);
}
