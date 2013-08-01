//
//  REPitchClass.cpp
//  Reflow
//
//  Created by Sebastien Bourgeois on 17/01/12.
//  Copyright (c) 2012 Gargant Studios. All rights reserved.
//

#include "REPitchClass.h"
#include "REOutputStream.h"
#include "REInputStream.h"
#include "REFunctions.h"

const REPitchClass REPitchClass::C = REPitchClass(0,0);
const REPitchClass REPitchClass::D = REPitchClass(1,2);
const REPitchClass REPitchClass::E = REPitchClass(2,4);
const REPitchClass REPitchClass::F = REPitchClass(3,5);
const REPitchClass REPitchClass::G = REPitchClass(4,7);
const REPitchClass REPitchClass::A = REPitchClass(5,9);
const REPitchClass REPitchClass::B = REPitchClass(6,11);

const REPitchClass REPitchClass::C_sharp = REPitchClass(0,1);
const REPitchClass REPitchClass::D_sharp = REPitchClass(1,3);
const REPitchClass REPitchClass::E_sharp = REPitchClass(2,5);
const REPitchClass REPitchClass::F_sharp = REPitchClass(3,6);
const REPitchClass REPitchClass::G_sharp = REPitchClass(4,8);
const REPitchClass REPitchClass::A_sharp = REPitchClass(5,10);
const REPitchClass REPitchClass::B_sharp = REPitchClass(6,0);

const REPitchClass REPitchClass::C_flat = REPitchClass(0,11);
const REPitchClass REPitchClass::D_flat = REPitchClass(1,1);
const REPitchClass REPitchClass::E_flat = REPitchClass(2,3);
const REPitchClass REPitchClass::F_flat = REPitchClass(3,4);
const REPitchClass REPitchClass::G_flat = REPitchClass(4,6);
const REPitchClass REPitchClass::A_flat = REPitchClass(5,8);
const REPitchClass REPitchClass::B_flat = REPitchClass(6,10);

const REPitchClass REPitchClass::C_double_sharp = REPitchClass(0,2);
const REPitchClass REPitchClass::D_double_sharp = REPitchClass(1,4);
const REPitchClass REPitchClass::E_double_sharp = REPitchClass(2,6);
const REPitchClass REPitchClass::F_double_sharp = REPitchClass(3,7);
const REPitchClass REPitchClass::G_double_sharp = REPitchClass(4,9);
const REPitchClass REPitchClass::A_double_sharp = REPitchClass(5,11);
const REPitchClass REPitchClass::B_double_sharp = REPitchClass(6,1);

const REPitchClass REPitchClass::C_double_flat = REPitchClass(0,10);
const REPitchClass REPitchClass::D_double_flat = REPitchClass(1,0);
const REPitchClass REPitchClass::E_double_flat = REPitchClass(2,2);
const REPitchClass REPitchClass::F_double_flat = REPitchClass(3,3);
const REPitchClass REPitchClass::G_double_flat = REPitchClass(4,5);
const REPitchClass REPitchClass::A_double_flat = REPitchClass(5,7);
const REPitchClass REPitchClass::B_double_flat = REPitchClass(6,9);

const REPitchClass REPitchClass::Tonic = REPitchClass(0,0);
const REPitchClass REPitchClass::Unison = REPitchClass(0,0);
const REPitchClass REPitchClass::Octave = REPitchClass(0,0);
const REPitchClass REPitchClass::MinorSecond = REPitchClass(1,1);
const REPitchClass REPitchClass::MajorSecond = REPitchClass(1,2);
const REPitchClass REPitchClass::MinorThird = REPitchClass(2,3);
const REPitchClass REPitchClass::MajorThird = REPitchClass(2,4);
const REPitchClass REPitchClass::PerfectFourth = REPitchClass(3,5);
const REPitchClass REPitchClass::DiminishedFifth = REPitchClass(4,6);
const REPitchClass REPitchClass::PerfectFifth = REPitchClass(4,7);
const REPitchClass REPitchClass::AugmentedFifth = REPitchClass(4,8);
const REPitchClass REPitchClass::MajorSixth = REPitchClass(5,9);
const REPitchClass REPitchClass::MajorSeventh = REPitchClass(6,11);

const REPitchClass REPitchClass::Ninth = REPitchClass(8,14);
const REPitchClass REPitchClass::Eleventh = REPitchClass(10,17);
const REPitchClass REPitchClass::Thirteenth = REPitchClass(12,21);

static const char* _Names[7][12] = 
/*      0      1      2      3      4      5      6      7      8      9      t      e                */
{   { "C"  , "C#" , "C##", ""   , ""   , ""   , ""   , ""   , ""   , ""   , "Cbb", "Cb" },   /* 0 - C */ 
    { "Dbb", "Db" , "D"  , "D#" , "D##", ""   , ""   , ""   , ""   , ""   , ""   , ""   },   /* 1 - D */ 
    { ""   , ""   , "Ebb", "Eb" , "E"  , "E#" , "E##", ""   , ""   , ""   , ""   , ""   },   /* 2 - E */ 
    { ""   , ""   , ""   , "Fbb", "Fb" , "F"  , "F#" , "F##", ""   , ""   , ""   , ""   },   /* 3 - F */ 
    { ""   , ""   , ""   , ""   , ""   , "Gbb", "Gb" , "G"  , "G#" , "G##", ""   , ""   },   /* 4 - G */ 
    { ""   , ""   , ""   , ""   , ""   , ""   , ""   , "Abb", "Ab" , "A"  , "A#" , "A##"},   /* 5 - A */ 
    { "B#" , "B##", ""   , ""   , ""   , ""   , ""   , ""   , ""   , "Bbb", "Bb" , "B"  } }; /* 6 - B */ 

static int _DiatonicAlterations[7][12] = 
{   {  0,  1,  2,  0,  0,  0,  0,  0,  0,  0, -2, -1 },
    { -2, -1,  0,  1,  2,  0,  0,  0,  0,  0,  0,  0 },
    {  0,  0, -2, -1,  0,  1,  2,  0,  0,  0,  0,  0 },
    {  0,  0,  0, -2, -1,  0,  1,  2,  0,  0,  0,  0 },
    {  0,  0,  0,  0,  0, -2, -1,  0,  1,  2,  0,  0 },
    {  0,  0,  0,  0,  0,  0,  0, -2, -1,  0,  1,  2 },
    {  1,  2,  0,  0,  0,  0,  0,  0,  0, -2, -1,  0 } };

static const char* _IntervalShortNames[7][12] = 
/*      0      1      2      3      4      5      6      7      8      9      t      e                */
{   { "P1" , "A1" , ""   , ""   , ""   , ""   , ""   , ""   , ""   , ""   , ""   , "d8" },   /* 0 - C */ 
    { "d2" , "m2" , "M2" , "A2" , ""   , ""   , ""   , ""   , ""   , ""   , ""   , ""   },   /* 1 - D */ 
    { ""   , ""   , "d3" , "m3" , "M3" , "A3" , ""   , ""   , ""   , ""   , ""   , ""   },   /* 2 - E */ 
    { ""   , ""   , ""   , ""   , "d4" , "P4" , "A4" , ""   , ""   , ""   , ""   , ""   },   /* 3 - F */ 
    { ""   , ""   , ""   , ""   , ""   , ""   , "d5" , "P5" , "A5" , ""   , ""   , ""   },   /* 4 - G */ 
    { ""   , ""   , ""   , ""   , ""   , ""   , ""   , "d6" , "m6" , "M6" , "A6" , ""   },   /* 5 - A */ 
    { "A7" , ""   , ""   , ""   , ""   , ""   , ""   , ""   , ""   , "d7" , "m7" , "M7" } }; /* 6 - B */ 

static int _Valid[7][12] = 
{   {  1,  1,  1,  0,  0,  0,  0,  0,  0,  0,  1,  1 },
    {  1,  1,  1,  1,  1,  0,  0,  0,  0,  0,  0,  0 },
    {  0,  0,  1,  1,  1,  1,  1,  0,  0,  0,  0,  0 },
    {  0,  0,  0,  1,  1,  1,  1,  1,  0,  0,  0,  0 },
    {  0,  0,  0,  0,  0,  1,  1,  1,  1,  1,  0,  0 },
    {  0,  0,  0,  0,  0,  0,  0,  1,  1,  1,  1,  1 },
    {  1,  1,  0,  0,  0,  0,  0,  0,  0,  1,  1,  1 } };

static int _NormalizedDiatonicStepWithChromaticUsingSharps[12] =
   /*  C   C#  D   D#  E   F   F#  G   G#  A   A#  B */
    {  0,  0,  1,  1,  2,  3,  3,  4,  4,  5,  5,  6  };

static int _NormalizedDiatonicStepWithChromaticUsingFlats[12] =
    /*  C  Db  D   Eb  E   F   Gb  G   Ab  A   Bb  B */
    {  0,  1,  1,  2,  2,  3,  4,  4,  5,  5,  6,  6  };

REPitchClass::REPitchClass(int diatonic, int chromatic)
{
    _diatonic = Reflow::Wrap7(diatonic);
    _chromatic = Reflow::Wrap12(chromatic);
}


void REPitchClass::EncodeTo(REOutputStream& coder) const
{
    coder.WriteInt8(_diatonic);
    coder.WriteInt8(_chromatic);
}
void REPitchClass::DecodeFrom(REInputStream& decoder)
{
    _diatonic = decoder.ReadInt8();
    _chromatic = decoder.ReadInt8();
}

REPitchClass& REPitchClass::operator=( const REPitchClass& rhs)
{
    _diatonic = rhs._diatonic;
    _chromatic = rhs._chromatic;
    return *this;
}

const char* const REPitchClass::Name() const
{
    return _Names[_diatonic][_chromatic];
}

const char* const REPitchClass::IntervalShortName() const
{
    return _IntervalShortNames[_diatonic][_chromatic];
}

int REPitchClass::DiatonicAlteration() const
{
    return _DiatonicAlterations[_diatonic][_chromatic];
}

bool REPitchClass::IsValid() const
{
    return 0 != _Valid[_diatonic][_chromatic];
}

bool REPitchClass::operator== (const REPitchClass& pitch) const
{
    return _diatonic == pitch._diatonic && _chromatic == pitch._chromatic;
}

bool REPitchClass::IsEnharmonicEquivalent(const REPitchClass& pitch) const
{
    return _chromatic == pitch._chromatic;
}

REPitchClass REPitchClass::operator+(const REPitchClass& interval) const
{
    int d = Reflow::Wrap7(_diatonic + interval._diatonic);
    int c = Reflow::Wrap12(_chromatic + interval._chromatic);
    return REPitchClass(d,c);
}

REPitchClass REPitchClass::operator-(const REPitchClass& interval) const
{
    int d = Reflow::Wrap7(_diatonic - interval._diatonic);
    int c = Reflow::Wrap12(_chromatic - interval._chromatic);
    return REPitchClass(d,c);
}

REPitchClass REPitchClass::operator*(int factor) const
{
    return REPitchClass(_diatonic * factor, _chromatic * factor);
}

REPitchClass operator*(int x, const REPitchClass& pitch)
{
    return REPitchClass(pitch.DiatonicStep() * x, pitch.ChromaticStep() * x);
}

REPitchClass REPitchClass::Augmented(int nbSharps) const
{
    return *this + REPitchClass(0, nbSharps);
}

REPitchClass REPitchClass::Diminished(int nbFlats) const
{
    return *this + REPitchClass(0,-nbFlats);
}

REPitchClass REPitchClass::Normalized(bool withFlats) const
{
    if(withFlats) {
        return REPitchClass(_NormalizedDiatonicStepWithChromaticUsingFlats[_chromatic], _chromatic);
    }
    else {
        return REPitchClass(_NormalizedDiatonicStepWithChromaticUsingSharps[_chromatic], _chromatic);
    }
}

REPitchClass REPitchClass::WithDiatonicStep(int step)
{
    switch(step) {
        case 0: return C;
        case 1: return D;
        case 2: return E;
        case 3: return F;
        case 4: return G;
        case 5: return A;
        case 6: return B;
    }
    return C;
}

REPitchClass REPitchClass::WithDiatonicStepAndAlteration(int step, int alteration)
{
    return WithDiatonicStep(step).Augmented(alteration);
}

int REPitchClass::SetFromString(const std::string& str)
{
    if(str.empty()) {
        return 0;
    }
    
    int step = 0;
    char c = str[0];
    switch(c) 
    {
        case 'C': step = 0; break;
        case 'D': step = 1; break;
        case 'E': step = 2; break;
        case 'F': step = 3; break;
        case 'G': step = 4; break;
        case 'A': step = 5; break;
        case 'B': step = 6; break;
        case 'c': step = 0; break;
        case 'd': step = 1; break;
        case 'e': step = 2; break;
        case 'f': step = 3; break;
        case 'g': step = 4; break;
        case 'a': step = 5; break;
        case 'b': step = 6; break;
        default: {
            return 0;
        }
    }
    
    if(str.size() >= 3) 
    {
        // Search ##
        if(str[1] == '#' && str[2] == '#') {
            *this = REPitchClass::WithDiatonicStep(step).Augmented(2);
            return 3;
        }
        
        // Search bb
        if(str[1] == 'b' && str[2] == 'b') {
            *this = REPitchClass::WithDiatonicStep(step).Diminished(2);
            return 3;
        }
    }
    if(str.size() >= 2) 
    {
        // Search #
        if(str[1] == '#') {
            *this = REPitchClass::WithDiatonicStep(step).Augmented(1);
            return 2;
        }
        // Search b
        if(str[1] == 'b') {
            *this = REPitchClass::WithDiatonicStep(step).Diminished(1);
            return 2;
        }
        // Search x
        if(str[1] == 'x') {
            *this = REPitchClass::WithDiatonicStep(step).Augmented(2);
            return 2;
        }
    }

    *this = REPitchClass::WithDiatonicStep(step);
    return 1;
}

bool REPitchClass::IsStringParseable(const std::string& str)
{
    REPitchClass pc;
    return 0 != pc.SetFromString(str);
}

REPitchClass REPitchClass::Parse(const std::string& str, bool *error)
{
    REPitchClass pc;
    int res = pc.SetFromString(str);
    if(res == 0 && error != NULL) {
        *error = true;
    }
    return pc;
}

REPitchClass REPitchClass::Complement() const
{
    return REPitchClass(7-_diatonic, 12-_chromatic);
}
