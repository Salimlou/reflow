#include "REXMLParser.h"

#include <QXmlSimpleReader>
#include <QXmlDefaultHandler>


class REXmlDefaultHandler : public QXmlDefaultHandler
{
public:
    REXMLParser* _parser;

public:
    virtual bool startDocument()
    {
        _parser->Delegate()->OnStartDocument(*_parser);
		return true;
    }

    virtual bool endDocument()
    {
        _parser->Delegate()->OnEndDocument(*_parser);
		return true;
    }

    virtual bool startElement ( const QString & namespaceURI, const QString & localName, const QString & qName, const QXmlAttributes & atts )
    {
        std::string namespaceURI_ = namespaceURI.toStdString();
        std::string localName_ = localName.toStdString();
        std::string qname_ = qName.toStdString();

        REXMLAttributeMap attributes;
        for(int i=0; i<atts.count(); ++i) {
            std::string key = atts.qName(i).toStdString();
            std::string val = atts.value(i).toStdString();
            attributes[key] = val;
        }

        _parser->Delegate()->OnStartElement(*_parser, localName_, namespaceURI_, qname_, attributes);
		return true;
	}

    virtual bool endElement ( const QString & namespaceURI, const QString & localName, const QString & qName )
    {
        std::string namespaceURI_ = namespaceURI.toStdString();
        std::string localName_ = localName.toStdString();
        std::string qname_ = qName.toStdString();

        _parser->Delegate()->OnEndElement(*_parser, localName_, namespaceURI_, qname_);
		return true;
    }
};

REXMLParser::REXMLParser()
{

}

REXMLParser::~REXMLParser()
{

}

bool REXMLParser::ParseData(const char* bytes, int size)
{
    REXmlDefaultHandler handler;
    handler._parser = this;

    // Xml Reader
    QXmlSimpleReader xmlReader;
    xmlReader.setContentHandler(&handler);
    xmlReader.setErrorHandler(&handler);

    // Xml Source
    QByteArray buffer(bytes, size);
    QXmlInputSource xmlSource;
    xmlSource.setData(buffer);

    return xmlReader.parse(xmlSource);
}
