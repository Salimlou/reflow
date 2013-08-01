//
//  REManipulator.h
//  Reflow
//
//  Created by Sebastien Bourgeois on 12/04/13.
//
//

#ifndef __Reflow__REManipulator__
#define __Reflow__REManipulator__

#include "REScoreNode.h"

class REHandle : public REScoreNode
{
    friend class REManipulator;
    
public:
    REHandle(const std::string& identifier, REManipulator* manipulator);
    
    virtual Reflow::ScoreNodeType NodeType() const {return Reflow::HandleNode;}
    
    inline const REManipulator* Manipulator() const {return _manipulator;}
    inline const std::string& Identifier() const {return _identifier;}
    
    bool IsDraggable() const {return _draggable;}
    void SetDraggable(bool draggable) {_draggable = draggable;}
    
    void SetParent(REScoreNode* p) {_parent = p;}
    
protected:
    REManipulator* _manipulator;
    std::string _identifier;
    bool _draggable;
};


/** REManipulator
 */
class REManipulator : public REScoreNode
{
    friend class REViewport;
    friend class REScoreController;
    
public:
    REManipulator(RETool* tool) : _tool(tool), _focusedHandle(nullptr), _editing(false) {}
    virtual ~REManipulator();
    
    virtual Reflow::ScoreNodeType NodeType() const {return Reflow::ManipulatorNode;}
    virtual Reflow::ManipulatorType ManipulatorType() const = 0;
    
    virtual void Draw(REPainter& painter) const = 0;
    
    REScoreController* ScoreController();
    const REScoreController* ScoreController() const;
    
    inline RETool* Tool() {return _tool;}
    inline const RETool* Tool() const {return _tool;}
    
    virtual bool MouseDown(const REPoint& pt, unsigned long flags);
    virtual bool MouseDragged(const REPoint& pt, unsigned long flags);
    virtual bool MouseUp(const REPoint& pt, unsigned long flags);
    virtual bool MouseDoubleClicked(const REPoint& pointInScene, unsigned long flags);
    
    bool IsEditing() const {return _editing;}
    virtual bool IsEditable() const {return false;}
    virtual void StartEditing() {}
    virtual void FinishedEditing() {}
    
    virtual bool IsDragging() const;
    
    virtual REHandle* HandleAtPoint(const REPoint& pt) const;
    virtual REHandle* DraggableHandleAtPoint(const REPoint& pt) const;
    
    virtual void RefreshBounds();
    
    virtual RERect ManipulatedItemBounds() {return RERect(0,0,0,0);}
    
protected:
    RETool* _tool;
    REHandleMap _handles;
    REHandle* _focusedHandle;
    REPoint _dragOrigin;
    REPoint _focusedHandleOriginPoint;
    bool _dragged;
    bool _editing;
};




/** RESymbolManipulator
 */
class RESymbolManipulator : public REManipulator
{
public:
    RESymbolManipulator(RETool* tool, const REStaff* staff, const REChord* chord, const RESymbol* psymbol, const REPoint& startPoint);
    
    virtual ~RESymbolManipulator() {}
    
    virtual Reflow::ManipulatorType ManipulatorType() const {return Reflow::SymbolManipulator;}
    
    virtual void Draw(REPainter& painter) const;
    
    virtual bool MouseDragged(const REPoint& pt, unsigned long flags);
    virtual bool MouseUp(const REPoint& pt, unsigned long flags);
    
    virtual RERect ManipulatedItemBounds();
    
    virtual bool IsEditable() const;
    virtual void StartEditing();
    virtual void FinishedEditing();
    
    const RESymbol* Symbol() const {return _psymbol;}
    const REStaff* Staff() const {return _staff;}
    
protected:
    void CreateHandles(const REPoint& startPoint);
    
protected:
    const REChord* _pchord;
    const RESymbol* _psymbol;
    const REStaff* _staff;
    REPoint _startPoint;
    REGlobalTimeDiv _newStartBeat;
    float _newStartBeatDeltaX;
};


/** RETextSymbolManipulator
 */
class RETextSymbolManipulator : public RESymbolManipulator
{
public:
    RETextSymbolManipulator(RETool* tool, const REStaff* staff, const REChord* chord, const RESymbol* psymbol, const REPoint& startPoint);
    
    virtual bool IsEditable() const;
};


/** RESlurManipulator
 */
class RESlurManipulator : public REManipulator
{
public:
    RESlurManipulator(RETool* tool, const REStaff* staff, int trackIndex, int slurIndex, const RESlur* pslur, const REPoint& startPoint, const REPoint& endPoint);
    
    virtual ~RESlurManipulator() {}
    
    virtual Reflow::ManipulatorType ManipulatorType() const {return Reflow::SlurManipulator;}

    virtual void Draw(REPainter& painter) const;
    
    virtual bool MouseDragged(const REPoint& pt, unsigned long flags);
    virtual bool MouseUp(const REPoint& pt, unsigned long flags);    
    
protected:
    void CreateHandles(const REPoint& startPoint, const REPoint& endPoint);
    
protected:
    const RESlur* _pslur;
    const REStaff* _staff;
    int _slurIndex;
    int _trackIndex;
    
    REPoint _startPoint;
    REPoint _endPoint;
    REGlobalTimeDiv _newStartBeat;
    REGlobalTimeDiv _newEndBeat;
    float _newStartBeatDeltaX;
    float _newEndBeatDeltaX;
};


/** 
 */
class RENoteToolHoverManipulator : public REManipulator
{
public:
    RENoteToolHoverManipulator(RETool* noteTool) : REManipulator(noteTool) {}
    virtual ~RENoteToolHoverManipulator() {}
    
    virtual Reflow::ManipulatorType ManipulatorType() const {return Reflow::NoteToolHoverManipulator;}
    
    virtual RERect ManipulatedItemBounds();
    virtual void Draw(REPainter& painter) const;
};

#endif /* defined(__Reflow__REManipulator__) */
