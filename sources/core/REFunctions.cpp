//
//  REFunctions.cpp
//  Reflow
//
//  Created by Sebastien on 13/04/11.
//  Copyright 2011 Gargant Studios. All rights reserved.
//

#include "REOutputStream.h"
#include "REFunctions.h"
#include "RESpecialCharacters.h"

#ifdef _WIN32
#  define _USE_MATH_DEFINES 
#endif
#include <cmath>
#include <sstream>

#ifdef _WIN32
#  define log2(n)  (log(n)/log(2.0))
#endif

int Reflow::Wrap12(int x) 
{
    if(x < 0) {
        int nbOctaves = 1 + ((-x)/12);
        return (x + nbOctaves*12) % 12;
    }
    else return x % 12;
}

int Reflow::Wrap7(int x) 
{
    if(x < 0) {
        int nbOctaves = 1 + ((-x)/7);
        return (x + nbOctaves*7) % 7;
    }
    else return x % 7;
}

float Reflow::Roundf(float x)
{
#ifdef _WIN32
    return floor(x + 0.5);
#else
    return roundf(x);
#endif
}

static int _stepToPitch[] = {
	0,
	2,
	4,
	5,
	7,
	9,
	11
};

typedef enum {
	STEP_C = 0,
	STEP_D,
	STEP_E,
	STEP_F,
	STEP_G,
	STEP_A,
	STEP_B
} StepName;

#define bb_ (-2)
#define b_  (-1)
#define n_  (0)
#define s_  (1)
#define ss_ (2)

static unsigned int _nbNoteNamesForPitch[]            = {3,      3,     3,       3,      3,      3,      3,      3,      2,      3,      3,      3};
static int8_t _stepWithPitch[]			 = {STEP_C,	STEP_C,	STEP_D, STEP_E,	STEP_E,	STEP_F,	STEP_F, STEP_G,	STEP_A,	STEP_A,	STEP_B,	STEP_B};
static int8_t _alterationWithPitch[] = {n_,     s_,     n_,     b_,     n_,     n_,     s_,     n_,     b_,     n_,     b_,     n_};

static int8_t _stepWithPitch1[]			 = {STEP_B,	STEP_B,	STEP_C, STEP_D,	STEP_D,	STEP_E, STEP_E,	STEP_F,	STEP_G,	STEP_G,	STEP_A,	STEP_A};
static int8_t _alterationWithPitch1[]= {s_,     ss_,    ss_,    s_,     ss_,    s_,     ss_,    ss_,    s_,     ss_,    s_,     ss_};

static int8_t _stepWithPitch2[]			 = {STEP_D,	STEP_D,	STEP_E, STEP_F,	STEP_F,	STEP_G, STEP_G,	STEP_A, 0,      STEP_B,	STEP_C,	STEP_C};
static int8_t _alterationWithPitch2[]= {bb_,    b_,     bb_,    bb_,    b_,     bb_,    b_,     bb_,    0,      bb_,    bb_,    b_};

unsigned int Reflow::EnharmonicEquivalentCount(int midi)
{
    return _nbNoteNamesForPitch[midi % 12];
}

unsigned int Reflow::PitchSetFromMidi(int8_t midi, RENotePitch* outPitchSet)
{
    int midiMod12 = midi%12;
    int nbNoteNames = _nbNoteNamesForPitch[midiMod12];
    int octave = midi/12;
	
    for(int i=0; i<nbNoteNames; ++i)
	{
		int8_t step = 0;
		int8_t alt = 0;
        int8_t oct = octave;
		switch(i) {
			case 0: 
				step = _stepWithPitch[midiMod12];
				alt = _alterationWithPitch[midiMod12];
				break;
			case 1: 
				step = _stepWithPitch1[midiMod12];
				alt = _alterationWithPitch1[midiMod12];
				if(step == STEP_B) {oct = octave-1;}
				break;
			case 2: 
				step = _stepWithPitch2[midiMod12];
				alt = _alterationWithPitch2[midiMod12];
				if(step == STEP_C) {oct = octave+1;}
				break;
		}
        
        outPitchSet[i].midi = midi;
        outPitchSet[i].step = step;
        outPitchSet[i].octave = oct;
        outPitchSet[i].alter = alt;
    }
    return nbNoteNames;
}

int8_t Reflow::MidiFromPitch(int8_t step, int8_t octave, int8_t alter)
{
    return alter + octave*12 + _stepToPitch[step];
}

RENotePitch Reflow::PitchFromStd(int staffLine, Reflow::ClefType clef, int octaveShift, Reflow::Accidental accidental)
{
    // X is note step (without modulo 7)
    int X;
    if(clef == Reflow::TrebleClef) {
        X = REFLOW_STEP_F1 - staffLine;
    }
    else {
        X = REFLOW_STEP_A_minus_1 - staffLine;
    }

    // Step and octave
    RENotePitch pitch;
    pitch.step = X % 7;
    pitch.octave = X / 7 + octaveShift;
    
    // Accidental
    switch (accidental) {
        case Reflow::Sharp: pitch.alter = 1; break;
        case Reflow::Flat: pitch.alter = -1; break;
        case Reflow::DoubleSharp: pitch.alter = 2; break;
        case Reflow::DoubleFlat: pitch.alter = -2; break;
        default: pitch.alter = 0;
    }
    
    // MIDI
    pitch.midi = MidiFromPitch(pitch.step, pitch.octave, pitch.alter);
    return pitch;
}

REStringFretPairVector Reflow::FindStringFretPairsForMidi(int midi, const uint8_t* tuningArray, int stringCount, int maxFret)
{
    REStringFretPairVector stringFrets;
    for(unsigned int stringIndex=0; stringIndex<stringCount; ++stringIndex) 
    {
        int tuning = tuningArray[stringIndex];
        int dt = midi - tuning;
        if(dt >= 0 && dt <= maxFret) {
            stringFrets.push_back(REStringFretPair(stringIndex, dt));
        }
    }
    return stringFrets;
}

double Reflow::PointsToInches(double points)
{
    return points / 72.0;
}

double Reflow::InchesToPoints(double inches)
{
    return inches * 72.0;
}

double Reflow::InchesToMillimeters(double inches)
{
    return inches * 25.4;
}

double Reflow::MillimetersToInches(double mm)
{
    return mm / 25.4;
}

double Reflow::MillimetersToScoreUnits(double mm)
{
    return 4.0 * mm;
}

double Reflow::ScoreUnitsToMillimeters(double su)
{
    return 0.25 * su;
}

double Reflow::PointsToScoreUnits(double points)
{
    return MillimetersToScoreUnits (InchesToMillimeters (PointsToInches(points)));
}

double Reflow::ScoreUnitsToPoints(double scoreUnits)
{
    return InchesToPoints(MillimetersToInches(ScoreUnitsToMillimeters(scoreUnits)));
}

unsigned int Reflow::StandardTuningForString(int stringIndex)
{
	switch(stringIndex)
	{
		case 0: return 64;
		case 1: return 59;
		case 2: return 55;
		case 3: return 50;
		case 4: return 45;
		case 5: return 40;
		case 6: return 35;
		case 7: return 30;
	}
	return 0;
}

static REDrumMapping _DrumMappingArray[128] = {
    /*0*/{0, 0, 0, NULL}, {1, 0, 0, NULL}, {2, 0, 0, NULL}, {3, 0, 0, NULL}, 
    /*4*/{4, 0, 0, NULL}, {5, 0, 0, NULL}, {6, 0, 0, NULL}, {7, 0, 0, NULL},
    /*8*/{8, 0, 0, NULL}, {9, 0, 0, NULL}, {10, 0, 0, NULL}, {11, 0, 0, NULL}, 
    /*12*/{12, 0, 0, NULL}, {13, 0, 0, NULL}, {14, 0, 0, NULL}, {15, 0, 0, NULL},
    /*16*/{16, 0, 0, NULL}, {17, 0, 0, NULL}, {18, 0, 0, NULL}, {19, 0, 0, NULL}, 
    /*20*/{20, 0, 0, NULL}, {21, 0, 0, NULL}, {22, 0, 0, NULL}, {23, 0, 0, NULL},
    /*24*/{24, 0, 0, NULL}, {25, 0, 0, NULL}, {26, 0, 0, NULL}, {27, 0, 0, NULL}, 
    /*28*/{28, 0, 0, NULL}, {29, 0, 0, NULL}, {30, 0, 0, NULL}, {31, 0, 0, NULL},
    
    /*32*/ {0, 0, 0, NULL}, {0, 0, 0, NULL}, {0, 0, 0, NULL}, {35, 7, 0, "Bass Drum"}, 
    /*36*/ {36, 7, 0, "Bass Drum"}, {37, 3, Reflow::CrossNoteHead, "Side Stick"}, {38, 3, 0, "Acoustic Snare"}, {0, 0, 0, NULL},
    /*40*/ {40, 3, 0, "Electric Snare"}, {41, 6, 0, "Low Tom 2"}, {42, -1, Reflow::CrossNoteHead, "Closed Hi Hat"}, {43, 5, 0, "Low Tom 1"}, 
    /*44*/ {44, 9, Reflow::CrossNoteHead, "Hi Hat Pedal"}, {45, 4, 0, "Mid Tom 2"}, {46, -1, Reflow::CircledCrossNoteHead, "Opened Hi Hat"}, {47, 2, 0, "Mid Tom 1"},
    /*48*/ {48, 1, 0, "High Tom 2"}, /*49*/ {49, -3, Reflow::CircledCrossNoteHead, "Crash Cymbal 1"}, /*50*/ {50, 0, 0, "High Tom 1"}, /*51*/ {51, -2, Reflow::CrossNoteHead, "Ride Cymbal 1"}, 
    /*52*/ {0, 0, 0, NULL}, /*53*/ {53, -2, Reflow::FilledDiamondNoteHead, "Ride Bell"}, {0, 0, 0, NULL}, {0, 0, 0, NULL},
    /*56*/ {0, 0, 0, NULL}, /*57*/ {57, -3, Reflow::CircledCrossNoteHead, "Crash Cymbal 2"}, {0, 0, 0, NULL}, /*59*/ {59, -2, Reflow::CrossNoteHead, "Ride Cymbal 2"}, 
    /*60*/ {0, 0, 0, NULL}, {0, 0, 0, NULL}, {0, 0, 0, NULL}, {0, 0, 0, NULL},
    {0, 0, 0, NULL}, {0, 0, 0, NULL}, {0, 0, 0, NULL}, {0, 0, 0, NULL}, 
    {0, 0, 0, NULL}, {0, 0, 0, NULL}, {0, 0, 0, NULL}, {0, 0, 0, NULL}, 
    {0, 0, 0, NULL}, {0, 0, 0, NULL}, {0, 0, 0, NULL}, {0, 0, 0, NULL}, 
    {0, 0, 0, NULL}, {0, 0, 0, NULL}, {0, 0, 0, NULL}, {0, 0, 0, NULL}, 
    {0, 0, 0, NULL}, {0, 0, 0, NULL}, {0, 0, 0, NULL}, {0, 0, 0, NULL}, 
    {0, 0, 0, NULL}, {0, 0, 0, NULL}, {0, 0, 0, NULL}, {0, 0, 0, NULL}, 
    {0, 0, 0, NULL}, {0, 0, 0, NULL}, {0, 0, 0, NULL}, {0, 0, 0, NULL}, 
    {0, 0, 0, NULL}, {0, 0, 0, NULL}, {0, 0, 0, NULL}, {0, 0, 0, NULL}, 
    {0, 0, 0, NULL}, {0, 0, 0, NULL}, {0, 0, 0, NULL}, {0, 0, 0, NULL}, 
    {0, 0, 0, NULL}, {0, 0, 0, NULL}, {0, 0, 0, NULL}, {0, 0, 0, NULL}, 
    {0, 0, 0, NULL}, {0, 0, 0, NULL}, {0, 0, 0, NULL}, {0, 0, 0, NULL}, 
    {0, 0, 0, NULL}, {0, 0, 0, NULL}, {0, 0, 0, NULL}, {0, 0, 0, NULL}, 
    {0, 0, 0, NULL}, {0, 0, 0, NULL}, {0, 0, 0, NULL}, {0, 0, 0, NULL}, 
    {0, 0, 0, NULL}, {0, 0, 0, NULL}, {0, 0, 0, NULL}, {0, 0, 0, NULL}, 
    {0, 0, 0, NULL}, {0, 0, 0, NULL}, {0, 0, 0, NULL}, {0, 0, 0, NULL}, 
    {0, 0, 0, NULL}, {0, 0, 0, NULL}, {0, 0, 0, NULL}, {0, 0, 0, NULL}
};

const REDrumMapping& Reflow::StandardDrumMapping(int midi)
{
    return _DrumMappingArray[midi%128];
}

unsigned int Reflow::DrumMappingsForLine(int line, std::vector<REDrumMapping> &mappings)
{
    mappings.clear();
    for(unsigned int i=0; i<128; ++i) {
        const REDrumMapping& mapping = _DrumMappingArray[i];
        if(mapping.line == line) {
            mappings.push_back(mapping);
        }
    }
    return mappings.size();
}

bool Reflow::IsValidDrumNote(int midi)
{
    const REDrumMapping& mapping = _DrumMappingArray[midi];    
    return mapping.name != NULL;
}

int8_t Reflow::DynamicsToVelocity(Reflow::DynamicsType dynamics)
{
    if(dynamics == Reflow::DynamicsUndefined) return 90;
    static int8_t velocityTable[] = {13, 26, 39, 52, 65, 78, 91, 104};
    return velocityTable[dynamics];
}

unsigned long Reflow::TimeDivToTicks(const RETimeDiv& td)
{
    return (td.numerator() * REFLOW_PULSES_PER_QUARTER) / td.denominator();    
}

RETimeDiv Reflow::TicksToTimeDiv(unsigned long ticks)
{
    return RETimeDiv(ticks, REFLOW_PULSES_PER_QUARTER);
}

void Reflow::WriteTimeDivToJson(const RETimeDiv& div, REJsonWriter& writer, uint32_t version)
{
    if(div.denominator() == 1) {
        writer.Int(div.numerator());
    }
    else {
        writer.StartObject();
        {
            writer.String("n"); writer.Int(div.numerator());
            writer.String("d"); writer.Int(div.denominator());
        }
        writer.EndObject();
    }
}
void Reflow::ReadTimeDivFromJson(RETimeDiv& div, const REJsonValue& obj, uint32_t version)
{
    if(obj.IsInt())
    {
        div = RETimeDiv(obj.GetInt(), 1);
    }
    else if(obj.IsObject())
    {
        const REJsonValue& n = obj["n"];
        const REJsonValue& d = obj["d"];
        if(n.IsNumber() && d.IsNumber()) {
            div = RETimeDiv(n.GetInt(), d.GetInt());
        }
    }
}

bool Reflow::RhythmFromTickDuration(unsigned int duration, Reflow::NoteValue* outNoteValue, int* outDots)
{
    // Assert duration is a multiple of 64th notes.
    if((duration % 30) == 0)
    {
        switch (duration) {
            case 1920           :     *outNoteValue = Reflow::WholeNote;          *outDots = 0; return true; break;
            case 1920+960       :     *outNoteValue = Reflow::WholeNote;          *outDots = 1; return true; break;
            case 1920+960+480   :     *outNoteValue = Reflow::WholeNote;          *outDots = 2; return true; break;
                
            case 960            :     *outNoteValue = Reflow::HalfNote;           *outDots = 0; return true; break;
            case 960+480        :     *outNoteValue = Reflow::HalfNote;           *outDots = 1; return true; break;
            case 960+480+240    :     *outNoteValue = Reflow::HalfNote;           *outDots = 2; return true; break;
                
            case 480            :     *outNoteValue = Reflow::QuarterNote;        *outDots = 0; return true; break;
            case 480+240        :     *outNoteValue = Reflow::QuarterNote;        *outDots = 1; return true; break;
            case 480+240+120    :     *outNoteValue = Reflow::QuarterNote;        *outDots = 2; return true; break;
                
            case 240            :     *outNoteValue = Reflow::EighthNote;         *outDots = 0; return true; break;
            case 240+120        :     *outNoteValue = Reflow::EighthNote;         *outDots = 1; return true; break;
            case 240+120+60     :     *outNoteValue = Reflow::EighthNote;         *outDots = 2; return true; break;
                
            case 120            :     *outNoteValue = Reflow::SixteenthNote;      *outDots = 0; return true; break;
            case 120+60         :     *outNoteValue = Reflow::SixteenthNote;      *outDots = 1; return true; break;
            case 120+60+30      :     *outNoteValue = Reflow::SixteenthNote;      *outDots = 2; return true; break;
                
            case 60             :     *outNoteValue = Reflow::ThirtySecondNote;   *outDots = 0; return true; break;
            case 60+30          :     *outNoteValue = Reflow::ThirtySecondNote;   *outDots = 1; return true; break;
                
            case 30             :     *outNoteValue = Reflow::SixtyFourthNote;    *outDots = 0; return true; break;
        }
    }
    
    return false;   // Could not find rhythm from this duration (need to quantize)
}

double Reflow::MidiToFrequency(double midi)
{
    return 440.0 * ::pow(2.0, (midi-57.0)/12.0);
}

double Reflow::DecibelsToPercent(double dB)
{
    return ::pow(10.0, dB * 0.1);
}

double Reflow::PercentToDecibels(double percent)
{
    return 10.0 * ::log10(percent);
}

double Reflow::AbsoluteTimecentsToSeconds(double timecents)
{
    return pow(2.0, timecents/1200.0);
}

double Reflow::SecondsToAbsoluteTimecents(double seconds)
{
    return 1200.0 * log2(seconds);
}

REColor Reflow::ColorForGizmoType(Reflow::GizmoType type)
{
    switch (type)
    {
        case Reflow::FirstBarlineGizmo: return REColor(1.0, 0.8, 0.6);
        case Reflow::MiddleBarlineGizmo: return REColor(1.0, 0.9, 0.7);
        case Reflow::LastBarlineGizmo: return REColor(1.0, 0.8, 0.6);
        default: 
            return REColor(0.8, 0.9, 0.98);
    }
}

static const char* midiProgramNames[] = {
	"Grand Piano",		// 1
	"Bright Piano",
	"Electric Piano",
	"Honky-Tonk",
	"E. Piano 1",
	"E. Piano 2",
	"Harpsichord",
	"Clavi",
	"Celesta",
	
	"Glockenspiel",		// 10
	"Music Box",
	"Vibraphone",
	"Marimba",
	"Xylophone",
	"Tubular Bells",
	"Dulcimer",
	"Drawbar Organ",
	"Perc Organ",
	"Rock Organ",
	"Church Organ",
	"Reed Organ",
	
	"Accordion",		// 22
	"Harmonica",
	"Tango Accord.",
	"Ac. Gtr. Nylon",
	"Ac. Gtr. Steel",
	"E. Guitar Jazz",
	"E. Guitar Clean",
	"E. Guitar Muted",
	"Overdriven Gtr",
	"Distortion Gtr",
	"Gtr Harmonics",
	
	"Ac. Bass",	// 33
	"E. Bass Finger",
	"E. Bass Pick",
	"Fretless Bass",
	"Slap Bass 1",
	"Slap Bass 2",
	"Synth Bass 1",
	"Synth Bass 2",
	"Violin",
	"Viola",
	"Cello",
	"Contrabass",
	"Trem. Strings",
	"Pizz. Strings",
	"Orch. Harp",
	"Timpani",
	"String Ens. 1",
	"String Ens. 2",
	"Synth Str. 1",
	"Synth Str. 2",
	"Choir Aahs",
	"Voice Oohs",
	"Synth Voice",
	"Orchestra Hit",
	
	"Trumpet",		// 57
	"Trombone",
	"Tuba",
	"Muted Trumpet",
	"French Horn",
	"Brass Section",
	"Synth Brass 1",
	"Synth Brass 2",
	
	"Soprano Sax",	// 65
	"Alto Sax",
	"Tenor Sax",
	"Baritone Sax",
	"Oboe",
	"English Horn",
	"Bassoon",
	"Clarinet",
	"Piccolo",
	"Flute",
	"Recorder",
	"Pan Flute",
	
	"Blown Bottle",		// 77
	"Shakuhachi",
	"Whistle",
	"Ocarina",
	
	"Lead (Square)",	// 81
	"Lead (Sawtooth)",
	"Lead (Calliope)",
	"Lead (Chiff)",
	"Lead (Charang)",
	"Lead (Voice)",
	"Lead (Fifths)",
	"Lead (Bass)",
	
	"Pad (New Age)",		// 89
	"Pad (Warm)",
	"Pad (Polysynth)",
	"Pad (Choir)",
	"Pad (Bowed)",
	"Pad (Metallic)",
	"Pad (Halo)",
	"Pad (Sweep)",
	
	"FX (Rain)",
	"FX (Soundtrack)",
	"FX (Crystal)",
	"FX (Atmosphere)",
	"FX (Brightness)",
	"FX (Goblins)",
	"FX (Echoes)",
	"FX (Sci-fi)",
	
	"Sitar",	// 105
	"Banjo",
	"Shamisen",
	"Koto",
	"Kalimba",
	"Bag pipe",
	"Fiddle",
	"Shanai",
	"Tinkle Bell",
	"Agogo",
	"Steel Drums",
	"Woodblock",
	"Taiko Drum",
	"Melodic Drum",
	"Synth Drum",
	"Reverse Cymbal",
	"Gtr Fret Noise",
	"Breath Noise",
	"Seashore",
	"Bird Tweet",
	"Telephone Ring",
	"Helicopter",
	"Applause",
	"Gunshot"		// 128
};

std::string Reflow::NameOfMidiProgram(int midi)
{
    if(midi >= 0 && midi < 128) {
        return std::string(midiProgramNames[midi]);
    }
    return "";
}


std::string RENotePitch::NoteName() const
{
    std::ostringstream oss;
    static const char* steps = "CDEFGAB";
    oss << steps[step];
    
    switch(this->alter) 
    {
        case -2: oss << "bb"; break;
        case -1: oss << "b"; break;
        case 1:  oss << "#"; break;
        case 2:  oss << "##"; break;
    }
    
    return oss.str();
}

std::string Reflow::FindNoteName(int pitch, bool withOctaves)
{
	int octave = pitch/12;
	static const char* noteNames[] = {"C", "C#", "D", "D#", "E", "F", "F#", "G", "G#", "A", "A#", "B"};
	
	if (withOctaves)
	{
        std::ostringstream oss;
        oss << noteNames[pitch%12] << octave-1;
        return oss.str();
	}
	else {
		return std::string(noteNames[pitch%12]);
	}
}


const char* REKeySignature::Name() const
{
    static const char* _majorNames[] = {"Cb",  "Gb",  "Db",  "Ab", "Eb", "Bb", "F", "C",   "G",  "D",  "A",   "E",   "B",   "F#",  "C#"};
    static const char* _minorNames[] = {"Abm", "Ebm", "Bbm", "Fm", "Cm", "Gm", "Dm", "Am", "Em", "Bm", "F#m", "C#m", "G#m", "D#m", "A#m"};
    
    return (minor ? _minorNames[key] : _majorNames[key]);
}

int REKeySignature::SharpCount() const
{
    if(key > 7) {
        return key - 7;
    }
    return 0;
}
int REKeySignature::FlatCount() const
{
    if(key < 7) {
        return 7 - key;
    }
    return 0;
}

int REKeySignature::DetermineLinesOfAccidentals(Reflow::ClefType clef, int *outLines) const
{
    static int _lines[] = {7,3,6,2,5,1,4,  0,  0,3,-1,2,5,1,4};
    int dl = (clef == Reflow::BassClef ? 2 : 0);
    int idx = 0;
    if(key < 7) 
    {
        for(int i=6; i>=key; --i, ++idx) {
            outLines[idx] = _lines[i] + dl;
        }
    }
    else if(key > 7) 
    {
        for(int i=8; i<=key; ++i, ++idx) {
            outLines[idx] = _lines[i] + dl;
        }
    }
    return idx;
}

int REKeySignature::DetermineLinesOfNaturals(Reflow::ClefType clef, const REKeySignature& keyBefore, int *outLines) const
{
    int nbSharps = SharpCount();
    int nbFlats = FlatCount();
    int nbFlatsBefore = keyBefore.FlatCount();
    int nbSharpsBefore = keyBefore.SharpCount();
    
    if(nbSharps)
    {
        // Key signature now has sharps
        if(nbFlatsBefore) 
        {
            // Going from Flats to Sharps
            int nb = keyBefore.DetermineLinesOfAccidentals(clef, outLines);
            if(nb + nbSharps > 7) {
                return 7 - nbSharps;
            }
            else return nb;
        }
        
        // Key signature now has less sharps
        else if(nbSharps < nbSharpsBefore) {
            int sharps[7];
            keyBefore.DetermineLinesOfAccidentals(clef, sharps);
            for(int i=nbSharps; i<nbSharpsBefore; ++i) {
                outLines[i-nbSharps] = sharps[i];
            }
            return nbSharpsBefore - nbSharps;
        }
        
        // Key signature has more sharps than before
        return 0;
    }
    else if(nbFlats)
    {
        // Key signature now has Flats
        if(nbSharpsBefore)
        {
            // Going from sharps to flats
            int nb = keyBefore.DetermineLinesOfAccidentals(clef, outLines);
            if(nb + nbFlats > 7) {
                return 7 - nbFlats;
            }
            else return nb;
        }
        
        // Key signature now has less flats
        else if(nbFlats < nbFlatsBefore)
        {
            int flats[7];
            keyBefore.DetermineLinesOfAccidentals(clef, flats);
            for(int i=nbFlats; i<nbFlatsBefore; ++i) {
                outLines[i-nbFlats] = flats[i];
            }
            return nbFlatsBefore - nbFlats;
        }
        
        // Key signature has more flats than before
        return 0;
    }
    else
    {
        // Key signature is C / Am, simply return previous key signature's accidentals as naturals
        return keyBefore.DetermineLinesOfAccidentals(clef, outLines);
    }
}


static const char* _OttaviaNames[] = {"", "8vb", "8va", "15mb", "15ma"};


const char* Reflow::NameOfOttavia (Reflow::OttaviaType ottavia)
{
    int idx = (int)ottavia;
    if(idx >= 0 && idx < 5) {
        return _OttaviaNames[idx];
    }
    else return "";
}

static const char* _DirectionJumpNames[] = 
{
    "D.C.", "D.C. al Fine", "D.C. al Coda", "D.C. al Double Coda", 
    "D.S.", "D.S. al Fine", "D.S. al Coda", "D.S. al Double Coda", 
    "D.S.S.", "D.S.S. al Fine", "D.S.S. al Coda", "D.S.S. al Double Coda", 
    "To Coda", "To Double Coda"
};

const char* Reflow::NameOfDirectionJump(Reflow::DirectionJump jump)
{
    return _DirectionJumpNames[jump];
}

std::string Reflow::NameOfBendQuarterTones(int quarterTones)
{
    if(quarterTones == 0) return "0";
    if(quarterTones == 4) return "Full";
    
    int tones = quarterTones/4;
    int q = quarterTones % 4;
    
    std::ostringstream oss;
    if(tones >= 1) {
        oss << tones;
    }
    switch(q) {
        case 1: oss << RESpecialCharacters::OneQuarterFractionUTF8; break;
        case 2: oss << RESpecialCharacters::OneHalfFractionUTF8; break;
        case 3: oss << RESpecialCharacters::ThreeQuartersFractionUTF8; break;
    }
    return oss.str();
}

static const char* _NameOfTool[] =
{
    "",
    "hand",
    "zoom",
    "select",
    "barselect",
    "note",
    "tablature",
    "eraser",
    "design",
    "symbol",
    "line",
    "slur",
    "diagram",
    "clef",
    "key",
    "time",
    "barline",
    "repeat",
    "tempo",
    "section",
    "lyrics",
    "text",
    "livevelocity",
    "liveduration",
    "liveoffset",
    "spacer",
    "memo",
    "grace",
    "tuplet"
};

const char* Reflow::NameOfTool(Reflow::ToolType tool)
{
    return _NameOfTool[(int)tool];
}

static const char* _NameOfNumberGlyphs[] =
{
    "num0", "num1", "num2", "num3", "num4", "num5", "num6", "num7", "num8", "num9"
};

const char* Reflow::NameOfNumberGlyph(int num)
{
    return _NameOfNumberGlyphs[num%10];
}

static const char* _NameOfDynamics[] = 
{
    "ppp", "pp", "p", "mp", "mf", "f", "ff", "fff"
};

const char* Reflow::NameOfDynamics(Reflow::DynamicsType dynamics)
{
    return _NameOfDynamics[(int)dynamics];
}

static const char* _NameOfPaperOrientation[] = {"Portrait", "Landscape"};
static const char* _NameOfTrackType[] = {"Standard", "Tablature", "Drums"};
static const char* _NameOfTablatureInstrumentType[] = {"Guitar", "Bass", "Banjo", "Ukulele"};
static const char* _NameOfClefType[] = {"Treble", "Bass", "Neutral"};
static const char* _NameOfOttaviaType[] = {"", "8vb", "8va", "15mb", "15ma"};
static const char* _NameOfTempoUnitType[] = {"Quarter", "QuarterDotted"};

const char* Reflow::NameOfPaperOrientation(Reflow::PaperOrientation orientation)
{
    return _NameOfPaperOrientation[(int)orientation];
}

const char* Reflow::NameOfTrackType(Reflow::TrackType trackType)
{
    return _NameOfTrackType[(int)trackType];
}

const char* Reflow::NameOfTablatureInstrumentType(Reflow::TablatureInstrumentType tabType)
{
    return _NameOfTablatureInstrumentType[(int)tabType];
}

const char* Reflow::NameOfClefType(Reflow::ClefType clef)
{
    return _NameOfClefType[(int)clef];
}
const char* Reflow::NameOfOttaviaType(Reflow::OttaviaType ottavia)
{
    return _NameOfOttaviaType[(int)ottavia];
}
const char* Reflow::NameOfTempoUnitType(Reflow::TempoUnitType unitType)
{
    return _NameOfTempoUnitType[(int)unitType];
}

Reflow::PaperOrientation Reflow::ParsePaperOrientation(const std::string& str)
{
    if(str == "Portrait") return Reflow::Portrait;
    if(str == "Landscape") return Reflow::Landscape;
    return Reflow::Portrait;
}

Reflow::TrackType Reflow::ParseTrackType(const std::string& str)
{
    if(str == "Standard") return Reflow::StandardTrack;
    if(str == "Tablature") return Reflow::TablatureTrack;
    if(str == "Drums") return Reflow::DrumsTrack;
    return Reflow::StandardTrack;
}

Reflow::TablatureInstrumentType Reflow::ParseTablatureInstrumentType(const std::string& str)
{
    if(str == "Guitar") return Reflow::GuitarInstrument;
    if(str == "Bass") return Reflow::BassInstrument;
    if(str == "Banjo") return Reflow::BanjoInstrument;
    if(str == "Ukulele") return Reflow::UkuleleInstrument;
    return Reflow::GuitarInstrument;
}

Reflow::ClefType Reflow::ParseClefType(const std::string &str)
{
    if(str == "Treble") return Reflow::TrebleClef;
    if(str == "Bass") return Reflow::BassClef;
    if(str == "Neutral") return Reflow::NeutralClef;
    return Reflow::TrebleClef;
}

Reflow::OttaviaType Reflow::ParseOttaviaType(const std::string &str)
{
    if(str == "8vb") return Reflow::Ottavia_8vb;
    if(str == "8va") return Reflow::Ottavia_8va;
    if(str == "15mb") return Reflow::Ottavia_15mb;
    if(str == "15ma") return Reflow::Ottavia_15ma;
    return Reflow::NoOttavia;
}

Reflow::TempoUnitType Reflow::ParseTempoUnitType(const std::string& str)
{
    if(str == "Quarter") return Reflow::QuarterTempoUnit;
    if(str == "QuarterDotted") return Reflow::QuarterDottedTempoUnit;
    return Reflow::QuarterTempoUnit;
}

std::string Reflow::Tail (std::string const& source, size_t const length)
{
    if (length >= source.size()) { return source; }
    return source.substr(source.size() - length);
}

