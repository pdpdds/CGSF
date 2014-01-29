#pragma once

template <typename T> class SFTSSyncQueue;

class SFNetworkHandler : public ACE_Task<ACE_MT_SYNCH>
{
public:
	SFNetworkHandler(SFTSSyncQueue<int>* pQueue){m_pQueue = pQueue;}
	virtual ~SFNetworkHandler(void){}

	virtual int svc(void)
	{
		for(int i = 0; i < 300000; i++)
		{
			int* p = new int;
			*p = i;
			m_pQueue->Push(p);
		}

		return 0;
	}


protected:

private:
	SFTSSyncQueue<int>* m_pQueue;
};
