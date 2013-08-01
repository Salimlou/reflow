//
//  RETool.h
//  Reflow
//
//  Created by Sebastien Bourgeois on 24/04/13.
//
//

#ifndef __Reflow__RETool__
#define __Reflow__RETool__

#include "RETypes.h"
#include "RESymbol.h"

class RESymbolManipulator;
class RETextSymbolManipulator;

/** RETool class
 */
class RETool
{
public:
    RETool(REScoreController* sc) : _scoreController(sc), _destroyingManipulators(false) {}
    
    virtual ~RETool() {}
    
    virtual Reflow::ToolType Type() const = 0;
    
    const char* Name() const;
    
    inline const REManipulatorMap& Manipulators() const {return _manipulators;}
    inline REManipulatorMap& Manipulators() {return _manipulators;}
    
    inline const REScoreController* ScoreController() const {return _scoreController;}
    inline REScoreController* ScoreController() {return _scoreController;}
    
    virtual bool MouseDown(const REPoint& pointInScene, unsigned long flags);
    virtual bool MouseUp(const REPoint& pointInScene, unsigned long flags);
    virtual bool MouseDragged(const REPoint& pointInScene, unsigned long flags);
    virtual bool MouseMoved(const REPoint& pointInScene, unsigned long flags);
    virtual bool MouseDoubleClicked(const REPoint& pointInScene, unsigned long flags);
    
    virtual void CreateManipulators(){}
    
    virtual void DestroyManipulators();
    void UpdateVisibleManipulators();
    
    bool IsDestroyingManipulators() const {return _destroyingManipulators;}
    
protected:
    REScoreController* _scoreController;
    REManipulatorMap _manipulators;
    bool _destroyingManipulators;
};

/** REHandTool
 */
class REHandTool : public RETool
{
public:
    REHandTool(REScoreController* sc) : RETool(sc) {}
    virtual ~REHandTool() {}
    
    virtual Reflow::ToolType Type() const {return Reflow::HandTool;}
};

/** REZoomTool
 */
class REZoomTool : public RETool
{
public:
    REZoomTool(REScoreController* sc) : RETool(sc) {}
    virtual ~REZoomTool() {}
    
    virtual Reflow::ToolType Type() const {return Reflow::ZoomTool;}
};

/** RESelectTool
 */
class RESelectTool : public RETool
{
public:
    RESelectTool(REScoreController* sc) : RETool(sc) {}
    virtual ~RESelectTool() {}
    
    virtual Reflow::ToolType Type() const {return Reflow::SelectTool;}
};

/** REBarSelectTool
 */
class REBarSelectTool : public RETool
{
public:
    REBarSelectTool(REScoreController* sc) : RETool(sc) {}
    virtual ~REBarSelectTool() {}
    
    virtual Reflow::ToolType Type() const {return Reflow::BarSelectTool;}
};

/** RENoteTool
 */
class RENoteTool : public RETool
{
public:
    RENoteTool(REScoreController* sc) : RETool(sc) {}
    virtual ~RENoteTool() {}
    
    virtual Reflow::ToolType Type() const {return Reflow::NoteTool;}

    virtual bool MouseMoved(const REPoint& pointInScene, unsigned long flags);
    virtual bool MouseUp(const REPoint& pointInScene, unsigned long flags);
    
    virtual void CreateManipulators();
};

/** RETablatureTool
 */
class RETablatureTool : public RETool
{
public:
    RETablatureTool(REScoreController* sc) : RETool(sc) {}
    virtual ~RETablatureTool() {}
    
    virtual Reflow::ToolType Type() const {return Reflow::TablatureTool;}

    virtual void CreateManipulators();
};

/** REEraserTool
 */
class REEraserTool : public RETool
{
public:
    REEraserTool(REScoreController* sc) : RETool(sc) {}
    virtual ~REEraserTool() {}
    
    virtual Reflow::ToolType Type() const {return Reflow::EraserTool;}
};

/** REDesignTool
 */
class REDesignTool : public RETool
{
public:
    REDesignTool(REScoreController* sc) : RETool(sc) {}
    virtual ~REDesignTool() {}
    
    virtual Reflow::ToolType Type() const {return Reflow::DesignTool;}
};

/** RESymbolTool
 */
class RESymbolTool : public RETool
{
public:
    RESymbolTool(REScoreController* sc) : RETool(sc) {}
    virtual ~RESymbolTool() {}
    
    virtual Reflow::ToolType Type() const {return Reflow::SymbolTool;}
    
    virtual bool MouseDown(const REPoint& pointInScene, unsigned long flags);
    virtual bool MouseUp(const REPoint& pointInScene, unsigned long flags);
    virtual bool MouseDragged(const REPoint& pointInScene, unsigned long flags);
    virtual bool MouseMoved(const REPoint& pointInScene, unsigned long flags);
    
    virtual void CreateManipulators();
    
protected:
    virtual RESymbolManipulator* CreateManipulatorForSymbol(const REConstSymbolAnchor& anchor, const REPoint& startPoint);
};


/** RETextTool
 */
class RETextTool : public RESymbolTool
{
public:
    RETextTool(REScoreController* sc) : RESymbolTool(sc) {}
    virtual ~RETextTool() {}
    
    virtual Reflow::ToolType Type() const {return Reflow::TextTool;}
    
protected:
    virtual RESymbolManipulator* CreateManipulatorForSymbol(const REConstSymbolAnchor& anchor, const REPoint& startPoint);
};


/** RELineTool
 */
class RELineTool : public RETool
{
public:
    RELineTool(REScoreController* sc) : RETool(sc) {}
    virtual ~RELineTool() {}
    
    virtual Reflow::ToolType Type() const {return Reflow::LineTool;}
};


/** RESlurTool
 */
class RESlurTool : public RETool
{
public:
    RESlurTool(REScoreController* sc) : RETool(sc) {}
    virtual ~RESlurTool() {}
    
    virtual Reflow::ToolType Type() const {return Reflow::SlurTool;}
    
    virtual void CreateManipulators();
};


/** REDiagramTool
 */
class REDiagramTool : public RETool
{
public:
    REDiagramTool(REScoreController* sc) : RETool(sc) {}
    virtual ~REDiagramTool() {}
    
    virtual Reflow::ToolType Type() const {return Reflow::DiagramTool;}
};


/** REClefTool
 */
class REClefTool : public RETool
{
public:
    REClefTool(REScoreController* sc) : RETool(sc) {}
    virtual ~REClefTool() {}
    
    virtual Reflow::ToolType Type() const {return Reflow::ClefTool;}
};


/** REKeyTool
 */
class REKeyTool : public RETool
{
public:
    REKeyTool(REScoreController* sc) : RETool(sc) {}
    virtual ~REKeyTool() {}
    
    virtual Reflow::ToolType Type() const {return Reflow::KeyTool;}
};


/** RETimeTool
 */
class RETimeTool : public RETool
{
public:
    RETimeTool(REScoreController* sc) : RETool(sc) {}
    virtual ~RETimeTool() {}
    
    virtual Reflow::ToolType Type() const {return Reflow::TimeTool;}
};


/** REBarlineTool
 */
class REBarlineTool : public RETool
{
public:
    REBarlineTool(REScoreController* sc) : RETool(sc) {}
    virtual ~REBarlineTool() {}
    
    virtual Reflow::ToolType Type() const {return Reflow::BarlineTool;}
};


/** RERepeatTool
 */
class RERepeatTool : public RETool
{
public:
    RERepeatTool(REScoreController* sc) : RETool(sc) {}
    virtual ~RERepeatTool() {}
    
    virtual Reflow::ToolType Type() const {return Reflow::RepeatTool;}
};


/** RETempoTool
 */
class RETempoTool : public RETool
{
public:
    RETempoTool(REScoreController* sc) : RETool(sc) {}
    virtual ~RETempoTool() {}
    
    virtual Reflow::ToolType Type() const {return Reflow::TempoTool;}
};


/** RESectionTool
 */
class RESectionTool : public RETool
{
public:
    RESectionTool(REScoreController* sc) : RETool(sc) {}
    virtual ~RESectionTool() {}
    
    virtual Reflow::ToolType Type() const {return Reflow::SectionTool;}
};


/** RELyricsTool
 */
class RELyricsTool : public RETool
{
public:
    RELyricsTool(REScoreController* sc) : RETool(sc) {}
    virtual ~RELyricsTool() {}
    
    virtual Reflow::ToolType Type() const {return Reflow::LyricsTool;}
};


/** RELiveVelocityTool
 */
class RELiveVelocityTool : public RETool
{
public:
    RELiveVelocityTool(REScoreController* sc) : RETool(sc) {}
    virtual ~RELiveVelocityTool() {}
    
    virtual Reflow::ToolType Type() const {return Reflow::LiveVelocityTool;}
};


/** RELiveDurationTool
 */
class RELiveDurationTool : public RETool
{
public:
    RELiveDurationTool(REScoreController* sc) : RETool(sc) {}
    virtual ~RELiveDurationTool() {}
    
    virtual Reflow::ToolType Type() const {return Reflow::LiveDurationTool;}
};


/** RELiveOffsetTool
 */
class RELiveOffsetTool : public RETool
{
public:
    RELiveOffsetTool(REScoreController* sc) : RETool(sc) {}
    virtual ~RELiveOffsetTool() {}
    
    virtual Reflow::ToolType Type() const {return Reflow::LiveOffsetTool;}
};


/** RESpacerTool
 */
class RESpacerTool : public RETool
{
public:
    RESpacerTool(REScoreController* sc) : RETool(sc) {}
    virtual ~RESpacerTool() {}
    
    virtual Reflow::ToolType Type() const {return Reflow::SpacerTool;}
};


/** REMemoTool
 */
class REMemoTool : public RETool
{
public:
    REMemoTool(REScoreController* sc) : RETool(sc) {}
    virtual ~REMemoTool() {}
    
    virtual Reflow::ToolType Type() const {return Reflow::MemoTool;}
};


/** RETupletTool
 */
class RETupletTool : public RETool
{
public:
    RETupletTool(REScoreController* sc) : RETool(sc) {}
    virtual ~RETupletTool() {}
    
    virtual Reflow::ToolType Type() const {return Reflow::TupletTool;}
};


/** REGraceTool
 */
class REGraceTool : public RETool
{
public:
    REGraceTool(REScoreController* sc) : RETool(sc) {}
    virtual ~REGraceTool() {}
    
    virtual Reflow::ToolType Type() const {return Reflow::GraceTool;}
};
#endif /* defined(__Reflow__RETool__) */
