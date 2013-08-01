//
//  REStaff.h
//  Reflow
//
//  Created by Sebastien on 01/05/11.
//  Copyright 2011 Gargant Studios. All rights reserved.
//

#ifndef _RESTAFF_H_
#define _RESTAFF_H_

#include "RETypes.h"
#include "RESlur.h"
#include "RESymbol.h"

class REStaff
{
    friend class RESystem;
    friend class REScore;
    friend class RELayout;
    
public:
    enum StaffFlag {
        HighVoiceTupletBand = (1 << 0),
        LowVoiceTupletBand  = (1 << 1),
        AccentBand          = (1 << 2),
        VibratoBand         = (1 << 3),
        TappingBand         = (1 << 4),
        StrummingBand       = (1 << 5),
        StickingBand        = (1 << 6),
        DynamicsBand        = (1 << 7),
        TextAboveStaffBand  = (1 << 8),
        TextBelowStaffBand  = (1 << 9),
        OttaviaBand         = (1 << 10),
        PalmMuteBand        = (1 << 11),
        LetRingBand         = (1 << 12),
        ChordDiagramBand    = (1 << 13),
        
        HideRhythm          = (1 << 16)
    };
    
    struct BeamCache {
        float x, yStem;
        int16_t chordIndex;
        int16_t sbarIndex;
        int8_t minLine, maxLine;
        int8_t flags;
    };
    typedef std::vector<BeamCache> BeamCacheVector;
    
    enum BeamCacheFlags {
        BeamIsRest = 0x01,
        BeamIsStemDown = 0x02
    };
    
public:
    REStaff();
    virtual ~REStaff();
    
public:
    const RESystem* System() const {return _parent;}
    RESystem* System() {return _parent;}
    int Index() const {return _index;}
    
    Reflow::StaffType Type() const {return _type;}
    
    const RETrack* Track() const {return _track;}
    
    float YOffset() const {return _yOffset;}
    float Height() const {return _height;}
    float TopSpacing() const {return _topSpacing;}
    float BottomSpacing() const {return _bottomSpacing;}
    float Interline() const {return _interline;}
    
    float YOffsetOfLine(int lineIndex) const;
    int LineAtYOffset(float y) const;
    bool IsYOffsetInside(float y, float* relativeY) const;
    
    const BeamCache* BeamCacheForChord(unsigned int chordIndex, unsigned int voiceIndex, unsigned int sbarIndex) const;
    
    bool FindCenterOfNote(const RENote*, REPoint* outCenter) const;
    
    bool ShouldDrawAccentsWithNoteHead() const {return true;}
    float ChordDiagramHeight() const;
    float ChordDiagramSize() const {return 5.0;}

    float YOffsetOfTupletLine(int voice) const;
    float YOffsetOfSticking() const;
    float YOffsetOfAccent() const;
    float YOffsetOfStrumming() const;
    float YOffsetOfVibrato() const;
    float YOffsetOfOttavia() const;
    float YOffsetOfTextPositionedAbove() const {return _textAboveYOffset;}
    float YOffsetOfTextPositionedBelow() const {return _textBelowYOffset;}
    float YOffsetOfPalmMute() const {return _palmMuteAndLetRingYOffset;}
    float YOffsetOfLetRing() const {return _palmMuteAndLetRingYOffset;}
    float YOffsetOfTSP() const {return _tappingYOffset;}
    float YOffsetOfChordDiagram() const {return _chordDiagramYOffset;}
    float YOffsetOfDynamics() const {return _dynamicsYOffset;}
    
    bool HasFlag(StaffFlag flag) const {return 0 != (_flags & flag);}
    void SetFlag(StaffFlag flag) {_flags |= flag;}
    void UnsetFlag(StaffFlag flag) {_flags &= ~flag;}
    
    virtual int HighVoiceIndex() const {return 0;}
    virtual int LowVoiceIndex() const {return 1;}
    
    virtual int FirstVoiceIndex() const {return 0;}
    virtual int LastVoiceIndex() const {return 2;}
    
    void SetIdentifier(Reflow::StaffIdentifier id_) {_identifier = id_;}
    Reflow::StaffIdentifier Identifier() const {return _identifier;}

    virtual void ListNoteGizmos(REGizmoVector& gizmos) const = 0;
    
    inline const REConstSlurPtrVector& Slurs() const {return _slurs;}
    
    virtual void IterateSymbolsOfSlice(int sliceIndex, const REConstSymbolAnchorOperation& op) const = 0;
    void IterateSymbols(const REConstSymbolAnchorOperation& op) const;
    
public:
    virtual unsigned int LineCount() const;
    virtual float UnitSpacing() const = 0;
    
    virtual void CalculateBeamingCoordinates(const REChord* firstChord, const REChord* lastChord, BeamCache* beamCache);
    
public:
    static void DrawTimeSignature(REPainter& painter, const RETimeSignature& timeSignature, const REPoint& pt, float size);
    
protected:
    virtual void DrawSlice(REPainter& painter, int sliceIndex) const = 0;
    virtual void DrawBarlines(REPainter& painter) const;
    virtual void DrawBeamingAndRests(REPainter& painter) const;
    virtual void DrawTimeSignature(REPainter& painter, const RETimeSignature& timeSignature, const REPoint& pt) const;
    void DrawSlur(REPainter& painter, const RESlur& slur) const;
    
    virtual void DrawBarlinesOfSlice(REPainter& painter, int sliceIndex) const;
    virtual void DrawBeamingAndRestsOfSlice(REPainter& painter, int sliceIndex) const;
    
    void DrawBrush(REPainter& painter, float x, float y0, float y1, bool down) const;
    void DrawArpeggio(REPainter& painter, float x, float y0, float y1, bool down) const;
    
    void DrawChordDiagramBand(REPainter& painter, float y, const RERange& sliceRange) const;
    void DrawVibratoBand(REPainter& painter, float y, const RERange& sliceRange) const;
    void DrawPalmMuteBand(REPainter& painter, float y, const RERange& sliceRange) const;
    void DrawLetRingBand(REPainter& painter, float y, const RERange& sliceRange) const;
    void DrawOttaviaBand(REPainter& painter, Reflow::OttaviaType ottavia, float y, const RERange& sliceRange) const;
    void DrawPickstrokeBand(REPainter& painter, float y, const RERange& sliceRange) const;
    void DrawTSPBand(REPainter& painter, float y, const RERange& sliceRange) const;    
    void DrawTextBand(REPainter& painter, Reflow::TextPositioning positioning, float y, const RERange& sliceRange) const;
    void DrawDynamicsBand(REPainter& painter, float y, const RERange& sliceRange) const;
    
protected:
    virtual void _DrawBeaming(REPainter& painter, const RESlice* sbar, const REChord* chord, float y, int orientation) const;
    virtual void _DrawStem(REPainter& painter, const RESlice* slice, const REChord* chord, const BeamCache* beam) const;
    virtual void _DrawSingleStem(REPainter& painter, const RESlice* sbar, const REChord* chord, float y, int orientation) const = 0;
    virtual void _DrawTupletGroup(REPainter& painter, const RESlice* sbar, const REChord* chord, float y, int orientation) const;
    virtual void _RefreshMetrics();
    
    virtual void _DrawBand(REPainter& painter, int flag, float y, const RERange& sliceRange) const;
    virtual void _DrawBandPartial(REPainter& painter, int flag, float startX, float endX, float y) const;
    virtual void _DrawOttaviaBandPartial(REPainter& painter, Reflow::OttaviaType ottavia, float startX, float endX, float y) const;
    
    void _ClearBeamCache();
    void _CalculateBeamCache();
    
protected:
    static int _CalculateInterchordBeamType(const REChord* c0, const REChord* c1, int noteValue);
    static void _CloseCurrentBeam(REPainter& painter, float xStart, float yStart, float xEnd, float yEnd, float xOffset, float yOffset, float h);
    
protected:
    RESystem* _parent;
    int _index;
    uint32_t _flags;
    Reflow::StaffType _type;
    Reflow::StaffIdentifier _identifier;
    const RETrack* _track;
    float _yOffset, _height, _topSpacing, _bottomSpacing;
    float _interline;
    BeamCacheVector _beams[REFLOW_MAX_VOICES];
    REConstSlurPtrVector _slurs;
    
protected:
    float _tupletLineYOffset[2];
    float _accentYOffset;
    float _stickingYOffset;    
    float _strummingYOffset;
    float _tappingYOffset;
    float _dynamicsYOffset;
    float _ottaviaYOffset;
    float _vibratoYOffset;
    float _textAboveYOffset;
    float _textBelowYOffset;
    float _palmMuteAndLetRingYOffset;
    float _chordDiagramYOffset;
    float _yMinBeaming;
    float _yMaxBeaming;
};

#endif
