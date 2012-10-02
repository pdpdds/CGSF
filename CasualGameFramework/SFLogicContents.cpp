#include "StdAfx.h"
#include "SFLogicContents.h"
#include "SFExcel.h"

SFLogicContents::SFLogicContents(void)
{
}

SFLogicContents::~SFLogicContents(void)
{
}

BOOL SFLogicContents::Load()
{
	LoadSpawn();

	return TRUE;
}

BOOL SFLogicContents::LoadSpawn()
{
	/*SFExcel Excel;
	Excel.Initialize();
	Excel.Read("SpawnPoint.xls", 0);
	Excel.Finally();*/

	m_SpawnList.push_back(1);
	m_SpawnList.push_back(2);
	m_SpawnList.push_back(3);
	m_SpawnList.push_back(4);

	return TRUE;
}