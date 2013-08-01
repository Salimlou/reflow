//
//  REScoreNode.h
//  Reflow
//
//  Created by Sebastien Bourgeois on 21/11/12.
//
//

#ifndef __Reflow__REScoreNode__
#define __Reflow__REScoreNode__

#include "RETypes.h"

class REScoreNode
{
protected:
    REScoreNode() : _parent(NULL), _position(0,0), _bounds(0,0,0,0), _viewportItem(NULL) {}
    
public:
    virtual ~REScoreNode();
    
    virtual Reflow::ScoreNodeType NodeType() const =0;
    
public:
    virtual void SetPosition(const REPoint& pos) {_position = pos;}
    virtual void SetBounds(const RERect& rc) {_bounds = rc;}
    virtual void SetOrigin(const REPoint& pt) {_bounds.origin = pt;}
    virtual void SetSize(const RESize& sz) {_bounds.size = sz;}
    
public:
    const REScoreNode* Parent() const {return _parent;}
    REScoreNode* Parent() {return _parent;}
    
    virtual const REScoreRoot* Root() const;
    virtual REScoreRoot* Root();
    
    const REPoint& Position() const {return _position;}
    const RERect& Bounds() const {return _bounds;}
    const REPoint& Origin() const {return _bounds.origin;}
    const RESize& Size() const {return _bounds.size;}
    float Height() const {return _bounds.size.h;}
    float Width() const {return _bounds.size.w;}
    
    RERect Frame() const;
    
    REPoint ScenePosition() const;
    RERect SceneFrame() const;
    
    REPoint PointFromSceneToLocal(const REPoint& pointInSceneSpace) const;
    REPoint PointFromLocalToScene(const REPoint& pointInLocalSpace) const;
    
    RERect RectFromSceneToLocal(const RERect& rectInSceneSpace) const;
    RERect RectFromLocalToScene(const RERect& rectInLocalSpace) const;
    
    void SetViewportItem(REViewportItem* viewportItem) {_viewportItem = viewportItem;}
    const REViewportItem* ViewportItem() const {return _viewportItem;}
    REViewportItem* ViewportItem() {return _viewportItem;}
    
public:
    void SetSize(double w, double h) {SetSize(RESize(w, h));}
    void SetOrigin(double x, double y) {SetOrigin(REPoint(x,y));}
    void SetWidth(double w) {SetSize(w, Height());}
    void SetHeight(double h) {SetSize(Width(), h);}
    
protected:
    REScoreNode* _parent;
    REPoint _position;          // Offset relative to parent
    RERect _bounds;             // Bounds in local space
    REViewportItem* _viewportItem;
};

#endif /* defined(__Reflow__REScoreNode__) */
