#include "stdafx.h"
#include "EchoCallback.h"
#include "SFJsonPacket.h"
#include <iostream>
#include "json\elements.h"

extern bool g_bExit;

EchoCallback::EchoCallback(void)
{
}

EchoCallback::~EchoCallback(void)
{
}

bool EchoCallback::HandleNetworkMessage(BasePacket* pPacket)
{
	SFJsonPacket* pJsonPacket = (SFJsonPacket*)pPacket;

	std::cout << "Received : " << pJsonPacket->GetData().GetValue<tstring>("ECHO") << std::endl;

//json array read method 1
	JsonArrayNode arrayNode = pJsonPacket->GetData().GetValue<JsonArrayNode>("ArrayData");
	std::cout << "ArrayData : " << arrayNode.ToString() << std::endl;

	json::Array* pArray = arrayNode.GetRawNode();

	json::Array::iterator iter = pArray->Begin();
	for (iter; iter != pArray->End(); iter++)
	{
		json::UnknownElement& element = *iter;
		json::Number e = element;
		std::cout << "num : " << e << std::endl;
	}

//json array read method 2
	/*
	for (int i = 0; i < arrayNode.Size(); i++)
	{
		JsonObjectNode node = arrayNode.GetObjectNode(i);
		int e = node.GetValue<int>("e");
		std::cout << "num : " << e << std::endl;
	}*/

	return true;
}