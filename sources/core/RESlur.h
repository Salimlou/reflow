//
//  RESlur.h
//  Reflow
//
//  Created by Sebastien Bourgeois on 12/04/13.
//
//

#ifndef __Reflow__RESlur__
#define __Reflow__RESlur__

#include "RETypes.h"

class RESlur
{
public:
    RESlur();
    RESlur(const RESlur& s);
    
    RESlur& operator=(const RESlur& s);
    
    inline const REGlobalTimeDiv& StartBeat() const {return _startBeat;}
    inline const REGlobalTimeDiv& EndBeat() const {return _endBeat;}
    inline const REPoint& StartOffset() const {return _startOffset;}
    inline const REPoint& EndOffset() const {return _endOffset;}
    inline const REPoint& StartControlPointOffset() const {return _startControlPointOffset;}
    inline const REPoint& EndControlPointOffset() const {return _endControlPointOffset;}
    inline float Thickness() const {return _thickness;}
    
    void SetStartBeat(const REGlobalTimeDiv& div) {_startBeat = div;}
    void SetEndBeat(const REGlobalTimeDiv& div) {_endBeat = div;}
    void SetStartOffset(const REPoint& off) {_startOffset = off;}
    void SetEndOffset(const REPoint& off) {_endOffset = off;}
    void SetStartControlPointOffset(const REPoint& off) {_startControlPointOffset = off;}
    void SetEndControlPointOffset(const REPoint& off) {_endControlPointOffset = off;}
    
    RERange BarRange() const;
    float MinY() const;
    float MaxY() const;
    
    
    void Draw(REPainter& painter, float x0, float x1, float y, const REColor& color) const;
    
    void EncodeTo(REOutputStream& coder) const;
    void DecodeFrom(REInputStream& decoder);
    
    void WriteJson(REJsonWriter& writer, uint32_t version) const;
    void ReadJson(const REJsonValue& obj, uint32_t version);
    
protected:
    REGlobalTimeDiv _startBeat;
    REGlobalTimeDiv _endBeat;
    REPoint _startOffset;
    REPoint _endOffset;
    REPoint _startControlPointOffset;
    REPoint _endControlPointOffset;
    float _thickness;
};

typedef std::vector<RESlur> RESlurVector;
typedef std::vector<const RESlur*> REConstSlurPtrVector;

#endif /* defined(__Reflow__RESlur__) */

