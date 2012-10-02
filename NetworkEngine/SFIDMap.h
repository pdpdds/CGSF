#pragma once
#include <ace/Map_Manager.h>
#include "SFQueue.h"

template <typename LockStrategy, typename T, int MaxIDCount>
class SFIDMap
{
	typedef ACE_Map_Manager<int, T*, LockStrategy> IDMap;

public:
	SFIDMap(void) :  m_idleIdQueue(0){}
	virtual ~SFIDMap(void){}

	int Register(T* p)
	{
		unsigned int id = m_idleIdQueue.Pop();
		if (id == INVALID_ID)
			return INVALID_ID;
		m_IDMap.bind(id, p);
		return id;
	}

	void UnRegister(int id)
	{
		T* t = NULL;
		if (-1 != m_IDMap.find(id, t))
		{
			m_IDMap.unbind(id);
			m_idleIdQueue.Push(id);
		}
	}

	T* Get(int id)
	{
		T* t = NULL;
		if (-1 == m_IDMap.find(id, t))
			return NULL;
		return t;
	}

	int Size()
	{
		return (uint32)m_IDMap.current_size();
	}

protected:
	IDMap m_IDMap;
	IDQueue<MaxIDCount> m_idleIdQueue;
};

template <typename LockStrategy, typename T, int MaxIDCount>
class SFProactorServiceIDMap : public SFIDMap<LockStrategy, T, MaxIDCount>
{
public:
	SFProactorServiceIDMap(){}
	virtual ~SFProactorServiceIDMap(){}

	/*BOOL Send(int Serial, USHORT PacketID, ::google::protobuf::Message* pMessage, int BufSize )
	{
		SFProactorService* pProactorService = Get(Serial);

		if(pProactorService != NULL)
		{
			pProactorService->Send(PacketID, pMessage, BufSize);
		}

		return TRUE;
	}*/

	BOOL Send(int Serial, USHORT PacketID, char* pMessage, int BufSize )
	{
		SFProactorService* pProactorService = Get(Serial);

		if(pProactorService != NULL)
		{
			pProactorService->Send(PacketID, pMessage, BufSize);
		}

		return TRUE;
	}

	BOOL Send(int Serial, SFPacket* pPacket)
	{
		SFProactorService* pProactorService = Get(Serial);

		if(pProactorService != NULL)
		{
			pProactorService->Send(pPacket);
		}

		return TRUE;
	}

	BOOL BroadCast(SFPacket* pPacket)
	{
		IDMap::ITERATOR iter = m_IDMap.begin();

		for(;iter != m_IDMap.end(); iter++)
		{
			int Serial = iter.advance();
			Send(Serial, pPacket);
		}

		return TRUE;
	}
};