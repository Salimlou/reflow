//
//  RELayout.cpp
//  Reflow
//
//  Created by Sebastien Bourgeois on 28/06/13.
//
//

#include "RELayout.h"
#include "RESong.h"
#include "REScore.h"
#include "REScoreSettings.h"
#include "REStyle.h"
#include "RESystem.h"
#include "RESlice.h"
#include "REBarMetrics.h"
#include "REBar.h"
#include "RETrack.h"
#include "REVoice.h"
#include "REPhrase.h"
#include "REChord.h"
#include "REStaff.h"
#include "REStandardStaff.h"
#include "RETablatureStaff.h"
#include "REScoreRoot.h"
#include "REPage.h"
#include "REOutputStream.h"
#include "REInputStream.h"

RELayout::RELayout()
{
    
}
RELayout::~RELayout()
{
    
}



RELayout* RELayout::CreateLayoutWithIdentifier(const std::string& identifier)
{
    if(identifier == "flex") return new REFlexibleLayout;
    if(identifier == "manual") return new REManualLayout;
    if(identifier == "fix") return new REFixedLayout;
    if(identifier == "horiz") return new REHorizontalLayout;
    return nullptr;
}

void RELayout::RefreshSystemVerticalGuides(RESystem* system)
{
    system->CalculateBarDimensions();
}
void RELayout::RefreshSystemHorizontalGuides(RESystem* system)
{
    system->_RefreshMetrics();    
}
void RELayout::RefreshStaffGuides(REStaff* staff)
{
    
}

REBarMetrics* RELayout::CalculateBarMetrics(const REScore* score, int barIndex)
{
    REBarMetrics* metrics = new REBarMetrics;
    metrics->_columns.clear();

    const float unitSpacing = REFLOW_DEFAULT_UNIT_SPACING;
    const REScoreSettings& settings = score->Settings();
    const RESong* song = score->Song();
    const REBar* bar = song->Bar(barIndex);
    
    // When we start, consider the bar as totally empty (we will change this if we find a not-rest chord)
    metrics->_empty = true;
    
    // Determine if this is collapsible
    metrics->_collapsibleWithFollowing = false;
    if(settings.HasFlag(REScoreSettings::UseMultiRests))
    {
        metrics->_collapsibleWithFollowing = true;
        for(unsigned int i=0; i<score->TrackCount(); ++i)
        {
            const RETrack* track = score->Track(i);
            
            if(!track->IsBarEmptyAndCollapsibleWithNextSibling(barIndex)) {
                metrics->_collapsibleWithFollowing = false;
                break;
            }
        }
    }
    
    // Columns are calculated from Tick set
    RETickSet ticks;
    song->CalculateTickSet(barIndex, settings.TrackSet(), ticks);
    RETickSet::const_iterator it = ticks.begin();
    for(; it != ticks.end(); ++it)
    {
        REBarMetrics::REBMColumn column;
        column.tick = *it;
        column.leftSpace = 4.0;
        column.rightSpace = 4.0;
        column.xOffset = 0.0;
        metrics->_columns.push_back(column);
    }
    
    // Calculate column Spacing
    unsigned int rowIndex = 0;
    for(unsigned int i=0; i<score->TrackCount(); ++i)
    {
        const RETrack* track = score->Track(i);
        for(unsigned int v=0; v<track->VoiceCount(); ++v)
        {
            const REVoice* voice = track->Voice(v);
            const REPhrase* phrase = voice->Phrase(barIndex);
            unsigned int nbChords = phrase->ChordCount();
            
            for(unsigned int chordIndex = 0; chordIndex < nbChords; ++chordIndex)
            {
                const REChord* chord = phrase->Chord(chordIndex);
                metrics->_empty = false;
                
                unsigned long tick = chord->OffsetInTicks();
                int columnIndex = metrics->ColumnIndexAtTick(tick);
                if(columnIndex != -1)
                {
                    REBarMetrics::REBMColumn& column = metrics->_columns[columnIndex];
                    float leftSpacing = 0.0f;
                    float rightSpacing = 0.0f;
                    chord->_CalculateSpacing(&leftSpacing, &rightSpacing, unitSpacing, settings.HasFlag(REScoreSettings::TransposingScore));
                    if(column.leftSpace < leftSpacing) column.leftSpace = leftSpacing;
                    if(column.rightSpace < rightSpacing) column.rightSpace = rightSpacing;
                }
            }
            
            ++rowIndex;
        }
    }
    
    metrics->_leadingSpaceIfFirst = unitSpacing;
    metrics->_leadingSpaceInMiddle = unitSpacing;
    
    metrics->_trailingSpaceIfLast = 2 * unitSpacing;
    metrics->_trailingSpaceInMiddle = 2 * unitSpacing;
    
    
    if(metrics->_empty) {
        if(settings.HasFlag(REScoreSettings::UseMultiRests) && metrics->_collapsibleWithFollowing) {
            metrics->_trailingSpaceIfLast = 20 * unitSpacing;
            metrics->_trailingSpaceInMiddle = 20 * unitSpacing;
        }
        else {
            metrics->_trailingSpaceIfLast = 16 * unitSpacing;
            metrics->_trailingSpaceInMiddle = 16 * unitSpacing;
        }
    }
    
    bool clefChange = false;
    for(unsigned int i=0; i<score->TrackCount(); ++i)
    {
        if(score->Track(i)->HasClefChangeAtBar(barIndex)) {
            clefChange = true;
        }
        
    }
    bool keyChange = bar->HasKeySignatureChange();
    int keySignatureSize = bar->AccidentalCountOnKeySignature();
    if(keySignatureSize > 0) ++keySignatureSize;
    float keyWidth = unitSpacing * keySignatureSize;  // TODO: determine width of key signature
    float clefWidth = 24.0;
    
    // Clef is drawn in middle if there is a change
    if(clefChange)
    {
        metrics->_clefOffsetInMiddle = metrics->_leadingSpaceInMiddle;
        metrics->_leadingSpaceInMiddle += clefWidth;
    }
    
    // Clef is always drawn in front of a system
    metrics->_clefOffsetIfFirst = metrics->_leadingSpaceIfFirst;
    metrics->_leadingSpaceIfFirst += clefWidth;
    
    // Key is drawn in middle of system if there is a change of clef or key
    if(clefChange || keyChange)
    {
        metrics->_keySignatureOffsetInMiddle = metrics->_leadingSpaceInMiddle;
        metrics->_leadingSpaceInMiddle += keyWidth;
    }
    
    // Key is always drawn in front of a system
    metrics->_keySignatureOffsetIfFirst = metrics->_leadingSpaceIfFirst;
    metrics->_leadingSpaceIfFirst += keyWidth;
    
    // Time Signature
    if(bar->HasTimeSignatureChange())
    {
        metrics->_timeSignatureOffsetIfFirst = metrics->_leadingSpaceIfFirst;
        metrics->_leadingSpaceIfFirst += 15.0;
        metrics->_timeSignatureOffsetInMiddle = metrics->_leadingSpaceInMiddle;
        metrics->_leadingSpaceInMiddle += 15.0;
    }
    
    // Repeat Start
    if(bar->HasFlag(REBar::RepeatStart))
    {
        metrics->_repeatStartOffsetIfFirst = metrics->_leadingSpaceIfFirst;
        metrics->_leadingSpaceIfFirst += 15.0;
        metrics->_repeatStartOffsetInMiddle = metrics->_leadingSpaceInMiddle;
        metrics->_leadingSpaceInMiddle += 15.0;
    }
    
    // Spacing to the first Note
    metrics->_leadingSpaceIfFirst += unitSpacing;
    metrics->_leadingSpaceInMiddle += unitSpacing;
    
    // Refresh columns
    metrics->_RefreshColumns();
    return metrics;
}

void RELayout::CalculateSystemFromBarRange(RESystem* system, unsigned int firstBarIndex, unsigned int barCount, const std::vector<REBarMetrics*> *barMetrics)
{
    const REScore* score = system->Score();
    const REScoreSettings& settings = score->Settings();
    const RESong* song = score->Song();
    
    system->_barRange = RERange(firstBarIndex, barCount);
    system->SetSize(settings.ContentRect().size.w, 100);
    
    // Calculate Bar Metrics of bars
    unsigned int i=0;
    while(i < barCount)
    {
        unsigned int barIndex = firstBarIndex + i;
        if(settings.HasFlag(REScoreSettings::UseMultiRests) && barMetrics != NULL)
        {
            const REBarMetrics* pbm = barMetrics->at(barIndex);
            if(pbm->IsCollapsibleWithFollowing())
            {
                REBarMetrics* metrics = CalculateBarMetrics(score, barIndex);
                REMultiRestSlice* slice = new REMultiRestSlice(metrics);
                system->InsertSystemBar(slice, system->SystemBarCount());
                
                int firstBarCollapsed = barIndex;
                int lastBarCollapsed = barIndex+1;
                
                ++i;
                ++barIndex;
                while(i < barCount && barMetrics->at(barIndex)->IsCollapsibleWithFollowing()) {
                    ++lastBarCollapsed;
                    ++i;
                    ++barIndex;
                }
                
                slice->SetBarCount(lastBarCollapsed - firstBarCollapsed + 1);
                ++i;
            }
            else {
                REBarMetrics* metrics = CalculateBarMetrics(score, barIndex);
                RESlice* systemBar = new RESlice(metrics);
                system->InsertSystemBar(systemBar, system->SystemBarCount());
                ++i;
            }
        }
        else {
            REBarMetrics* metrics = CalculateBarMetrics(score, barIndex);
            RESlice* systemBar = new RESlice(metrics);
            system->InsertSystemBar(systemBar, system->SystemBarCount());
            ++i;
        }
    }
    
    // Create Staves
    unsigned int staffIndex = 0;
    for(unsigned int i=0; i<score->TrackCount(); ++i)
    {
        const RETrack* track = score->Track(i);
        REStaff* staff = NULL;
        switch(track->Type())
        {
            case Reflow::StandardTrack:
            {
                staff = new REStandardStaff;
                staff->_type = Reflow::StandardStaff;
                staff->_identifier = Reflow::FirstStandardStaffIdentifier;
                staff->_track = track;
                static_cast<REStandardStaff*>(staff)->_hand = REStandardStaff::RightHand;
                system->InsertStaff(staff, staffIndex++);
                track->FindSlursInBarRange(system->BarRange(), staff->Identifier(), staff->_slurs);
                
                if(track->HasFlag(RETrack::GrandStaff)) {
                    REStandardStaff* staff2 = new REStandardStaff;
                    staff2->_type = Reflow::StandardStaff;
                    staff2->_identifier = Reflow::SecondStandardStaffIdentifier;
                    staff2->_track = track;
                    static_cast<REStandardStaff*>(staff2)->_hand = REStandardStaff::LeftHand;
                    system->InsertStaff(staff2, staffIndex++);
                    track->FindSlursInBarRange(system->BarRange(), staff2->Identifier(), staff2->_slurs);
                }
                break;
            }
            case Reflow::TablatureTrack:
            {
                bool showStandard = !settings.HasFlag(REScoreSettings::HideStandard);
                bool showTablature = !settings.HasFlag(REScoreSettings::HideTablature);
                if(!showStandard && !showTablature) {
                    showTablature = true;
                }
                
                if(showStandard && showTablature)
                {
                    REStaff* autoStaff = new REStandardStaff;
                    autoStaff->_type = Reflow::StandardStaff;
                    autoStaff->_identifier = Reflow::FirstStandardStaffIdentifier;
                    autoStaff->_track = track;
                    system->InsertStaff(autoStaff, staffIndex++);
                    track->FindSlursInBarRange(system->BarRange(), autoStaff->Identifier(), autoStaff->_slurs);
                    
                    staff = new RETablatureStaff;
                    staff->_type = Reflow::TablatureStaff;
                    staff->_identifier = Reflow::TablatureStaffIdentifier;
                    staff->_track = track;
                    staff->SetFlag(REStaff::HideRhythm);
                    system->InsertStaff(staff, staffIndex++);
                    track->FindSlursInBarRange(system->BarRange(), staff->Identifier(), staff->_slurs);
                }
                else if(showStandard)
                {
                    REStaff* autoStaff = new REStandardStaff;
                    autoStaff->_type = Reflow::StandardStaff;
                    autoStaff->_identifier = Reflow::FirstStandardStaffIdentifier;
                    autoStaff->_track = track;
                    system->InsertStaff(autoStaff, staffIndex++);
                    track->FindSlursInBarRange(system->BarRange(), autoStaff->Identifier(), autoStaff->_slurs);
                }
                else if(showTablature)
                {
                    staff = new RETablatureStaff;
                    staff->_type = Reflow::TablatureStaff;
                    staff->_identifier = Reflow::TablatureStaffIdentifier;
                    staff->_track = track;
                    system->InsertStaff(staff, staffIndex++);
                    track->FindSlursInBarRange(system->BarRange(), staff->Identifier(), staff->_slurs);
                }
                break;
            }
            case Reflow::DrumsTrack:
            {
                staff = new REStandardStaff;
                staff->_type = Reflow::StandardStaff;
                staff->_identifier = Reflow::FirstStandardStaffIdentifier;
                staff->_track = track;
                system->InsertStaff(staff, staffIndex++);
                track->FindSlursInBarRange(system->BarRange(), staff->Identifier(), staff->_slurs);
            }
                break;
            default:
                assert(false);
                break;
        }
        
    }
    
    RefreshSystemVerticalGuides(system);
    RefreshSystemHorizontalGuides(system);
}

void RELayout::DispatchSystems(REScore* score)
{
    unsigned int currentPageIndex = 0;
    int currentSystemIndexInPage = 0;
    double currentY = score->HeaderSizeOnFirstPage();
    double maxY = score->ContentRect().size.h;
    float spacingBetweenSystems = 12.0;
    
    unsigned int systemCount = score->SystemCount();
    REScoreRoot* root = score->Root();
    
    for(unsigned int i=0; i<systemCount; ++i)
    {
        RESystem* system = score->System(i);
        float h = system->Height();
        
        // New page ?
        if(currentY + h > maxY) {
            ++currentPageIndex;
            currentSystemIndexInPage = 0;
            currentY = 0.0;
        }
        
        system->SetPosition(score->ContentRect().origin + REPoint(0.0, currentY));
        
        // Retrieve or create Page
        REPage* page = root->Page(currentPageIndex);
        if(page == NULL) page = root->CreatePageAtEnd();
        
        system->_parent = page;
        system->_indexInPage = currentSystemIndexInPage;
        
        currentY += h + spacingBetweenSystems;
        currentSystemIndexInPage++;
    }
    
    int newPageCount = currentPageIndex + 1;
    while(newPageCount < root->PageCount()) {
        root->DeletePageAtEnd();
    }
}

#pragma mark REFlexibleLayout
#pragma mark -

REFlexibleLayout::REFlexibleLayout()
{
}
REFlexibleLayout::~REFlexibleLayout()
{
}
RELayout* REFlexibleLayout::Clone() const
{
    auto layout = new REFlexibleLayout;
    
    REBufferOutputStream buffer;
    EncodeTo(buffer);
    
    REConstBufferInputStream in(buffer.Data(), buffer.Size());
    layout->DecodeFrom(in);
    
    return layout;
}
std::string REFlexibleLayout::Identifier() const
{
    return "flex";
}
void REFlexibleLayout::EncodeTo(REOutputStream& coder) const
{
    
}
void REFlexibleLayout::DecodeFrom(REInputStream& decoder)
{
    
}

void REFlexibleLayout::CalculateSystems(REScore* score)
{
    const RESong* song = score->Song();
    const REScoreSettings& settings = score->Settings();
    const REStyle* style = settings.Style();
    if(!style) style = REStyle::DefaultReflowStyle();
    
    // Calculate all bar metrics
    std::vector<REBarMetrics*> barMetrics;
    unsigned int nbBars = song->BarCount();
    //std::cout << "  REScore::_CalculateSystemsFlexible (" << nbBars << " bars)" << std::endl;
    barMetrics.reserve(nbBars);
    for(unsigned int barIndex=0; barIndex<nbBars; ++barIndex)
    {
        REBarMetrics* bm = CalculateBarMetrics(score, barIndex);
        barMetrics.push_back(bm);
    }
    
    unsigned int barIndex = 0;
    unsigned int systemIndex = 0;
    while(barIndex < nbBars)
    {
        bool systemBreak = settings.HasSystemBreakAtBarIndex(barIndex);
        unsigned int firstBarIndex = barIndex;
        //std::cout << "   >> Calculating system " << (systemIndex+1) << std::endl;
        
        // Create a System
        RESystem* system = new RESystem;
        if(systemIndex == 0) {
            system->SetLeftMargin(style->LeftMarginOfFirstSystem());
        }
        else {
            system->SetLeftMargin(style->LeftMarginOfOtherSystems());
        }
        score->InsertSystem(system, score->SystemCount());
        
        float totalIdealWidth = 0.0f;
        float widthTreshold = /*1.20 **/ settings.ContentRect().Width();
        
        // Add at least one Bar
        totalIdealWidth = barMetrics.at(barIndex)->IdealWidth(REFLOW_BAR_METRICS_FIRST);
        ++ barIndex;
        
        // Consume collapsible bars
        if(settings.HasFlag(REScoreSettings::UseMultiRests)) {
            while(barIndex < nbBars && barMetrics.at(barIndex)->IsCollapsibleWithFollowing()) {
                ++barIndex;
            }
        }
        
        // Add bars while we still have room for
        while(!systemBreak && barIndex < nbBars)
        {
            systemBreak = settings.HasSystemBreakAtBarIndex(barIndex);
            const REBarMetrics* bm = barMetrics.at(barIndex);
            float nw = bm->IdealWidth(0);
            if(totalIdealWidth + nw < widthTreshold) {
                totalIdealWidth += nw;
                ++ barIndex;
                
                if(systemBreak) break;
                
                // Consume collapsible bars
                if(settings.HasFlag(REScoreSettings::UseMultiRests)) {
                    while(barIndex < nbBars && barMetrics.at(barIndex)->IsCollapsibleWithFollowing()) {
                        ++barIndex;
                    }
                }
            }
            else break;
        }
        
        // Leave system this way
        unsigned int systemBarCount = (barIndex - firstBarIndex);
        //std::cout << "       >> from " << (firstBarIndex+1) << " to " << (barIndex+1) << std::endl;
        CalculateSystemFromBarRange(system, firstBarIndex, systemBarCount, &barMetrics);
        ++ systemIndex;
    }
    
    // Clear bar metrics
    for(REBarMetrics* bm : barMetrics) {
        delete bm;
    }
    barMetrics.clear();
}



#pragma mark REManualLayout
#pragma mark -

REManualLayout::REManualLayout()
{
    
}
REManualLayout::~REManualLayout()
{
    
}
RELayout* REManualLayout::Clone() const
{
    auto layout = new REManualLayout;
    
    REBufferOutputStream buffer;
    EncodeTo(buffer);
    
    REConstBufferInputStream in(buffer.Data(), buffer.Size());
    layout->DecodeFrom(in);
    
    return layout;
}
std::string REManualLayout::Identifier() const
{
    return "manual";
}
void REManualLayout::EncodeTo(REOutputStream& coder) const
{
    
}
void REManualLayout::DecodeFrom(REInputStream& decoder)
{
    
}

void REManualLayout::CalculateSystems(REScore *score)
{
    const RESong* song = score->Song();
    unsigned int barsPerSystem = 4;
    
    unsigned int nbBars = song->BarCount();
    for(unsigned int barIndex=0; barIndex<nbBars; barIndex += barsPerSystem)
    {
        unsigned int lastBarIndex = std::min(barIndex + barsPerSystem - 1, nbBars-1);
        unsigned int systemBarCount = (lastBarIndex - barIndex + 1);
        
        RESystem* system = new RESystem;
        score->InsertSystem(system, score->SystemCount());
        CalculateSystemFromBarRange(system, barIndex, systemBarCount, NULL);
    }
}



#pragma mark REFixedLayout
#pragma mark -

REFixedLayout::REFixedLayout()
{
    
}
REFixedLayout::~REFixedLayout()
{
    
}
RELayout* REFixedLayout::Clone() const
{
    auto layout = new REFixedLayout;
    
    REBufferOutputStream buffer;
    EncodeTo(buffer);
    
    REConstBufferInputStream in(buffer.Data(), buffer.Size());
    layout->DecodeFrom(in);
    
    return layout;
}
std::string REFixedLayout::Identifier() const
{
    return "fix";
}
void REFixedLayout::EncodeTo(REOutputStream& coder) const
{
    
}
void REFixedLayout::DecodeFrom(REInputStream& decoder)
{
    
}
void REFixedLayout::CalculateSystems(REScore *score)
{
    const RESong* song = score->Song();
    unsigned int barsPerSystem = 4;
    
    unsigned int nbBars = song->BarCount();
    for(unsigned int barIndex=0; barIndex<nbBars; barIndex += barsPerSystem)
    {
        unsigned int lastBarIndex = std::min(barIndex + barsPerSystem - 1, nbBars-1);
        unsigned int systemBarCount = (lastBarIndex - barIndex + 1);
        
        RESystem* system = new RESystem;
        score->InsertSystem(system, score->SystemCount());
        CalculateSystemFromBarRange(system, barIndex, systemBarCount, NULL);
    }
}



#pragma mark REHorizontalLayout
#pragma mark -

REHorizontalLayout::REHorizontalLayout()
{
    
}
REHorizontalLayout::~REHorizontalLayout()
{
    
}
RELayout* REHorizontalLayout::Clone() const
{
    auto layout = new REHorizontalLayout;
    
    REBufferOutputStream buffer;
    EncodeTo(buffer);
    
    REConstBufferInputStream in(buffer.Data(), buffer.Size());
    layout->DecodeFrom(in);
    
    return layout;
}
std::string REHorizontalLayout::Identifier() const
{
    return "horiz";
}
void REHorizontalLayout::EncodeTo(REOutputStream& coder) const
{
    
}
void REHorizontalLayout::DecodeFrom(REInputStream& decoder)
{
    
}
void REHorizontalLayout::CalculateSystems(REScore *score)
{
    const RESong* song = score->Song();
    
    RESystem* hsystem = new RESystem;
    hsystem->_horizontalSystem = true;
    hsystem->SetPosition(REPoint(0,0));
    score->InsertSystem(hsystem, 0);
    CalculateSystemFromBarRange(hsystem, 0, song->BarCount(), NULL);
}

void REHorizontalLayout::DispatchSystems(REScore* score)
{
    RESystem* hsystem = score->System(0);
    REScoreRoot* root = score->Root();
    
    REPage* page = root->Page(0);
    if(page == NULL) page = root->CreatePageAtEnd();
    page->SetBounds(hsystem->Bounds());
    
    hsystem->_parent = page;
    hsystem->_indexInPage = 0;
    
    score->SetContentSize(page->Size());
}
