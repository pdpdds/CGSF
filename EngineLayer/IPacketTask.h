#pragma once

class IPacketTask
{
public:
	IPacketTask(){}
	virtual ~IPacketTask(){}

	virtual bool Execute() = 0;
	void SetPacket(BasePacket* pPacket, std::vector<int>& ownerList){ m_pPacket = pPacket; m_vecOwner = ownerList; }

protected:
	BasePacket* m_pPacket;
	std::vector<int> m_vecOwner;
};