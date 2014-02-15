#pragma once

namespace SFUtil
{
	typedef enum eExtractPathInfoType
	{

		PATH_DRIVE = 0,
		PATH_DIR,
		PATH_FILENAME,
		PATH_EXTENSION,
	};

	DWORD GetPESize(LPCTSTR filename); //PE(Portable Executable) 파일의 사이즈를 얻는다.
	INT	GetProgramName( LPWSTR buf, INT len ); //실행파일의 이름을 얻는다.
	TCHAR* ExtractPathInfo(TCHAR* szPath, eExtractPathInfoType InfoType); //파일 경로로부터 필요 정보를 분해한다.
	TCHAR* BuildPath(TCHAR* szDrive, TCHAR* szDirectory, TCHAR* szFileName, TCHAR* szExtension); //해당 파일과 관련된 완전한 경로를 구축한다.
	void ShowFileProperties(HWND hwnd, LPTSTR lpszFile); //파일의 정보를 얻는다.
	WCHAR* CreateGuid(int& iGuidSize); //GUID 생성

#ifdef _DEBUG
	char* CustomFormatA(LPCSTR msg, ...);
#else
	wchar_t* CustomFormatW(LPCWSTR msg, ...);
#endif

	DWORD CountSetBits(ULONG_PTR bitMask); // Helper function to count set bits in the processor mask.
	BOOL ConvertBytesToMB(DWORD dwBytes, DWORD& dwMB, DWORD& dwKB);  //바이트수를 메가바이트, 킬로바이트로 변환
	BOOL Is64BitOS(); //현재 운영체제가 64비트 또는 32비트 운영체제 상에서 실행되고 있는지를 확인시켜 준다.
	void GetScreenResolution(RECT& DesktopRect); //스크린 해상도 얻기
	void CheckOSVersion(DWORD& dwVersion, DWORD dwMajorVersion, DWORD dwMinorVersion, DWORD& dwBuild); //OS Version 얻기
	BOOL CreateSharedFolder(TCHAR* szNetworkName, TCHAR* szComment, TCHAR* szDirPath); //공유 폴더 생성
	float GetWindowAssetPoint(); //로컬 컴퓨터 성능 지수 측정.. 비스타 이상에서만 제대로 동작

	void SetCurDirToModuleDir();

	std::string GetPluginDirectory ( void );

};