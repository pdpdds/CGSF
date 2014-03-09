#pragma once

namespace SevenGame
{
	typedef enum eSevenGamePacketID
	{
		InitCardCount = 10000,
		MyCardInfo,
		TableUpdate,
		CurrentTurn,
		TurnPass,
		CardSubmit,
		Winner,
		UserDie,
		PlayerID,
	};
};
