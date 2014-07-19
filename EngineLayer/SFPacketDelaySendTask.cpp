#include "stdafx.h"
#include "SFPacketDelaySendTask.h"
#include "BasePacket.h"
#include "SFEngine.h"

SFPacketDelaySendTask::SFPacketDelaySendTask()
{
}

SFPacketDelaySendTask::~SFPacketDelaySendTask()
{
}

bool SFPacketDelaySendTask::Execute()
{	
	SFEngine::GetInstance()->SendRequest(m_pPacket, m_vecOwner);
	SFEngine::GetInstance()->GetPacketProtocol()->DisposePacket(m_pPacket);

	m_pPacket = NULL;

	return true;
}