#include "stdafx.h"
#include "SFShouter.h"
#include "SFXMLStreamWriter.h"

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
