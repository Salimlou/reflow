//
//  REChordName.cpp
//  Reflow
//
//  Created by Sebastien Bourgeois on 16/01/12.
//  Copyright (c) 2012 Gargant Studios. All rights reserved.
//

#include "REChordName.h"
#include "REOutputStream.h"
#include "REInputStream.h"

REChordName::REChordName()
: _root(0,0)
{
}

REChordName::REChordName(const REPitchClass& root, const REChordFormula& formula)
: _root(root), _formula(formula)
{
}

const REPitchClass& REChordName::Root() const
{
    return _root;
}

const REPitchClass& REChordName::Tonic() const
{
    return _root;
}

const REChordFormula& REChordName::Formula() const
{
    return _formula;
}

int REChordName::DegreeCount() const
{
    return _formula.DegreeCount();
}

REPitchClass REChordName::Degree(int degree) const
{
    return _root + _formula.Degree(degree);
}

std::string REChordName::ToString() const
{
    std::string result = Root().Name() + Formula().Symbol();
    if(_inversion > 0 && _inversion < DegreeCount()) {
        result = result + "/" + Degree(_inversion).Name();
    }
    return result;
}

REChordName& REChordName::operator=(const REChordName& rhs)
{
    _root = rhs.Root();
    _formula = rhs.Formula();
    _inversion = rhs._inversion;
    return *this;
}

void REChordName::EncodeTo(REOutputStream& coder) const
{
    _root.EncodeTo(coder);
    _formula.EncodeTo(coder);
    coder.WriteInt8(_inversion);
}
void REChordName::DecodeFrom(REInputStream& decoder)
{
    _root.DecodeFrom(decoder);
    _formula.DecodeFrom(decoder);
    _inversion = decoder.ReadInt8();
}