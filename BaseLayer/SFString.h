#pragma once

#define DECLARE_STRING_TYPE(sType, iLength) \
struct sType : public SFString \
{ \
private: \
	TCHAR szBuf[iLength]; \
public: \
	sType() : SFString(szBuf, iLength) \
	{ \
	} \
	sType(sType& sData) : SFString(szBuf, iLength) \
	{ \
	_tcsncpy_s(szBuf, sData.GetBuffer(), GetStringLength()); \
	szBuf[iLength - 1] = 0; \
	} \
	inline sType& sType::operator=(const TCHAR* pData) \
	{ \
		if(CompareSize(pData) == FALSE) \
		{\
			return *this; \
		}\
		_tcsncpy_s(szBuf, pData, _tcslen(pData)); \
		szBuf[iLength - 1] = 0; \
		return *this; \
	} \
}

struct SFString
{
private:
	TCHAR* _pBuffer;
	int _iStringLength;

public:
	SFString(TCHAR* pData, int iLength)
	{
		_pBuffer = pData;
		_iStringLength = iLength - 1;
	}

	TCHAR* GetBuffer()
	{
		return _pBuffer;
	}

	void Clear()
	{
		memset(_pBuffer, 0, sizeof(TCHAR) * (_iStringLength * 1));
	}

	int GetStringLength()
	{
		return _iStringLength;
	}

	inline SFString& SFString::operator= (SFString& sData)
	{
		if(GetStringLength() != sData.GetStringLength())
		{
			__asm
			{
				int 3
			}

			return *this;
		}

		_tcsncpy_s(_pBuffer, _iStringLength + 1, sData.GetBuffer(), _iStringLength);
		_pBuffer[_iStringLength] = 0;

		return *this;
	}

	inline BOOL SFString::operator ==(SFString& sData)
	{
		if(GetStringLength() != sData.GetStringLength())
		{
			return FALSE;
		}

		if(_tcsncmp(_pBuffer, sData.GetBuffer(), sizeof(TCHAR) * _iStringLength) == 0)
		{
			return TRUE;
		}

		return FALSE;
	}

	inline BOOL SFString::operator != (const TCHAR* szData)
	{
		if(_tcsncmp(_pBuffer, szData, sizeof(TCHAR) * _iStringLength) == 0)
		{
			return FALSE;
		}
		
		return TRUE;
	}

	BOOL CompareSize(const TCHAR* pData)
	{
		if (GetStringLength() < (int)_tcslen(pData))
		{
			SFASSERT(0);

			return FALSE;
		}

		return TRUE;
	}
};