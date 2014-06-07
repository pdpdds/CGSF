#pragma once
#include "SFAvroPacketImpl.h"
#include "avro/Encoder.hh"
#include "avro/Decoder.hh"

template<class T>
class SFAvroPacket : public SFAvroPacketImpl
{
	friend class SFAvroProtocol;

public:
	SFAvroPacket(USHORT usPacketId){ SetPacketID(usPacketId); GetHeader()->packetID = usPacketId; }
	virtual ~SFAvroPacket(){}

	T& GetData()
	{
		return m_Packet;
	}

	virtual void* GetBuffer() 
	{
		return &m_Packet;
	}
	virtual int GetBufferSize()
	{
		return sizeof(T);
	}

	virtual bool Encode(avro::EncoderPtr e) override;
	virtual bool Decode(char* pBuf, unsigned int nSize) override;

protected:

private:
	SFAvroPacket();
	
	T m_Packet;
};

template <typename T>
bool SFAvroPacket<T>::Encode(avro::EncoderPtr e)
{
	avro::encode(*e, m_Packet);

	return true;
}

template <typename T>
bool SFAvroPacket<T>::Decode(char* pBuf, unsigned int nSize)
{
	/*std::auto_ptr<avro::InputStream> in = avro::memoryInputStream((const uint8_t*)pBuf, (size_t)nSize);
	avro::DecoderPtr d = avro::binaryDecoder();
	d->init(*in);

	avro::decode(*d, m_Packet);*/
	memcpy(&m_Packet, pBuf, sizeof(T));
	return true;
}
