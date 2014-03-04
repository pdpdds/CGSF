#pragma once

namespace SevenGame
{
	typedef enum eSevenGamePacketID
	{
		InitCardCount = 5000,
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
