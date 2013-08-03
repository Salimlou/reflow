//
//  REScoreSettings.h
//  Reflow
//
//  Created by Sebastien Bourgeois on 11/06/12.
//  Copyright (c) 2012 __MyCompanyName__. All rights reserved.
//

#ifndef Reflow_REScoreSettings_h
#define Reflow_REScoreSettings_h

#include "RETypes.h"
#include "RETrackSet.h"


class REScoreSettings
{
    friend class RESong;
    friend class REScore;
    friend class REArchive;
    
public:
    enum ScoreFlag {
        TransposingScore = 0x00000001,
        //DirtyScore = 0x00000002,
        HideTablature = 0x00000004,
        HideStandard = 0x00000008,
        
        UseMultiRests =     0x00000010,
        
        HideTempo =         0x00000100,
        HideRehearsal =     0x00000200,
        HideDynamics =      0x00000400
    };
    
public:
    REScoreSettings();
    REScoreSettings(const REScoreSettings& rhs);
    ~REScoreSettings();
    
    int Index() const {return _index;}
    
    bool HasFlag(ScoreFlag flag) const {return 0 != (_flags & flag);}
    void SetFlag(ScoreFlag flag) {_flags |= flag;}
    void UnsetFlag(ScoreFlag flag) {_flags &= ~flag;}
    
    const RESize& PaperSize() const {return _paperSize;}
    RESize PaperSizeInMillimeters() const;
    void SetPaperSize(const RESize& sz) {_paperSize = sz;}
    
    void SetPaperOrientation(Reflow::PaperOrientation orientation) {_paperOrientation = orientation;}
    Reflow::PaperOrientation PaperOrientation() const {return _paperOrientation;}
    
    void SetScalingFactor(float scaling) {_scalingFactor = scaling;}
    float ScalingFactor() const {return _scalingFactor;}
    
    void SetPaperName(const std::string& paperName) {_paperName = paperName;}
    const std::string& PaperName() const {return _paperName;}
    
    void SetName(const std::string& nameUTF8) {_name= nameUTF8;}
    const std::string& Name() const {return _name;}
    
    double VirtualMargin(Reflow::MarginLocation margin) const {return _virtualMargins[margin];}
    double VirtualMarginInMillimeters(Reflow::MarginLocation margin) const;
    RERect ContentRect() const;
    RERect PageRect() const;
    
    bool InConcertTone() const;
    void SetInConcertTone(bool ct);
    
    bool HasSystemBreakAtBarIndex(int barIndex) const;
    void SetSystemBreakAtBarIndex(int barIndex, bool systemBreak);
    
    void SetTrackSet(const RETrackSet& trackSet);
    const RETrackSet& TrackSet() const {return _trackSet;}
    
    const RETrack* FirstTrack(const RESong*) const;
    REConstTrackVector Tracks(const RESong* song) const;
    RETrackVector Tracks(RESong* song) const;
    void SetTracks(const RETrackVector& tracks);
    
    REScoreSettings* Clone() const;
    
    const REStyle* Style() const {return _style;}
    REStyle* Style() {return _style;}
    void SetStyle(REStyle* style);
    
    const RELayout* Layout() const {return _layout;}
    RELayout* Layout() {return _layout;}
    void SetLayout(RELayout* layout);
    
    void EncodeTo(REOutputStream& coder) const;
	void DecodeFrom(REInputStream& decoder);
    
    void WriteJson(REJsonWriter& writer, uint32_t version) const;
    void ReadJson(const REJsonValue& obj, uint32_t version);
    
public:
    REScoreSettings& operator=(const REScoreSettings& rhs);
    
    bool operator==(const REScoreSettings& rhs) const;
    bool operator!=(const REScoreSettings& rhs) const {return !(*this == rhs);}
    
private:
    int _index;    
    uint32_t _flags;
    std::string _name;
    std::string _paperName;
    RESize _paperSize;
    float _scalingFactor;
    Reflow::PaperOrientation _paperOrientation;
    double _virtualMargins[4];
    RETrackSet _trackSet;
    REIntSet _systemBreaks;
    RELayout* _layout;
    REStyle* _style;
};


#endif
