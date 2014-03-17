#pragma once
#include "IGameMain.h"

class BasePacket;
class SGManager;

class SevenGameMain : public IGameMain
{
public:
	SevenGameMain(void);
	virtual ~SevenGameMain(void);

	virtual bool Initialize() override;
    virtual bool Finally() override;
	virtual bool Notify(BasePacket* pPacket) override;
	bool ProcessInput(UINT uMsg, WPARAM wParam, LPARAM lParam) override;
	virtual void OnRender(float fElapsedTime) override;

	SGManager* GetSevenGameManager(){return m_SevenGameManger;}

	BOOL GetPassButtonOverapped(){return m_bPassButtonOverapped;}

	void InitializeTable(BasePacket* pPacket);
	void SetMyCard(BasePacket* pPacket);
	void SetPlayerID(BasePacket* pPacket);
	void UpdateTable(BasePacket* pPacket);
	void MakeDisplayOrder();
	void SetCurrentTurn(BasePacket* pPacket);
	
	void TurnPass(BasePacket* pPacket);
	void CardSubmit(BasePacket* pPacket);
	void UserDie(BasePacket* pPacket);
	void Winner(BasePacket* pPacket);

private:
	SGManager *m_SevenGameManger;
	BOOL m_bSecondGame;

	// mouse button presses
	INT			m_iMouseButtons;

	// current location of Mouse
	INT			m_iMouseX;
	INT			m_iMouseY;

	BOOL m_bPassButtonOverapped;
};