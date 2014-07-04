#include "StdAfx.h"
#include "ChatUser.h"

ChatUser::ChatUser(void)
: m_serial(-1)
{
}

ChatUser::~ChatUser(void)
{
}

void ChatUser::SetName(int serial)
{
	m_szName = "test";
	m_szName += std::to_string(serial);
}
