#include "stdafx.h"
#include "CPstream.h"
#include "IPersistence.h"

// initialize static member of CPstream
CPstream::PFNCREATE CPstream::s_apfnCreate[] ;
int CPstream::s_iRegNum ;


void CPstream::copyData(char * target, string::const_iterator& it, int count)
{
   int ii = 0;
   for (;ii<count;ii++,it++)
      target[ii] = *it;
}

CPstream& CPstream::operator <<  (IPersistence &shp)
{
   shp.Serialize(*this);
   return *this ;
}

CPstream& CPstream::operator << (IPersistence *pshp)
{
   //assert(pshp);

	int iIndex = FindObject (pshp) ;
	if (iIndex != -1) // already write before!
	{
		return *this << (int)ptIndexed // write index tag
			<< iIndex ; // write index
	}
	else
	{
		AddObject (pshp) ; // record it!
		return *this << (int)ptObject // write object tag
			<< pshp->GetRegisterNo () // write register No.
			<< *pshp ; // write object
	}
}

CPstream& CPstream::operator >> (IPersistence &shp)
{
   shp.Serialize (*this) ; return *this ;
}

CPstream& CPstream::operator >> (IPersistence *&pshp)
{
	int iTag, iIndexOrNo ;
	*this >> iTag >> iIndexOrNo ;
	IPersistence *pshpTemp ;
	switch (iTag)
	{
	case ptIndexed :
		//assert (iIndexOrNo < m_iCur) ;
		pshp = m_apObj[iIndexOrNo] ;
		break ;
	case ptObject :
		assert (iIndexOrNo < s_iRegNum) ;
		pshpTemp = (s_apfnCreate[iIndexOrNo])() ; // create object!
		AddObject (pshpTemp) ; // record it!
		*this >> *pshpTemp ; // read object
		pshp = pshpTemp ;
		break ;
	default :
		assert (0) ; // error!
		break ;
	}
	return *this ;
}

CPstream::CPstream (): fs(), curIt(fs.begin()), _in(false)
{

   // for NULL pointer
   //m_apObj[0] = NULL ;
   //m_iCur = 1 ;
   m_apObj.push_back(NULL);
}

CPstream::CPstream(CPstream& strm )
:fs(strm.fs),curIt(fs.begin()), _in(true)
{
   // for NULL pointer
   //m_apObj[0] = NULL ;
   //m_iCur = 1 ;
   m_apObj.push_back(NULL);
}

CPstream::CPstream(char * buf, unsigned int bufSize)
:fs(buf,bufSize),curIt(fs.begin()), _in(true)
{
   m_apObj.push_back(NULL);
}


CPstream::~CPstream ()
{
}

int CPstream::_RegisterClass_ (PFNCREATE pfnCreate)
{
   assert (s_iRegNum != MAXCLASS) ;
	s_apfnCreate[s_iRegNum] = pfnCreate ;
	return s_iRegNum++ ;
}

void CPstream::AddObject (IPersistence *pObj)
{
   //assert (m_iCur != MAXOBJECT) ;
	//m_apObj[m_iCur++] = pObj ;
   m_apObj.push_back(pObj);
}

int CPstream::FindObject (IPersistence *pObj)
{
   int size = (int)m_apObj.size();
   for (int i = 0 ; i < size ; ++i)
	{
	   if (m_apObj[i] == pObj)
		   return i ; // find it, return index
   }
	return -1 ; // not find, return -1
}



char* CPstream::buf()
{
   return (char*)fs.data();
}
unsigned int CPstream::bufSize()
{
	return (unsigned int)fs.size();
}

bool CPstream::isLoading()
{
   return _in;
}
bool CPstream::isStoring()
{
   return !_in;
}