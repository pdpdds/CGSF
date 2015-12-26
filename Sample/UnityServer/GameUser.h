#pragma once

class GameUser
{
public:
	GameUser(void);
	virtual ~GameUser(void);

	void SetSerial(int serial){m_serial = serial;}
	
	void SetName(int serial);
	std::string& GetName(){ return m_szName; }

private:
	int m_serial; 
	std::string m_szName;
};