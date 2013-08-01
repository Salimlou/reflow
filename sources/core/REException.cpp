#include "REException.h"

#include <sstream>

REException::REException (const std::string& msg, const char* file, const char* function, int line)
: _msg(msg), _file(file), _function (function), _line(line)
{}

REException::~REException() throw()
{
}

const char* REException::what() const throw()
{
	return Message().c_str();
}

std::string REException::Message() const throw()
{
    std::ostringstream os;
    os << _msg << " (file: " << _file << " line: " << _line << " func: " << _function << ")";
    return os.str();
}
