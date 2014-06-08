#ifndef __TEST_H
#define __TEST_H

#include "arg.hpp"
#include "typetraits.hpp"
#include "CPstream.h"

template<class Service>
class IServiceMethod
{
public:
   virtual void dispatch(Service& serviceImpl, CPstream &iAr, CPstream &oAr) = 0;
   virtual unsigned int getMethodIntAddr() = 0;
   
   virtual ~IServiceMethod(){}

   //
   // Usage Example: 
   //   typedef void(T::*Func)();
   //   Func func = Class::Foo;
   //   unsigned int addr = getReinterpretFuncAddr(&fun);
   //
   static unsigned int getReinterpretFuncAddr(unsigned int funcAddrRef)
   {
      return *(reinterpret_cast<unsigned int*>(funcAddrRef));
   }
};

////////////////////////////////////////////////////////////////////////////
//
//  M e t h o d   W i t h   0    P a r a m e t e r
//
template <typename Service, typename R>
class ServiceMethod_0 :public IServiceMethod<Service>
{
private:
   template<typename ReturnType>
   Arg<R,PrimitiveTraits<R>::val > *  InvokeMethod(
      ReturnType,
      Service& serviceImpl)
   {
      return new Arg<R,PrimitiveTraits<R>::val >((serviceImpl.*_memFunc)(),true);
   }

   //template<>
   Arg<R,PrimitiveTraits<R>::val > *  InvokeMethod(
      PrimitiveTraits<void> voidTrait,
      Service& serviceImpl)
   {
      (serviceImpl.*_memFunc)();
      return NULL;
   }

public:      
   typedef R (Service::*MemFunc)();
   ServiceMethod_0(MemFunc memFunc): 
      _memFunc(memFunc){}

   
   virtual void dispatch(Service& serviceImpl, CPstream &iAr, CPstream &oAr)
   {
      //
      // Decode arguments
      //
      // Return value
	   Arg<R, PrimitiveTraits<R>::val > * retVal = 0;;

      // suppress any exception
      try {
         retVal = InvokeMethod(PrimitiveTraits<R>(),serviceImpl);
      }catch(...){}

      if (retVal != NULL)
      {
         oAr << (*retVal);
         delete retVal;
      }
   }
   virtual unsigned int getMethodIntAddr()
   {
      return getReinterpretFuncAddr((unsigned int)&_memFunc);         
   }

   virtual ~ServiceMethod_0(){}
private:
   MemFunc  _memFunc;
};



template <typename Service, typename R>
IServiceMethod<Service>* getServiceMethod(
   R (Service::*_memFunc)()) 
{

   return new ServiceMethod_0<Service,R>(_memFunc);
}


////////////////////////////////////////////////////////////////////////////
//
//  M e t h o d   W i t h   1    P a r a m e t e r
//

template <typename Service, typename R, typename A1>
class ServiceMethod_1 :public IServiceMethod<Service>
{
private:
   template<typename ReturnType>
   Arg<R,PrimitiveTraits<R>::val > *  InvokeMethod(
      ReturnType,
      Service& serviceImpl,
      Arg<A1,PrimitiveTraits<A1>::val > & arg1)
   {
      return new Arg<R,PrimitiveTraits<R>::val >((serviceImpl.*_memFunc)(arg1.ref()),true);
   }

   //template<>
   Arg<R,PrimitiveTraits<R>::val > *  InvokeMethod(
      PrimitiveTraits<void> voidTrait,
      Service& serviceImpl, 
      Arg<A1,PrimitiveTraits<A1>::val > & arg1)
   {
      (serviceImpl.*_memFunc)(arg1.ref());
      return NULL;
   }

public:      
   typedef R (Service::*MemFunc)(A1);
   ServiceMethod_1(MemFunc memFunc): 
      _memFunc(memFunc){}

   
   virtual void dispatch(Service& serviceImpl, CPstream &iAr, CPstream &oAr)
   {
      //
      // Decode arguments
      //

      // Server side arg 
      Arg<A1,PrimitiveTraits<A1>::val > arg1 (true);
      iAr >> arg1;

      // Return value
      Arg<R,PrimitiveTraits<R>::val > * retVal = 0;

      // suppress any exception
      try {
         retVal = InvokeMethod(PrimitiveTraits<R>(),serviceImpl, arg1);
      }catch(...){}

      oAr << arg1;
      
      if (retVal != NULL)
      {
         oAr << (*retVal);
         delete retVal;
      }
   }
   virtual unsigned int getMethodIntAddr()
   {
      return getReinterpretFuncAddr((unsigned int)&_memFunc);         
   }

   virtual ~ServiceMethod_1(){}
private:
   MemFunc  _memFunc;
};

template <typename Service, typename R, typename A1>
IServiceMethod<Service>* getServiceMethod(
   R (Service::*_memFunc)(A1)) 
{

   return new ServiceMethod_1<Service,R,A1>(_memFunc);
}


////////////////////////////////////////////////////////////////////////////
//
//  M e t h o d   W i t h   2    P a r a m e t e r
//

template <typename Service, typename R, typename A1, typename A2>
class ServiceMethod_2 :public IServiceMethod<Service>
{
private:
   template<typename ReturnType>
   Arg<R,PrimitiveTraits<R>::val > *  InvokeMethod(
      ReturnType,
      Service& serviceImpl,
      Arg<A1,PrimitiveTraits<A1>::val > & arg1,
      Arg<A2,PrimitiveTraits<A2>::val > & arg2
      )
   {
      return new Arg<R,PrimitiveTraits<R>::val >(
         (serviceImpl.*_memFunc)(arg1.ref(), arg2.ref()),true);
   }

   //template<>
   Arg<R,PrimitiveTraits<R>::val > *  InvokeMethod(
      PrimitiveTraits<void> voidTrait,
      Service& serviceImpl, 
      Arg<A1,PrimitiveTraits<A1>::val > & arg1,
      Arg<A2,PrimitiveTraits<A2>::val > & arg2
      )
   {
      (serviceImpl.*_memFunc)(arg1.ref(), arg2.ref());
      return NULL;
   }

public:      
   typedef R (Service::*MemFunc)(A1,A2);
   ServiceMethod_2(MemFunc memFunc): 
      _memFunc(memFunc){}

   
   virtual void dispatch(Service& serviceImpl, CPstream &iAr, CPstream &oAr)
   {
      //
      // Decode arguments
      //

      // Server side arg 
      Arg<A1,PrimitiveTraits<A1>::val > arg1 (true);
      Arg<A2,PrimitiveTraits<A2>::val > arg2 (true);
      iAr >> arg1 >> arg2;

      // Return value
	  Arg<R, PrimitiveTraits<R>::val > * retVal = 0;;

      // suppress any exception
      try {
         retVal = InvokeMethod(PrimitiveTraits<R>(),serviceImpl, arg1, arg2);
      }catch(...){}

      oAr << arg1 << arg2;
      
      if (retVal != NULL)
      {
         oAr << (*retVal);
         delete retVal;
      }
   }
   virtual unsigned int getMethodIntAddr()
   {
      return getReinterpretFuncAddr((unsigned int)&_memFunc);         
   }

   virtual ~ServiceMethod_2(){}
private:
   MemFunc  _memFunc;
};

template <typename Service, typename R, typename A1, typename A2>
IServiceMethod<Service>* getServiceMethod(
   R (Service::*_memFunc)(A1,A2)) 
{

   return new ServiceMethod_2<Service,R,A1,A2>(_memFunc);
}


////////////////////////////////////////////////////////////////////////////
//
//  M e t h o d   W i t h   3    P a r a m e t e r
//

template <typename Service, typename R, typename A1, typename A2, typename A3>
class ServiceMethod_3 :public IServiceMethod<Service>
{
private:
   template<typename ReturnType>
   Arg<R,PrimitiveTraits<R>::val > *  InvokeMethod(
      ReturnType,
      Service& serviceImpl,
      Arg<A1,PrimitiveTraits<A1>::val > & arg1,
      Arg<A2,PrimitiveTraits<A2>::val > & arg2,
      Arg<A3,PrimitiveTraits<A3>::val > & arg3
      )
   {
      return new Arg<R,PrimitiveTraits<R>::val >(
         (serviceImpl.*_memFunc)(arg1.ref(), arg2.ref(), arg3.ref()),true);
   }

   //template<>
   Arg<R,PrimitiveTraits<R>::val > *  InvokeMethod(
      PrimitiveTraits<void> voidTrait,
      Service& serviceImpl, 
      Arg<A1,PrimitiveTraits<A1>::val > & arg1,
      Arg<A2,PrimitiveTraits<A2>::val > & arg2,
      Arg<A3,PrimitiveTraits<A3>::val > & arg3
      )
   {
      (serviceImpl.*_memFunc)(arg1.ref(), arg2.ref(), arg3.ref());
      return NULL;
   }

public:      
   typedef R (Service::*MemFunc)(A1,A2,A3);
   ServiceMethod_3(MemFunc memFunc): 
      _memFunc(memFunc){}

   
   virtual void dispatch(Service& serviceImpl, CPstream &iAr, CPstream &oAr)
   {
      //
      // Decode arguments
      //

      // Server side arg 
      Arg<A1,PrimitiveTraits<A1>::val > arg1 (true);
      Arg<A2,PrimitiveTraits<A2>::val > arg2 (true);
      Arg<A3,PrimitiveTraits<A3>::val > arg3 (true);
      iAr >> arg1 >> arg2 >> arg3;

      // Return value
	  Arg<R, PrimitiveTraits<R>::val > * retVal = 0;;

      // suppress any exception
      try {
         retVal = InvokeMethod(PrimitiveTraits<R>(),serviceImpl, arg1, arg2, arg3);
      }catch(...){}

      oAr << arg1 << arg2 << arg3;
      
      if (retVal != NULL)
      {
         oAr << (*retVal);
         delete retVal;
      }
   }
   virtual unsigned int getMethodIntAddr()
   {
      return getReinterpretFuncAddr((unsigned int)&_memFunc);         
   }

   virtual ~ServiceMethod_3(){}
private:
   MemFunc  _memFunc;
};

template <typename Service, typename R, typename A1, typename A2, typename A3>
IServiceMethod<Service>* getServiceMethod(
   R (Service::*_memFunc)(A1,A2,A3)) 
{

   return new ServiceMethod_3<Service,R,A1,A2,A3>(_memFunc);
}

////////////////////////////////////////////////////////////////////////////
//
//  M e t h o d   W i t h   4    P a r a m e t e r
//

template <typename Service, typename R, typename A1, typename A2, typename A3, typename A4>
class ServiceMethod_4 :public IServiceMethod<Service>
{
private:
   template<typename ReturnType>
   Arg<R,PrimitiveTraits<R>::val > *  InvokeMethod(
      ReturnType,
      Service& serviceImpl,
      Arg<A1,PrimitiveTraits<A1>::val > & arg1,
      Arg<A2,PrimitiveTraits<A2>::val > & arg2,
      Arg<A3,PrimitiveTraits<A3>::val > & arg3,
      Arg<A4,PrimitiveTraits<A4>::val > & arg4
      )
   {
      return new Arg<R,PrimitiveTraits<R>::val >(
         (serviceImpl.*_memFunc)(arg1.ref(), arg2.ref(), arg3.ref(), arg4.ref()),true);
   }

   //template<>
   Arg<R,PrimitiveTraits<R>::val > *  InvokeMethod(
      PrimitiveTraits<void> voidTrait,
      Service& serviceImpl, 
      Arg<A1,PrimitiveTraits<A1>::val > & arg1,
      Arg<A2,PrimitiveTraits<A2>::val > & arg2,
      Arg<A3,PrimitiveTraits<A3>::val > & arg3,
      Arg<A4,PrimitiveTraits<A4>::val > & arg4
      )
   {
      (serviceImpl.*_memFunc)(arg1.ref(), arg2.ref(), arg3.ref(), arg4.ref());
      return NULL;
   }

public:      
   typedef R (Service::*MemFunc)(A1,A2,A3,A4);
   ServiceMethod_4(MemFunc memFunc): 
      _memFunc(memFunc){}

   
   virtual void dispatch(Service& serviceImpl, CPstream &iAr, CPstream &oAr)
   {
      //
      // Decode arguments
      //

      // Server side arg 
      Arg<A1,PrimitiveTraits<A1>::val > arg1 (true);
      Arg<A2,PrimitiveTraits<A2>::val > arg2 (true);
      Arg<A3,PrimitiveTraits<A3>::val > arg3 (true);
      Arg<A4,PrimitiveTraits<A4>::val > arg4 (true);
      iAr >> arg1 >> arg2 >> arg3 >> arg4;

      // Return value
	  Arg<R, PrimitiveTraits<R>::val > * retVal = 0;;

      // suppress any exception
      try {
         retVal = InvokeMethod(PrimitiveTraits<R>(),serviceImpl, arg1, arg2, arg3, arg4);
      }catch(...){}

      oAr << arg1 << arg2 << arg3 << arg4;
      
      if (retVal != NULL)
      {
         oAr << (*retVal);
         delete retVal;
      }
   }
   virtual unsigned int getMethodIntAddr()
   {
      return getReinterpretFuncAddr((unsigned int)&_memFunc);         
   }

   virtual ~ServiceMethod_4(){}
private:
   MemFunc  _memFunc;
};

template <typename Service, typename R, typename A1, typename A2, typename A3, typename A4>
IServiceMethod<Service>* getServiceMethod(
   R (Service::*_memFunc)(A1,A2,A3,A4)) 
{

   return new ServiceMethod_4<Service,R,A1,A2,A3,A4>(_memFunc);
}

////////////////////////////////////////////////////////////////////////////
//
//  M e t h o d   W i t h   5    P a r a m e t e r
//

template <typename Service, typename R, typename A1, typename A2, 
   typename A3, typename A4, typename A5>
class ServiceMethod_5 :public IServiceMethod<Service>
{
private:
   template<typename ReturnType>
   Arg<R,PrimitiveTraits<R>::val > *  InvokeMethod(
      ReturnType,
      Service& serviceImpl,
      Arg<A1,PrimitiveTraits<A1>::val > & arg1,
      Arg<A2,PrimitiveTraits<A2>::val > & arg2,
      Arg<A3,PrimitiveTraits<A3>::val > & arg3,
      Arg<A4,PrimitiveTraits<A4>::val > & arg4,
      Arg<A5,PrimitiveTraits<A5>::val > & arg5
      )
   {
      return new Arg<R,PrimitiveTraits<R>::val >(
         (serviceImpl.*_memFunc)(arg1.ref(), arg2.ref(), 
         arg3.ref(), arg4.ref(), arg5.ref()),true);
   }

   //template<>
   Arg<R,PrimitiveTraits<R>::val > *  InvokeMethod(
      PrimitiveTraits<void> voidTrait,
      Service& serviceImpl, 
      Arg<A1,PrimitiveTraits<A1>::val > & arg1,
      Arg<A2,PrimitiveTraits<A2>::val > & arg2,
      Arg<A3,PrimitiveTraits<A3>::val > & arg3,
      Arg<A4,PrimitiveTraits<A4>::val > & arg4,
      Arg<A5,PrimitiveTraits<A5>::val > & arg5
      )
   {
      (serviceImpl.*_memFunc)(arg1.ref(), arg2.ref(), arg3.ref(), arg4.ref(), arg5.ref());
      return NULL;
   }

public:      
   typedef R (Service::*MemFunc)(A1,A2,A3,A4,A5);
   ServiceMethod_5(MemFunc memFunc): 
      _memFunc(memFunc){}

   
   virtual void dispatch(Service& serviceImpl, CPstream &iAr, CPstream &oAr)
   {
      //
      // Decode arguments
      //

      // Server side arg 
      Arg<A1,PrimitiveTraits<A1>::val > arg1 (true);
      Arg<A2,PrimitiveTraits<A2>::val > arg2 (true);
      Arg<A3,PrimitiveTraits<A3>::val > arg3 (true);
      Arg<A4,PrimitiveTraits<A4>::val > arg4 (true);
      Arg<A5,PrimitiveTraits<A5>::val > arg5 (true);
      iAr >> arg1 >> arg2 >> arg3 >> arg4 >> arg5;

      // Return value
	  Arg<R, PrimitiveTraits<R>::val > * retVal = 0;;

      // suppress any exception
      try {
         retVal = InvokeMethod(PrimitiveTraits<R>(),serviceImpl, arg1, arg2, arg3, arg4, arg5);
      }catch(...){}

      oAr << arg1 << arg2 << arg3 << arg4 << arg5;
      
      if (retVal != NULL)
      {
         oAr << (*retVal);
         delete retVal;
      }
   }
   virtual unsigned int getMethodIntAddr()
   {
      return getReinterpretFuncAddr((unsigned int)&_memFunc);         
   }

   virtual ~ServiceMethod_5(){}
private:
   MemFunc  _memFunc;
};

template <typename Service, typename R, typename A1, typename A2, 
   typename A3, typename A4, typename A5>
IServiceMethod<Service>* getServiceMethod(
   R (Service::*_memFunc)(A1,A2,A3,A4,A5)) 
{

   return new ServiceMethod_5<Service,R,A1,A2,A3,A4,A5>(_memFunc);
}

////////////////////////////////////////////////////////////////////////////
//
//  M e t h o d   W i t h   6    P a r a m e t e r
//

template <typename Service, typename R, typename A1, typename A2, 
   typename A3, typename A4, typename A5, typename A6>
class ServiceMethod_6 :public IServiceMethod<Service>
{
private:
   template<typename ReturnType>
   Arg<R,PrimitiveTraits<R>::val > *  InvokeMethod(
      ReturnType,
      Service& serviceImpl,
      Arg<A1,PrimitiveTraits<A1>::val > & arg1,
      Arg<A2,PrimitiveTraits<A2>::val > & arg2,
      Arg<A3,PrimitiveTraits<A3>::val > & arg3,
      Arg<A4,PrimitiveTraits<A4>::val > & arg4,
      Arg<A5,PrimitiveTraits<A5>::val > & arg5,
      Arg<A6,PrimitiveTraits<A6>::val > & arg6
      )
   {
      return new Arg<R,PrimitiveTraits<R>::val >(
         (serviceImpl.*_memFunc)(arg1.ref(), arg2.ref(), 
         arg3.ref(), arg4.ref(), arg5.ref(), arg6.ref()),true);
   }

   //template<>
   Arg<R,PrimitiveTraits<R>::val > *  InvokeMethod(
      PrimitiveTraits<void> voidTrait,
      Service& serviceImpl, 
      Arg<A1,PrimitiveTraits<A1>::val > & arg1,
      Arg<A2,PrimitiveTraits<A2>::val > & arg2,
      Arg<A3,PrimitiveTraits<A3>::val > & arg3,
      Arg<A4,PrimitiveTraits<A4>::val > & arg4,
      Arg<A5,PrimitiveTraits<A5>::val > & arg5,
      Arg<A6,PrimitiveTraits<A6>::val > & arg6
      )
   {
      (serviceImpl.*_memFunc)(arg1.ref(), arg2.ref(), 
         arg3.ref(), arg4.ref(), arg5.ref(), arg6.ref());
      return NULL;
   }

public:      
   typedef R (Service::*MemFunc)(A1,A2,A3,A4,A5,A6);
   ServiceMethod_6(MemFunc memFunc): 
      _memFunc(memFunc){}

   
   virtual void dispatch(Service& serviceImpl, CPstream &iAr, CPstream &oAr)
   {
      //
      // Decode arguments
      //

      // Server side arg 
      Arg<A1,PrimitiveTraits<A1>::val > arg1 (true);
      Arg<A2,PrimitiveTraits<A2>::val > arg2 (true);
      Arg<A3,PrimitiveTraits<A3>::val > arg3 (true);
      Arg<A4,PrimitiveTraits<A4>::val > arg4 (true);
      Arg<A5,PrimitiveTraits<A5>::val > arg5 (true);
      Arg<A6,PrimitiveTraits<A6>::val > arg6 (true);
      iAr >> arg1 >> arg2 >> arg3 >> arg4 >> arg5 >> arg6;

      // Return value
	  Arg<R, PrimitiveTraits<R>::val > * retVal = 0;;

      // suppress any exception
      try {
         retVal = InvokeMethod(PrimitiveTraits<R>(),serviceImpl, arg1, arg2, 
            arg3, arg4, arg5, arg6);
      }catch(...){}

      oAr << arg1 << arg2 << arg3 << arg4 << arg5 << arg6;
      
      if (retVal != NULL)
      {
         oAr << (*retVal);
         delete retVal;
      }
   }
   virtual unsigned int getMethodIntAddr()
   {
      return getReinterpretFuncAddr((unsigned int)&_memFunc);         
   }

   virtual ~ServiceMethod_6(){}
private:
   MemFunc  _memFunc;
};

template <typename Service, typename R, typename A1, typename A2, 
   typename A3, typename A4, typename A5, typename A6>
IServiceMethod<Service>* getServiceMethod(
   R (Service::*_memFunc)(A1,A2,A3,A4,A5,A6)) 
{

   return new ServiceMethod_6<Service,R,A1,A2,A3,A4,A5,A6>(_memFunc);
}

////////////////////////////////////////////////////////////////////////////
//
//  M e t h o d   W i t h   7    P a r a m e t e r
//

template <typename Service, typename R, typename A1, typename A2, 
   typename A3, typename A4, typename A5, typename A6, typename A7>
class ServiceMethod_7 :public IServiceMethod<Service>
{
private:
   template<typename ReturnType>
   Arg<R,PrimitiveTraits<R>::val > *  InvokeMethod(
      ReturnType,
      Service& serviceImpl,
      Arg<A1,PrimitiveTraits<A1>::val > & arg1,
      Arg<A2,PrimitiveTraits<A2>::val > & arg2,
      Arg<A3,PrimitiveTraits<A3>::val > & arg3,
      Arg<A4,PrimitiveTraits<A4>::val > & arg4,
      Arg<A5,PrimitiveTraits<A5>::val > & arg5,
      Arg<A6,PrimitiveTraits<A6>::val > & arg6,
      Arg<A7,PrimitiveTraits<A7>::val > & arg7
      )
   {
      return new Arg<R,PrimitiveTraits<R>::val >(
         (serviceImpl.*_memFunc)(arg1.ref(), arg2.ref(), 
         arg3.ref(), arg4.ref(), arg5.ref(), arg6.ref(), arg7.ref()),true);
   }

   //template<>
   Arg<R,PrimitiveTraits<R>::val > *  InvokeMethod(
      PrimitiveTraits<void> voidTrait,
      Service& serviceImpl, 
      Arg<A1,PrimitiveTraits<A1>::val > & arg1,
      Arg<A2,PrimitiveTraits<A2>::val > & arg2,
      Arg<A3,PrimitiveTraits<A3>::val > & arg3,
      Arg<A4,PrimitiveTraits<A4>::val > & arg4,
      Arg<A5,PrimitiveTraits<A5>::val > & arg5,
      Arg<A6,PrimitiveTraits<A6>::val > & arg6,
      Arg<A7,PrimitiveTraits<A7>::val > & arg7
      )
   {
      (serviceImpl.*_memFunc)(arg1.ref(), arg2.ref(), 
         arg3.ref(), arg4.ref(), arg5.ref(), arg6.ref(), arg7.ref());
      return NULL;
   }

public:      
   typedef R (Service::*MemFunc)(A1,A2,A3,A4,A5,A6,A7);
   ServiceMethod_7(MemFunc memFunc): 
      _memFunc(memFunc){}

   
   virtual void dispatch(Service& serviceImpl, CPstream &iAr, CPstream &oAr)
   {
      //
      // Decode arguments
      //

      // Server side arg 
      Arg<A1,PrimitiveTraits<A1>::val > arg1 (true);
      Arg<A2,PrimitiveTraits<A2>::val > arg2 (true);
      Arg<A3,PrimitiveTraits<A3>::val > arg3 (true);
      Arg<A4,PrimitiveTraits<A4>::val > arg4 (true);
      Arg<A5,PrimitiveTraits<A5>::val > arg5 (true);
      Arg<A6,PrimitiveTraits<A6>::val > arg6 (true);
      Arg<A7,PrimitiveTraits<A7>::val > arg7 (true);
      iAr >> arg1 >> arg2 >> arg3 >> arg4 >> arg5 >> arg6 >> arg7;

      // Return value
	  Arg<R, PrimitiveTraits<R>::val > * retVal = 0;;

      // suppress any exception
      try {
         retVal = InvokeMethod(PrimitiveTraits<R>(),serviceImpl, arg1, arg2, 
            arg3, arg4, arg5, arg6, arg7);
      }catch(...){}

      oAr << arg1 << arg2 << arg3 << arg4 << arg5 << arg6 << arg7;
      
      if (retVal != NULL)
      {
         oAr << (*retVal);
         delete retVal;
      }
   }
   virtual unsigned int getMethodIntAddr()
   {
      return getReinterpretFuncAddr((unsigned int)&_memFunc);         
   }

   virtual ~ServiceMethod_7(){}
private:
   MemFunc  _memFunc;
};

template <typename Service, typename R, typename A1, typename A2, 
   typename A3, typename A4, typename A5, typename A6, typename A7>
IServiceMethod<Service>* getServiceMethod(
   R (Service::*_memFunc)(A1,A2,A3,A4,A5,A6,A7)) 
{

   return new ServiceMethod_7<Service,R,A1,A2,A3,A4,A5,A6,A7>(_memFunc);
}

////////////////////////////////////////////////////////////////////////////
//
//  M e t h o d   W i t h   8    P a r a m e t e r
//

template <typename Service, typename R, typename A1, typename A2, 
   typename A3, typename A4, typename A5, typename A6, typename A7, typename A8>
class ServiceMethod_8 :public IServiceMethod<Service>
{
private:
   template<typename ReturnType>
   Arg<R,PrimitiveTraits<R>::val > *  InvokeMethod(
      ReturnType,
      Service& serviceImpl,
      Arg<A1,PrimitiveTraits<A1>::val > & arg1,
      Arg<A2,PrimitiveTraits<A2>::val > & arg2,
      Arg<A3,PrimitiveTraits<A3>::val > & arg3,
      Arg<A4,PrimitiveTraits<A4>::val > & arg4,
      Arg<A5,PrimitiveTraits<A5>::val > & arg5,
      Arg<A6,PrimitiveTraits<A6>::val > & arg6,
      Arg<A7,PrimitiveTraits<A7>::val > & arg7,
      Arg<A8,PrimitiveTraits<A8>::val > & arg8
      )
   {
      return new Arg<R,PrimitiveTraits<R>::val >(
         (serviceImpl.*_memFunc)(arg1.ref(), arg2.ref(), 
         arg3.ref(), arg4.ref(), arg5.ref(), arg6.ref(), arg7.ref(), arg8.ref()),true);
   }

   //template<>
   Arg<R,PrimitiveTraits<R>::val > *  InvokeMethod(
      PrimitiveTraits<void> voidTrait,
      Service& serviceImpl, 
      Arg<A1,PrimitiveTraits<A1>::val > & arg1,
      Arg<A2,PrimitiveTraits<A2>::val > & arg2,
      Arg<A3,PrimitiveTraits<A3>::val > & arg3,
      Arg<A4,PrimitiveTraits<A4>::val > & arg4,
      Arg<A5,PrimitiveTraits<A5>::val > & arg5,
      Arg<A6,PrimitiveTraits<A6>::val > & arg6,
      Arg<A7,PrimitiveTraits<A7>::val > & arg7,
      Arg<A8,PrimitiveTraits<A8>::val > & arg8
      )
   {
      (serviceImpl.*_memFunc)(arg1.ref(), arg2.ref(), 
         arg3.ref(), arg4.ref(), arg5.ref(), arg6.ref(), arg7.ref(), arg8.ref());
      return NULL;
   }

public:      
   typedef R (Service::*MemFunc)(A1,A2,A3,A4,A5,A6,A7,A8);
   ServiceMethod_8(MemFunc memFunc): 
      _memFunc(memFunc){}

   
   virtual void dispatch(Service& serviceImpl, CPstream &iAr, CPstream &oAr)
   {
      //
      // Decode arguments
      //

      // Server side arg 
      Arg<A1,PrimitiveTraits<A1>::val > arg1 (true);
      Arg<A2,PrimitiveTraits<A2>::val > arg2 (true);
      Arg<A3,PrimitiveTraits<A3>::val > arg3 (true);
      Arg<A4,PrimitiveTraits<A4>::val > arg4 (true);
      Arg<A5,PrimitiveTraits<A5>::val > arg5 (true);
      Arg<A6,PrimitiveTraits<A6>::val > arg6 (true);
      Arg<A7,PrimitiveTraits<A7>::val > arg7 (true);
      Arg<A8,PrimitiveTraits<A8>::val > arg8 (true);
      iAr >> arg1 >> arg2 >> arg3 >> arg4 >> arg5 >> arg6 >> arg7 >> arg8;

      // Return value
	  Arg<R, PrimitiveTraits<R>::val > * retVal = 0;;

      // suppress any exception
      try {
         retVal = InvokeMethod(PrimitiveTraits<R>(),serviceImpl, arg1, arg2, 
            arg3, arg4, arg5, arg6, arg7, arg8);
      }catch(...){}

      oAr << arg1 << arg2 << arg3 << arg4 << arg5 << arg6 << arg7 << arg8;
      
      if (retVal != NULL)
      {
         oAr << (*retVal);
         delete retVal;
      }
   }
   virtual unsigned int getMethodIntAddr()
   {
      return getReinterpretFuncAddr((unsigned int)&_memFunc);         
   }

   virtual ~ServiceMethod_8(){}
private:
   MemFunc  _memFunc;
};

template <typename Service, typename R, typename A1, typename A2, 
   typename A3, typename A4, typename A5, typename A6, typename A7, typename A8>
IServiceMethod<Service>* getServiceMethod(
   R (Service::*_memFunc)(A1,A2,A3,A4,A5,A6,A7,A8)) 
{

   return new ServiceMethod_8<Service,R,A1,A2,A3,A4,A5,A6,A7,A8>(_memFunc);
}

#endif
