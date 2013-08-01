//
//  REPlaylistCompiler.cpp
//  Reflow
//
//  Created by Sebastien Bourgeois on 29/01/12.
//  Copyright (c) 2012 Gargant Studios. All rights reserved.
//

#include "REPlaylistCompiler.h"
#include "RESong.h"
#include "REBar.h"

REPlaylistCompiler::REPlaylistCompiler() 
: _song(NULL)
{
}

void REPlaylistCompiler::Compile(const RESong *song, RESongErrorVector& errors)
{
    _song = song;
    _playlist.clear();
    _playlist.reserve(4 * _song->BarCount());
    
    _barIndex = 0;
    _barCount = _song->BarCount();
    _finished = false;
    _repeatStartBarIndex = 0;
    _justJumpedFromEndRepeat = false;
    _iteration = 0;
    
    _jumpAtCoda = false;
    _jumpAtDoubleCoda = false;
    _stopsAtFine = false;
    
    // Reset symbols
    for(int i=0; i<Reflow::DirectionJump_Count; ++i) {_barOfDirectionJump[i] = -1;}
    for(int i=0; i<Reflow::DirectionTarget_Count; ++i) {_barOfDirectionTarget[i] = -1;}
    
    // Find musical directions in the score
    for(int barIndex=0; barIndex < _barCount; ++barIndex)
    {
        const REBar* bar = _song->Bar(barIndex);
        
        // Jumps
        for(int i=0; i<Reflow::DirectionJump_Count; ++i) {
            if(bar->HasDirectionJump((Reflow::DirectionJump)i)) {
                if(_barOfDirectionJump[i] == -1)
                {
                    _barOfDirectionJump[i] = barIndex;
                }
                else {
                    REPrintf("Duplicate direction jump: %d\n", i);
                }
            }
        }
        
        // Target symbols
        for(int i=0; i<Reflow::DirectionTarget_Count; ++i) {
            if(bar->HasDirectionTarget((Reflow::DirectionTarget)i)) {
                if(_barOfDirectionTarget[i] == -1)
                {
                    _barOfDirectionTarget[i] = barIndex;
                }
                else {
                    REPrintf("Duplicate direction target: %d, keeping the first one\n", i);
                }
            }
        }
    }
    
    // Process to compilation
    _Compile();
}

bool REPlaylistCompiler::_HasDirectionTarget(Reflow::DirectionTarget target) const
{
    return _barOfDirectionTarget[target] != -1;
}

bool REPlaylistCompiler::_IsJumpInstructionConsumed(Reflow::DirectionJump jump) const
{
    return _barOfDirectionJump[jump] == -1;
}

void REPlaylistCompiler::_ConsumeJumpInstruction(Reflow::DirectionJump jump)
{
    _barOfDirectionJump[jump] = -1;
}

bool REPlaylistCompiler::_HasCurrentBarJumpInstruction(Reflow::DirectionJump jump) const
{
    const REBar* bar = _song->Bar(_barIndex);
    return !_IsJumpInstructionConsumed(jump) && bar->HasDirectionJump(jump);
}

void REPlaylistCompiler::_JumpToBar(int barIndex)
{
    _barIndex = barIndex;
    _iteration = 0;
    _justJumpedFromEndRepeat = false;
}

void REPlaylistCompiler::_Compile()
{
    //REPrintf("compiling song playlist...\n");
    while(!_finished && _barIndex < _barCount)
    {
        const REBar* bar = _song->Bar(_barIndex);
        bool hasRepeatEnd = bar->HasFlag(REBar::RepeatEnd);
        bool hasRepeatStart = bar->HasFlag(REBar::RepeatStart);
        bool hasAlternateEnding = bar->HasAnyAlternateEnding();
        //REPrintf(" > processing bar %d\n", _barIndex+1);
        
        
        
        if(hasAlternateEnding)
        {
            if(bar->HasAlternateEnding(_iteration)) {
                // Play this bar
                _playlist.push_back(_barIndex);
            }
            else {
                // Move to the next one ...
                ++_barIndex;
                _justJumpedFromEndRepeat = false;
                
                // ... and skip
                continue;
            }
        }
        else {
            // Play this bar
            _playlist.push_back(_barIndex);
        }
        
        
        
        
        
        // Did we enter in a repeat start bar without following a repeat start jump ?
        if(hasRepeatStart && !_justJumpedFromEndRepeat)
        {
            // Start a new iteration
            //REPrintf("  .. starting new iteration with start bar (%d)\n", _repeatStartBarIndex+1);
            _iteration = 0;
            _repeatStartBarIndex = _barIndex;
        }
        
        // Repeat end ?
        if(hasRepeatEnd)
        {
            // Jump to the repeat start (if count is not exceeded)
            int lastIteration = std::max<int>(2, bar->RepeatCount()) - 1;
            
            //REPrintf("  .. end repeat (iteration: %d, last iteration: %d)\n", _iteration, lastIteration);
            if(hasAlternateEnding)
            {
                // Alternate ending on repeat bar end ignores repeat count (see unit test for explanation)
                _barIndex = _repeatStartBarIndex;
                ++_iteration;
                _justJumpedFromEndRepeat = true;
            }
            else if(_iteration >= lastIteration)
            {
                // Do not jump, move to the next one
                ++_barIndex;
                _justJumpedFromEndRepeat = false;
            }
            else {
                // Jump to the last found repeat start bar
                _barIndex = _repeatStartBarIndex;
                ++_iteration;
                _justJumpedFromEndRepeat = true;
            }
        }
        
        
        // Da Capo Musical Jumps
        // ------------------------------------------------------------------
        else if(_HasCurrentBarJumpInstruction(Reflow::DaCapo))
        {
            // Jump to the beginning
            _JumpToBar(0);
            _ConsumeJumpInstruction(Reflow::DaCapo);
        }
        else if(_HasCurrentBarJumpInstruction(Reflow::DaCapo_AlFine))
        {
            // Jump to the beginning
            _JumpToBar(0);
            _stopsAtFine = true;
            _ConsumeJumpInstruction(Reflow::DaCapo_AlFine);
        }
        else if(_HasCurrentBarJumpInstruction(Reflow::DaCapo_AlCoda))
        {
            // Jump to the beginning
            _JumpToBar(0);
            _jumpAtCoda = true;
            _ConsumeJumpInstruction(Reflow::DaCapo_AlCoda);
        }
        else if(_HasCurrentBarJumpInstruction(Reflow::DaCapo_AlDoubleCoda))
        {
            // Jump to the beginning
            _JumpToBar(0);
            _jumpAtDoubleCoda = true;
            _ConsumeJumpInstruction(Reflow::DaCapo_AlDoubleCoda);
        }
        
        
        // Dal Segno Musical Jumps
        // ------------------------------------------------------------------
        else if(_HasCurrentBarJumpInstruction(Reflow::DalSegno) && _HasDirectionTarget(Reflow::Segno))
        {
            // Jump to the segno
            _JumpToBar(_barOfDirectionTarget[Reflow::Segno]);
            _ConsumeJumpInstruction(Reflow::DalSegno);
        }
        else if(_HasCurrentBarJumpInstruction(Reflow::DalSegno_AlFine) && _HasDirectionTarget(Reflow::Segno))
        {
            // Jump to the segno
            _JumpToBar(_barOfDirectionTarget[Reflow::Segno]);
            _stopsAtFine = true;
            _ConsumeJumpInstruction(Reflow::DalSegno_AlFine);
        }
        else if(_HasCurrentBarJumpInstruction(Reflow::DalSegno_AlCoda) && _HasDirectionTarget(Reflow::Segno))
        {
            // Jump to the segno
            _JumpToBar(_barOfDirectionTarget[Reflow::Segno]);
            _jumpAtCoda = true;
            _ConsumeJumpInstruction(Reflow::DalSegno_AlCoda);
        }
        else if(_HasCurrentBarJumpInstruction(Reflow::DalSegno_AlDoubleCoda) && _HasDirectionTarget(Reflow::Segno))
        {
            // Jump to the segno
            _JumpToBar(_barOfDirectionTarget[Reflow::Segno]);
            _jumpAtDoubleCoda = true;
            _ConsumeJumpInstruction(Reflow::DalSegno_AlDoubleCoda);
        }
        
        
        // Dal Segno Segno Musical Jumps
        // ------------------------------------------------------------------
        else if(_HasCurrentBarJumpInstruction(Reflow::DalSegnoSegno) && _HasDirectionTarget(Reflow::SegnoSegno))
        {
            // Jump to the segno segno
            _JumpToBar(_barOfDirectionTarget[Reflow::SegnoSegno]);
            _ConsumeJumpInstruction(Reflow::DalSegnoSegno);
        }
        else if(_HasCurrentBarJumpInstruction(Reflow::DalSegnoSegno_AlFine) && _HasDirectionTarget(Reflow::SegnoSegno))
        {
            // Jump to the segno segno
            _JumpToBar(_barOfDirectionTarget[Reflow::SegnoSegno]);
            _stopsAtFine = true;
            _ConsumeJumpInstruction(Reflow::DalSegnoSegno_AlFine);
        }
        else if(_HasCurrentBarJumpInstruction(Reflow::DalSegnoSegno_AlCoda) && _HasDirectionTarget(Reflow::SegnoSegno))
        {
            // Jump to the segno segno
            _JumpToBar(_barOfDirectionTarget[Reflow::SegnoSegno]);
            _jumpAtCoda = true;
            _ConsumeJumpInstruction(Reflow::DalSegnoSegno_AlCoda);
        }
        else if(_HasCurrentBarJumpInstruction(Reflow::DalSegnoSegno_AlDoubleCoda) && _HasDirectionTarget(Reflow::SegnoSegno))
        {
            // Jump to the segno segno
            _JumpToBar(_barOfDirectionTarget[Reflow::SegnoSegno]);
            _jumpAtDoubleCoda = true;
            _ConsumeJumpInstruction(Reflow::DalSegnoSegno_AlDoubleCoda);
        }
        
        
        //  To Coda / To Double Coda Musical Jumps
        // ------------------------------------------------------------------
        else if(_jumpAtCoda && _HasCurrentBarJumpInstruction(Reflow::ToCoda) && _HasDirectionTarget(Reflow::Coda))
        {
            // Jump to the Coda sign
            _JumpToBar(_barOfDirectionTarget[Reflow::Coda]);
            _jumpAtCoda = false;
            _ConsumeJumpInstruction(Reflow::ToCoda);
        }
        else if(_jumpAtDoubleCoda && _HasCurrentBarJumpInstruction(Reflow::ToDoubleCoda) && _HasDirectionTarget(Reflow::DoubleCoda))
        {
            // Jump to the Double Coda sign
            _JumpToBar(_barOfDirectionTarget[Reflow::DoubleCoda]);
            _jumpAtDoubleCoda = false;
            _ConsumeJumpInstruction(Reflow::ToDoubleCoda);
        }
        
        
        //  Fine Stop condition
        // ------------------------------------------------------------------
        else if(_stopsAtFine && bar->HasDirectionTarget(Reflow::Fine))
        {
            _finished = true;
        }
        
        // ------------------------------------------------------------------
        else 
        {
            // Move to the next one
            ++_barIndex;
            _justJumpedFromEndRepeat = false;            
        }
        
    }
    //REPrintf("finished compiling (%d playlist bars)\n", (int)_playlist.size());
}
