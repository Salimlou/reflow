//
//  RETable.h
//  Reflow
//
//  Created by Sebastien Bourgeois on 26/04/13.
//
//

#ifndef __Reflow__RETable__
#define __Reflow__RETable__

#include "RETypes.h"


/** RETable
 */
class RETable
{
public:
    inline int SectionCount() const {return _sections.size();}
    
    void Clear();
    void AddSection(RETableSection* section);
    
    void UpdateIndices();
    
    RETableSection* Section(int index);
    RETableSection* SectiontAtFlatIndex(int flatIndex);

    RETableRow* RowAtFlatIndex(int flatIndex);
    int FlatRowCount() const;
    
protected:
    RETableSectionVector _sections;
    int _flatRowCount;
};


/** RETableSection
 */
class RETableSection
{
    friend class RETable;
    
public:
    RETableSection(const std::string& title);
    ~RETableSection();
    
    void Clear();
    void AddRow(RETableRow* row);
    
    inline int RowCount() const {return _rows.size();}
    RETableRow* Row(int idx);
    
    inline const std::string& Title() const {return _title;}
    
protected:
    RETable* _table;
    int _flatIndex;
    RETableRowVector _rows;
    std::string _title;
};


/** RETableRow
 */
class RETableRow
{
    friend class RETableSection;
    
public:
    RETableRow();
    
    inline const std::string& Title() const {return _title;}
    void SetTitle(const std::string& title) {_title = title;}
    
protected:
    RETableSection* _section;    
    std::string _title;
};

#endif /* defined(__Reflow__RETable__) */
