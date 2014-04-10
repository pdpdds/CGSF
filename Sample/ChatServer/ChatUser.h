#pragma once

class ChatUser
{
public:
	ChatUser(void);
	virtual ~ChatUser(void);

	void SetSerial(int Serial){m_Serial = Serial;}
	
	void SetName(int serial);
	std::string& GetName(){ return m_szName; }

private:
	int m_Serial; 
	std::string m_szName;
};