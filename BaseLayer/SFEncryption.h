#pragma once

template <typename T>
class SFEncrytion
{
public:
	SFEncrytion(void);
	virtual ~SFEncrytion(void);

	static bool Encrypt(BYTE* pSrc, DWORD Length);
	static bool Decrypt(BYTE* pSrc, DWORD Length);
};

template <typename T>
bool SFEncrytion<T>::Encrypt(BYTE* pSrc, DWORD Length)
{
	return T::Encrypt(pSrc, Length);
}

template <typename T>
bool SFEncrytion<T>::Decrypt(BYTE* pSrc, DWORD Length)
{
	return T::Decrypt(pSrc, Length);
}