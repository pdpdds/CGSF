#pragma once

class ChatUser
{
public:
	ChatUser(void);
	virtual ~ChatUser(void);

	void SetSerial(int Serial){m_Serial = Serial;}

private:
	int m_Serial;
};
