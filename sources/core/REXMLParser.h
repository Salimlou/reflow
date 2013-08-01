//
//  REXMLParser.h
//  Reflow
//
//  Created by Sebastien Bourgeois on 10/02/12.
//  Copyright (c) 2012 Gargant Studios. All rights reserved.
//

#ifndef Reflow_REXMLParser_h
#define Reflow_REXMLParser_h

#include "RETypes.h"

class REXMLParser;
class REXMLParserDelegate;

typedef std::map<std::string, std::string> REXMLAttributeMap;


class REXMLParserDelegate
{
public:
    virtual void OnStartDocument(const REXMLParser& parser) {}
    virtual void OnEndDocument(const REXMLParser& parser) {}
    virtual void OnStartElement(const REXMLParser& parser, const std::string& name, const std::string& namespaceURI, const std::string &qName, const REXMLAttributeMap& attributes) {}
    virtual void OnEndElement(const REXMLParser& parser, const std::string& name, const std::string& namespaceURI, const std::string &qName) {}
};


class REXMLParser
{
public:
    REXMLParser();
    ~REXMLParser();
    
    void SetDelegate(REXMLParserDelegate* delegate) {_delegate = delegate;}
    REXMLParserDelegate* Delegate() {return _delegate;}
    
    bool ParseData(const char* bytes, int size);
    
private:
    REXMLParserDelegate* _delegate;
    
/*private:
    class Impl;
    Impl * const _d;*/
};


#endif
