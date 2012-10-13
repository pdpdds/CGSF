/** 
 *  @file		NFChecksum.h
 *  @brief		NFChecksum 생성 클래스
 *  @remarks	
 *  @author		강동명(edith2580@gmail.com)
 *  @date		2009-04-02
 */


#pragma once

/** 
*  @class        NFChecksum
*  @brief        체크섬을 생성합니다.
*  @remarks    DWORD dwCrc = 0;													\r\n
*				DWORD dwTemp;														\r\n
*				sprintf(strScriptPath, "data1.z");									\r\n
*				if (NFChecksum::FromFileWin32(strScriptPath, dwTemp) != NO_ERROR)	\r\n
*				{																	\r\n
*					return FALSE;													\r\n
*				}																	\r\n
*				dwCrc = dwCrc^dwTemp;												\r\n
*                
*  @par          
*  @author  Edith
*  @date    2009-04-03
*/
class SFChecksum
{
public:
	/**
	* @brief		메모리에서 체크섬을 구한다
	* @param buffer	체크섬을 구할 정보
	* @param size		buffer의 사이즈
	* @param &dwCrc32	CRC의 값
	* @return			성공여부 성공시 NO_ERROR 리턴
	*/
	static DWORD FromBuffer(BYTE* buffer, size_t size, DWORD &dwCrc32);

	/**
	* @brief		Win32 함수를 이용해 체크섬을 구한다.
	* @param filename	파일명
	* @param &dwCrc32  CRC의 값
	* @return			성공여부 성공시 NO_ERROR 리턴
	*/
	static DWORD FromFileWin32(LPCWSTR filename, DWORD &dwCrc32);

	/**
	* @brief		Filemap 함수를 이용해 체크섬을 구한다.
	* @param filename	파일명
	* @param &dwCrc32  CRC의 값
	* @return			성공여부 성공시 NO_ERROR 리턴
	*/

	static DWORD FromFileFilemap(LPCWSTR filename, DWORD &dwCrc32);

	/**
	* @brief		Assembly 를 이용해 체크섬을 구한다.
	* @param filename	파일명
	* @param &dwCrc32  CRC의 값
	* @return			성공여부 성공시 NO_ERROR 리턴
	*/
	static DWORD FromFileAssembly(LPCWSTR filename, DWORD &dwCrc32);
};
