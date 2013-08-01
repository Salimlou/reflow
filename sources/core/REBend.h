//
//  REBend.h
//  Reflow
//
//  Created by Sebastien Bourgeois on 27/01/12.
//  Copyright (c) 2012 Gargant Studios. All rights reserved.
//

#ifndef Reflow_REBend_h
#define Reflow_REBend_h

#include "RETypes.h"

class REBend
{
public:
    REBend();
    REBend(Reflow::BendType type);
    REBend(Reflow::BendType type, int bentPitch);
    REBend(Reflow::BendType type, int bentPitch, int releasePitch);
    
    bool IsValid() const {return _type != Reflow::NoBend;}
    
    Reflow::BendType Type() const {return (Reflow::BendType)_type;}
    void SetType(Reflow::BendType type) {_type = type;}
    
    int BentPitch() const {return _bentPitch;}
    float BentPitchInTones() const {return (float)_bentPitch / 4.0;}
    void SetBentPitch(int quarterTones) {_bentPitch = quarterTones;}
    
    int ReleasePitch() const {return _releasePitch;}
    float ReleasePitchInTones() const {return (float)_releasePitch / 4.0;}
    void SetReleasePitch(int quarterTones) {_releasePitch = quarterTones;}
    
    void PitchFactors(float* initialPitchFactor, float* middlePitchFactor, float* finalPitchFactor) const;
    
    void Draw(REPainter& painter, const REPoint& point, const RESize& size) const;
    
    void EncodeTo(REOutputStream& coder) const;
    void DecodeFrom(REInputStream& decoder);
    
    void WriteJson(REJsonWriter& writer, uint32_t version) const;
    void ReadJson(const REJsonValue& obj, uint32_t version);
    
private:
    static void _DrawAux(REPainter& painter, const REPoint& point, const RESize& size, float initialPitch, float middlePitch, float finalPitch);
    
private:
    int8_t _type;
    int8_t _bentPitch;
    int8_t _releasePitch;
};

#endif
