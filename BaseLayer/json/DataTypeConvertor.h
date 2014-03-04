#ifndef __datatypeconverter_h__
#define __datatypeconverter_h__

#pragma once

#include <stdlib.h>
#include <string>

typedef std::basic_string<char> tstring;

namespace Datatype
{

	//////////////////////////////////////////////////////////////////////////
	template<typename T>
	inline void ToOwnValue(const char* text, T& numeric) 
	{}

	template<>
	inline void ToOwnValue(const char* text, tstring& numeric) 
	{ 
		numeric = text; 
	}

	template<>
	inline void ToOwnValue(const char* text, int& numeric) 
	{ 
		numeric = atoi(text); 
	}

	template<>
	inline void ToOwnValue(const char* text, char& numeric) 
	{ 
		numeric = atoi(text); 
	}

	template<>
	inline void ToOwnValue(const char* text, unsigned char& numeric) 
	{ 
		numeric = atoi(text);	
	}

	template<>
	inline void ToOwnValue(const char* text, short& numeric) 
	{ 
		numeric = atoi(text); 
	}

	template<>
	inline void ToOwnValue(const char* text, unsigned short& numeric) 
	{ 
		numeric = atoi(text);	
	}

	template<>
	inline void ToOwnValue(const char* text, unsigned int& numeric) 
	{ 
		numeric = strtoul(text, NULL, 10);	
	}

	template<>
	inline void ToOwnValue(const char* text, float& numeric)
	{ 
		numeric = (float)atof(text); 
	}

	template<>
	inline void ToOwnValue(const char* text, double& numeric)
	{ 
		numeric = atof(text); 
	}

	template<>
	inline void ToOwnValue(const char* text, bool& numeric) 
	{ 
		numeric = (strcmp(text, "true")==0);
	}


	//////////////////////////////////////////////////////////////////////////

	template<typename T>
	inline void ToString(char* dstStr, T srcValue, unsigned int bufferLen )
	{}

	template<>
	inline void ToString(char* dstStr, tstring srcValue, unsigned int bufferLen ) 
	{ 
		sprintf_s(dstStr, bufferLen, "%s", srcValue.c_str());
	}

	template<>
	inline void ToString(char* dstStr, char* srcValue, unsigned int bufferLen ) 
	{ 
		sprintf_s(dstStr, bufferLen, "%s", srcValue);
	}

	template<>
	inline void ToString(char* dstStr, const char* srcValue, unsigned int bufferLen ) 
	{ 
		sprintf_s(dstStr, bufferLen, "%s", srcValue);
	}

	template<>
	inline void ToString(char* dstStr, int srcValue, unsigned int bufferLen ) 
	{
		sprintf_s(dstStr, bufferLen, "%d", srcValue);
	}

	template<>
	inline void ToString(char* dstStr, char srcValue, unsigned int bufferLen ) 
	{
		sprintf_s(dstStr, bufferLen, "%d", srcValue);
	}

	template<>
	inline void ToString(char* dstStr, unsigned char srcValue, unsigned int bufferLen ) 
	{
		sprintf_s(dstStr, bufferLen, "%d", srcValue);
	}

	template<>
	inline void ToString(char* dstStr, short srcValue, unsigned int bufferLen ) 
	{
		sprintf_s(dstStr, bufferLen, "%d", srcValue);
	}

	template<>
	inline void ToString(char* dstStr, unsigned short srcValue, unsigned int bufferLen ) 
	{
		sprintf_s(dstStr, bufferLen, "%d", srcValue);
	}

	template<>
	inline void ToString(char* dstStr, unsigned int srcValue, unsigned int bufferLen ) 
	{
		sprintf_s(dstStr, bufferLen, "%d", srcValue);
	}

	template<>
	inline void ToString(char* dstStr, float srcValue, unsigned int bufferLen ) 
	{
		sprintf_s(dstStr, bufferLen, "%.4f", srcValue);
	}

	template<>
	inline void ToString(char* dstStr, double srcValue, unsigned int bufferLen ) 
	{
		sprintf_s(dstStr, bufferLen, "%.4f", srcValue);
	}

	template<>
	inline void ToString(char* dstStr, bool srcValue, unsigned int bufferLen ) 
	{
		if (srcValue)
			sprintf_s(dstStr, bufferLen, "true");
		else
			sprintf_s(dstStr, bufferLen, "false");
	}
} // namespace HalfNetwork


#endif // __datatypeconverter_h__