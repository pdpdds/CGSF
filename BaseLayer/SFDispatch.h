#pragma once
#include <map>

/////////////////////////////////////////////////////////////////////////////
//20120426 템플릿 속성
// TMsgType : 메세지의 타입(BYTE, USHORT, int)
// TMsgHandler 메세지를 처리하는 핸들러 주소
// TMsg 핸들러에 넘어오는 인자
/////////////////////////////////////////////////////////////////////////////
template <typename TMsgType, typename TMsgHandler, typename TMsg>
class SFDispatch
{
	typedef std::map<TMsgType, TMsgHandler> MsgMap;
public:
	SFDispatch(void){}
	virtual ~SFDispatch(void){m_MsgMap.clear();}

	void RegisterMessage(TMsgType Msg, TMsgHandler Method);
	BOOL HandleMessage(TMsgType Msg, TMsg& Message);

protected:

private:	
	MsgMap m_MsgMap; 
};

template <typename TMsgType, typename TMsgHandler, typename TMsg>
void SFDispatch<typename TMsgType, typename TMsgHandler, typename TMsg>::RegisterMessage(TMsgType MsgType, TMsgHandler Method)
{
	m_MsgMap.insert(std::make_pair(MsgType, Method));
}

template <typename TMsgType, typename TMsgHandler, typename TMsg>
BOOL SFDispatch<typename TMsgType, typename TMsgHandler, typename TMsg>::HandleMessage(TMsgType MsgType, TMsg& Message)
{
	MsgMap::iterator iter = m_MsgMap.find(MsgType);

	if(iter != m_MsgMap.end())
	{
		(*iter).second(Message);
	}
	else
	{
		return FALSE;
	}

	return TRUE;
}

/////////////////////////////////////////////////////////////////////////////
//20120823 템플릿 속성
// TMsgType : 메세지의 타입(BYTE, USHORT, int)
// TMsgHandler 메세지를 처리하는 핸들러 주소
// TMsg 핸들러에 넘어오는 첫번째 인자
// TMsg2 핸들러에 넘어오는 두번째 인자
/////////////////////////////////////////////////////////////////////////////
//타입리스트를 사용하면 하나로 만들 수 있을 것 같기도 하지만....

template <typename TMsgType, typename TMsgHandler, typename TMsg, typename TMsg2>
class SFDispatch_2
{
	typedef std::map<TMsgType, TMsgHandler> MsgMap;
public:
	SFDispatch_2(void){}
	virtual ~SFDispatch_2(void){m_MsgMap.clear();}

	void RegisterMessage(TMsgType Msg, TMsgHandler Method);
	BOOL HandleMessage(TMsgType Msg, TMsg& Message, TMsg2& Message2);

protected:

private:	
	MsgMap m_MsgMap; 
};

template <typename TMsgType, typename TMsgHandler, typename TMsg, typename TMsg2>
void SFDispatch_2<typename TMsgType, typename TMsgHandler, typename TMsg, typename TMsg2>::RegisterMessage(TMsgType MsgType, TMsgHandler Method)
{
	m_MsgMap.insert(std::make_pair(MsgType, Method));
}

template <typename TMsgType, typename TMsgHandler, typename TMsg, typename TMsg2>
BOOL SFDispatch_2<typename TMsgType, typename TMsgHandler, typename TMsg, typename TMsg2>::HandleMessage(TMsgType MsgType, TMsg& Message, TMsg2& Message2)
{
	MsgMap::iterator iter = m_MsgMap.find(MsgType);

	if(iter != m_MsgMap.end())
	{
		(*iter).second(Message, Message2);
	}
	else
	{
		return FALSE;
	}

	return TRUE;
}