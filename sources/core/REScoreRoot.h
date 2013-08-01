//
//  REScoreRoot.h
//  Reflow
//
//  Created by Sebastien Bourgeois on 21/11/12.
//
//

#ifndef __Reflow__REScoreRoot__
#define __Reflow__REScoreRoot__

#include "REScoreNode.h"

class REScoreRoot : public REScoreNode
{
    friend class REScore;
    
public:
    virtual ~REScoreRoot();
    
protected:
    REScoreRoot(REScore* score) : REScoreNode(), _score(score) {}
    
public:
    void Clear();
    
    virtual Reflow::ScoreNodeType NodeType() const {return Reflow::RootNode;}

    virtual const REScoreRoot* Root() const;
    virtual REScoreRoot* Root();
    
    const REScore* Score() const {return _score;}
    REScore* Score() {return _score;}
    
    int PageCount() const {return _pages.size();}
    
    const REPage* Page(int idx) const;
    REPage* Page(int idx);
    
    REPage* CreatePageAtEnd();
    void DeletePageAtEnd();
    
    const REPageVector& Pages() const {return _pages;}
    
protected:
    REScore* _score;
    REPageVector _pages;
};

#endif /* defined(__Reflow__REScoreRoot__) */
