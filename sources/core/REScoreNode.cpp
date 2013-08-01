//
//  REScoreNode.cpp
//  Reflow
//
//  Created by Sebastien Bourgeois on 21/11/12.
//
//

#include "REScoreNode.h"
#include "REViewport.h"

REScoreNode::~REScoreNode()
{
    assert(_viewportItem == nullptr);
    //if(_viewportItem) {
    //    delete _viewportItem;
    //}
}

const REScoreRoot* REScoreNode::Root() const
{
    return _parent ? _parent->Root() : NULL;
}

REScoreRoot* REScoreNode::Root()
{
    return _parent ? _parent->Root() : NULL;
}

RERect REScoreNode::Frame() const
{
    return _bounds.Translated(_position);
}

REPoint REScoreNode::ScenePosition() const
{
    return _parent ? _parent->ScenePosition() + Position() : Position();
}

RERect REScoreNode::SceneFrame() const
{
    return _bounds.Translated(ScenePosition());
}

REPoint REScoreNode::PointFromSceneToLocal(const REPoint& pointInSceneSpace) const
{
    return pointInSceneSpace - ScenePosition();
}

REPoint REScoreNode::PointFromLocalToScene(const REPoint& pointInLocalSpace) const
{
    return pointInLocalSpace + ScenePosition();
}

RERect REScoreNode::RectFromSceneToLocal(const RERect& rectInSceneSpace) const
{
    return RERect(rectInSceneSpace.origin - ScenePosition(), rectInSceneSpace.size);
}

RERect REScoreNode::RectFromLocalToScene(const RERect& rectInLocalSpace) const
{
    return RERect(rectInLocalSpace.origin + ScenePosition(), rectInLocalSpace.size);
}

