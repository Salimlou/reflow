//
//  REPlaylistCompiler.h
//  Reflow
//
//  Created by Sebastien Bourgeois on 29/01/12.
//  Copyright (c) 2012 Gargant Studios. All rights reserved.
//

#ifndef Reflow_REPlaylistCompiler_h
#define Reflow_REPlaylistCompiler_h

#include "RETypes.h"
#include "RESongError.h"

class REPlaylistCompiler
{
    friend class RESong;
    
public:
    REPlaylistCompiler();
    
    void Compile(const RESong* song, RESongErrorVector& errors);
    
    const std::vector<int>& Playlist() const {return _playlist;}
    
private:
    void _Compile();
    
    bool _IsJumpInstructionConsumed(Reflow::DirectionJump) const;
    void _ConsumeJumpInstruction(Reflow::DirectionJump);
    
    bool _HasDirectionTarget(Reflow::DirectionTarget) const;
    
    bool _HasCurrentBarJumpInstruction(Reflow::DirectionJump jump) const;
    void _JumpToBar(int barIndex);
    
private:
    const RESong* _song;
    std::vector<int> _playlist;
    int _barIndex;
    int _barCount;
    int _iteration;
    int _repeatStartBarIndex;
    bool _finished;
    bool _justJumpedFromEndRepeat;
    bool _jumpAtCoda;                   // true when we walk on a [DC,DS,DSS] al Coda
    bool _jumpAtDoubleCoda;             // true when we walk on a [DC,DS,DSS] al Double Coda 
    bool _stopsAtFine;                  // true when we walk on a [DC,DS,DSS] al Fine
    int _barOfDirectionTarget[Reflow::DirectionTarget_Count];
    int _barOfDirectionJump[Reflow::DirectionJump_Count];
};

#endif
