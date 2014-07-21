#pragma once

class JsonObjectNode;

class JsonBuilder
{
public:
	JsonBuilder();
	~JsonBuilder();

public:
	bool PushBuffer(const char* buffer, size_t bufferLen);
	bool PopCompleteNode(JsonObjectNode& node, unsigned short dataSize);

	static unsigned int MakeBuffer( const JsonObjectNode& node, char* buffer, unsigned int bufferLen );
	void PrepareBuffer(int ioSize);

	unsigned int GetUsedBufferSize() { return m_writtenOffset - m_readOffset; }
	char* GetBuffer(){ return &m_bufferPtr[m_readOffset]; }
	void IncReadOffset(unsigned int inc){ m_readOffset += inc; }

protected:
	bool AddBuffer(const char* buffer, unsigned int bufferLen);

private:
	char*		m_bufferPtr;
	unsigned int	m_writtenOffset;
	unsigned int	m_readOffset;
	int m_ioSize;
};
