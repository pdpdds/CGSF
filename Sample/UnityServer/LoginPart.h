#pragma once
class BasePacket;
#include "CommonData.h"
class LoginPart
{
public:
	void REQD_LOGIN(BasePacket* pPacket);
	void REQD_IDISIDVALID(BasePacket* pPacket);
	void REQD_REGISTRATION(BasePacket* pPacket);
};