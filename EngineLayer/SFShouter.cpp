#include "stdafx.h"
#include "SFShouter.h"
#include <xml/tinyxml.h>
#include <xml/IXMLStreamReader.h>
#include <xml/IXMLStreamWriter.h>
#include <xml/SFXMLStreamWriter.h>
#include <xml/SFXMLStreamReader.h>

SFShouter::SFShouter(void)
{
}


SFShouter::~SFShouter(void)
{
}

BOOL SFShouter::Initialize()
{
	m_ShouterInfo.Clear();

	return TRUE;
}

BOOL SFShouter::Read(const WCHAR* pFileName)
{
	IXMLStreamReader* pReader = SFXMLStreamReader::CreateXmlStreamReader(pFileName);

	if(NULL == pReader)
		return FALSE;

	pReader->Read(pFileName, *this);

	delete pReader;

	return TRUE;
}

BOOL SFShouter::Write(const WCHAR* pFileName)
{
	IXMLStreamWriter* pWriter = SFXMLStreamWriter::CreateXmlStreamWriter(pFileName);
	if(NULL == pWriter)
		return FALSE;

	pWriter->Write(pFileName, *this);

	delete pWriter;

	return TRUE;
}

void SFShouter::Serialize(IXMLStreamWriter* pOut) const
{
	IXMLStreamWriter& out = *pOut;
	out.Begin(L"Struct");
	out.Write(L"ShouterTitle", m_ShouterInfo.ShouterTitle);
	out.Write(L"StartTime", m_ShouterInfo.StartTime);
	out.Write(L"RepeatCount", m_ShouterInfo.RepeatCount);
	out.Write(L"MessageInterval", m_ShouterInfo.MessageInterval);
	out.Write(L"SentenceInterval", m_ShouterInfo.SentenceInterval);
	out.Write(L"SentenceList", m_ShouterInfo.SentenceList);
}

void SFShouter::Deserialize(IXMLStreamReader* in) {

	(*in).Begin(L"Struct");
	(*in).Read(L"ShouterTitle", m_ShouterInfo.ShouterTitle);
	(*in).Read(L"StartTime", m_ShouterInfo.StartTime);
	(*in).Read(L"RepeatCount", m_ShouterInfo.RepeatCount);
	(*in).Read(L"MessageInterval", m_ShouterInfo.MessageInterval);
	(*in).Read(L"SentenceInterval", m_ShouterInfo.SentenceInterval);
	(*in).Read(L"SentenceList", m_ShouterInfo.SentenceList);
}
