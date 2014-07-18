#pragma once
#include "IDMap.h"
#include "Lock.h"

template <typename LockStrategy, typename T, int MaxIDCount>
class ProactorServiceManager : public SFIDMap<LockStrategy, T, MaxIDCount>
{
public:
	ProactorServiceManager(){}
	virtual ~ProactorServiceManager(){}

	bool SendInternal(int ownerSerial, char* buffer, unsigned int bufferSize)
	{
		SFLockHelper lock(&m_lock);

		T* pProactorService = Get(ownerSerial);

		if (pProactorService != NULL)
		{
			pProactorService->SendInternal(buffer, bufferSize);
		}

		return true;
	}

	bool Disconnect(int serial)
	{
		SFLockHelper lock(&m_lock);

		T* pProactorService = Get(serial);

		if (pProactorService != NULL)
		{
			pProactorService->Disconnect();
			return true;
		}

		return false;
	}

	bool BroadCast(int ownerSerial, int destSerial[], int destSize, char* buffer, unsigned int bufferSize)
	{
		SFLockHelper lock(&m_lock);

		for (int i = 0; i < destSize; i++)
		{
			T* pProactorService = Get(destSerial[i]);

			if (pProactorService != NULL)
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

		for (; iter != m_IDMap.end(); iter++)
		{
			int serial = iter.advance();

			T* pProactorService = Get(serial);

			if (pProactorService != NULL)
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
};