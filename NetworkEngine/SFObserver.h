#pragma once

class SFObject;
class SFMessage;

class SFObserver
{
public:
	SFObserver(void);
	virtual ~SFObserver(void);

	virtual BOOL OnMessage(SFObject* pSender, SFMessage* pMessage);
};
