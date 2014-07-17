#include "stdafx.h"
#include "SFServerConnectionManager.h"
#include "SFEngine.h"
#include "XML/StringConversion.h"
#include "XML/Markup.h"

SFServerConnectionManager::SFServerConnectionManager()
{
}


SFServerConnectionManager::~SFServerConnectionManager()
{
}

/*
bool SFServerConnectionManager::Save()
{
	CMarkup xml;

	xml.AddElem(L"SERVERLIST");
	xml.AddChildElem(L"SERVER");
	xml.IntoElem();
	xml.AddChildElem(L"IP", L"127.0.0.1");
	xml.AddChildElem(L"PORT", 10000);
	xml.AddChildElem(L"IDENTIFER", 10000);
	xml.AddChildElem(L"DESC", L"GAME DATABASE SERVER");
	xml.OutOfElem();

	xml.AddChildElem(L"SERVER");
	xml.IntoElem();
	xml.AddChildElem(L"IP", L"127.0.0.1");
	xml.AddChildElem(L"PORT", 10001);
	xml.AddChildElem(L"IDENTIFER", 9000);
	xml.AddChildElem(L"DESC", L"AUTH SERVER");
	xml.OutOfElem();

	std::wstring csXML = xml.GetDoc();
	xml.Save(L"ServerConnection.xml");

	wprintf(L"%s", csXML.c_str());

	return true;
}*/

bool SFServerConnectionManager::InitServerList(WCHAR* szFileName)
{
	CMarkup xml;
	xml.Load(L"ServerConnection.xml");

	while (xml.FindChildElem(L"SERVER"))
	{
		_ServerInfo serverInfo;
		xml.IntoElem();

		xml.FindChildElem(L"IP");
		serverInfo.szIP = xml.GetChildData();

		xml.FindChildElem(L"PORT");
		serverInfo.port = _ttoi(xml.GetChildData().c_str());

		xml.FindChildElem(L"IDENTIFER");
		serverInfo.identifer = _ttoi(xml.GetChildData().c_str());

		xml.FindChildElem(L"DESC");
		serverInfo.szDesc = xml.GetChildData();

		xml.OutOfElem();

		SFServerBridge* pServer = new SFServerBridge(serverInfo);

		m_mapServerInfo.insert(std::make_pair(serverInfo.identifer, pServer));
	}

	return true;
}

bool SFServerConnectionManager::LoadConnectionServerList(WCHAR* szFileName)
{
	if (InitServerList(szFileName) == false)
		return false;

	for (auto& iter : m_mapServerInfo)
	{
		SFServerBridge* pServer = iter.second;		
		_ServerInfo& info = pServer->GetServerInfo();

		int serial = -1;
		serial = SFEngine::GetInstance()->AddConnector((char*)StringConversion::ToASCII(info.szIP.c_str()).c_str(), info.port);

		if (serial >= 0)
		{
			m_mapConnectedServer.insert(std::make_pair(serial, pServer));
		}
		else
		{
			m_listDisonnectedServer.push_back(info);
		}
	}

	return true;
}
