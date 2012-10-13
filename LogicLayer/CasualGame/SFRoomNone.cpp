#include "StdAfx.h"
#include "SFRoomNone.h"

SFRoomNone::SFRoomNone( SFRoom* pOwner, eRoomState State )
: SFRoomState(pOwner, State)
{

}

SFRoomNone::~SFRoomNone(void)
{
}
