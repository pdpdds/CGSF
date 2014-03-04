#include "StdAfx.h"
#include "PictureBoxMgr.h"
#include "PictureBox.h"

CPictureBoxMgr::CPictureBoxMgr(void)
{
}

CPictureBoxMgr::~CPictureBoxMgr(void)
{
}

BOOL CPictureBoxMgr::AddPictureBox(ComponentInfo* pinfo)
{	
	mapPictureBox::iterator iter = m_mapPictureBox.find(pinfo->iIdentifier);

	if(iter != m_mapPictureBox.end())
		return FALSE;

	CPictureBox* pPictureBox = new CPictureBox();
	pPictureBox->m_Info = *pinfo;

	m_mapPictureBox.insert(mapPictureBox::value_type(pinfo->iIdentifier, pPictureBox));

	return TRUE;
}

BOOL CPictureBoxMgr::AddPictureBox( int iIdentifer, CPictureBox* pPictureBox )
{
	mapPictureBox::iterator iter = m_mapPictureBox.find(iIdentifer);

	if(iter != m_mapPictureBox.end())
		return FALSE;

	m_mapPictureBox.insert(mapPictureBox::value_type(iIdentifer, pPictureBox));

	return TRUE;
}

BOOL CPictureBoxMgr::OnRender(float fElapsedTime)
{
	mapPictureBox::iterator iter = m_mapPictureBox.begin();

	for(iter; iter != m_mapPictureBox.end(); iter++)
	{
		CPictureBox* pPictureBox = (CPictureBox*)(iter->second);
		pPictureBox->OnRender(fElapsedTime);
	}

	return TRUE;
}

BOOL CPictureBoxMgr::OnResetDevice()
{
	mapPictureBox::iterator iter = m_mapPictureBox.begin();

	for(iter; iter != m_mapPictureBox.end(); iter++)
	{
		CPictureBox* pPictureBox = (CPictureBox*)(iter->second);
		pPictureBox->OnResetDevice();
	}

	return TRUE;
}

BOOL CPictureBoxMgr::ChangeNewPictureBox(int iIdentifier, TCHAR* szNewPictureName)
{
	mapPictureBox::iterator iter = m_mapPictureBox.find(iIdentifier);

	if(iter == m_mapPictureBox.end())
		return FALSE;

	CPictureBox* pPictureBox = (CPictureBox*)(iter->second);
	_tcscpy_s(pPictureBox->m_Info.szGUICaption, szNewPictureName);

	pPictureBox->OnResetDevice();
	
	return TRUE;
}

CPictureBox* CPictureBoxMgr::GetPictureBox(int iIdentifer)
{	
	mapPictureBox::iterator iter = m_mapPictureBox.find(iIdentifer);

	if(iter == m_mapPictureBox.end())
		return NULL;

	return iter->second;
}