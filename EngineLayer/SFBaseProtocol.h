#pragma once
class SFBaseProtocol
{
public:
	SFBaseProtocol();
	virtual ~SFBaseProtocol();

	static int m_ioSize;
	static USHORT m_packetSize;

	inline static int GetIOSize(){ return m_ioSize; }
	inline static USHORT GetPacketSize(){ return m_packetSize; }
};

