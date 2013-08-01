//
//  REOutputStream.cpp
//  Reflow
//
//  Created by Sebastien on 12/04/11.
//  Copyright 2011 Gargant Studios. All rights reserved.
//

#include <cassert>
#include <string.h>

#include "REOutputStream.h"
#include "RETypes.h"


void REOutputStream::WriteUInt32(uint32_t val)
{
    if(REFLOW_IS_HOST_ENDIANNESS(_endianness)) {
        Write(reinterpret_cast<const char*>(&val), sizeof(uint32_t));
    }
    else {
        unsigned char bytes[4] = {static_cast<unsigned char>((val >> 24) & 0xFF), static_cast<unsigned char>((val >> 16) & 0xFF), static_cast<unsigned char>((val >> 8) & 0xFF), static_cast<unsigned char>(val & 0xFF)};
        Write((const char*)bytes, 4);
    }
}

void REOutputStream::WriteUInt16(uint16_t val)
{
    if(REFLOW_IS_HOST_ENDIANNESS(_endianness)) {
        Write(reinterpret_cast<const char*>(&val), sizeof(uint16_t));
    }
    else {
        unsigned char bytes[2] = {static_cast<unsigned char>((val >> 8) & 0xFF), static_cast<unsigned char>(val & 0xFF)};
        Write((const char*)bytes, 2);        
    }
}

void REOutputStream::WriteUInt8(uint8_t val)
{
    Write(reinterpret_cast<const char*>(&val), sizeof(uint8_t));
}

void REOutputStream::WriteInt32(int32_t val)
{
    if(REFLOW_IS_HOST_ENDIANNESS(_endianness)) {
        Write(reinterpret_cast<const char*>(&val), sizeof(int32_t));
    }
    else {
        unsigned char bytes[4] = {static_cast<unsigned char>((val >> 24) & 0xFF), static_cast<unsigned char>((val >> 16) & 0xFF), static_cast<unsigned char>((val >> 8) & 0xFF), static_cast<unsigned char>(val & 0xFF)};
        Write((const char*)bytes, 4);
    }
}

void REOutputStream::WriteInt16(int16_t val)
{
    if(REFLOW_IS_HOST_ENDIANNESS(_endianness)) {
        Write(reinterpret_cast<const char*>(&val), sizeof(int16_t));
    }
    else {
        unsigned char bytes[2] = {static_cast<unsigned char>((val >> 8) & 0xFF), static_cast<unsigned char>(val & 0xFF)};
        Write((const char*)bytes, 2);        
    }
}

void REOutputStream::WriteInt8(int8_t val)
{
    Write(reinterpret_cast<const char*>(&val), sizeof(int8_t));
}

void REOutputStream::WriteUInt24(uint32_t val)
{
    if(REFLOW_IS_HOST_ENDIANNESS(_endianness)) {
        unsigned char bytes[3] = {static_cast<unsigned char>(val & 0xFF), static_cast<unsigned char>((val >> 8) & 0xFF), static_cast<unsigned char>((val >> 16) & 0xFF)};
        Write((const char*)bytes, 3);
    }
    else {
        unsigned char bytes[3] = {static_cast<unsigned char>((val >> 16) & 0xFF), static_cast<unsigned char>((val >> 8) & 0xFF), static_cast<unsigned char>(val & 0xFF)};
        Write((const char*)bytes, 3);
    }
}

void REOutputStream::WriteVLV(int x)
{
    // Variable length value does not consider endianness
    unsigned long value = (unsigned long) x;
	
	unsigned char bytes[4];
	bytes[0] = (value >> 21) & 0x7F;
	bytes[1] = (value >> 14) & 0x7F;
	bytes[2] = (value >> 7) & 0x7F;
	bytes[3] = (value) & 0x7F;
	
	int i;
	int flag = 0;
	for (i = 0; i < 3; ++i) {
		if (bytes[i] != 0)
			flag = 1;
		if (flag)
			bytes[i] |= 0x80;
	}
	
	for (i = 0; i < 4; ++i) {
		if (bytes[i] >= 0x80 || i == 3) {
            Write((const char*)&bytes[i], 1);
		}
	}   
}

void REOutputStream::WriteDouble(double val) {
    Write(reinterpret_cast<const char*>(&val), sizeof(double));
}

void REOutputStream::WriteFloat(float val) {
    Write(reinterpret_cast<const char*>(&val), sizeof(float));
}

void REOutputStream::WriteString(const std::string& val) {
    const char* ptr = val.data();
    size_t nb = val.size();
    WriteUInt32(nb);
    if(nb > 0) {
        Write(ptr, nb);
    }
}

REBufferOutputStream::REBufferOutputStream()
: _pos(0UL)
{
    SetVersion(REFLOW_IO_VERSION);
    SetSubType(REFLOW_IO_REFLOW2);
    SetEndianness(Reflow::LittleEndian);
    
    _buffer.reserve(1024 * 1024);
}

void REBufferOutputStream::Write(const char* bytes, unsigned long size)
{
    unsigned long total = _buffer.size();
    
    if(_pos == total) {
        _buffer.append(bytes, size);
    }
    else if (_pos + size <= total) {
		char* data = (char*)_buffer.data();
        memcpy(data + _pos, bytes, size);
    }
    else {
        unsigned long nb = (total - _pos);
		char* data = (char*)_buffer.data();
        memcpy(data + _pos, bytes, nb);
        _buffer.append(bytes + (nb), size - nb);
    }
    _pos += size;
}

const char* REBufferOutputStream::Data() const 
{
    return _buffer.data();
}

unsigned long REBufferOutputStream::Size() const
{
    return _buffer.size();
}

unsigned long REBufferOutputStream::Pos() const
{
    return _pos;
}
void REBufferOutputStream::SeekTo(unsigned long pos)
{
    assert(pos <= Size());
    _pos = pos;
}

void REBufferOutputStream::Skip(unsigned long size)
{
    if(size == 0) return;
    
    unsigned long total = _buffer.size();
    
    if(_pos == total) {
        _buffer.append(size, '\0');
    }
    else if (_pos + size <= total) {
		/* Nothing to do, just skipping. */
    }
    else {
        unsigned long nb = (total - _pos);
		/* Skip */
        _buffer.append(size - nb, '\0');
    }
    _pos += size;
}

