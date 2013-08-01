//
//  REScore.cpp
//  Reflow
//
//  Created by Sebastien on 13/04/11.
//  Copyright 2011 Gargant Studios. All rights reserved.
//

#include "REScore.h"
#include "RESong.h"
#include "RESystem.h"
#include "RESlice.h"
#include "REStaff.h"
#include "RETablatureStaff.h"
#include "REStandardStaff.h"
#include "RETrack.h"
#include "REFunctions.h"
#include "REPainter.h"
#include "REBarMetrics.h"
#include "REVoice.h"
#include "REPhrase.h"
#include "REChord.h"
#include "REBar.h"
#include "REOutputStream.h"
#include "REInputStream.h"
#include "REScoreRoot.h"
#include "REPage.h"
#include "REFrame.h"
#include "REViewport.h"
#include "REStyle.h"
#include "RELayout.h"

#include <cmath>
#include <iostream>
#include <sstream>
#include <boost/foreach.hpp>
#include <boost/bind.hpp>
#include <boost/format.hpp>
#include <algorithm>

REScore::REScore(const RESong* song)
: _parent(song), _root(NULL), _headerSizeOnFirstPage(0), _layoutType(Reflow::PageScoreLayout), _pageLayoutType(Reflow::HorizontalPageLayout)
{
}

REScore::~REScore()
{
    Clear();
}

void REScore::Clear()
{
    delete _root;
    _root = NULL;

    for(unsigned int i=0; i<_systems.size(); ++i) {
        delete _systems[i];
    }
    _systems.clear();
}

const RESystem* REScore::System(int idx) const
{
    if(idx >= 0 && idx < _systems.size()) {
        return _systems[idx];
    }
    return 0;
}


RESystem* REScore::System(int idx)
{
    if(idx >= 0 && idx < _systems.size()) {
        return _systems[idx];
    }
    return 0;
}

void REScore::InsertSystem(RESystem* sys, int idx)
{
    _systems.insert(_systems.begin() + idx, sys);
    sys->_score = this;
    _UpdateIndices();
}
void REScore::RemoveSystem(int idx)
{
    if(idx >= 0 && idx < _systems.size()) {
        RESystem* sys = _systems[idx];
        delete sys;
        _systems.erase(_systems.begin() + idx);
        _UpdateIndices();
    }
}

void REScore::_UpdateIndices() {
    for(unsigned int i=0; i<_systems.size(); ++i) {
        _systems[i]->_index = i;
    }
}


void REScore::SetTracks(const RETrackVector& tracks)
{
    RETrackSet trackSet;
    RETrackVector::const_iterator it = tracks.begin();
    for(; it != tracks.end(); ++it) {
        trackSet.Set((*it)->Index());
    }
    
    SetTrackSet(trackSet);
}

void REScore::SetTrackSet(const RETrackSet& trackSet)
{
    _settings.SetTrackSet(trackSet);
    _tracks.clear();
    for(unsigned int i=0; i<Song()->TrackCount(); ++i) {
        if(_settings._trackSet.IsSet(i)) {
            _tracks.push_back(Song()->Track(i));
        }
    }
}

const RESystem* REScore::SystemWithBarIndex(int barIndex) const
{
    for(unsigned int i=0; i<_systems.size(); ++i) {
        const RERange rg = _systems[i]->BarRange();
        if(barIndex >= rg.FirstIndex() && barIndex <= rg.LastIndex()) {
            return _systems[i];
        }
    }
    return NULL;
}

RESystem* REScore::SystemWithBarIndex(int barIndex)
{
    int nbSystems = SystemCount();
    for(int systemIndex=0; systemIndex<nbSystems; ++systemIndex)
    {
        RESystem* system = _systems[systemIndex];
        if(system->BarRange().IsInRange(barIndex))
        {
            return system;
        }
    }
    return NULL;
}

void REScore::FindSystemsWithBarIndexSet(RESystemSet* systemSet, const REIntSet& barIndexSet)
{
    REIntSet::const_iterator it = barIndexSet.begin();
    for(; it != barIndexSet.end(); ++it)
    {
        RESystem* system = SystemWithBarIndex(*it);
        if(system) {
            systemSet->insert(system);
        }
    }
}

void REScore::FindSystemsWithBarIndexSet(REConstSystemSet* systemSet, const REIntSet& barIndexSet) const
{
    REIntSet::const_iterator it = barIndexSet.begin();
    for(; it != barIndexSet.end(); ++it)
    {
        const RESystem* system = SystemWithBarIndex(*it);
        if(system) {
            systemSet->insert(system);
        }
    }
}

RERect REScore::ContentRect() const
{
    return _settings.ContentRect();
}

RERect REScore::PageRect() const
{
    return _settings.PageRect();
}

bool REScore::IsTransposing() const
{
    return _settings.HasFlag(REScoreSettings::TransposingScore);
}

unsigned int REScore::TrackCount() const
{
    return (unsigned int)_tracks.size();
}

const RETrack* REScore::Track(int indexInScore) const
{
    if (indexInScore >= 0 && indexInScore < TrackCount()) {
        return _tracks[indexInScore];
    }
    return NULL;
}

bool REScore::ContainsTrack(const RETrack* track) const
{
    return -1 != IndexOfTrackInScore(track);
}
int REScore::IndexOfTrackInScore(const RETrack* track) const
{
    for(unsigned int i=0; i<TrackCount(); ++i) {
        if(track == _tracks[i]) {
            return i;
        }
    }
    return -1;    
}

float REScore::HeaderSizeOnFirstPage() const
{
    return _headerSizeOnFirstPage;
}

const RETrack* REScore::FirstTrack() const
{
    return _tracks.empty() ? NULL : _tracks.front();
}

unsigned int REScore::PageCount() const
{
    return _root ? _root->PageCount() : 0;
}


void REScore::_DispatchSystemsInScreenMode()
{
    unsigned int currentPageIndex = 0;
    int currentSystemIndexInPage = 0;
    double currentY = HeaderSizeOnFirstPage();
    float spacingBetweenSystems = 12.0;
    
    // Retrieve or create Page
    REPage* page = _root->Page(currentPageIndex);
    if(page == NULL) page = _root->CreatePageAtEnd();
    
    for(unsigned int i=0; i<_systems.size(); ++i)
    {
        RESystem* system = System(i);
        float h = system->Height();
        
        system->SetPosition(REPoint(0.0, currentY));
        
        system->_parent = page;
        system->_indexInPage = currentSystemIndexInPage;
        
        currentY += h + spacingBetweenSystems;
        currentSystemIndexInPage++;
    }
    
    int newPageCount = 1;
    while(newPageCount < _root->PageCount()) {
        _root->DeletePageAtEnd();
    }
    
    RESize size = ContentRect().size;
    size.h = currentY;
    page->SetSize(size);    
    _contentSize = page->Size();
}

void REScore::Refresh()
{
    Clear();
    if(!_parent) return;
    
    _root = new REScoreRoot(this);
    
    SetTrackSet(_settings.TrackSet());
    if(_layoutType == Reflow::PageScoreLayout)
    {
        REFlexibleLayout defaultLayout;
        RELayout* layout = _settings.Layout();
        if(layout == nullptr) layout = &defaultLayout;
        
        layout->CalculateSystems(this);
        _RefreshFrames();
        layout->DispatchSystems(this);
        _LayoutPages();
        _CreateFrames();
    }
    /*else if(_layoutType == Reflow::ScreenScoreLayout)
    {
        _CalculateSystemsFlexible();
        _RefreshFrames();
        _DispatchSystemsInScreenMode();
        _CreateFrames();
    }*/
    else if(_layoutType == Reflow::HorizontalScoreLayout)
    {
        REHorizontalLayout layout;
        layout.CalculateSystems(this);
        layout.DispatchSystems(this);
    }
}

void REScore::RefreshSingleBar(int barIndex)
{
#if 0
    // Calculate all bar metrics
    std::vector<REBarMetrics*> barMetrics;
    unsigned int nbBars = _parent->BarCount();
    barMetrics.reserve(nbBars);
    for(unsigned int i=0; i<nbBars; ++i)
    {
        REBarMetrics* bm = new REBarMetrics;
        CalculateBarMetrics(i, *bm);
        barMetrics.push_back(bm);
    }

	// Refresh system
	if(_layoutType == Reflow::PageScoreLayout)
    {
		RESystem* system = SystemWithBarIndex(barIndex);
		system->Clear();
		_CalculateSystemFromBarRange(system, system->BarRange().FirstIndex(), system->BarRange().count, &barMetrics);
        REViewportSystemItem* item = static_cast<REViewportSystemItem*>(system->ViewportItem());
        if(item) {
            item->UpdateFrame();
        }
		//_RefreshFrames();
        //ForceSystemReflow();
        //_LayoutPages();
        //_CreateFrames();
    }
    else if(_layoutType == Reflow::HorizontalScoreLayout)
    {
        assert(false && "TODO! we need to refresh affected slice only");
        RESystem* system = System(0);
        system->Clear();
        _CalculateSystemFromBarRange(system, 0, Song()->BarCount(), NULL);
    }
    
    for(REBarMetrics* bm : barMetrics) {
        delete bm;
    }
    barMetrics.clear();
#endif
}

void REScore::Rebuild(const REScoreSettings& settings)
{
    _settings = settings;
    Refresh();
}

bool REScore::HasTitleFrame() const {return !_parent->Title().empty();}
bool REScore::HasSubtitleFrame() const {return !_parent->SubTitle().empty();}
bool REScore::HasArtistFrame() const {return !_parent->Artist().empty();}
bool REScore::HasAlbumFrame() const {return !_parent->Album().empty();}
bool REScore::HasLyricsByFrame() const {return !_parent->LyricsBy().empty();}
bool REScore::HasMusicByFrame() const {return !_parent->MusicBy().empty();}
bool REScore::HasTranscriberFrame() const {return !_parent->Transcriber().empty();}
bool REScore::HasCopyrightFrame() const {return !_parent->Copyright().empty();}
bool REScore::HasPaginationFrame() const {return true;}

float REScore::ContinuousXOffsetOfSystem(const RESystem* system) const
{
    float x = 0.0;
    for(const RESystem* s : _systems) {
        if(s == system) {
            break;
        }
        else {
            x += s->Width();
        }
    }
    return x;
}

void REScore::_LayoutPages()
{
    RERect pageRect = PageRect();
    
    float interpageSpaceY = 20.0;
    float interpageSpaceX = 20.0;
#if defined(REFLOW_MAC) && defined(REFLOW_2)
    float x = interpageSpaceX + pageRect.size.w / 2;
    float y = interpageSpaceY + pageRect.size.h / 2;
#else
    float x = interpageSpaceX;
    float y = interpageSpaceY;
#endif
    float w = pageRect.size.w;
    float h = pageRect.size.h;
    
    unsigned int nbPages = PageCount();
    for(unsigned int pageIndex=0; pageIndex < nbPages; ++pageIndex)
    {
        if(_pageLayoutType == Reflow::TwoColumnPageLayout)
        {
            int rowIndex = pageIndex / 2;
            int colIndex = pageIndex % 2;
            x = interpageSpaceX + (interpageSpaceX + w) * colIndex;
            y = interpageSpaceY + (interpageSpaceY + h) * rowIndex;
        }
        
        REPage* page = _root->Page(pageIndex);
        page->SetPosition(REPoint(x,y));
        
        if(_pageLayoutType == Reflow::VerticalPageLayout) {
            y += h + interpageSpaceY;
        }
        else if(_pageLayoutType == Reflow::HorizontalPageLayout) {
            x += w + interpageSpaceX;
        }
    }
    
#if defined(REFLOW_MAC) && defined(REFLOW_2)
    x += pageRect.size.w / 2;
    y += pageRect.size.h / 2;
#endif
    
    if(_pageLayoutType == Reflow::VerticalPageLayout)
    {
        _contentSize = RESize(x+w+interpageSpaceX, y);
    }
    else if(_pageLayoutType == Reflow::HorizontalPageLayout) {
        _contentSize = RESize(x+interpageSpaceX, y+h+interpageSpaceY);
    }
    else if(_pageLayoutType == Reflow::TwoColumnPageLayout)
    {
        int nbColumns = (nbPages >= 2 ? 2 : 1);
        int nbRows = (nbPages % 2 ? nbPages / 2 + 1 : nbPages/2);
        float totalW = (nbColumns * w) + (interpageSpaceX * (nbColumns + 1));
        float totalH = (nbRows * h) + (interpageSpaceY * (nbRows + 1));
        _contentSize = RESize(totalW, totalH);
    }
}

void REScore::_RefreshFrames()
{
    RERect rc = ContentRect();
    float w = rc.Width();
    float x = rc.origin.x;
    float y = rc.origin.y;
    float h = 0;
    
    bool hasTitle = HasTitleFrame();
    bool hasSubtitle = HasSubtitleFrame();
    bool hasArtist = HasArtistFrame();
    bool hasAlbum = HasAlbumFrame();
    bool hasLyricsBy = HasLyricsByFrame();
    bool hasMusicBy = HasMusicByFrame();
    bool hasTranscriber = HasTranscriberFrame();
    bool hasCopyright = HasCopyrightFrame();
    bool hasPagination = HasPaginationFrame();
    
    if(hasTitle) { 
        h = 40;
        _titleFrame = RERect(x, y, w, h);
        y += h;
    }
    
    if(hasSubtitle) {
        h = 20;
        _subtitleFrame = RERect(x,y,w,h);
        y += h;
    }
   
    if(hasArtist) {
        h = 20;
        _artistFrame = RERect(x,y,w,h);
        y += h;
    }
    
    if(hasAlbum) {
        h = 20;
        _albumFrame = RERect(x,y,w,h);
        y += h;
    }
    
    if(hasLyricsBy) {
        h = 14;
        _lyricsByFrame = RERect(x, y, w/2, h);
        if(!hasMusicBy && !hasTranscriber) {
            y += h;
        }
    }

    if(hasMusicBy) {
        h = 14;
        _musicByFrame = RERect(x + w/2, y, w/2, h);
        y += h;
    }
    
    if(hasTranscriber) {
        h = 14;
        _transcriberFrame = RERect(x + w/2, y, w/2, h);
        y += h;
    }
    
    // End of Header
    _headerSizeOnFirstPage = y + 20;
    
    
    // Copyright
    if(hasCopyright) {
        _copyrightFrame = RERect(x, rc.Bottom()+1, w, 12);
    }
    
    // Pagination
    if(hasPagination) {
        _paginationFrame = RERect(x + w/2, rc.Bottom()+1,  w/2, 12);
    }
}

void REScore::_CreateFrames()
{
    REFontDesc titleFont = {std::string("Times New Roman"), 36.0f, false, false};
    REFontDesc subtitleFont = {std::string("Times New Roman"), 20.0f, false, false};
    REFontDesc artistFont = {std::string("Arial"), 14.0f, true, false};
    REFontDesc albumFont = {std::string("Arial"), 14.0f, false, false};
    REFontDesc musicByFont = {std::string("Times New Roman"), 12.0f, false, false};
    REFontDesc lyricsByFont = {std::string("Times New Roman"), 12.0f, false, false};
    REFontDesc transcriberFont = {std::string("Times New Roman"), 12.0f, false, false};
    REFontDesc copyrightFont = {std::string("Times New Roman"), 10.0f, false, false};
    REFontDesc paginationFont = {std::string("Times New Roman"), 10.0f, false, false};
    
    for(REPage* page : _root->Pages())
    {
        page->ClearTextFrames();
        
        if(page->Number() == 0 && HasTitleFrame()) {
            REFrame* frame = new REFrame(RectOfTitleFrame(), _parent->Title(), titleFont, Reflow::CenterTextAlign);
            page->AddTextFrame(frame);
        }
        
        if(page->Number() == 0 && HasSubtitleFrame()) {
            REFrame* frame = new REFrame(RectOfSubtitleFrame(), _parent->SubTitle(), subtitleFont, Reflow::CenterTextAlign);
            page->AddTextFrame(frame);
        }
        
        if(page->Number() == 0 && HasArtistFrame()) {
            REFrame* frame = new REFrame(RectOfArtistFrame(), _parent->Artist(), artistFont, Reflow::CenterTextAlign);
            page->AddTextFrame(frame);
        }
        
        if(page->Number() == 0 && HasAlbumFrame()) {
            REFrame* frame = new REFrame(RectOfAlbumFrame(), _parent->Album(), albumFont, Reflow::CenterTextAlign);
            page->AddTextFrame(frame);
        }
        
        if(page->Number() == 0 && HasLyricsByFrame()) {
            REFrame* frame = new REFrame(RectOfLyricsByFrame(), Reflow::LocalizedTextForLyricsByFrame(this), lyricsByFont, Reflow::LeftTextAlign);
            page->AddTextFrame(frame);
        }
        
        if(page->Number() == 0 && HasMusicByFrame()) {
            REFrame* frame = new REFrame(RectOfMusicByFrame(), Reflow::LocalizedTextForMusicByFrame(this), musicByFont, Reflow::RightTextAlign);
            page->AddTextFrame(frame);
        }
        
        if(page->Number() == 0 && HasTranscriberFrame()) {
            REFrame* frame = new REFrame(RectOfTranscriberFrame(), Reflow::LocalizedTextForTranscriberFrame(this), transcriberFont, Reflow::RightTextAlign);
            page->AddTextFrame(frame);
        }
        
        if(page->Number() == 0 && HasCopyrightFrame()) {
            REFrame* frame = new REFrame(RectOfCopyrightFrame(), _parent->Copyright(), copyrightFont, Reflow::CenterTextAlign);
            page->AddTextFrame(frame);
        }

        if(HasPaginationFrame()) {
            std::string txt = boost::str(boost::format("%1% / %2%") % (page->Number()+1) % PageCount());
            REFrame* frame = new REFrame(RectOfPaginationFrame(), txt, paginationFont, Reflow::RightTextAlign);
            page->AddTextFrame(frame);
        }
    }
    
}

void REScore::DrawPage(REPainter& painter, unsigned int pageIndex) const
{
    // Draw Bounds
    RERect bounds = ContentRect();
    RERect rc = RERect(0.5, 0.5, bounds.size.w-1, bounds.size.h-1);
    //painter.StrokeRect(rc);
    
    REPoint offset = -ContentRect().origin;
    
    if(pageIndex == 0) 
    {   
        // Title
        if(HasTitleFrame()) {
            RERect rect = RectOfTitleFrame().Translated(offset);
            painter.DrawTextInRect(_parent->Title(), rect, "Times New Roman", 0, 36.0, REColor::Black);
        }
        
        // Subtitle
        if(HasSubtitleFrame()) {
            RERect rect = RectOfSubtitleFrame().Translated(offset);
            painter.DrawTextInRect(_parent->SubTitle(), rect, "Times New Roman", 0, 20.0, REColor::Black);
        }
        
        // Artist
        if(HasArtistFrame()) {
            RERect rect = RectOfArtistFrame().Translated(offset);
            painter.DrawTextInRect(_parent->Artist(), rect, "Times New Roman", REPainter::Bold, 14.0, REColor::Black);
        }
        
        // Album
        if(HasAlbumFrame()) {
            RERect rect = RectOfAlbumFrame().Translated(offset);
            painter.DrawTextInRect(_parent->Album(), rect, "Arial", 0, 14.0, REColor::Black);
        }
        
        // Lyrics By
        if(HasLyricsByFrame()) {
            RERect rect = RectOfLyricsByFrame().Translated(offset);
            painter.DrawTextInRect(Reflow::LocalizedTextForLyricsByFrame(this), rect, "Times New Roman", REPainter::LeftAligned, 12.0, REColor::Black);
        }
        
        // Music By
        if(HasMusicByFrame()) {
            RERect rect = RectOfMusicByFrame().Translated(offset);
            painter.DrawTextInRect(Reflow::LocalizedTextForMusicByFrame(this), rect, "Times New Roman", REPainter::RightAligned, 12.0, REColor::Black);
        }

        // Transcriber
        if(HasTranscriberFrame()) {
            RERect rect = RectOfTranscriberFrame().Translated(offset);
            painter.DrawTextInRect(Reflow::LocalizedTextForTranscriberFrame(this), rect, "Times New Roman", REPainter::RightAligned, 12.0, REColor::Black);
        }
        
        // Copyright
        if(HasCopyrightFrame()) {
            RERect rect = RectOfCopyrightFrame().Translated(offset);
            painter.DrawTextInRect("Copyright " + _parent->Copyright(), rect, "Times New Roman", 0, 10.0, REColor::Black);
        }
    }
    
    // Page Numbers
    {
        RERect rect = RectOfPaginationFrame().Translated(offset);
        std::ostringstream oss;
        oss << (int)pageIndex+1 << " / " << this->PageCount();
        painter.DrawTextInRect(oss.str(), rect, "Times New Roman", REPainter::RightAligned, 10.0, REColor::Black);
    }
    
    // Draw every system
    for(unsigned int i=0; i<SystemCount(); ++i)
    {
        const RESystem* system = System(i);
        if(system->PageNumber() != pageIndex) continue;
        
        const REPoint& pos = system->Position();
        
        // Save graphics state & translate
        painter.Save();
        painter.Translate(pos.x + offset.x, pos.y + offset.y);
        
        // Draw system
        system->Draw(painter);
        
        // Restore graphics state
        painter.Restore();
    }
}

const RESystem* REScore::PickSystem(const REPoint& pos) const
{
    BOOST_FOREACH(const RESystem* system, Systems())
    {
        if(system->SceneFrame().PointInside(pos)) {
            return system;
        }
    }
    return NULL;
}

