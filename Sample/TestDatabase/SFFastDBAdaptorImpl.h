#pragma once
#include "SFFastDBAdaptor.h"

class BasePacket;

class SFFastDBAdaptorImpl : public SFFastDBAdaptor
{
public:
	SFFastDBAdaptorImpl(void);
	virtual ~SFFastDBAdaptorImpl(void);

	virtual BOOL RegisterDBService() override;

protected:

private:	
	BOOL OnBookInfo( BasePacket* pMessage );
	BOOL OnSongInfo( BasePacket* pMessage );
};