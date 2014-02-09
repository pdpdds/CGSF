#pragma once
#include <ace/Map_Manager.h>
#include "Queue.h"
#include "Lock.h"

using namespace CGSF;

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
class ProactorServiceIDMap : public SFIDMap<LockStrategy, T, MaxIDCount>
{
public:
	ProactorServiceIDMap(){}
	virtual ~ProactorServiceIDMap(){}

	bool SendInternal(int ownerSerial, char* buffer, unsigned int bufferSize)
	{
		SFLockHelper lock(&m_lock);

		T* pProactorService = Get(ownerSerial);

		if(pProactorService != NULL)
		{
			pProactorService->SendInternal(buffer, bufferSize);
		}

		

		return true;
	}

	bool BroadCast(int ownerSerial, int destSerial[], int destSize, char* buffer, unsigned int bufferSize)
	{
		SFLockHelper lock(&m_lock);

		for(int i = 0; i < destSize; i++)
		{			
			T* pProactorService = Get(destSerial[i]);

			if(pProactorService != NULL)
			{
				pProactorService->SendInternal(buffer, bufferSize);
			}
		}		

		return true;
	}

	bool BroadCastAll(int ownerSerial, char* buffer, unsigned int bufferSize)
	{
		SFLockHelper lock(&m_lock);

		IDMap::ITERATOR iter = m_IDMap.begin();

		for(;iter != m_IDMap.end(); iter++)
		{		
			int serial = iter.advance();

			T* pProactorService = Get(serial);

			if(pProactorService != NULL)
			{
				pProactorService->SendInternal(buffer, bufferSize);
			}
		}

		return true;
	}	

	int Register(T* p)
	{		
		SFLockHelper lock(&m_lock);

		return SFIDMap<LockStrategy, T, MaxIDCount>::Register(p);
	}

	void UnRegister(int id)
	{		
		SFLockHelper lock(&m_lock);

		SFIDMap<LockStrategy, T, MaxIDCount>::UnRegister(id);
	}

private:
	SFLock m_lock;	

	/*BOOL Send(int Serial, USHORT PacketID, ::google::protobuf::Message* pMessage, int BufSize )
	{
		SFProactorService* pProactorService = Get(Serial);

		if(pProactorService != NULL)
		{
			pProactorService->Send(PacketID, pMessage, BufSize);
		}

		return TRUE;
	}*/


	/*BOOL Send(int Serial, SFPacket* pPacket)
	{
		SFProactorService* pProactorService = Get(Serial);

		if(pProactorService != NULL)
		{
			pProactorService->Send(pPacket);
		}

		return TRUE;
	}

	*/
};