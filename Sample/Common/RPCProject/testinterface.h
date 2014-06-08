#ifndef __TEST_INTERFACE
#define __TEST_INTERFACE

#include "IPersistence.h"
#include "CPstream.h"

class TestArg : public IPersistence
{
public:
   virtual void Serialize(CPstream & strm)
   {
      ;
   }
public:
   virtual void doSomething(){};
   virtual void show(){}
   declare_serial(TestArg)
};

struct TestArg1: public IPersistence
{
   long ll;
   TestArg1(): ll(1111) {}
   TestArg1(int ii): ll(ii) {}
   virtual void Serialize(CPstream&);
   declare_serial(TestArg1)
};

class TestInterface
{
public:
   virtual std::string& echo(std::string& str) = 0;
   virtual void echo1(TestArg& testArg) = 0;
   virtual string echo2 ( string str) = 0;
   virtual TestArg1 echo3 () = 0;
   virtual double echo4 (unsigned long & ll, float& ff) = 0;
   virtual char* echo5 (char * cp, int * ip, char cc) = 0;
   virtual double * echo6(float* ff, TestArg1 arg, unsigned short* ss, long* ll) =0 ; 
   virtual double * echo7(float* ff, TestArg1 arg, unsigned short* ss, long* ll, string& str1) =0;
   virtual TestArg * echo8(float* ff, TestArg1 arg, unsigned short* ss, long* ll, string& str1, bool* bb) =0;
   virtual TestArg& echo9(float* ff, TestArg1 arg, unsigned short* ss, long* ll, string& str1, string& str2, bool bb) =0;
   virtual double* echo10(float* ff, TestArg1 arg, unsigned short* ss, long* ll, string* str, string& str2, int & ii, TestArg* bb) =0;

   virtual ~TestInterface(){}
};

#endif