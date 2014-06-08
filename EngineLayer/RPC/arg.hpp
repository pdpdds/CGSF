#ifndef __ARGUMENT_H
#define __ARGUMENT_H

//#include "IPersistence.h"
#include "CPStream.h"


template <class A, bool IsPrimitive>
class Arg //: public IPersistence
{
public:
   virtual void Serialize(CPstream & ar)
   {
      if (ar.isLoading())
      {
          ar >> _a;
      }
      else
      {
         ar << _a; 
      }
   }
 
public:
   Arg(A a,bool isServerSide):_a(a),_isServerSide(isServerSide) {}
   Arg(bool isServerSide = true):_isServerSide(isServerSide) {}
   ~Arg() 
   {}
   A ref() 
   { 
      return _a;
   }

private:
   A _a;
   bool _isServerSide;

   //declare_serial(Arg)
};



//////////////////////////////////////////////////////////////////////////////////////
//
//   N o n - P r i m i t i v e    T y p e    P o i n t e r  &  R e f e r e n c e
//

template <class A>
class Arg <A&,false> //: public IPersistence
{
public:
   virtual void Serialize(CPstream & ar)
   {
      if (ar.isLoading())
      {
         if ( _isServerSide )
         {
            // For ISO standard conformance. 
            // Cause the operand of >> is of IPersistence *&, not *
            IPersistence *ip;
            ar >> ip; // create
            _a = dynamic_cast<A*>(ip);
            assert (_a);
         }   
         else
         {
            if ( _isReturnVal )
            {
               // For ISO standard conformance. 
               // Cause the operand of >> is of IPersistence *&, not *
               IPersistence *ip;
               ar >> ip; 
               _a = dynamic_cast<A*>(ip);
               assert (_a);
            }   
            else
               ar >> *_a;
         }
      }
      else
      {
         if ( _isServerSide )
         {
            if (_isReturnVal)
               ar << _a; // for peer update
            else
               ar << *_a;
         }
         else
            ar << _a; 
      }
   }
public:
   
   Arg(A& a, bool isServerSide): _a(&a),_isServerSide(isServerSide)
   {
      if (_isServerSide) 
         _isReturnVal = true;
      else
         _isReturnVal = false;
   }
   
   Arg(bool isServerSide = true):
      _a(NULL),_isServerSide(isServerSide) 
   {
      if (_isServerSide) 
         _isReturnVal = false;
      else
         _isReturnVal = true;
   }
   ~Arg() 
   {
      if (_isServerSide)
         delete _a;
   }
   A& ref() 
   { 
      return *_a;
   }

private:
   A* _a;
   bool _isServerSide,_isReturnVal;

   //declare_serial(Arg)
};



template <class A>
class Arg <A*,false> //: public IPersistence
{

public:
   virtual void Serialize(CPstream & ar)
   {
      if (ar.isLoading())
      {
         if ( _isServerSide )
         {
            // For ISO standard conformance. 
            // Cause the operand of >> is of IPersistence *&, not *
            IPersistence * ip;
            ar >> ip; // create.
            _a = dynamic_cast<A*>(ip);
         }   
         else
         {
            if (_isReturnVal) // 
            {
               // For ISO standard conformance. 
               // Cause the operand of >> is of IPersistence *&, not *
               IPersistence * ip;
               ar >> ip; // It's return pointer. It's invoker's responsibility to free the mem.
               _a = dynamic_cast<A*>(ip);
            }   
            else
            {
               if (_a)
                  ar >> *_a;
               else
                  ; // Null pointer passed by invoker, just ignore.
            }
         }
      }
      else
      {
         if ( _isServerSide )
         {
            if (_isReturnVal)
               ar << _a; 
            else
            {
               if ( _a)
                  ar << *_a;
               else
                  ; // NULL pointer passed by client. Just ignore.
            }
         }
         else
            ar << _a;  // create 
      }
   }

public:
   Arg(A* a, bool isServerSide):_a(a),_isServerSide(isServerSide)
   {
      if (_isServerSide) 
         _isReturnVal = true;
      else
         _isReturnVal = false;
   }
   Arg(bool isServerSide = true):_a(NULL),_isServerSide(isServerSide) 
   {
      if (_isServerSide) 
         _isReturnVal = false;
      else
         _isReturnVal = true;
   }
   ~Arg() 
   {
      if ( _isServerSide)
         delete _a;
   }
   A* ref() 
   { 
      return _a;
   }

private:
   A* _a;
   bool _isServerSide, _isReturnVal;

   //declare_serial(Arg)
};

//////////////////////////////////////////////////////////////////////////////////////
//
//   P r i m i t i v e    T y p e    P o i n t e r  &  R e f e r e n c e
//

template <class A>
class Arg <A&, true> //: public IPersistence
{
public:
   virtual void Serialize(CPstream & ar)
   {
      if (ar.isLoading())
         ar >> _a;
      else
         ar << _a;
   }
public:
   Arg(A& a,bool isServerSide):_a(a),_isServerSide(isServerSide){}
   Arg(bool isServerSide = true):_a(*(new A())),_isServerSide(isServerSide){}
   ~Arg() 
   {
       if (_isServerSide )
         delete &_a;   
   }
   A& ref() 
   { 
      return _a;
   }

private:
   A& _a;
   bool _isServerSide;
   //declare_serial(Arg)
};



template <class A>
class Arg <A*, true> //: public IPersistence
{
public:
   virtual void Serialize(CPstream & ar)
   {
      if (ar.isLoading())
      {
         if ( _isServerSide )
         {
            ar >> _a; // create.
         }   
         else
         {
            if (_isReturnVal) // 
            {
               ar >> _a; // It's return pointer. It's invoker's responsibility to free the mem.
            }   
            else
            {
               if (_a)
                  ar >> *_a;
               else
                  ; // Null pointer passed by invoker, just ignore.
            }
         }
      }
      else
      {
         if ( _isServerSide )
         {
            if (_isReturnVal)
               ar << _a; 
            else
            {
               if ( _a)
                  ar << *_a;
               else
                  ; // NULL pointer passed by client. Just ignore.
            }
         }
         else
            ar << _a;  // create 
      }
   }

public:
   Arg(A* a,bool isServerSide):_a(a),_isServerSide(isServerSide)
   {
      if (_isServerSide) 
         _isReturnVal = true;
      else
         _isReturnVal = false;
   }
   // If the type is char *, there would be 1 byte mem leak here.
   Arg(bool isServerSide = true):_a(NULL),_isServerSide(isServerSide)
   {
      if (_isServerSide) 
         _isReturnVal = false;
      else
         _isReturnVal = true;   
   }
   ~Arg() 
   {
      if (_isServerSide )
         delete _a;   
   }
   A* ref() 
   { 
      return _a;
   }

private:
   A* _a;
   bool _isServerSide, _isReturnVal;
   //declare_serial(Arg)
   
};


///////////////////////////////////////////////
// For the special mem deallocation of char *
//
template <>
class Arg <char*, true> //: public IPersistence
{
public:
   virtual void Serialize(CPstream & ar)
   {
      if (ar.isLoading())
      {
         if (_isServerSide)
            ar >> _a;
         else // Client side 
         {
            if ( _a)
            {  // write back to the parameter of type char*
               char * tmp;
               ar >> tmp;
               strcpy (_a, tmp); 
               delete [] tmp; 
            }
            else
            {  // It's returned char *, need to be dynamically created.
               // It's the invoker's responsibility to release the returned mem.
               ar >> _a;
            }
         }
      }
      else
         ar << _a;
   }

public:
   Arg(char * a,bool isServerSide):_a(a),_isServerSide(isServerSide){}
   // If the type is char *, there would be 1 byte mem leak here.
   Arg(bool isServerSide = true):_a(NULL),_isServerSide(isServerSide){}
   ~Arg() 
   {
      if (_isServerSide )
         // !!!! Important: CPstream allocate the mem of char * in 'new char[*] mode'.
         delete [] _a;   
   }
   char* ref() 
   { 
      return _a;
   }

private:
   char* _a;
   bool _isServerSide;
   //declare_serial(Arg)
   
};

//////////////////////////////////////////////////////////////////////////////////////
//
//   V o i d    T y p e
//
template <>
class Arg <void,true> //: public IPersistence
{
public:
   virtual void Serialize(CPstream & ar)
   {}

public:
   Arg(void (void),bool isServerSide){}
   Arg(bool isServerSide = true){}
   ~Arg() {}
   void ref() {}
   void deRef() {}

   //declare_serial(Arg)
};



template <class A, bool IsPrimitive>
CPstream& operator >> (CPstream& sp, Arg<A, IsPrimitive> & arg) 
{
   arg.Serialize (sp) ; return sp;
}
template <class A, bool IsPrimitive>
CPstream& operator << (CPstream& sp, Arg<A, IsPrimitive> & arg)
{
	arg.Serialize (sp) ; return sp; ;
}

#endif
