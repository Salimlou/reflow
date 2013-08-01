//
//  REPitch.cpp
//  Reflow
//
//  Created by Sebastien on 12/12/12.
//
//

#include "REPitch.h"
#include "REFunctions.h"

#include <sstream>

using namespace std;

static int _DeltaOctave[7][12] =
{   {  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  1,  1 },
    {  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0 },
    {  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0 },
    {  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0 },
    {  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0 },
    {  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0 },
    { -1, -1,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0 } };

int _NormalizedDiatonicStepWithChromatic[4][12] = {
    /* C   C#  D   D#  E   F   F#  G   G#  A   A#  B */
    {  0,  0,  1,  1,  2,  3,  3,  4,  4,  5,  5,  6  },

    /* C   Db  D   Eb  E   F   Gb  G   Ab  A   Bb  B */
    {  0,  1,  1,  2,  2,  3,  4,  4,  5,  5,  6,  6  },
    
    /* B#  Bx  Cx  D#  Dx  E#  Ex  Fx  G#  Gx  A#  Ax */
    {  6,  6,  0,  1,  1,  2,  2,  3,  4,  4,  5,  5  },
    
    /* Dbb Db  Ebb Fbb Fb  Gbb Gb  Abb Ab  Bbb Cbb Cb */
    {  1,  1,  2,  3,  3,  4,  4,  5,  5,  6,  0,  0  }
};


REPitch::REPitch()
: _midi(0), _flags(0), _class(REPitchClass::C)
{
}

REPitch::REPitch(int midi)
: _midi(midi), _flags(0)
{
    _class._chromatic = midi % 12;
    _class = _class.Normalized();
}

REPitch::REPitch(const REPitchClass& pitchClass, int octave)
: _flags(0)
{
    _class = (pitchClass.IsValid() ? pitchClass : pitchClass.Normalized());
    int deltaOctave = _DeltaOctave[_class.DiatonicStep()][_class.ChromaticStep()];
    _midi = (octave - deltaOctave) * 12 + _class.ChromaticStep();
    
    switch(_class.DiatonicAlteration()) {
        case -2: _flags = REPitch::UseFlats | REPitch::UseAlternateName; break;
        case -1: _flags = REPitch::UseFlats; break;
        case 2: _flags = REPitch::UseAlternateName; break;
    }
}

int REPitch::Octave() const
{
    int octave = _midi / 12;
    int deltaOctave = _DeltaOctave[_class.DiatonicStep()][_class.ChromaticStep()];
    return octave + deltaOctave;
}

std::string REPitch::Name() const
{
    ostringstream oss;
    oss << _class.Name() << (Octave() - 5);
    return oss.str();
}

REPitch REPitch::operator-(const REPitchClass& pc) const
{
    REPitch newPitch = *this;
    newPitch._midi = _midi - pc.ChromaticStep();
    newPitch._class._chromatic = Reflow::Wrap12(_class.ChromaticStep() - pc.ChromaticStep());
    newPitch._class._diatonic = Reflow::Wrap7(_class.DiatonicStep() - pc.DiatonicStep());
    newPitch._flags = _flags;
    newPitch._NormalizeIfNeeded();
    return newPitch;
}

REPitch REPitch::Transposed(int semitones) const
{
    REPitch newPitch = *this;
    newPitch._midi = _midi + semitones;
    newPitch._class._chromatic = Reflow::Wrap12(_class.ChromaticStep() + semitones);
    newPitch._flags = _flags;
    newPitch.Normalize();
    return newPitch;
}

REPitch REPitch::Transposed(int semitones, int flags) const
{
    REPitch newPitch = *this;
    newPitch._midi = _midi + semitones;
    newPitch._class._chromatic = Reflow::Wrap12(_class.ChromaticStep() + semitones);
    newPitch._flags = flags;
    newPitch.Normalize();
    return newPitch;
}

void REPitch::SetAlteration(int alter)
{
    int delta = alter - Alteration();
    if(delta == 0) return;
    
    _midi += delta;
    _class._chromatic += delta;
    switch(alter)
    {
        case -2: _flags = REPitch::UseFlats | REPitch::UseAlternateName; break;
        case -1: _flags = REPitch::UseFlats; break;
        case 1: _flags = 0; break;
        case 2: _flags = REPitch::UseAlternateName; break;
    }
    _NormalizeIfNeeded();
}

void REPitch::Normalize()
{
    int table = _flags & 0x03;
    _class._diatonic = _NormalizedDiatonicStepWithChromatic[table][_class._chromatic];
}

void REPitch::_NormalizeIfNeeded()
{
    if(_class.IsValid()) return;
    Normalize();
}

