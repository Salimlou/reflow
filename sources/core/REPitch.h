//
//  REPitch.h
//  Reflow
//
//  Created by Sebastien on 12/12/12.
//
//

#ifndef __Reflow__REPitch__
#define __Reflow__REPitch__

#include "REPitchClass.h"

class REPitch
{
public:
    enum PitchFlag
    {
        UseFlats = 1 << 0,
        UseAlternateName = 1 << 1
    };
    
public:
    REPitch();
    explicit REPitch(int midi);
    REPitch(const REPitchClass& pitchClass, int octave);
    
public:
    inline int Midi() const {return _midi;}
    inline int ChromaticStep() const {return _class._chromatic;}
    inline int DiatonicStep() const {return _class._diatonic;}
    inline int MidiOctave() const {return _midi / 12;}
    inline int Alteration() const {return _class.DiatonicAlteration();}

    int Octave() const;

    void SetAlteration(int alter);
    
    std::string Name() const;
    
    inline const REPitchClass& Class() const {return _class;}
    
    REPitch operator-(const REPitchClass& pc) const;
    
    REPitch Transposed(int semitones) const;
    REPitch Transposed(int semitones, int flags) const;
    
    bool HasFlag(PitchFlag flag) const {return 0 != (_flags & flag);}
    void SetFlag(PitchFlag flag) {_flags |= flag; _NormalizeIfNeeded();}
    void SetFlag(PitchFlag flag, bool set) {if(set) SetFlag(flag); else UnsetFlag(flag);}
    void UnsetFlag(PitchFlag flag) {_flags &= ~flag; _NormalizeIfNeeded();}
    
    void Normalize();
    
protected:
    void _NormalizeIfNeeded();
    
protected:
    int8_t _midi;
    uint8_t _flags;
    REPitchClass _class;
};


#endif /* defined(__Reflow__REPitch__) */
