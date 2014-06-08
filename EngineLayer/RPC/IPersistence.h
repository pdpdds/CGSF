#ifndef __PERSISTENCE_HEADER
#define __PERSISTENCE_HEADER
#include <iostream>
//#include <typeinfo.h>

using namespace std;

class IPersistence
{
	friend class CPstream ;
protected :
	virtual void Serialize (CPstream &)  = 0;
	//virtual void ReadData (CPstream &) = 0;
private:
	virtual int GetRegisterNo () const{return 0;};
public:
   virtual ~IPersistence(){};
} ;

#endif
