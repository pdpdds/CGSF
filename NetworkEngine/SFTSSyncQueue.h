#pragma once
///////////////////////////////////////////////////////////////////////////
//쓰레드 세이프 동기화 큐
///////////////////////////////////////////////////////////////////////////

template <typename T>
class SFTSSyncQueue
{
public:
	SFTSSyncQueue(void){}
	virtual ~SFTSSyncQueue(void){}

	virtual BOOL Push(T* pMsg) = 0;
	virtual T* Pop() = 0;

protected:
	virtual BOOL Initialize() = 0;
	virtual BOOL Finally() = 0;

private:
};