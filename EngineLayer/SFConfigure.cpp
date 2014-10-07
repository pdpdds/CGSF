#include "stdafx.h"
#include "SFConfigure.h"
#include <XML/SFXMLStreamWriter.h>
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
