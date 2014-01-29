#pragma once


class SFPacketHandler
{
public:
	SFPacketHandler(void);
	virtual ~SFPacketHandler(void);

	void OnAuth(protobuf::io::ArrayInputStream& Msg);
};
