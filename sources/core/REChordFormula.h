//
//  REChordFormula.h
//  Reflow
//
//  Created by Sebastien Bourgeois on 16/01/12.
//  Copyright (c) 2012 Gargant Studios. All rights reserved.
//

#ifndef Reflow_REChordFormula_h
#define Reflow_REChordFormula_h

#include "RETypes.h"
#include "REPitchClass.h"

class REChordFormula
{
public:
    REChordFormula();
    REChordFormula(const REChordFormula& rhs);
    explicit REChordFormula(const std::string& rawFormula);
    explicit REChordFormula(const std::string& rawFormula, const std::string& symbol);
    
    bool Parse(const std::string& rawFormula);
    
    bool IsValid() const;
    
    int DegreeCount() const;
    const REPitchClass& Degree(int degree) const;
    bool IsDegreeOptional(int degree) const;
    
    void CalculatePitchesWithRootNote(const REPitchClass& rootNote, REPitchClassVector& pitches) const;
    
    void SetSymbol(const std::string& symbol);
    const std::string& Symbol() const;
    
    const std::string& RawFormula() const {return _rawFormula;}
    
public:
    REChordFormula& operator=(const REChordFormula& rhs);
    
    void EncodeTo(REOutputStream& coder) const;
	void DecodeFrom(REInputStream& decoder);
    
public:
    void WriteJson(REJsonWriter& writer, uint32_t version) const;
    void ReadJson(const REJsonValue& obj, uint32_t version);
    
public:
    static REPitchClass IntervalByName(const std::string& interval, bool *error);
    
private:
    REPitchClass _intervals[6];
    int8_t _intervalCount;
    int8_t _optionalDegrees;
    std::string _rawFormula;
    std::string _symbol;
    
private:
    static std::map<std::string, REPitchClass> _intervalsByName;
    
    static void InitializeIntervalsByNameLookupTable();
};




#endif
