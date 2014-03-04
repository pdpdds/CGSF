#pragma once

class CPictureBox;

class CPictureBoxMgr
{
	typedef std::map<int, CPictureBox*> mapPictureBox;
public:
	CPictureBoxMgr(void);
	virtual ~CPictureBoxMgr(void);

	BOOL OnRender(float fElapsedTime);
	BOOL OnResetDevice();
	BOOL AddPictureBox(ComponentInfo* pinfo);
	BOOL AddPictureBox(int iIdentifer, CPictureBox* pPictureBox);
	BOOL ChangeNewPictureBox(int iIdentifier, TCHAR* szNewPictureName);
	CPictureBox* GetPictureBox(int iIdentifer);

protected:

private:
	mapPictureBox m_mapPictureBox;
};
