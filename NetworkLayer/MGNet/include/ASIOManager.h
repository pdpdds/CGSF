#pragma once
#include "asios.h"

class ASIOManager
{
public:
	ASIOManager(void);
	virtual ~ASIOManager(void);

protected:

private:
	INetworkReceiver* m_pNetworkReceiver;
};

