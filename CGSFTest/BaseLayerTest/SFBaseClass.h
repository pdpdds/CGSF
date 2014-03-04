#pragma once

class SFBaseClass
{
public:
	SFBaseClass(void);
	virtual ~SFBaseClass(void);	

	virtual int ProcessTask() {return 0;};

protected:

private:
	int m_iBaseVar;	
};
