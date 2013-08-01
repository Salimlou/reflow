//
//  RETable.cpp
//  Reflow
//
//  Created by Sebastien Bourgeois on 26/04/13.
//
//

#include "RETable.h"

void RETable::Clear()
{
    for(RETableSection* s : _sections) {
        delete s;
    }
    _sections.clear();
}

void RETable::AddSection(RETableSection* section)
{
    section->_table = this;
    _sections.push_back(section);
}

void RETable::UpdateIndices()
{
    int idx = 0;
    for(RETableSection* section : _sections)
    {
        section->_flatIndex = idx++;
        idx += section->RowCount();
    }
    _flatRowCount = idx;
}

RETableSection* RETable::Section(int index)
{
    return (index >= 0 && index < _sections.size() ? _sections.at(index) : nullptr);
}

RETableSection* RETable::SectiontAtFlatIndex(int flatIndex)
{
    for(RETableSection* section : _sections)
    {
        if(section->_flatIndex == flatIndex) return section;
    }
    return nullptr;
}

RETableRow* RETable::RowAtFlatIndex(int flatIndex)
{
    for(RETableSection* section : _sections)
    {
        int firstRowIndex = section->_flatIndex + 1;
        int lastRowIndex = section->_flatIndex + section->RowCount();
        if(firstRowIndex <= flatIndex && flatIndex <= lastRowIndex)
        {
            return section->Row(flatIndex - firstRowIndex);
        }
    }
    return nullptr;
}

int RETable::FlatRowCount() const
{
    return _flatRowCount;
}

#pragma mark -
RETableSection::RETableSection(const std::string& title)
: _table(nullptr), _title(title)
{
}

RETableSection::~RETableSection()
{
    Clear();
}

void RETableSection::Clear()
{
    for(RETableRow* row : _rows) delete row;
    _rows.clear();
}

void RETableSection::AddRow(RETableRow* row)
{
    row->_section = this;
    _rows.push_back(row);
}

RETableRow* RETableSection::Row(int idx)
{
    return idx >= 0 && idx < _rows.size() ? _rows[idx] : nullptr;
}



#pragma mark -
RETableRow::RETableRow()
: _title(""), _section(nullptr)
{
}
