//
//  RELayout.h
//  Reflow
//
//  Created by Sebastien Bourgeois on 28/06/13.
//
//

#ifndef __Reflow__RELayout__
#define __Reflow__RELayout__

#include "RETypes.h"

class RELayout
{
public:
    RELayout();
    virtual ~RELayout();

public:
    virtual RELayout* Clone() const = 0;
    virtual std::string Identifier() const = 0;
    virtual void EncodeTo(REOutputStream& coder) const = 0;
    virtual void DecodeFrom(REInputStream& decoder) = 0;
    virtual void CalculateSystems(REScore* score) = 0;
    
public:
    virtual void DispatchSystems(REScore* score);
    virtual REBarMetrics* CalculateBarMetrics(const REScore* score, int barIndex);
    virtual void RefreshSystemVerticalGuides(RESystem* system);
    virtual void RefreshSystemHorizontalGuides(RESystem* system);
    virtual void RefreshStaffGuides(REStaff* staff);
    
public:
    static RELayout* CreateLayoutWithIdentifier(const std::string& identifier);
    
protected:
    virtual void CalculateSystemFromBarRange(RESystem* system, unsigned int firstBarIndex, unsigned int barCount, const std::vector<REBarMetrics*> *barMetrics);
};


/** Flexible Layout
 */
class REFlexibleLayout : public RELayout
{
public:
    REFlexibleLayout();
    virtual ~REFlexibleLayout();
    
public:
    virtual RELayout* Clone() const;
    virtual std::string Identifier() const;
    virtual void EncodeTo(REOutputStream& coder) const;
    virtual void DecodeFrom(REInputStream& decoder);
    virtual void CalculateSystems(REScore* score);
};


/** Manual Layout
 */
class REManualLayout : public RELayout
{
public:
    REManualLayout();
    virtual ~REManualLayout();
    
public:
    virtual RELayout* Clone() const;
    virtual std::string Identifier() const;
    virtual void EncodeTo(REOutputStream& coder) const;
    virtual void DecodeFrom(REInputStream& decoder);
    virtual void CalculateSystems(REScore* score);
};

/** Fixed Layout
 */
class REFixedLayout : public RELayout
{
public:
    REFixedLayout();
    virtual ~REFixedLayout();
    
public:
    virtual RELayout* Clone() const;
    virtual std::string Identifier() const;
    virtual void EncodeTo(REOutputStream& coder) const;
    virtual void DecodeFrom(REInputStream& decoder);
    virtual void CalculateSystems(REScore* score);    
};

/** Horizontal Layout
 */
class REHorizontalLayout : public RELayout
{
public:
    REHorizontalLayout();
    virtual ~REHorizontalLayout();
    
public:
    virtual RELayout* Clone() const;
    virtual std::string Identifier() const;
    virtual void EncodeTo(REOutputStream& coder) const;
    virtual void DecodeFrom(REInputStream& decoder);
    virtual void CalculateSystems(REScore* score);
    
    virtual void DispatchSystems(REScore* score);
};

#endif /* defined(__Reflow__RELayout__) */
