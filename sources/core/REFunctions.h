//
//  REFunctions.h
//  Reflow
//
//  Created by Sebastien on 13/04/11.
//  Copyright 2011 Gargant Studios. All rights reserved.
//

#ifndef _REFUNCTIONS_H_
#define _REFUNCTIONS_H_

#include "RETypes.h"

namespace Reflow
{
    double PointsToInches(double points);
    double InchesToPoints(double inches);
    
    double InchesToMillimeters(double inches);
    double MillimetersToInches(double mm);
    
    double MillimetersToScoreUnits(double mm);
    double ScoreUnitsToMillimeters(double su);
    
    double PointsToScoreUnits(double points);
    double ScoreUnitsToPoints(double su);
    
    float StringToFloat(std::string str);
    inline bool IsPowerOfTwo(unsigned int x) {return ((x != 0) && ((x & (~x + 1)) == x));}
    
    unsigned int StandardTuningForString(int stringIndex);
    
    bool IsValidDrumNote(int midi);
    const REDrumMapping& StandardDrumMapping(int midi);
    unsigned int DrumMappingsForLine(int line, std::vector<REDrumMapping> &mappings);
    
    unsigned long TimeDivToTicks(const RETimeDiv&);
    RETimeDiv TicksToTimeDiv(unsigned long ticks);
    
    void WriteTimeDivToJson(const RETimeDiv& div, REJsonWriter& writer, uint32_t version);
    void ReadTimeDivFromJson(RETimeDiv& div, const REJsonValue& obj, uint32_t version);
    
    bool RhythmFromTickDuration(unsigned int duration, Reflow::NoteValue* outNoteValue, int* outDots);
	
    unsigned int EnharmonicEquivalentCount(int midi);
	unsigned int PitchSetFromMidi(int8_t midi, RENotePitch* outPitchSet);
    
	int8_t MidiFromPitch(int8_t step, int8_t octave, int8_t alter);
    
    RENotePitch PitchFromStd(int staffLine, Reflow::ClefType clef, int octaveShift, Reflow::Accidental accidental);
    
    REStringFretPairVector FindStringFretPairsForMidi(int midi, const uint8_t* tuningArray, int stringCount, int maxFret=30);
    
    REColor ColorForGizmoType(Reflow::GizmoType type);
    
    double MidiToFrequency(double midi);
    double DecibelsToPercent(double dB);
    double PercentToDecibels(double percent);
    double AbsoluteTimecentsToSeconds(double timecents);
    double SecondsToAbsoluteTimecents(double seconds);
    
    std::string FindNoteName(int pitch, bool withOctaves);

    std::string NameOfMidiProgram(int midi);
    const char* NameOfOttavia (Reflow::OttaviaType ottavia);
    const char* NameOfDirectionJump(Reflow::DirectionJump jump);
    std::string NameOfBendQuarterTones(int quarterTones);
    const char* NameOfNumberGlyph(int num);
    const char* NameOfDynamics(Reflow::DynamicsType dynamics);
    const char* NameOfTool(Reflow::ToolType tool);

    const char* NameOfPaperOrientation(Reflow::PaperOrientation orientation);
    const char* NameOfTrackType(Reflow::TrackType trackType);
    const char* NameOfTablatureInstrumentType(Reflow::TablatureInstrumentType tabType);
    const char* NameOfClefType(Reflow::ClefType clef);
    const char* NameOfOttaviaType(Reflow::OttaviaType ottavia);
    const char* NameOfTempoUnitType(Reflow::TempoUnitType unitType);
    
    Reflow::PaperOrientation ParsePaperOrientation(const std::string& str);
    Reflow::TrackType ParseTrackType(const std::string& str);
    Reflow::TablatureInstrumentType ParseTablatureInstrumentType(const std::string& str);
    Reflow::ClefType ParseClefType(const std::string& str);
    Reflow::OttaviaType ParseOttaviaType(const std::string& str);
    Reflow::TempoUnitType ParseTempoUnitType(const std::string& str);
    
    std::string LocalizedTextForLyricsByFrame(const REScore*);
    std::string LocalizedTextForMusicByFrame(const REScore*);
    std::string LocalizedTextForTranscriberFrame(const REScore*);
    
    int8_t DynamicsToVelocity(Reflow::DynamicsType dynamics);
    
    int Wrap12(int x);
    int Wrap7(int x);

	float Roundf(float x);
    
    template<typename T>
    T Clamp(T val_, T min_, T max_) {
        return std::min<T>(std::max<T>(val_, min_), max_);
    }
    
    std::string Tail (std::string const& source, size_t const length);
    
    RESize SizeOfText(const std::string& textUTF8, const REFontDesc& fontDesc);
    RERect BoundsOfText(const std::string& textUTF8, const REFontDesc& fontDesc);
}


#endif
