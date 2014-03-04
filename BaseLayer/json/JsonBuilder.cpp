#include "StdAfx.h"
#include "JsonBuilder.h"
#include "JsonNode.h"

const unsigned int	MaxBufferSize = 1024*4;

const char* Seperator = "\r\n";
const int SeperatorLen = strlen(Seperator);

JsonBuilder::JsonBuilder() 
	: m_bufferPtr(NULL)
	, m_writtenOffset(0)
	, m_readOffset(0)
{
	m_bufferPtr = new char[MaxBufferSize];
	memset(m_bufferPtr, 0, MaxBufferSize);
}

JsonBuilder::~JsonBuilder()
{
	if (NULL != m_bufferPtr)
	{
		delete [] m_bufferPtr;
		m_bufferPtr = NULL;
	}
}

void JsonBuilder::PushBuffer( const char* buffer, size_t bufferLen )
{
	if (false == AddBuffer(buffer, bufferLen))
	{
		// Todo : AllocBuffer
		// AddBuffer(buffer, bufferLen);
	}
}

bool JsonBuilder::PopCompleteNode( JsonObjectNode& node )
{
	std::string totalContent(&m_bufferPtr[m_readOffset], m_writtenOffset);
	size_t pos = totalContent.find(Seperator);
	if (std::string::npos == pos)
		return false;

	unsigned int usedSize = node.Parse(&m_bufferPtr[m_readOffset], pos);
	if (0 == usedSize)
		return false;
	
	m_readOffset += usedSize+SeperatorLen;
	if (m_readOffset == m_writtenOffset)
	{
		m_readOffset = 0;
		m_writtenOffset = 0;
	}
	return true;
}

bool JsonBuilder::AddBuffer( const char* buffer, unsigned int bufferLen )
{
	if ((m_writtenOffset+bufferLen)>MaxBufferSize)
		return false;

	memcpy(&m_bufferPtr[m_writtenOffset], buffer, bufferLen);
	m_writtenOffset += bufferLen;
	return true;
}

unsigned int JsonBuilder::MakeBuffer( const JsonObjectNode& node, char* buffer, unsigned int bufferLen )
{
	tstring contentStr = node.ToString();
	if (bufferLen < contentStr.length())
		return 0;
	memcpy(buffer, contentStr.c_str(), contentStr.length());
	memcpy(&buffer[contentStr.length()], Seperator, SeperatorLen);
	return (unsigned int)(contentStr.length()+SeperatorLen);
}