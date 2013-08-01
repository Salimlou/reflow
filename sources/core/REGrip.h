//
//  REGrip.h
//  Reflow
//
//  Created by Sebastien Bourgeois on 19/01/12.
//  Copyright (c) 2012 Gargant Studios. All rights reserved.
//

#ifndef Reflow_REGrip_h
#define Reflow_REGrip_h

#include "RETypes.h"

class REGrip
{
public:
    REGrip();
    REGrip(const REGrip& rhs);
    explicit REGrip(const char* grip);
    
public:
    REGrip& operator=(const REGrip& rhs);
    
public:
    void Clear();
    
    bool IsValid() const;
    
    int StringCount() const;
    bool IsStringPlayed(int str) const;
    
    int Fret(int idx) const;
    
    bool IsOpenChord() const;
    bool IsTransposable() const;
    
    bool BuildWithString(const char* grip);
    
    RERange FretRange() const;
    
    void Transpose(int dfret);
    REGrip Transposed(int dfret) const;
    
    std::string ToString() const;
    
    void EncodeTo(REOutputStream& coder) const;
	void DecodeFrom(REInputStream& decoder);
    
public:
    void WriteJson(REJsonWriter& writer, uint32_t version) const;
    void ReadJson(const REJsonValue& obj, uint32_t version);
    
private:
    int8_t _frets[REFLOW_MAX_STRINGS];
    int16_t _playStringFlags;
    int8_t _stringCount;
};

typedef std::vector<REGrip> REGripVector;

#endif
