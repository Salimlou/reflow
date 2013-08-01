#ifndef _REEXCEPTION_H_
#define _REEXCEPTION_H_

#include <stdexcept>
#include <string>

class REException : public std::exception
{
public:
	REException(const std::string& msg, const char* file, const char* function, int line);

	virtual ~REException() throw();

	virtual const char* what() const throw();
    std::string Message() const throw();
    
private:
    int _line;
    std::string _file;
    std::string _function;
    std::string _msg;
};

#define REThrow(msg) throw(REException((msg),__FILE__,__FUNCTION__,__LINE__))

#endif
