#include "stdafx.h"
#include "SFConfigure.h"
#include <xml/tinyxml.h>
#include <xml/IXMLStreamReader.h>
#include <xml/IXMLStreamWriter.h>
#include <xml/SFXMLStreamWriter.h>
#include <xml/SFXMLStreamReader.h>
#include "SFUtil.h"

SFConfigure::SFConfigure(void)
{
}


SFConfigure::~SFConfigure(void)
{
}

BOOL SFConfigure::Initialize()
{
	m_ConfigureInfo.Clear();

	return TRUE;
}

BOOL SFConfigure::Read(const WCHAR* pFileName)
{
	// 실행파일 디렉토리를 현재 디렉토리로 설정하고, 로그를 남긴다.
	WCHAR szFilePath[MAX_PATH] = { 0, };
	GetModuleFileName(NULL, szFilePath, MAX_PATH);

	WCHAR* path = SFUtil::ExtractPathInfo(szFilePath, SFUtil::PATH_DIR);
	SetCurrentDirectory(path);

	LOG(INFO) << "SetCurrentDirectory. " << path;
	google::FlushLogFiles(google::GLOG_INFO);


	IXMLStreamReader* pReader = SFXMLStreamReader::CreateXmlStreamReader(pFileName);

	if (NULL == pReader || pReader->m_isXmlFileLoadSuccess == false)
	{
		return FALSE;
	}

	pReader->Read(pFileName, *this);

	delete pReader;

	return TRUE;
}

BOOL SFConfigure::Write(const WCHAR* pFileName)
{
	IXMLStreamWriter* pWriter = SFXMLStreamWriter::CreateXmlStreamWriter(pFileName);
	if(NULL == pWriter)
		return FALSE;

	pWriter->Write(pFileName, *this);

	delete pWriter;

	return TRUE;
}

void SFConfigure::Serialize(IXMLStreamWriter* out) const
{
	(*out).Begin(L"Struct");
	(*out).Write(L"EngineName", m_ConfigureInfo.engineName);
	(*out).Write(L"ServerIP", m_ConfigureInfo.serverIP);
	(*out).Write(L"ServerPort", m_ConfigureInfo.serverPort);
	(*out).Write(L"MaxAccept", m_ConfigureInfo.maxAccept);
}

void SFConfigure::Deserialize(IXMLStreamReader* in) {
	(*in).Begin(L"Struct");
	(*in).Read(L"EngineName", m_ConfigureInfo.engineName);
	(*in).Read(L"ServerIP", m_ConfigureInfo.serverIP);
	(*in).Read(L"ServerPort", m_ConfigureInfo.serverPort);
	(*in).Read(L"MaxAccept", m_ConfigureInfo.maxAccept);
}
