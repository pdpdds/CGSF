#pragma once

class JsonObjectNode;

class JsonBuilder
{
public:
	JsonBuilder();
	~JsonBuilder();

public:
	void PushBuffer(const char* buffer, size_t bufferLen);
	bool PopCompleteNode(JsonObjectNode& node);

	static unsigned int MakeBuffer( const JsonObjectNode& node, char* buffer, unsigned int bufferLen );

protected:
	bool AddBuffer(const char* buffer, size_t bufferLen);

private:
	char*		m_bufferPtr;
	unsigned int	m_writtenOffset;
	unsigned int	m_readOffset;

};
