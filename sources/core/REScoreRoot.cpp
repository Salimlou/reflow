//
//  REScoreRoot.cpp
//  Reflow
//
//  Created by Sebastien Bourgeois on 21/11/12.
//
//

#include "REScoreRoot.h"
#include "REScore.h"
#include "REPage.h"

#include <boost/foreach.hpp>

REScoreRoot::~REScoreRoot()
{
    Clear();
}

void REScoreRoot::Clear()
{
    BOOST_FOREACH(REPage* page, _pages) {delete page;}
    _pages.clear();
}

const REScoreRoot* REScoreRoot::Root() const
{
    return this;
}

REScoreRoot* REScoreRoot::Root()
{
    return this;
}

const REPage* REScoreRoot::Page(int idx) const
{
    return idx >= 0 && idx < PageCount() ? _pages[idx] : NULL;
}

REPage* REScoreRoot::Page(int idx)
{
    return idx >= 0 && idx < PageCount() ? _pages[idx] : NULL;
}

REPage* REScoreRoot::CreatePageAtEnd()
{
    REPage* page = new REPage;
    page->_parent = this;
    page->SetNumber(PageCount());
    page->SetBounds(_score->PageRect());
    _pages.push_back(page);
    return page;
}

void REScoreRoot::DeletePageAtEnd()
{
    int pageCount = PageCount();
    if(pageCount > 0) {
        delete _pages[pageCount-1];
        _pages.pop_back();
    }
}
