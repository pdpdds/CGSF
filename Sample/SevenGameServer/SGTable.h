// SGTable.h: interface for the SGTable class.
//
//////////////////////////////////////////////////////////////////////

#pragma once

class SGTable  
{
public:
	SGTable();
	virtual ~SGTable();	
	EdgeCard CheckBoundary(int iCardType);
	void InitializeTable();
	void UpdateTableState(int iCardNum, int iCardType);
	void SetSevenCard();

	void SetSpadeCard(int iCard, int index){SpadeCardArray[index] = iCard;};
	void SetHeartCard(int iCard, int index){HeartCardArray[index] = iCard;};
	void SetDiamondCard(int iCard, int index){DiamondCardArray[index] = iCard;};
	void SetCloverCard(int iCard, int index){CloverCardArray[index] = iCard;};
	
	int * GetSpadeTableArray(){return SpadeCardArray;};
	int * GetHeartTableArray(){return HeartCardArray;};
	int * GetDiamondTableArray(){return DiamondCardArray;};		
	int * GetCloverTableArray(){return CloverCardArray;};

private:
	int SpadeCardArray[MAX_CARD_NUM+1];
	int HeartCardArray[MAX_CARD_NUM+1];
	int DiamondCardArray[MAX_CARD_NUM+1];
	int CloverCardArray[MAX_CARD_NUM+1];
};