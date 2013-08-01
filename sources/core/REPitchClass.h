//
//  REPitchClass.h
//  Reflow
//
//  Created by Sebastien Bourgeois on 17/01/12.
//  Copyright (c) 2012 Gargant Studios. All rights reserved.
//

#ifndef Reflow_REPitchClass_h
#define Reflow_REPitchClass_h

#include "RETypes.h"

/** REPitchClass class.
 */
class REPitchClass
{
    friend class REPitch;
    
public:
    REPitchClass() : _diatonic(0), _chromatic(0) {}
    REPitchClass(int diatonic, int chromatic);
    REPitchClass(const REPitchClass& rhs) {*this = rhs;}
    
    const char* const Name() const;
    
    const char* const IntervalShortName() const;
    
    int DiatonicStep() const {return _diatonic;}
    int ChromaticStep() const {return _chromatic;}
    
    int DiatonicAlteration() const;
    
    REPitchClass operator+(const REPitchClass& interval) const;
    REPitchClass operator-(const REPitchClass& interval) const;
    REPitchClass operator*(int factor) const;
    
    bool operator== (const REPitchClass& pitch) const;
    
    REPitchClass& operator=( const REPitchClass& rhs);
    
    REPitchClass Augmented(int nbSharps) const;
    REPitchClass Diminished(int nbFlats) const;
    
    REPitchClass Normalized(bool withFlats=false) const;
    
    REPitchClass Complement() const;
    
    bool IsValid() const;
    
    bool IsEnharmonicEquivalent(const REPitchClass& pitch) const;
    
    int SetFromString(const std::string& str);
    
    void EncodeTo(REOutputStream& coder) const;
	void DecodeFrom(REInputStream& decoder);
    
public:
    void WriteJson(REJsonWriter& writer, uint32_t version) const;
    void ReadJson(const REJsonValue& obj, uint32_t version);
    
public:
    static REPitchClass Parse(const std::string& str, bool *error=NULL);
    static bool IsStringParseable(const std::string& str);
    static REPitchClass WithDiatonicStep(int step);
    static REPitchClass WithDiatonicStepAndAlteration(int step, int alteration);
    
private:
    int8_t _diatonic;
    int8_t _chromatic;
    
public:
    static const REPitchClass C;
    static const REPitchClass D;
    static const REPitchClass E;
    static const REPitchClass F;
    static const REPitchClass G;
    static const REPitchClass B;
    static const REPitchClass A;
    
    static const REPitchClass C_sharp;
    static const REPitchClass D_sharp;
    static const REPitchClass E_sharp;
    static const REPitchClass F_sharp;
    static const REPitchClass G_sharp;
    static const REPitchClass B_sharp;
    static const REPitchClass A_sharp;
    
    static const REPitchClass C_flat;
    static const REPitchClass D_flat;
    static const REPitchClass E_flat;
    static const REPitchClass F_flat;
    static const REPitchClass G_flat;
    static const REPitchClass B_flat;
    static const REPitchClass A_flat;
    
    static const REPitchClass C_double_sharp;
    static const REPitchClass D_double_sharp;
    static const REPitchClass E_double_sharp;
    static const REPitchClass F_double_sharp;
    static const REPitchClass G_double_sharp;
    static const REPitchClass B_double_sharp;
    static const REPitchClass A_double_sharp;
    
    static const REPitchClass C_double_flat;
    static const REPitchClass D_double_flat;
    static const REPitchClass E_double_flat;
    static const REPitchClass F_double_flat;
    static const REPitchClass G_double_flat;
    static const REPitchClass B_double_flat;
    static const REPitchClass A_double_flat;
    
    static const REPitchClass Tonic;
    static const REPitchClass Unison;
    static const REPitchClass Octave;
    
    static const REPitchClass MinorSecond;
    static const REPitchClass MajorSecond;
    
    static const REPitchClass MinorThird;
    static const REPitchClass MajorThird;

    static const REPitchClass PerfectFourth;
    
    static const REPitchClass DiminishedFifth;
    static const REPitchClass PerfectFifth;
    static const REPitchClass AugmentedFifth;
    
    static const REPitchClass MajorSixth;
    
    static const REPitchClass MajorSeventh;
    
    static const REPitchClass Ninth;
    
    static const REPitchClass Eleventh;
    
    static const REPitchClass Thirteenth;
};

REPitchClass operator*(int x, const REPitchClass& pitch);

/** REPitchClassVector.
 */
typedef std::vector<REPitchClass> REPitchClassVector;


#endif
