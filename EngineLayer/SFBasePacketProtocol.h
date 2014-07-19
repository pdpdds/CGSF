#pragma once

class SFBasePacketProtocol
{
public:
	SFBasePacketProtocol();
	virtual ~SFBasePacketProtocol();

	inline int GetIOSize()			{ return m_ioSize; }
	inline unsigned short GetPacketSize()	{ return m_packetSize; }
	inline int GetPacketOption()	{ return m_packetOption; }

	void CopyBaseProtocol(SFBasePacketProtocol& source);

protected:
	int m_ioSize;
	unsigned short m_packetSize;
	int m_packetOption;
};

