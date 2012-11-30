#pragma once

template<typename T>
class SFCompressor
{
public:
	SFCompressor(void){}
	virtual ~SFCompressor(void){}

	static T* GetCompressor(){return &m_Compress;}

protected:

private:
	static T m_Compress;
};

template<typename T>
T SFCompressor<T>::m_Compress;

