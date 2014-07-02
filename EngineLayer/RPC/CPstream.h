#ifndef SERIAL2_H
#define SERIAL2_H

#include <string>
#include <vector>
#include <cassert>
using namespace std;

class IPersistence;


/////////////// persistence stream ///////////////
class CPstream
{
private :
   string fs;
   string::const_iterator curIt;
   bool _in;

   void copyData(char * target, string::const_iterator& it, int count);

public:
   template<class T>
   CPstream& primitiveSave(T t)
   {
    //  t = TransportHelper::h2NByteOrder(t);
      fs.append((const char*)&t, sizeof(T));
      return *this ;
   }

   template<class T>
   CPstream& primitiveLoad(T& t)
   {
      copyData((char*)&t, curIt, sizeof(T)); 
      //t = TransportHelper::n2HByteOrder(t);
      return *this ;
   }

   template<class T>
   CPstream& primitiveSave(T* t)
   {
      bool isNull = (t ? false:true);
      
      *this << isNull;
      if ( !isNull) 
         *this << *t;

      return *this;
   }
   template<class T>
   CPstream& primitiveLoad(T *&t)
   {
      bool isNull;

      *this >> isNull;
      if (isNull )
      {
         t = NULL;
      }
      else
      {
         t = new T;
         *this >> *t;
      }

      return *this;         
   }

   //template<>
   CPstream& primitiveLoad(string &ch)
   {
      ch.erase();
      int size;
      copyData((char*)&size, curIt, sizeof(size));
      //size = TransportHelper::n2HByteOrder(size);
      string::const_iterator lastIt = curIt + size;
      ch.append(curIt, lastIt);
      curIt += size;

      return *this;
   }

   //template<>
   CPstream& primitiveSave(char * ch)
   {
      bool isNull = (ch? false:true);
      *this << isNull;
      if (!isNull)
      {
         string tmpStr(ch,strlen(ch));
         (*this) << tmpStr;
      }

      return *this;
   }

   //template <>
   CPstream& primitiveLoad(char * & ch)
   {
      bool isNull;

      *this >> isNull;
      if (!isNull )
      {
         string tmpStr;
         (*this) >> tmpStr;
         ch = new char[tmpStr.size()+1];
         string::const_iterator it = tmpStr.begin();
         copyData(ch, it, (int)tmpStr.size());
         ch[tmpStr.size()] = '\0';
      }
      else
      {
         ch = NULL;
      }
      return *this;
   }

public :

   char* buf();
   unsigned int bufSize();
   bool isLoading();
   bool isStoring();

   CPstream& operator << (bool b) { return primitiveSave (b);}
   CPstream& operator >> (bool& ch) { return primitiveLoad(ch);}

	CPstream& operator << (char ch){ return primitiveSave (ch);}
   CPstream& operator >> (char &ch){ return primitiveLoad(ch);}

	CPstream& operator << (signed char ch){ return primitiveSave (ch);}
   CPstream& operator >> (signed char &ch){ return primitiveLoad(ch);}

	CPstream& operator << (unsigned char ch){ return primitiveSave (ch);}
   CPstream& operator >> (unsigned char &ch){ return primitiveLoad(ch);}

	CPstream& operator << (short sh){ return primitiveSave (sh);}
   CPstream& operator >> (short &sh){ return primitiveLoad(sh);}

	CPstream& operator << (unsigned short sh){ return primitiveSave (sh);}
   CPstream& operator >> (unsigned short &sh){ return primitiveLoad(sh);}

	CPstream& operator << (int i){ return primitiveSave (i);}
   CPstream& operator >> (int &i){ return primitiveLoad(i);}

	CPstream& operator << (unsigned int i){ return primitiveSave (i);}
   CPstream& operator >> (unsigned int &i){ return primitiveLoad(i);}

	CPstream& operator << (long l){ return primitiveSave (l);}
   CPstream& operator >> (long &l){ return primitiveLoad(l);}

	CPstream& operator << (unsigned long l){ return primitiveSave (l);}
   CPstream& operator >> (unsigned long &l){ return primitiveLoad(l);}

   CPstream& operator << (float f){ return primitiveSave (f);}
	CPstream& operator >> (float &f){ return primitiveLoad(f);}

   CPstream& operator << (double d){ return primitiveSave (d);}
   CPstream& operator >> (double &d){ return primitiveLoad(d);}

   CPstream& operator << (long double ld){ return primitiveSave (ld);}
	CPstream& operator >> (long double &ld){ return primitiveLoad(ld);}

   CPstream& operator >> (string &ch){ return primitiveLoad (ch);}
   CPstream& operator << (string& ch)
   {
      // For performance , the saving op use reference input.
    //  unsigned int size = TransportHelper::h2NByteOrder(ch.size());
	   unsigned int size = (unsigned int)ch.size();
      fs.append((const char*)&size, sizeof(size));
      fs.append(ch.data(), ch.size());
      return *this;
   }

   CPstream& operator << (const string& ch)
   { 
      // For performance , the saving op use reference input.
     // unsigned int size = TransportHelper::h2NByteOrder(ch.size());
	   unsigned int size = (unsigned int)ch.size();
      fs.append((const char*)&size, sizeof(size));
      fs.append(ch.data(), ch.size());
      return *this;
   }



   CPstream& operator >> (IPersistence &shp);
   CPstream& operator <<  ( IPersistence &shp);

	CPstream& operator << ( IPersistence *pshp);
   CPstream& operator >> (IPersistence *&pshp);

   CPstream& operator <<( char* ch){ return primitiveSave (ch);}
   CPstream& operator >> (char* &ch){ return primitiveLoad (ch);}

   CPstream& operator << (int* ip) { return primitiveSave (ip);}
   CPstream& operator >> (int* &ip) { return primitiveLoad (ip);}

   CPstream& operator << (unsigned int* ip) { return primitiveSave (ip);}
   CPstream& operator >> (unsigned int* &ip) { return primitiveLoad (ip);}

   CPstream& operator << (long* ip) { return primitiveSave (ip);}
   CPstream& operator >> (long* &ip) { return primitiveLoad (ip);}

   CPstream& operator << (unsigned long* ip) { return primitiveSave (ip);}
   CPstream& operator >> (unsigned long* &ip) { return primitiveLoad (ip);}

   CPstream& operator << (short* ip) { return primitiveSave (ip);}
   CPstream& operator >> (short* &ip) { return primitiveLoad (ip);}
   
   CPstream& operator << (unsigned short* ip) { return primitiveSave (ip);}
   CPstream& operator >> (unsigned short* &ip) { return primitiveLoad (ip);}

   CPstream& operator << (float* ip) { return primitiveSave (ip);}
   CPstream& operator >> (float* &ip) { return primitiveLoad (ip);}

   CPstream& operator << (double* ip) { return primitiveSave (ip);}
   CPstream& operator >> (double* &ip) { return primitiveLoad (ip);}

   CPstream& operator << (long double* ip) { return primitiveSave (ip);}
   CPstream& operator >> (long double* &ip) { return primitiveLoad (ip);}

   CPstream& operator << (bool* ip) { return primitiveSave (ip);}
   CPstream& operator >> (bool* &ip) { return primitiveLoad (ip);}

   CPstream& operator << (string* ip) { return primitiveSave (ip);}
   CPstream& operator >> (string* &ip) { return primitiveLoad (ip);}

public:
   CPstream ();
   CPstream(CPstream& strm );
   CPstream(char * buf, unsigned int bufSize);

   CPstream::~CPstream ();
private :
	enum {MAXCLASS = 128} ;
	typedef IPersistence* (*PFNCREATE) () ;
	static PFNCREATE s_apfnCreate[MAXCLASS] ;
	static int s_iRegNum ;

public :
	static int _RegisterClass_ (PFNCREATE pfnCreate);

	// record write/read object's address
private :
    enum PointerTypes {ptIndexed, ptObject} ; // pointer tag
	enum {MAXOBJECT = 1024} ;
	//const IPersistence *m_apObj[MAXOBJECT] ;
   vector<IPersistence *> m_apObj;
	//int m_iCur ; // current obj number

private :
	void AddObject ( IPersistence *pObj);
   int FindObject ( IPersistence *pObj);
} ;

/////////////// macro for persistence ///////////////
#define declare_serial(class_name) \
	private : \
		static int s_iRegisterNo ; \
	 \
	inline friend CPstream& operator >> (CPstream& sp, const class_name *&pObj) \
	{ \
		return sp >> (IPersistence *&)pObj ; \
	}\
	inline static IPersistence* CreateObject () \
	{ \
		return new class_name ; \
	} \
	inline virtual int GetRegisterNo () const \
	{ \
		return s_iRegisterNo ; \
	}

#define implement_serial(class_name) \
	int class_name::s_iRegisterNo = \
		CPstream::_RegisterClass_(class_name::CreateObject);


#endif
