#include "StdAfx.h"
#include "SFExcel.h"
#include "objbase.h"
#include <iostream>
#include <sstream>
#include <fstream>

SFExcel::SFExcel(void)
{
}

SFExcel::~SFExcel(void)
{
}

BOOL SFExcel::Initialize()
{
	if(FAILED(::CoInitialize(NULL))) 
		return FALSE;

	return TRUE;
}

BOOL SFExcel::Finally()
{
	::CoUninitialize();
		
	return TRUE;
}

BOOL SFExcel::Read( char* szFileName, int SheetIndex )
{
	clock_t t1 = clock();
	_RecordsetPtr RecordSetPtr;

	int iCellCount = 0;
	RecordSetPtr.CreateInstance(__uuidof(Recordset));

	_bstr_t ConnectionStr(MakeConnectionString(szFileName).c_str());

	RecordSetPtr->Open(MakeSelectedSheet(ConnectionStr, SheetIndex).c_str(), ConnectionStr, adOpenStatic, adLockOptimistic, adCmdText);

	while(!RecordSetPtr->adoEOF)
	{
		for(long i = 0; i < RecordSetPtr->Fields->GetCount(); ++i)
		{
			//	if(i > 0) stream << ";";                    
			_variant_t v = RecordSetPtr->Fields->GetItem(i)->Value;
			//if(v.vt == VT_R8)                                           
			//	stream << v.dblVal;
			//	if(v.vt == VT_BSTR)
			//		stream << (char*)(_bstr_t)v.bstrVal;                
			++iCellCount;
		}                                    

		RecordSetPtr->MoveNext();
	}


	clock_t t2 = clock();
	double t = (double)(t2 - t1) / CLOCKS_PER_SEC;    

	return TRUE;
}

std::string SFExcel::MakeConnectionString(std::string filename)
{
	std::stringstream stream;

	std::string Header = "NO";

	if(!filename.empty())       
		if(*--filename.end() == 'x')        
			stream << "Provider=Microsoft.ACE.OLEDB.12.0;Data Source=" << filename << ";Extended Properties=\"Excel 12.0 Xml;HDR=" << Header << "\"";                    
		else        
			stream << "Provider='Microsoft.JET.OLEDB.4.0';Data Source=" << filename << ";Extended Properties=\"Excel 8.0;HDR=" << Header << "\"";

	return stream.str();
}

std::string SFExcel::MakeSelectedSheet(_bstr_t ConnectionString, int sheetIndex)
{
	_ConnectionPtr pCon = NULL;
	_RecordsetPtr pSchema = NULL;

	pCon.CreateInstance(__uuidof(Connection));
	pCon->Open(ConnectionString, "", "", NULL);

	pSchema = pCon->OpenSchema(adSchemaTables);      

	for(int i = 0; i < sheetIndex; ++i) pSchema->MoveNext();

	std::string sheetName = (char*)(_bstr_t)pSchema->Fields->GetItem("TABLE_NAME")->Value.bstrVal;

	std::stringstream stream;
	stream << "SELECT * FROM [" << sheetName << "]";
	return stream.str();
}

void SFExcel::Write(std::string filename)
{   
	clock_t t1 = clock();   
	const int writeRows = 50000;

	std::cout << "writing " << filename;

	DeleteFileA(filename.c_str());

	if(FAILED(::CoInitialize(NULL))) return;

	_ConnectionPtr pCon = NULL;
	_CommandPtr pCmd = NULL;
	_RecordsetPtr pRec = NULL;
        
	_bstr_t connStr(MakeConnectionString(filename).c_str());

	pCon.CreateInstance(__uuidof(Connection));
	pCon->Open(connStr, "", "", NULL);

	pCmd.CreateInstance(__uuidof(Command));
	pCmd->ActiveConnection = pCon;       
	pCmd->CommandText = "CREATE TABLE MySheet(A int, B varchar, C int, D int, E int, F int, G int, H int, I int, J varchar)";        
	pCmd->Execute(NULL, NULL, adCmdText);

	pRec.CreateInstance(__uuidof(Recordset));
	pRec->Open("SELECT * FROM MySheet", _variant_t((IDispatch*)pCon), adOpenKeyset, adLockOptimistic, adCmdText);

	for(int i = 0; i < writeRows; ++i)
	{
		pRec->AddNew();

		char str[11] = {0}; for(int j = 0; j < 10; ++j) str[j] = 'a' + (rand() % 26);

		pRec->Fields->GetItem("A")->Value = _variant_t(i);            
		pRec->Fields->GetItem("B")->Value = _variant_t(str);            
		pRec->Fields->GetItem("C")->Value = _variant_t(i);
		pRec->Fields->GetItem("D")->Value = _variant_t(i);
		pRec->Fields->GetItem("E")->Value = _variant_t(i);
		pRec->Fields->GetItem("F")->Value = _variant_t(i);
		pRec->Fields->GetItem("G")->Value = _variant_t(i);
		pRec->Fields->GetItem("H")->Value = _variant_t(i);
		pRec->Fields->GetItem("I")->Value = _variant_t(i);
		pRec->Fields->GetItem("J")->Value = _variant_t(str);            
	}
	pRec->Update();
	pRec->Close();                

	if(pCon != 0 && pCon->State == adStateOpen) pCon->Close();   

	clock_t t2 = clock();
	double t = (double)(t2 - t1) / CLOCKS_PER_SEC;
	std::wcout << ": " << t << " sec; " << writeRows * 10 / t << " cells/sec" << std::endl;    
}