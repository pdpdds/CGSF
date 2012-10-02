#pragma once
#include <vector>

class SFFString;

typedef std::vector<SFFString> SFFStringVector;

//======================================================================================================================

#define SFFString_ALLOC_BLOCK_SIZE      64

enum SFFStringType
{
    SFFSTRType_ANSI,
    SFFSTRType_Unicode16,
    SFFSTRType_UTF8
};

class SFFString
{
public:
    SFFString();
    SFFString(SFFStringType type, USHORT length);
    ~SFFString();

    SFFString(const char* data);
    SFFString(const USHORT* data);
    SFFString(const wchar_t* data);
    SFFString(const SFFString& data);

    USHORT initRawSFFSTR(char* data, SFFStringType type);

    // Operator overloads
    bool operator ==(char* string);
    bool operator ==(const char* data);
    bool operator ==(const SFFString& data);
    bool operator ==(SFFString data);

    SFFString& operator =(const char* data);

    SFFString& operator =(char* data);
    SFFString& operator =(const USHORT* data);
    SFFString& operator =(const wchar_t* data);
    SFFString& operator =(const SFFString& data);

    SFFString& operator <<(const char* data);
    SFFString& operator <<(const USHORT* data);
    SFFString& operator <<(char* data);
    SFFString& operator <<(USHORT* data);

    // Accessor Methods
    const char* getAnsi() const;
    const wchar_t* getUnicode16() const;

    void convert(SFFStringType type);
    void substring(SFFString& dest, USHORT start, USHORT end);

    int  split(SFFStringVector& retVec,char delimiter);
    void toLower();
    void toUpper();
    void toUpperFirst();
    void toLowerFirst();
    bool isNumber();

    void setLength(USHORT length);
    void setType(SFFStringType type);

    USHORT getLength() const;
    DWORD getDataLength() const;
    DWORD getCharacterWidth() const;

    SFFStringType getType() const;
    char* getRawData() const;
    DWORD getAllocated() const;
    DWORD getCrc() const;

    static DWORD CRC(char* data);

private:
    void _allocate();

    char* m_szBuffer;          // Pointer to the allocated buffer
    SFFStringType m_Type;            // What format is the current string in.
    USHORT m_Allocated;       // Length of the allocated buffer which should be a multiple of
    USHORT m_CharacterWidth;  // Size of a single character in bytes
    USHORT m_Length;          // Length of the string itself.  BStrings are NOT null terminated

    static DWORD mCrcTable[256];
};