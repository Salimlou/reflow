//
//  RETrackSet.cpp
//  Reflow
//
//  Created by Sebastien on 26/04/11.
//  Copyright 2011 Gargant Studios. All rights reserved.
//

#include "RETrackSet.h"
#include "REOutputStream.h"
#include "REInputStream.h"

RETrackSet::RETrackSet()
{
    Clear();
}

RETrackSet::RETrackSet(unsigned int idx) {
    Clear();
    Set(idx);
}

RETrackSet::~RETrackSet()
{
}

RETrackSet::RETrackSet(const RETrackSet& rhs)
{
    *this = rhs;
}


RETrackSet& RETrackSet::operator=(const RETrackSet& rhs)
{
    ::memcpy(_bits, rhs._bits, sizeof(_bits));
    return *this;
}

void RETrackSet::Set(unsigned int idx)
{
    _bits[idx/8] |= (1 << (idx % 8));
}

void RETrackSet::Unset(unsigned int idx)
{
    _bits[idx/8] &= ~(1 << (idx%8));
}

void RETrackSet::Clear()
{
    ::memset(_bits, 0, sizeof(_bits));
}

void RETrackSet::SetAll()
{
    ::memset(_bits, 0xff, sizeof(_bits));
}

bool RETrackSet::IsSet(unsigned int idx) const
{
    return 0 != (_bits[idx/8] & (1 << (idx%8)));
}

void RETrackSet::WriteJson(REJsonWriter& writer, uint32_t version) const
{
    writer.StartArray();
    for(unsigned int i=0; i<REFLOW_MAX_TRACKS; ++i) {
        if(IsSet(i)) {
            writer.Int(i);
        }
    }
    writer.EndArray();
}

void RETrackSet::ReadJson(const REJsonValue& obj, uint32_t version)
{
    Clear();
    if(obj.IsArray()) {
        for(auto it = obj.Begin(); it != obj.End(); ++it) {
            if(it->IsNumber()) {
                Set(it->GetInt());
            }
        }
    }
}

void RETrackSet::EncodeTo(REOutputStream& coder) const
{
    coder.Write((const char*)_bits, sizeof(_bits));
}
void RETrackSet::DecodeFrom(REInputStream& decoder)
{
    decoder.Read((char*)_bits, sizeof(_bits));
}
