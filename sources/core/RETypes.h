//
//  RETypes.h
//  ReflowMac
//
//  Created by Sebastien on 12/04/11.
//  Copyright 2011 Gargant Studios. All rights reserved.
//

#ifndef _RETYPES_H_
#define _RETYPES_H_

#ifdef _WIN32
#  define _USE_MATH_DEFINES
#endif

#ifndef REFLOW_QT
#  include "GSTypes.h"
#endif

#include <stdint.h>
#include <vector>
#include <string>
#include <map>
#include <set>
#include <deque>
#include <stack>
#include <algorithm>
#include <functional>
#include <memory>
#ifndef Q_MOC_RUN
#include <boost/rational.hpp>
#endif
#include <rapidjson/prettywriter.h>
#include <rapidjson/document.h>

#undef minor

#ifdef REFLOW_QT
#  include <QColor>
#  include <QPointF>
#  include <QSizeF>
#  include <QRectF>
#  include <QTransform>
#  include <qdebug.h>
#  include <QModelIndex>
#endif

#ifdef REFLOW_VERBOSE
#  ifdef REFLOW_QT
#    define RELog
	 void REPrintf(const char* fmt, ...);
#  else
#    define RELog NSLog
#    define REPrintf  printf
#  endif
#else
#  define RELog(...) 
#  define REPrintf(...)
#endif


#ifdef REFLOW_2
#  define REFLOW_IO_VERSION           (2000)
#else
#  define REFLOW_IO_VERSION           (1070)
#endif
#define REFLOW_IO_VERSION_1_8_0     (1080)
#define REFLOW_IO_VERSION_1_7_0     (1070)
#define REFLOW_IO_VERSION_1_7_0_b   (1002)
#define REFLOW_IO_VERSION_1_5_2     (1001)
#define REFLOW_IO_VERSION_1_5_1     (1000)

#define REFLOW_IO_GENERIC           (0)
#define REFLOW_IO_REFLOW2           (1)

#define REFLOW_MAX_STRINGS          (12)

#define REFLOW_MAX_TRACKS           (8*16)
#define REFLOW_MAX_VOICES           (4)
#define REFLOW_PULSES_PER_QUARTER   (480)
#define REFLOW_STEP_F1              (45)
#define REFLOW_STEP_C1              (42)
#define REFLOW_STEP_C0              (35)
#define REFLOW_STEP_A_minus_1       (33)
#define REFLOW_KEY_Cmajor           (7)
#define REFLOW_DEFAULT_UNIT_SPACING (7.0)


#define REFLOW_WORK_BUFFER_SIZE     (256)

#define REFLOW_MIN_TEMPO            (30)
#define REFLOW_MAX_TEMPO            (300)


#define REFLOW_URL_REFLOW_WEBSITE       "http://www.reflowapp.com"
#define REFLOW_URL_REFLOW_FORUMS        "https://reflowforums.com"
#define REFLOW_URL_REFLOW_WIKI          "http://www.reflowwiki.com"
#define REFLOW_URL_REFLOW_TWITTER       "http://twitter.com/reflowapp"
#define REFLOW_URL_REFLOW_FACEBOOK      "http://www.facebook.com/reflowapp"
#define REFLOW_URL_REFLOW_GITHUB        "http://github.com/gargant/reflow"
#define REFLOW_URL_REFLOW_DEVBLOG       "http://reflowapp.tumblr.com"

class REObject;
class REException;
class RESong;
class REBar;
class RETrack;
class REVoice;
class REPhrase;
class REChord;
class RENote;
class REScore;
class REScoreSettings;
class REStyle;
class RELayout;
class REScoreNode;
class REScoreRoot;
class REManipulator;
class REHandle;
class RESlurManipulator;
class REPage;
class REFrame;
class RESystem;
class REStaff;
class REStandardStaff;
class RETablatureStaff;
class RESlice;
class REPitchClass;
class REChordName;
class REChordFormula;
class REChordDiagram;
class REGrip;
class REBend;
class RESymbol;
class RELineSymbol;
class RESlur;
class REMultivoiceIterator;
class REBeatText;
class RESongController;
class REScoreController;
class REViewport;
class REViewportItem;
class REViewportPageItem;
class REViewportSystemItem;
class REViewportSliceItem;
class REViewportFrameItem;
class REViewportManipulatorItem;
class RETool;
class RENoteTool;
class RESlurTool;
class RERenderer;

class RECursor;
class RELocator;
class REOutputStream;
class REBufferOutputStream;
class REInputStream;
class REBufferInputStream;
class REConstBufferInputStream;
class RETrackSet;
class REBarMetrics;
class REPainter;
class REBezierPath;
class RELogger;

class REMusicalFont;
class REMusicalGlyph;

class REAudioEngine;
class REAudioSettings;
class REMidiEngine;
class REMidiInputPort;
class REMidiInputListener;
class RESequencer;
class RESequencerListener;
class REMusicRack;
class REMusicRackDelegate;
class REMusicDevice;
class REPlaylistBar;
class REMidiClip;
class REMonoSample;
class RESamplePlayer;
class REMonophonicSynthVoice;
class RESoundFont;
class RESampleGenerator;
class REWavFileWriter;
class REAudioStream;

class RESF2Patch;
class RESF2Generator;
class RESF2Synth;
class RESF2SynthVoice;
class RESF2GeneratorPlayer;
class RESynthChannel;

class RERenderDevice;
class RERenderTarget;
class RETexture;
class REWidget;

class RETable;
class RETableSection;
class RETableRow;

typedef RENote REGraceNote;

typedef std::vector<REGraceNote*> REGraceNoteVector;
typedef std::vector<REBar*> REBarVector;
typedef std::vector<RETrack*> RETrackVector;
typedef std::vector<REVoice*> REVoiceVector;
typedef std::vector<REPhrase*> REPhraseVector;
typedef std::vector<REChord*> REChordVector;
typedef std::vector<RENote*> RENoteVector;
typedef std::vector<REScore*> REScoreVector;
typedef std::vector<REScoreSettings*> REScoreSettingsVector;
typedef std::vector<REPage*> REPageVector;
typedef std::vector<RESystem*> RESystemVector;
typedef std::vector<REStaff*> REStaffVector;
typedef std::vector<RESlice*> RESystemBarVector;
typedef std::vector<RESlice*> RESliceVector;
typedef std::vector<REFrame*> REFrameVector;
typedef std::vector<REPlaylistBar> REPlaylistBarVector;
typedef std::vector<RESequencerListener*> RESequencerListenerVector;
typedef std::vector<REMusicRack*> REMusicRackVector;
typedef std::vector<REMusicDevice*> REMusicDeviceVector;
typedef std::vector<REMonophonicSynthVoice*> REMonophonicSynthVoiceVector;
typedef std::vector<RESynthChannel*> RESynthChannelVector;
typedef std::vector<RESF2Generator*> RESF2GeneratorVector;
typedef std::vector<REBezierPath*> REBezierPathVector;
typedef std::vector<REScoreController*> REScoreControllerVector;
typedef std::vector<REWidget*> REWidgetVector;
typedef std::vector<REManipulator*> REManipulatorVector;
typedef std::vector<REHandle*> REHandleVector;
typedef std::vector<RETool*> REToolVector;
typedef std::vector<RESymbol*> RESymbolVector;
typedef std::vector<RETableSection*> RETableSectionVector;
typedef std::vector<RETableRow*> RETableRowVector;

typedef std::vector<const REBar*> REConstBarVector;
typedef std::vector<const RETrack*> REConstTrackVector;
typedef std::vector<const REVoice*> REConstVoiceVector;
typedef std::vector<const REPhrase*> REConstPhraseVector;
typedef std::vector<const REChord*> REConstChordVector;
typedef std::vector<const RENote*> REConstNoteVector;
typedef std::vector<const REManipulator*> REConstManipulatorVector;

typedef uint16_t REUInt16;

typedef std::set<int> REIntSet;
typedef std::vector<int> REIntVector;
typedef std::vector<REUInt16> REUInt16Vector;
typedef std::set<const RENote*> REConstNoteSet;
typedef std::set<RENote*> RENoteSet;
typedef std::set<const RESystem*> REConstSystemSet;
typedef std::set<RESystem*> RESystemSet;
typedef std::pair<const REChord*, const REChord*> REConstChordPair;

typedef std::map<std::string, REManipulator*> REManipulatorMap;
typedef std::map<std::string, REHandle*> REHandleMap;

typedef std::set<uint32_t> RETickSet;

typedef std::vector<float> REFloatVector;
typedef std::vector<uint16_t> REUint16Vector;
typedef std::vector<std::string> REStringVector;
typedef std::vector<std::wstring> REWideStringVector;

typedef std::function<bool(const RENote*)>		RENotePredicate;
typedef std::function<bool(const REChord*)>		REChordPredicate;

typedef std::function<void(RESongController*)>	RESongControllerOperation;
typedef std::function<void(REScoreController*)>	REScoreControllerOperation;
typedef std::function<void(RESong*)>				RESongOperation;
typedef std::function<void(REBar*)>				REBarOperation;
typedef std::function<void(RENote*)>				RENoteOperation;
typedef std::function<void(REChord*)>				REChordOperation;

typedef std::function<void(RERenderDevice&)>        RERenderOperation;
typedef std::function<void(REPainter&)>             REPaintOperation;

typedef std::vector<RESongOperation>				RESongOperationVector;
typedef std::vector<REBarOperation>					REBarOperationVector;

#ifdef DEBUG
typedef rapidjson::PrettyWriter<REOutputStream> REJsonWriter;
#else
typedef rapidjson::Writer<REOutputStream> REJsonWriter;
#endif
typedef rapidjson::Document REJsonDocument;
typedef rapidjson::Value REJsonValue;


#ifdef REFLOW_MAC
typedef double REReal;
#else
typedef float REReal;
#endif

#define REFLOW_HOST_ENDIANNESS      (Reflow::LittleEndian)
#define REFLOW_IS_HOST_ENDIANNESS(x)    ((x) == REFLOW_HOST_ENDIANNESS)

#ifdef WIN32
#  define roundf(x)		(floor((x) + 0.5))
#  define roundtol(x)	(long)(roundf(x))
#endif

#ifdef EMSCRIPTEN
#  define roundtol(x)	(long)(roundf(x))
#endif

namespace Reflow
{
	enum PlatformType {
        MacPlatform,
        iPhonePlatform,
        iPadPlatform
    };

    enum TrackType {
        StandardTrack = 0,
        TablatureTrack = 1,
        DrumsTrack = 2
    };
    
    enum StaffType {
        StandardStaff = 0,
        TablatureStaff = 1,
    };
    
    enum StaffIdentifier {
        FirstStandardStaffIdentifier = 0,
        SecondStandardStaffIdentifier = 1,
        TablatureStaffIdentifier = 2
    };
    
    enum ScoreNodeType {
        GenericNode =   0,
        RootNode =      1,
        PageNode =      2,
        SystemNode =    3,
        StaffNode =     4,
        SliceNode =     5,
        FrameNode =     6,
        ManipulatorNode = 7,
        HandleNode = 8
    };
    
    enum ManipulatorType {
        GenericManipulator = 0,
        SlurManipulator = 1,
        SymbolManipulator = 2,
        
        NoteToolHoverManipulator,
    };
    
    enum TablatureInstrumentType {
        GuitarInstrument = 0,
        BassInstrument = 1,
        BanjoInstrument = 2,
        UkuleleInstrument = 3
    };
    
    enum MidiEventType {
        NoteOffEvent = 0x8,
        NoteOnEvent  = 0x9
    };
    
    enum TempoUnitType {
        QuarterTempoUnit = 0,
        QuarterDottedTempoUnit = 1
    };
    
    enum NoteValue {
        WholeNote = 0,
        HalfNote = 1,
        QuarterNote = 2,
        EighthNote = 3,
        SixteenthNote = 4,
        ThirtySecondNote = 5,
        SixtyFourthNote = 6
    };
    
    enum MarginLocation {
        TopMargin = 0,
        RightMargin,
        BottomMargin,
        LeftMargin
    };
    
    enum OttaviaType {
        NoOttavia = 0,
        Ottavia_8vb,
        Ottavia_8va,
        Ottavia_15mb,
        Ottavia_15ma
    };
    
    enum Accidental {
        NoAccidental = 0,
        Sharp,
        DoubleSharp,
        Flat,
        DoubleFlat,
        Natural
    };
    
    enum SymbolType
    {
        NullSymbol          = 0,
        TextSymbol          = 1,
        PickstrokeSymbol    = 2,
        BrushSymbol         = 3,
        AccentSymbol        = 4,
        StaccatoSymbol      = 5,
        FermataSymbol       = 6,
        DynamicsSymbol      = 7,
    };
    
    enum NoteHeadType {
        DefaultNoteHead = 0,
        CrossNoteHead,
        CircledCrossNoteHead,
        DiamondNoteHead,
        FilledDiamondNoteHead,
        DoubleSharpNoteHead,
    };
    
    enum GizmoType
    {
        FirstBarlineGizmo,
        MiddleBarlineGizmo,
        LastBarlineGizmo,
        
        NoteFretGizmo,
        NoteHeadGizmo,
        
        GizmoCount
    };
    
    enum ToolType
    {
        NoTool          = 0,
        HandTool,
        ZoomTool,
        SelectTool,
        BarSelectTool,
        NoteTool,
        TablatureTool,
        EraserTool,
        DesignTool,
        SymbolTool,
        LineTool,
        SlurTool,
        DiagramTool,
        ClefTool,
        KeyTool,
        TimeTool,
        BarlineTool,
        RepeatTool,
        TempoTool,
        SectionTool,
        LyricsTool,
        TextTool,
        LiveVelocityTool,
        LiveDurationTool,
        LiveOffsetTool,
        SpacerTool,
        MemoTool,
        GraceTool,
        TupletTool,
        
        ToolCount
    };
    
    enum PanelType {
        UnknownPanel = 0,
        ToolPanel,
        NavigatorPanel,
        InspectorPanel,
        ElementPanel,
        ScorePanel,
        SectionPanel,
        PlaybackPanel,
        OptionsPanel
    };
    
    enum TextAlignment {
        CenterTextAlign = 0,
        LeftTextAlign,
        RightTextAlign
    };
    
    enum MusicDeviceType {
        NullMusicDevice = 0,
        DLSMusicDevice,
        SynthMusicDevice
    };
    
    enum EnvelopePhase {
        InitialPhase    = 0,
        DelayPhase      = 1,
        AttackPhase     = 2,
        HoldPhase       = 3,
        DecayPhase      = 4,
        SustainPhase    = 5,
        ReleasePhase    = 6,
        FinalPhase      = 7
    };
    
    enum Endianness
    {
        LittleEndian = 0,
        BigEndian = 1
    };
    
    enum ClefType {
        TrebleClef = 0,
        BassClef,
        NeutralClef
    };
    
    enum BendType {
        NoBend              = 0,
        Bend                = 1,
        BendAndRelease      = 2,
        PreBend             = 3,
        PreBendAndRelease   = 4
    };
    
    enum SlideOutType {
        NoSlideOut = 0,
        ShiftSlide = 1,
        SlideOutHigh = 2,
        SlideOutLow = 3
    };
    
    enum SlideInType {
        NoSlideIn = 0,
        SlideInFromBelow = 1,
        SlideInFromAbove = 2
    };
    
    enum TextPositioning {
        TextAboveStandardStaff = 0,
        TextBelowStandardStaff,
        TextAboveTablatureStaff,
        TextBelowTablatureStaff,
    };
    
    enum DirectionJump {
        DaCapo                      = 0,
        DaCapo_AlFine               = 1,
        DaCapo_AlCoda               = 2,
        DaCapo_AlDoubleCoda         = 3,
        
        DalSegno                    = 4,
        DalSegno_AlFine             = 5,
        DalSegno_AlCoda             = 6,
        DalSegno_AlDoubleCoda       = 7,
        
        DalSegnoSegno               = 8,
        DalSegnoSegno_AlFine        = 9,
        DalSegnoSegno_AlCoda        = 10,
        DalSegnoSegno_AlDoubleCoda  = 11,
        
        ToCoda                      = 12,
        ToDoubleCoda                = 13,
        
        DirectionJump_Count         = 14
    };
    
    enum DirectionTarget {
        Coda        = 0,
        DoubleCoda  = 1,
        Segno       = 2,
        SegnoSegno  = 3,
        Fine        = 4,
        
        DirectionTarget_Count = 5
    };
    
    enum PaperOrientation {
        Portrait,
        Landscape
    };
    
    enum ScoreLayoutType
    {
        PageScoreLayout = 0,
        HorizontalScoreLayout = 1,
    };
    
    enum PageLayoutType
    {
        HorizontalPageLayout = 0,
        VerticalPageLayout,
        TwoColumnPageLayout,
    };
    
    enum DynamicsType {
        DynamicsUndefined = -1,
        DynamicsPPP     = 0,
        DynamicsPP      = 1,
        DynamicsP       = 2,
        DynamicsMP      = 3,        
        DynamicsMF      = 4,        
        DynamicsF       = 5,        
        DynamicsFF      = 6,        
        DynamicsFFF     = 7,   
        
        DynamicsCount   = 8
    };
    
    enum CloneOptions
    {
        IgnoreRehearsals = 1 << 0
    };
}

/** REAudioStream interface.
 */
class REAudioStream
{
public:
    virtual ~REAudioStream() {}
    
    virtual void Process(unsigned int nbSamples, float* workBufferL, float* workBufferR) = 0;
};


/** RESequencerListener
 */
class RESequencerListener
{
public:
    virtual void OnSequencerUpdateRT(const RESequencer* sequencer) = 0;
    
    virtual ~RESequencerListener() {}
};

/** REColor class.
 */
class REColor
{
public:
    float r, g, b, a;
    
public:
    REColor() : r(0), g(0), b(0), a(0) {}
    REColor(float r, float g, float b) : r(r), g(g), b(b), a(1.0f) {}
    REColor(float r, float g, float b, float a) : r(r), g(g), b(b), a(a) {}

#ifdef REFLOW_QT
    inline QColor ToQColor() const {return QColor(int(r*255.0), int(g*255.0), int(b*255.0), int(a*255.0));}
#endif
    
    REColor Darker() const {return REColor(r * .75, g * .75, b * .75);}
    REColor Inverted() const {return REColor(1.0-r, 1.0-g, 1.0-b, a);}
    
    static REColor Lerp(const REColor& a, const REColor& b, float t);
    
public:
    static const REColor White;
    static const REColor Black;
    static const REColor Red;
    static const REColor Green;
    static const REColor Blue;
    static const REColor DarkGray;
    static const REColor Gray;
    static const REColor LightGray;
    static const REColor TransparentBlack;
};


/** REPoint class.
 */
class REPoint
{
public:
    float x, y;
    
public:
    REPoint() : x(0), y(0) {}
    REPoint(float x, float y) : x(x), y(y) {}

#ifdef REFLOW_QT
    inline REPoint(const QPointF& point) : x(point.x()), y(point.y()) {}
    inline QPointF ToQPointF() const {return QPointF(x,y);}
#endif
    
    inline REPoint operator-() const {return REPoint(-x,-y);}
    
    REPoint Translated(float dx, float dy) const {return REPoint(x+dx, y+dy);}
    
    inline REPoint operator+(const REPoint& rhs) const {return REPoint(x+rhs.x, y+rhs.y);}
    inline REPoint operator-(const REPoint& rhs) const {return REPoint(x-rhs.x, y-rhs.y);}
    inline REPoint operator*(float t) const {return REPoint(x*t, y*t);}
    
    inline bool operator!=(const REPoint& rhs) const {return x != rhs.x || y != rhs.y;}
};


/** RESize class.
 */
class RESize
{
public:
    float w, h;
    
public:
    RESize() : w(0), h(0) {}
    RESize(float w, float h) : w(w), h(h) {}

#ifdef REFLOW_QT
    inline RESize(const QSize& qs) : w(qs.width()), h(qs.height()) {}
    
    inline QSizeF ToQSizeF() const {return QSizeF(w,h);}
    inline QSize ToQSize() const {return QSize(w,h);}
#endif
};


/** RERect class.
 */
class RERect
{
public:
    REPoint origin;
    RESize size;
    
public:
    RERect() : origin(0,0), size(0,0) {}
    RERect(const REPoint& origin, const RESize& size) : origin(origin), size(size) {}
    RERect(double x, double y, double w, double h) : origin(x,y), size(w,h) {}
    
#ifdef REFLOW_QT
    explicit RERect(const QRectF& rc) : origin(rc.x(), rc.y()), size(rc.width(), rc.height()) {}
    inline QRectF ToQRectF() const {return QRectF(origin.ToQPointF(), size.ToQSizeF());}
#endif
    
    RERect Translated(float dx, float dy) const {return RERect(origin.Translated(dx,dy), size);}
    RERect Translated(const REPoint& pt) const {return RERect(origin.Translated(pt.x,pt.y), size);}
    
    float MiddleX() const {return origin.x + 0.5 * size.w;}
    float MiddleY() const {return origin.y + 0.5 * size.h;}
    float Left() const {return origin.x;}
    float Right() const {return origin.x + size.w;}
    float Top() const {return origin.y;}
    float Bottom() const {return origin.y + size.h;}
    float Width() const {return size.w;}
    float Height() const {return size.h;}
    float HalfWidth() const {return 0.5*size.w;}
    float HalfHeight() const {return 0.5*size.h;}
    
    REPoint Center() const {return REPoint(MiddleX(),MiddleY());}
    
    bool PointInside(const REPoint& pt) const;
    bool Contains(const RERect& rc) const;
    
    RERect Union(const RERect& rc) const;
    RERect Inset(float top, float left, float bottom, float right) const;

    static bool Intersects(const RERect& a, const RERect& b);
    static RERect FromPoints(const REPoint& a, const REPoint& b);
    
    RERect WithOriginAtZero() const {return RERect(0, 0, size.w, size.h);}
};

typedef std::vector<RERect> RERectVector;


/** REPadding class
 */
class REPadding
{
public:
    float top, right, bottom, left;
    
    REPadding() : top(0), right(0), bottom(0), left(0) {}
    REPadding(float top_, float right_, float bottom_, float left_)
        : top(top_), right(right_), bottom(bottom_), left(left_)
    {}
};


/** REAffineTransform class.
 *
 *  | m11 m12  0 |
 *  | m21 m22  0 |
 *  |  tx  ty  1 |
 */
class REAffineTransform
{
public:
    float m11, m12, m21, m22, tx, ty;
    
    REAffineTransform() : m11(1), m12(0), m21(0), m22(1), tx(0), ty(0) {}
    
    REPoint Transform(const REPoint& pt) const
    {
        float x = m11 * pt.x + m21 * pt.y + tx;
        float y = m22 * pt.y + m12 * pt.x + ty;
        return REPoint(x,y);
    }
    
    static REAffineTransform Scaling(float dx, float dy)
    {
        REAffineTransform res;
        res.m11 = dx;
        res.m22 = dy;
        return res;
    }
    
    static REAffineTransform Translation(float dx, float dy)
    {
        REAffineTransform res;
        res.tx = dx;
        res.ty = dy;
        return res;
    }
    
    static REAffineTransform Multiply(const REAffineTransform& a, const REAffineTransform& b)
    {
        REAffineTransform res;
        res.m11 = a.m11 * b.m11 + a.m12 * b.m21;
        res.m12 = a.m11 * b.m12 + a.m12 * b.m22;
        res.m21 = a.m21 * b.m11 + a.m22 * b.m21;
        res.m22 = a.m21 * b.m12 + a.m22 * b.m22;
        res.tx  = a.tx  * b.m11 + a.ty  * b.m21 + b.tx;
        res.ty  = a.tx  * b.m12 + a.ty  * b.m22 + b.ty;
        return res;
    }
    
#ifdef REFLOW_QT
    inline QTransform ToQTransform() const 
    {
        return QTransform(m11, m12, m21, m22, tx, ty);
    }
#endif
    
};



template<typename T>
class RESmoother
{
public:
    T src;
    T dest;
    T cur;
    float time;
    float speed;
    
public:
    RESmoother()
    : time(1.0), speed(3.0)
    {
    }
    
    void SetTime(float t)
    {
        time = std::min<float>(1.0f, std::max<float>(0.0f, t));
        float k = 2.0 * time - (time*time);
        cur = src * (1.0 - k) + dest * k;
    }
    
    void Set(T val)
    {
        src = cur;
        dest = val;
        time = 0.0;
    }
    
    void Reset(T val)
    {
        src = dest = cur = val;
        time = 1.0;
    }
    
    void Update(float delta)
    {
        SetTime(time + speed * delta);
    }
};

typedef RESmoother<REPoint> REPointSmoother;



// on Mac {m11, m12, m21, m22, tx, ty} maps to NSAffineTransformStruct {m11, m12, m21, m22, tx, ty}
// on iOS {m11, m12, m21, m22, tx, ty} maps to CGAffineTransform {a, b, c, d, tx, ty}
// on Qt  convert with QTransform ( m11, m12, m21, m22, dx, dy )

/** RERange class.
 */
class RERange
{
public:
    int index, count;
    
    RERange() : index(0), count(0) {}
    explicit RERange(int index) : index(index), count(1) {}
    RERange(int index, int count) : index(index), count(count) {}
    
    int FirstIndex() const {return index;}
    int LastIndex() const {return index+count-1;}
    
    bool IsInRange(int i) const {return (i >= index && i<(index+count));}
    
    bool IntersectsRange(const RERange& rg) const {return FirstIndex() <= rg.LastIndex() && LastIndex() >= rg.FirstIndex();}
};


/** RETimeDiv class.
 */
typedef boost::rational<int32_t> RETimeDiv;


/** REIndexPair class.
 */
typedef std::pair<int,int> REIndexPair;

/** REGlobalTick class.
 */
class REGlobalTick
{
public:
    int bar, tick;
    
    REGlobalTick() : bar(0), tick(0) {}
    REGlobalTick(int bar_, int tick_) : bar(bar_), tick(tick_) {}
    
    bool operator==(const REGlobalTick& rhs) const {
        return tick == rhs.tick && bar == rhs.bar;
    }
    
    bool operator!=(const REGlobalTick& rhs) const {
        return tick != rhs.tick || bar != rhs.bar;
    }
    
    bool operator < (const REGlobalTick& rhs) const {
        if(bar < rhs.bar) {
            return true;
        }
        else if(bar > rhs.bar) {
            return false;
        }
        else {  // bar == rhs.bar
            return tick < rhs.tick;
        }
    }
    
    bool operator <= (const REGlobalTick& rhs) const {
        if(bar < rhs.bar) {
            return true;
        }
        else if(bar > rhs.bar) {
            return false;
        }
        else {  // bar == rhs.bar
            return tick <= rhs.tick;
        }
    }
    
    bool operator >  (const REGlobalTick& rhs) const {return !(*this <= rhs);}
    bool operator >= (const REGlobalTick& rhs) const {return !(*this < rhs);}
};



/** REGlobalTimeDiv
 */
class REGlobalTimeDiv
{
public:
    int bar;
    RETimeDiv timeDiv;
    
    REGlobalTimeDiv() : bar(0), timeDiv(0) {}
    REGlobalTimeDiv(int bar_, const RETimeDiv& tick_) : bar(bar_), timeDiv(tick_) {}
    REGlobalTimeDiv(const REGlobalTimeDiv& td) : bar(0), timeDiv(0) {
        *this = td;
    }
    
    REGlobalTimeDiv& operator=(const REGlobalTimeDiv& td) {
        bar = td.bar;
        timeDiv = td.timeDiv;
        return *this;
    }
    
    bool operator==(const REGlobalTimeDiv& rhs) const {
        return timeDiv == rhs.timeDiv && bar == rhs.bar;
    }
    
    bool operator!=(const REGlobalTimeDiv& rhs) const {
        return timeDiv != rhs.timeDiv || bar != rhs.bar;
    }
    
    bool operator < (const REGlobalTimeDiv& rhs) const {
        if(bar < rhs.bar) {
            return true;
        }
        else if(bar > rhs.bar) {
            return false;
        }
        else {  // bar == rhs.bar
            return timeDiv < rhs.timeDiv;
        }
    }
    
    bool operator <= (const REGlobalTimeDiv& rhs) const {
        if(bar < rhs.bar) {
            return true;
        }
        else if(bar > rhs.bar) {
            return false;
        }
        else {  // bar == rhs.bar
            return timeDiv <= rhs.timeDiv;
        }
    }
    
    bool operator >  (const REGlobalTimeDiv& rhs) const {return !(*this <= rhs);}
    bool operator >= (const REGlobalTimeDiv& rhs) const {return !(*this < rhs);}
            
    void EncodeTo(REOutputStream& coder) const;
    void DecodeFrom(REInputStream& decoder);
};


/** RETimeSignature class.
 */
class RETimeSignature
{
public:
    uint8_t numerator, denominator;
    
    RETimeSignature() : numerator(4), denominator(4) {}
    RETimeSignature(unsigned int num, unsigned int den) : numerator(num), denominator(den) {}
    
    bool operator != (const RETimeSignature& ts) const {return numerator != ts.numerator || denominator != ts.denominator;}
    bool operator == (const RETimeSignature& ts) const {return numerator == ts.numerator && denominator == ts.denominator;}
};


/** REKeySignature class.
 */
class REKeySignature
{
public:
    int8_t key;     // [0 .. 14]
    bool minor;
    
    REKeySignature() : key(7), minor(false) {}
    REKeySignature(int key_, bool minor_) : key(key_), minor(minor_) {}
    
    bool operator!= (const REKeySignature& ks) const {return key != ks.key || minor != ks.minor;}
    bool operator== (const REKeySignature& ks) const {return key == ks.key && minor == ks.minor;}
    
    const char* Name() const;
    
    int SharpCount() const;
    int FlatCount() const;
    
    int DetermineLinesOfAccidentals(Reflow::ClefType clef, int *outLines) const;
    int DetermineLinesOfNaturals(Reflow::ClefType clef, const REKeySignature& keyBefore, int *outLines) const;    
};
        

/** REBeamingPattern class.
 */
class REBeamingPattern
{
    friend class REBar;
    
public:
    enum {
        MaxGroupCount = 4
    };
    
public:
    REBeamingPattern();
    bool Parse(const char* str, int len=-1);
    void Clear();
    std::string ToString() const;
    int ToTickPattern(unsigned long* pattern) const;
    
    int GroupCount() const {return _groupCount;}
    int Group(int idx) const {return _groups[idx];}
    
private:
    int _groupCount;
    int8_t _groups[MaxGroupCount];
};
        

/** RETuplet class.
 */
class RETuplet
{
public:
    uint8_t tuplet, tupletFor;
    
    RETuplet() : tuplet(0), tupletFor(0) {}
    RETuplet(unsigned int tuplet_, unsigned int tupletFor_) : tuplet(tuplet_), tupletFor(tupletFor_) {}
    
    bool operator==(const RETuplet& rhs) const {return tuplet == rhs.tuplet && tupletFor == rhs.tupletFor;}
    
    bool IsValid() const {return tuplet != 0 && tupletFor != 0;}
};


/** RENotePitch class.
 */
class RENotePitch
{
public:
	int8_t midi;		// [0 .. 127]
	int8_t step;		// [CDEFGAB]
	int8_t octave;		// [0 .. 7]
	int8_t alter;		// [-2 .. +2]
	
	RENotePitch() :midi(0), step(0), octave(0), alter(0) {}
    RENotePitch(const RENotePitch& rhs) {*this = rhs;}
    
    RENotePitch& operator=(const RENotePitch& rhs) {
        midi = rhs.midi; 
        step = rhs.step;
        octave = rhs.octave;
        alter = rhs.alter;
        return *this;
    }
    
    std::string NoteName() const;
    
    RENotePitch Transposed(const REPitchClass& interval) const;
};

/** REStringFretPair class.
 */
class REStringFretPair
{
public:
    int8_t string;
    int8_t fret;
    
    REStringFretPair() : string(0), fret(0) {}
    REStringFretPair(int string_, int fret_) : string(string_), fret(fret_) {}
    REStringFretPair(const REStringFretPair& rhs) {*this = rhs;}
    
    REStringFretPair& operator= (const REStringFretPair& rhs) {
        string = rhs.string;
        fret = rhs.fret;
        return *this;
    }
    
    bool operator< (const REStringFretPair& rhs) const {
        if(string < rhs.string) return true;
        else if(string == rhs.string) {
            return fret < rhs.fret;
        }
        return false;
    }
};

typedef std::vector<REStringFretPair> REStringFretPairVector;


/** REDrumMapping struct
 */
struct REDrumMapping {
    int8_t midi;
    int8_t line;
    int8_t noteHeadSymbol;
    const char* name;
};

/** REGizmo struct
 */
struct REGizmo
{
    uint32_t type;              // Reflow::GizmoType
    RERect box;                 // bounding box of the gizmo (in system space)
    const REObject* object;     // Object this gizmo refers to
    uint16_t barIndex;
};

typedef std::vector<REGizmo> REGizmoVector;

            
/** REFontDesc struct
 */
struct REFontDesc
{
    std::string name;
    float size;
    bool italic;
    bool bold;
};
            


/** REPacketRingBuffer template class.
 */
template<typename T, int N>
class REPacketRingBuffer
{
public:
    REPacketRingBuffer () : _wp(0), _rp(0) {
        for(int i=0; i<N; ++i) {_data[i] = T();}
    }
    
    ~REPacketRingBuffer() {}
    
    void Push (const T& msg) {
        _data[_wp] = msg;
        _wp = (_wp+1) % N;
    }
    
    T Pop () {
        if(_wp != _rp) {
            T msg = _data[_rp];
            _rp = (_rp+1) % N;
            return msg;
        }
        else return T();
    }
    
    bool PacketAvailable() const {return _wp != _rp;}
    
private:
    int _wp;
    int _rp;
    T _data[N];
};


/** REMidiEvent struct.
 */
struct REMidiEvent
{
    int32_t tick;
    uint8_t data[3];
    
    inline int8_t Type() const {return (data[0] & 0xF0)>>4;}
    inline int8_t Channel() const {return (data[0] & 0x0F);}
    inline int8_t Pitch() const {return data[1];}
    inline int8_t Velocity() const {return data[2];}
    
    inline void SetType(int8_t type) {data[0] = (type << 4) | Channel();}
    inline void SetChannel(int8_t channel) {data[0] = (Type() << 4) | channel;}
    inline void SetPitch(int8_t pitch) {data[1] = pitch;}
    inline void SetVelocity(int8_t vel) {data[2] = vel;}
};

typedef std::vector<REMidiEvent> REMidiEventVector;
typedef std::pair<REMidiEvent, REMidiEvent> REMidiEventPair;



/** REMidiNoteEvent struct.
 */
struct REMidiNoteEvent
{
    enum MidiNoteFlags {
        UseSoundOff = 0x01
    };
    
    int32_t tick;
    uint32_t duration;
    uint8_t channel;
    uint8_t pitch;
    uint8_t velocity;
    uint8_t flags;
    
    REMidiEventPair SplitIntoNoteOnAndNoteOffEvents(bool overrideChannel) const
    {
        REMidiEvent noteOn;
        noteOn.tick = tick;
        if(overrideChannel) {
            noteOn.data[0] = 0x90;      // override channel to 0
        }
        else {
            noteOn.data[0] = 0x90 | channel;
        }
        noteOn.data[1] = pitch;
        noteOn.data[2] = velocity;
        
        REMidiEvent noteOff = noteOn;
        noteOff.tick += duration;
        /*if(flags & REMidiNoteEvent::UseSoundOff) {
            noteOff.type = 0xB;         // Channel message
            noteOff.pitch = 120;        // Sound off
            noteOff.velocity = 0;
        }
        else {*/
            noteOff.SetType(0x8);
        /*}*/
        
        return REMidiEventPair(noteOn, noteOff);
    }
};

typedef std::vector<REMidiNoteEvent> REMidiNoteEventVector;



/** REMidiInstantPacket
 */
struct REMidiInstantPacket 
{
    uint8_t port;
    uint8_t data[3];
    
    REMidiInstantPacket() {}
    REMidiInstantPacket(const REMidiInstantPacket& rhs) : port(rhs.port) {
        memcpy(data, rhs.data, sizeof(data));
    }
    
    REMidiInstantPacket* Clone() const {
        return new REMidiInstantPacket(*this);
    }
};


typedef REPacketRingBuffer<REMidiInstantPacket, 4096> REMidiInstantPacketRingBuffer;


/** REMidiInputListener interface.
 */
class REMidiInputListener
{
public:
    virtual void OnMidiPacketReceived(const REMidiInstantPacket* inPacket) =0;
};


/** REMusicRackDelegate interface.
 */
class REMusicRackDelegate
{
public:
    virtual void WillRenderRack (REMusicRack* rack, unsigned int nbFrames, float* workBufferL, float* workBufferR) = 0;
    virtual void DidRenderRack (REMusicRack* rack, unsigned int nbFrames, float* workBufferL, float* workBufferR) = 0;
    
    virtual void WillRenderDevice (REMusicRack* rack, REMusicDevice* device, unsigned int nbFrames, float* workBufferL, float* workBufferR) = 0;
    virtual void DidRenderDevice (REMusicRack* rack, REMusicDevice* device, unsigned int nbFrames, float* workBufferL, float* workBufferR) = 0;
};


/** REBot
 */
class REBot
{
public:
    REBot() {}
    virtual ~REBot() {}
    
    virtual void GenerateBar(RETrack* track, int barIndex) = 0;
};

#endif
