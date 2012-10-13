#pragma once

template <typename T>
class SFEncrytion
{
public:
	SFEncrytion(void);
	virtual ~SFEncrytion(void);

	static BOOL Encrypt(BYTE* pSrc, DWORD Length);
	static BOOL Decrypt(BYTE* pSrc, DWORD Length);
};

template <typename T>
BOOL SFEncrytion<T>::Encrypt(BYTE* pSrc, DWORD Length)
{
	return T::Encrypt(pSrc, Length);
}

template <typename T>
BOOL SFEncrytion<T>::Decrypt(BYTE* pSrc, DWORD Length)
{
	return T::Decrypt(pSrc, Length);
}