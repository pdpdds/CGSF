#pragma once

#include "DataBuffer.h"
#include "BasePacket.h"

/**
A subclass of this class defines a communication protocol between Push Framework and remote clients (or a category of remote
clients if you are using multiple protocols). Server business code is expected to send responses in form of OutgoingPacket
instances. The framework requests the protocol instance to serialize these instances by triggering encoding and framing jobs.
Incoming data is handed to the de-serialization functions which in turns calls on user-defined deframing and decoding in order
to construct the IncomingPacket instance and its accompanying service id.
*/
class SFProtocol
{
public:
	typedef enum Result
	{
		Success = 0,
		eInsufficientBuffer,
		eIncompletePacket,
		eCorruptPacket,
		eEncodingFailure,
		eDecodingFailure,
		eUndefinedFailure,
		ePacketizingError,
	};

	/** @name TORs **/
	//@{
	SFProtocol(void);
	~SFProtocol(void);
	//@}

	/** @name Consumed by Push Framework internal code. **/
	int serializeOutgoingPacket(BasePacket& packet, DataBuffer& Buffer,  unsigned int& nWrittenBytes);
	int tryDeserializeIncomingPacket(DataBuffer& Buffer,   BasePacket*& pPacket, int& serviceId, unsigned int& nExtractedBytes);
	//@}
public:
	/** @name Serialization. **/
	/**
	\param packet OutgoingPacket instance to encode.

	Override to encode the OutgoingPacket instance. Save the result into the same input.
	*/
	virtual int encodeOutgoingPacket(BasePacket& packet) =  0;
	/**
	\param packet OutgoingPacket instance to encode.
	\param buffer Buffer to write the result to.
	\param nWrittenBytes Report the total bytes written.

	Override to write the encoded packet into the intermediate sending buffer.
	*/
	virtual int frameOutgoingPacket(BasePacket& packet, DataBuffer& Buffer,  unsigned int& nWrittenBytes) = 0;
	//@}
	/** @name De-serialization. **/
	/**
	\param buffer to read data from.
	\param pPacket Reference to the output pointer that should store the address of the potentially created instance.
	\param serviceId Service id value used to route the created instance.
	\param nExtractedBytes Number of bytes extracted from the buffer.

	Override to deframe incoming packets from within the received bytes.
	*/
	virtual int tryDeframeIncomingPacket(DataBuffer& Buffer,  BasePacket*& pPacket, int& serviceId, unsigned int& nExtractedBytes) = 0;
	/**
	\param pPacket The previously deframed instance.
	\param serviceId Service id value used to route the created instance.

	Override to decode the created instance.
	*/
	virtual int decodeIncomingPacket(BasePacket* pPacket, int& serviceId) = 0;
	/**
	\param pPacket IncomingPacket instance to delete.

	Incoming requests are created by the Protocol at time of de-serialization. When the servicing job that treats
	a requests finishes, the instance is handed back to its creator for deletion. Developer can call on delete or
	any other way of releasing the instance if a pool of requests is organized.
	*/
	virtual void disposeIncomingPacket(BasePacket* pPacket) = 0;
	//@}

};