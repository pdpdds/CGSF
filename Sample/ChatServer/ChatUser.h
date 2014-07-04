#pragma once

class ChatUser
{
public:
	ChatUser(void);
	virtual ~ChatUser(void);

	void SetSerial(int serial){m_serial = serial;}
	
	void SetName(int serial);
	std::string& GetName(){ return m_szName; }

private:
	int m_serial; 
	std::string m_szName;
};