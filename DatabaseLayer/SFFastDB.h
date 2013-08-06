#pragma once
#include "SFDatabase.h"

class SFFastDB //: public SFDatabase
{
public:
	SFFastDB(void);
	~SFFastDB(void);

	BOOL Initialize(TCHAR* szDB);
	BOOL Execute(char* szQuery);
};

