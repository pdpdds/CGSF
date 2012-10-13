#include "stdafx.h"
#include <stdlib.h>

#include "StringConversion.h"

namespace StringConversion 
{ 

	unsigned int Hash(const WCHAR* str)
	{
		//wchar_t* p=const_cast<wchar_t*>(str); <--
		//replaced this, we shouldnt be doing const_cast's for no reason @juhnu
		const WCHAR* p=str;
		unsigned int hashcode=0;
		wchar_t c=0;
		while(*p)
		{
			c=*p; hashcode=(31*hashcode+(c%31))%64000000; ++p;
		}
		return hashcode;
	}
	unsigned int Hash(const CHAR* str)
	{
		//wchar_t* p=const_cast<wchar_t*>(str); <--
		//replaced this, we shouldnt be doing const_cast's for no reason @juhnu
		const CHAR* p=str;
		unsigned int hashcode=0;
		wchar_t c=0;
		while(*p)
		{
			c=*p; 
			hashcode=(31*hashcode+(c%31))%64000000; ++p;
		}
		return hashcode;
	}

	template<> std::string To(const wchar_t* wstr) {
		//int iWstrSize = (int)wcslen(wstr);
		int iSize = WideCharToMultiByte(CP_ACP,0,wstr,-1,NULL,0,NULL,NULL);
		std::string str(iSize -1, 0);
		::WideCharToMultiByte(CP_ACP, 0, wstr, -1, &str[0], iSize, NULL, NULL);
		return str;
	}

	template<> std::wstring To(const wchar_t* wstr) {
		//int iWstrSize = (int)wcslen(wstr);
		std::wstring tstr(wstr);
		return tstr;
	}

	template <> std::string To(const wstringIteratorPair& StrPair) {
		//int iWstrSize = (int)(StrPair.second - StrPair.first);
		int iSize = WideCharToMultiByte(CP_ACP,0,&(*StrPair.first), -1 ,NULL,0,NULL,NULL);
		//int iSize = (int)(StrPair.second - StrPair.first);
		std::string str(iSize -1, 0);
		::WideCharToMultiByte(CP_ACP, 0, &(*StrPair.first), -1, &str[0], iSize, NULL, NULL);
		return str;
	}

	template<> std::string To(const char* str) {
		std::string tstr(str);
		return tstr;
	}

	template<> std::wstring To(const char* str) {
		//int iStrSize = (int)strlen(str);
		int iSize = MultiByteToWideChar(CP_ACP,0,str,-1,NULL,0);
		//int iSize = (int)strlen(str);
		std::wstring wstr(iSize-1, 0);
		::MultiByteToWideChar(CP_ACP, 0, str, -1, &wstr[0], iSize);
		return wstr;
	}

	template <> std::wstring To(const stringIteratorPair& StrPair) {
		//int iStrSize =(int)(StrPair.second - StrPair.first);
		int iSize = MultiByteToWideChar(CP_ACP,0,&(*StrPair.first),-1,NULL,0);
		//int iSize = (int)(StrPair.second - StrPair.first);
		std::wstring wstr(iSize-1, 0);
		::MultiByteToWideChar(CP_ACP, 0, &(*StrPair.first), -1, &wstr[0], iSize);
		return wstr;
	}

	template <> std::string To(int value) {
		char szBuffer[12];					//mamximum 10 + 1 sign + 1 null.
		_itoa(value, szBuffer, 10);
		return std::string(szBuffer);
	}

	template <> std::wstring To(int value) {
		wchar_t wszBuffer[12];
		_itow(value, wszBuffer, 10);
		return std::wstring(wszBuffer);
	}

	template <> std::string To(short value) {
		char szBuffer[12];
		_itoa((int)value, szBuffer, 10);
		return std::string(szBuffer);
	}

	template <> std::wstring To(short value) {
		wchar_t wszBuffer[12];
		_itow((int)value, wszBuffer, 10);
		return std::wstring(wszBuffer);
	}

	template <> std::string To(unsigned short value) {
		char szBuffer[12];
		_itoa((int)value, szBuffer, 10);
		return std::string(szBuffer);
	}

	template <> std::wstring To(unsigned short value) {
		wchar_t wszBuffer[12];
		_itow((int)value, wszBuffer, 10);
		return std::wstring(wszBuffer);
	}

	template <> std::string To(unsigned int value) {
		char szBuffer[24];
		_ui64toa((unsigned long)value, szBuffer, 10);
		return std::string(szBuffer);
	}

	template <> std::wstring To(unsigned int value) {
		wchar_t wszBuffer[24];
		_ui64tow((unsigned long)value, wszBuffer, 10);
		return std::wstring(wszBuffer);
	}

	template <> std::string To(__int64 value) {
		char szBuffer[24];
		_i64toa((unsigned long)value, szBuffer, 10);
		return std::string(szBuffer);
	}

	template <> std::wstring To(__int64 value) {
		wchar_t wszBuffer[24];
		_ui64tow((unsigned long)value, wszBuffer, 10);
		return std::wstring(wszBuffer);
	}

	template <> std::string To(unsigned __int64 value) {
		char szBuffer[24];
		_ui64toa((unsigned long)value, szBuffer, 10);
		return std::string(szBuffer);
	}

	template <> std::wstring To(unsigned __int64 value) {
		wchar_t wszBuffer[24];
		_ui64tow((unsigned long)value, wszBuffer, 10);
		return std::wstring(wszBuffer);
	}

	template <> std::string To(long value) {
		char szBuffer[48];
		_i64toa(value, szBuffer, 10);
		return std::string(szBuffer);
	}

	template <> std::wstring To(long value) {
		wchar_t wszBuffer[48];
		_i64tow(value, wszBuffer, 10);
		return std::wstring(wszBuffer);
	}

	template <> std::string To(unsigned long value) {
		char szBuffer[100];
		_ui64toa(value, szBuffer, 10);
		return std::string(szBuffer);
	}

	template <> std::wstring To(unsigned long value) {
		wchar_t wszBuffer[100];
		_ui64tow(value, wszBuffer, 10);
		return std::wstring(wszBuffer);
	}

	template <> std::string To(float value) {
		char szBuffer[100];
		sprintf(szBuffer, "%f", value);
		return std::string(szBuffer);
	}

	template <> std::wstring To(float value) {
		wchar_t wszBuffer[100];
		swprintf(wszBuffer, L"%f", value);
		return std::wstring(wszBuffer);
	}

	template <> std::string To(double value) {
		char szBuffer[100];
		sprintf(szBuffer, "%f", value);
		return std::string(szBuffer);
	}

	template <> std::wstring To(double value) {
		wchar_t wszBuffer[100];
		swprintf(wszBuffer, L"%f", value);
		return std::wstring(wszBuffer);
	}

	template <> double	To(const char* str)								{ return atof(str); }
	template <> double	To(const stringIteratorPair& StrPair)			{ return To<double>(std::string(StrPair.first, StrPair.second)); }

	template <> float	To(const char* str)								{ return (float)To<double>(str); }
	template <> float	To(const stringIteratorPair& StrPair)			{ return (float)To<double>(StrPair); }

	template <> int		To(const char* str)								{ return atoi(str); }
	template <> int		To(const stringIteratorPair& StrPair)			{ return To<int>(std::string(StrPair.first, StrPair.second)); }

	template <> double	To(const wchar_t* wstr)							{ return _wtof(wstr); }
	template <> double	To(const wstringIteratorPair& StrPair)			{ return To<double>(std::wstring(StrPair.first, StrPair.second)); }

	template <> float	To(const wchar_t* wstr)							{ return (float)To<double>(wstr); }
	template <> float	To(const wstringIteratorPair& StrPair)			{ return (float)To<double>(StrPair); }

	template <> int		To(const wchar_t* str)							{ return _wtoi(str); }
	template <> int		To(const wstringIteratorPair& StrPair)			{ return To<int>(std::wstring(StrPair.first, StrPair.second)); }




/*
	template <> short	To(const wchar_t* str)							{ return (short)_wtoi(str); }
	template <> short	To(const wstringIteratorPair& StrPair)			{ return To<short>(std::wstring(StrPair.first, StrPair.second)); }

	template <> long	To(const wchar_t* str)							{ return _wtol(str); }
	template <> long	To(const wstringIteratorPair& StrPair)			{ return To<long>(std::wstring(StrPair.first, StrPair.second)); }

	template <> unsigned int To(const wchar_t* str)						{ return (unsigned int)_wtoi(str); }
	template <> unsigned int To(const wstringIteratorPair& StrPair)		{ return To<unsigned int>(std::wstring(StrPair.first, StrPair.second)); }

	template <> unsigned short To(const wchar_t* str)					{ return (unsigned short)_wtoi(str); }
	template <> unsigned short To(const wstringIteratorPair& StrPair)	{ return To<unsigned short>(std::wstring(StrPair.first, StrPair.second)); }

	template <> unsigned long	To(const wchar_t* str)					{ return (unsigned long)_wtol(str); }
	template <> unsigned long	To(const wstringIteratorPair& StrPair)	{ return To<unsigned long>(std::wstring(StrPair.first, StrPair.second)); }
*/
	
	namespace Private {
		template<typename T>
		class Tokens {};

		template<>
		class Tokens<char> {
		public:
			static const char		Minus				= '-';
			static const char		LeftParenthesis		= '(';
			static const char		RightParenthesis	= ')';
			static const char		Comma				= ',';
			static const char		Point				= '.';
			static const char		Null				= '\0';
			static const char		Zero				= '0';
			static const char		Nine				= '9';
			static const char		Space				= ' ';
			static const char		Tab					= '\t';
			static const char		LineFeed			= '\n';
			static const char		CarriageReturn		= '\r';
		};

		template<>
		class Tokens<wchar_t> {
		public:
			static const wchar_t	Minus				= L'-';
			static const wchar_t	LeftParenthesis		= L'(';
			static const wchar_t	RightParenthesis	= L')';
			static const wchar_t	Comma				= L',';
			static const wchar_t	Point				= L'.';
			static const wchar_t	Null				= L'\0';
			static const wchar_t	Zero				= L'0';
			static const wchar_t	Nine				= L'9';
			static const wchar_t	Space				= L' ';
			static const wchar_t	Tab					= L'\t';
			static const wchar_t	LineFeed			= L'\n';
			static const wchar_t	CarriageReturn		= L'\r';
		};

		template<typename T>
		BOOL GetToken(T** pStr, T tToken) {
			if( **pStr == tToken ) { (*pStr)++; return TRUE; }
			return FALSE;
		}

		template<typename T>
		BOOL IsNumber(T tChar) {
			return (Tokens<T>::Zero <= tChar && tChar <= Tokens<T>::Nine) ? TRUE : FALSE;
		}

		template<typename T>
		BOOL IsWhiteSpace(T tChar) {
			return (tChar == Tokens<T>::Space || tChar == Tokens<T>::Tab ||
				tChar == Tokens<T>::LineFeed || tChar == Tokens<T>::CarriageReturn) ? TRUE : FALSE;
		}

		template<typename T>
		void SkipWhitespace(T** pStr) {
			while( IsWhiteSpace(**pStr) )	(*pStr)++;
		}

		template<typename T>
		void SkipNumbers(T** pStr) {
			while( IsNumber(**pStr) )		(*pStr)++;
		}

		template<typename T>
		void SkipFloat(T** pStr) {
			//sign
			GetToken(pStr, Tokens<T>::Minus);
			//number before point
			SkipNumbers(pStr);
			//point
			GetToken(pStr, Tokens<T>::Point);
			//number after point
			SkipNumbers(pStr);
		}
	}
}