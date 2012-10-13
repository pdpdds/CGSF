#pragma once
#import "C:\Program Files\Common Files\System\ADO\msado15.dll" no_namespace rename("EOF", "adoEOF") rename("BOF", "adoBOF")

class SFExcel
{
public:
	SFExcel(void);
	virtual ~SFExcel(void);

	BOOL Initialize();
	BOOL Finally();

	BOOL Read( char* szFileName, int SheetIndex );
	std::string MakeConnectionString(std::string filename);
	std::string MakeSelectedSheet(_bstr_t ConnectionString, int sheetIndex);
	void Write(std::string filename);
	

protected:

private:
};