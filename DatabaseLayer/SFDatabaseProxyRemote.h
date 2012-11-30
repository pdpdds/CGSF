#pragma once

template <typename T>
class SFDatabaseProxyRemote : public SFDatabaseProxy<T>
{
public:
	SFDatabaseProxyRemote(void){}
	virtual ~SFDatabaseProxyRemote(void){}

	//virtual BOOL Initialize() override;
	//virtual BOOL SendDBRequest(T* pMessage) override;
};
