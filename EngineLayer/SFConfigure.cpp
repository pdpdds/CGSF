#include "stdafx.h"
#include "SFConfigure.h"
#include "SFXMLStreamWriter.h"

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
	IXMLStreamReader* pReader = SFXMLStreamReader::CreateXmlStreamReader(pFileName);

	if(NULL == pReader)
		return FALSE;

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
