//
//  RETrackSet.h
//  Reflow
//
//  Created by Sebastien on 26/04/11.
//  Copyright 2011 Gargant Studios. All rights reserved.
//

#ifndef _RETRACKSET_H_
#define _RETRACKSET_H_

#include "RETypes.h"

class RETrackSet
{
public:
    RETrackSet();
    RETrackSet(unsigned int idx);
    explicit RETrackSet(const RETrackSet& rhs);
    
    ~RETrackSet();
    
public:
    RETrackSet& operator=(const RETrackSet& rhs);
    
public:
    void Set(unsigned int idx);
    void Unset(unsigned int idx);
    void Clear();
    void SetAll();
    bool IsSet(unsigned int idx) const;
    
    void EncodeTo(REOutputStream& coder) const;
    void DecodeFrom(REInputStream& decoder);
    
    void WriteJson(REJsonWriter& writer, uint32_t version) const;
    void ReadJson(const REJsonValue& obj, uint32_t version);
    
private:
    uint8_t _bits[REFLOW_MAX_TRACKS / 8];
};

#endif
