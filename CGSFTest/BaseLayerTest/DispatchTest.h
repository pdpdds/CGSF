#pragma once
#include "ITest.h"
#include "EngineInterface/EngineStructure.h"
#include "SFProtobufPacket.h"
#include "PacketCore.pb.h"

class DispatchTest : public ITest
{
public:
	DispatchTest(void);
	virtual ~DispatchTest(void);

	virtual bool Run() override;

private:
	void ProcessCDispatch(BasePacket* userAuth);
	void ProcessCPPDispatch(BasePacket* userAuth);

};
