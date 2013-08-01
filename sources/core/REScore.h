//
//  REScore.h
//  Reflow
//
//  Created by Sebastien on 13/04/11.
//  Copyright 2011 Gargant Studios. All rights reserved.
//

#ifndef _RESCORE_H_
#define _RESCORE_H_

#include "RETypes.h"
#include "RETrackSet.h"
#include "REScoreSettings.h"


class REScore
{
    friend class RESong;
    friend class REScoreController;
    
public:
    REScore(const RESong*);
    ~REScore();
    
public:
    const RESystemVector& Systems() const {return _systems;}
    RESystemVector& Systems() {return _systems;}
    
    const RESystem* System(int idx) const;
    RESystem* System(int idx);
    
    const REScoreRoot* Root() const {return _root;}
    REScoreRoot* Root() {return _root;}
    
    const REScoreSettings& Settings() const {return _settings;}
    
    unsigned int SystemCount() const {return (unsigned int)_systems.size();}
    
    const RESystem* PickSystem(const REPoint& pos) const;
    
    void InsertSystem(RESystem* sys, int idx);
    void RemoveSystem(int idx);
    
    void Clear();
    void ForceSystemReflow();
    void Refresh();
    void RefreshSingleBar(int barIndex);
    void Rebuild(const REScoreSettings& settings);
    
    float ContinuousXOffsetOfSystem(const RESystem* system) const;
    
    const RESong* Song() const {return _parent;}
    
    unsigned int TrackCount() const;
    const RETrack* Track(int indexInScore) const;
    bool ContainsTrack(const RETrack* track) const;
    int IndexOfTrackInScore(const RETrack* track) const;
    const REConstTrackVector& Tracks() const {return _tracks;}
    void SetTracks(const RETrackVector& tracks);
    const RETrack* FirstTrack() const;
    
    bool IsTransposing() const;
    RERect ContentRect() const;
    RERect PageRect() const;
    unsigned int PageCount() const;
    
    RESize ContentSize() const {return _contentSize;}
    void SetContentSize(const RESize& sz) {_contentSize = sz;}
    
    void CalculateBarMetrics(unsigned int barIndex, REBarMetrics& outBarMetrics, float unitSpacing=REFLOW_DEFAULT_UNIT_SPACING) const;

    
    float HeaderSizeOnFirstPage() const;

    const RESystem* SystemWithBarIndex(int barIndex) const;
    RESystem* SystemWithBarIndex(int barIndex);
    void FindSystemsWithBarIndexSet(RESystemSet* systemSet, const REIntSet& barIndexSet);
    void FindSystemsWithBarIndexSet(REConstSystemSet* systemSet, const REIntSet& barIndexSet) const;
    
    const RERect& RectOfTitleFrame() const {return _titleFrame;}
    const RERect& RectOfArtistFrame() const {return _artistFrame;}
    const RERect& RectOfSubtitleFrame() const {return _subtitleFrame;}
    const RERect& RectOfAlbumFrame() const {return _albumFrame;}
    const RERect& RectOfLyricsByFrame() const {return _lyricsByFrame;}
    const RERect& RectOfMusicByFrame() const {return _musicByFrame;}
    const RERect& RectOfTranscriberFrame() const {return _transcriberFrame;}
    const RERect& RectOfCopyrightFrame() const {return _copyrightFrame;}
    const RERect& RectOfPaginationFrame() const {return _paginationFrame;}
    
    bool HasTitleFrame() const;
    bool HasSubtitleFrame() const;
    bool HasArtistFrame() const;
    bool HasAlbumFrame() const;
    bool HasLyricsByFrame() const;
    bool HasMusicByFrame() const;
    bool HasTranscriberFrame() const;
    bool HasCopyrightFrame() const;
    bool HasPaginationFrame() const;
    
    void SetTrackSet(const RETrackSet& trackSet);
    
    
    Reflow::ScoreLayoutType LayoutType() const {return _layoutType;}
    Reflow::PageLayoutType PageLayoutType() const {return _pageLayoutType;}
    void SetLayoutType(Reflow::ScoreLayoutType lt) {_layoutType = lt;}
    void SetPageLayoutType(Reflow::PageLayoutType plt) {_pageLayoutType = plt;}
    
public:
    void DrawPage(REPainter& painter, unsigned int pageIndex) const;
    void RenderPage(RERenderer& renderer, unsigned int pageIndex) const;
    
private:
    void _UpdateIndices();
    void _LayoutPages();
    void _RefreshFrames();
    void _CreateFrames();
    void _DispatchSystemsInScreenMode();
    
private:
    const RESong* _parent;
    REScoreSettings _settings;
    REScoreRoot* _root;
    
    RESystemVector _systems;
    REConstTrackVector _tracks;
    
    RESize _contentSize;
    Reflow::ScoreLayoutType _layoutType;
    Reflow::PageLayoutType _pageLayoutType;
    
    RERect _titleFrame;
    RERect _artistFrame;
    RERect _subtitleFrame;
    RERect _albumFrame;    
    RERect _lyricsByFrame;
    RERect _musicByFrame;    
    RERect _transcriberFrame;
    RERect _copyrightFrame;
    RERect _paginationFrame;
    float _headerSizeOnFirstPage;
};

#endif
