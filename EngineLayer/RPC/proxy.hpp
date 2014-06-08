#ifndef __PROXY_H
#define __PROXY_H


#include <string>
#include "CPstream.h"

#include "transport.hpp"
#include "service_err.hpp"
#include "service_method.h"

template<typename Service>
class Proxy
{
public:
   //class ServiceDefinition<Service,InAr,OutAr>;

   Proxy(IClientTransport& trans):
      _trans(trans),_serviceDef( *new ServiceDefinition<Service>())
   {}
   ~Proxy() { delete &_serviceDef; }

   /////////////////////////////////////////////////////////////////
   // 
   //   0 parameter (void) invoking
   //
   template</*typename Service, */typename R>
   class invoke_0 
   {
   private:
      IClientTransport & _trans;
      ServiceDefinition<Service> & _serviceDef_1;
   public:
      typedef R (Service::*MemFunc)();
      MemFunc _memFunc;
      typedef ServiceDefinition<Service> ServiceDef;

      invoke_0(MemFunc memFunc,ServiceDef& serviceDef, 
         IClientTransport& trans        
         ): _memFunc(memFunc),_trans(trans),_serviceDef_1(serviceDef){}

      R operator() ()
      {
         CPstream oa;

         // Serialize service name
         oa << _serviceDef_1.getServiceName();
         // Serialize method id
         int methodId = 
            _serviceDef_1.getMethodId(
               IServiceMethod<Service>::getReinterpretFuncAddr((unsigned int)&_memFunc)
               );
         oa << methodId;

         Arg<R, PrimitiveTraits<R>::val> retVal(false);

         _trans.send(oa.buf(),oa.bufSize());


         std::string recvBuf;
         _trans.recv(recvBuf);

         CPstream ia((char*)recvBuf.data(), recvBuf.length());
         
         int ret;
         ia >> ret;
		 if (ret != RPC_OK)
            throw ServiceException("Service error", ret);

		 int resMethodId = -1;
		 ia >> resMethodId;

		 if (methodId != resMethodId)
			 throw ServiceException("Service error", ret);
        
         if (VoidTraits<R>::val == false )
         {
            ia >> retVal;
         } 
         
         return retVal.ref();
      }

   };

  
   template<typename R>
   invoke_0</*Service,*/R>  invoke(R (Service::*memFunc)())
   {  
      return invoke_0</*Service,*/R>(memFunc,_serviceDef,_trans);
   }

   /////////////////////////////////////////////////////////////////
   // 
   //   1 parameters invoking
   //

   template</*typename Service,*/ typename R, typename A1>
   class invoke_1 
   {
   private:
      IClientTransport & _trans;
      ServiceDefinition<Service> & _serviceDef_1;
   public:
      typedef R (Service::*MemFunc)(A1);
      MemFunc _memFunc;
      typedef ServiceDefinition<Service> ServiceDef;

      invoke_1(MemFunc memFunc,ServiceDef& serviceDef, 
         IClientTransport& trans        
         ): _memFunc(memFunc),_trans(trans),_serviceDef_1(serviceDef){}

      R operator() (A1 a1)
      {
         CPstream oa;

         // Serialize service name
         oa << _serviceDef_1.getServiceName();
         // Serialize method id
         int methodId = 
            _serviceDef_1.getMethodId(
               IServiceMethod<Service>::getReinterpretFuncAddr((unsigned int)&_memFunc)
               );
         oa << methodId;

         Arg<A1,PrimitiveTraits<A1>::val> arg1(a1,false);
         Arg<R, PrimitiveTraits<R>::val> retVal(false);

         oa << arg1;

         _trans.send(oa.buf(),oa.bufSize());
         
         std::string recvBuf = "";         
         _trans.recv(recvBuf);

         CPstream ia((char*)recvBuf.data(), recvBuf.length());
         
         int ret;
         ia >> ret;
         if ( ret != RPC_OK)
            throw ServiceException("Service error", ret);

		 int resMethodId = -1;
		 ia >> resMethodId;
		 
		 if (methodId != resMethodId)
			 throw ServiceException("Service error", ret);

         ia >> arg1;
         
         if (VoidTraits<R>::val == false )
         {
            ia >> retVal;
         } 
         
         return retVal.ref();
      }

   };

  
   template<typename R, typename A1>
   invoke_1</*Service,*/R,A1>  invoke(R (Service::*memFunc)(A1))
   {  
      return invoke_1</*Service,*/R,A1>(memFunc,_serviceDef,_trans);
   }

   /////////////////////////////////////////////////////////////////
   // 
   //   2 parameters invoking
   //

   template</*typename Service,*/ typename R, typename A1, typename A2>
   class invoke_2 
   {
   private:
      IClientTransport & _trans;
      ServiceDefinition<Service> & _serviceDef_1;
   public:
      typedef R (Service::*MemFunc)(A1,A2);
      MemFunc _memFunc;
      typedef ServiceDefinition<Service> ServiceDef;

      invoke_2(MemFunc memFunc,ServiceDef& serviceDef, 
         IClientTransport& trans        
         ): _memFunc(memFunc),_trans(trans),_serviceDef_1(serviceDef){}

      R operator() (A1 a1, A2 a2)
      {
         CPstream oa;

         // Serialize service name
         oa << _serviceDef_1.getServiceName();
         // Serialize method id
         int methodId = 
            _serviceDef_1.getMethodId(
               IServiceMethod<Service>::getReinterpretFuncAddr((unsigned int)&_memFunc)
               );
         oa << methodId;

         Arg<A1,PrimitiveTraits<A1>::val> arg1(a1,false);
         Arg<A2,PrimitiveTraits<A2>::val> arg2(a2,false);
         Arg<R, PrimitiveTraits<R>::val> retVal(false);

         // Regardless in and out argument
         oa << arg1 << arg2;

         _trans.send(oa.buf(),oa.bufSize());
         
         std::string recvBuf;
         _trans.recv(recvBuf);
         
         CPstream ia((char*)recvBuf.data(), recvBuf.length());
         
         int ret;
         ia >> ret;
		 if (ret != RPC_OK)
            throw ServiceException("Service error", ret);

		 int resMethodId = -1;
		 ia >> resMethodId;

		 if (methodId != resMethodId)
			 throw ServiceException("Service error", ret);

         ia >> arg1 >> arg2;
         
         if (VoidTraits<R>::val == false )
         {
            ia >> retVal;
         } 
         
         return retVal.ref();
      }

   };

  
   template<typename R, typename A1, typename A2>
   invoke_2</*Service,*/R,A1,A2>  invoke(R (Service::*memFunc)(A1,A2))
   {  
      return invoke_2</*Service,*/R,A1,A2>(memFunc,_serviceDef,_trans);
   }

   /////////////////////////////////////////////////////////////////
   // 
   //   3 parameters invoking
   //

   template</*typename Service,*/ typename R, typename A1, typename A2, typename A3>
   class invoke_3 
   {
   private:
      IClientTransport & _trans;
      ServiceDefinition<Service> & _serviceDef_1;
   public:
      typedef R (Service::*MemFunc)(A1,A2,A3);
      MemFunc _memFunc;
      typedef ServiceDefinition<Service> ServiceDef;

      invoke_3(MemFunc memFunc,ServiceDef& serviceDef, 
         IClientTransport& trans        
         ): _memFunc(memFunc),_trans(trans),_serviceDef_1(serviceDef){}

      R operator() (A1 a1, A2 a2, A3 a3)
      {
         CPstream oa;

         // Serialize service name
         oa << _serviceDef_1.getServiceName();
         // Serialize method id
         int methodId = 
            _serviceDef_1.getMethodId(
               IServiceMethod<Service>::getReinterpretFuncAddr((unsigned int)&_memFunc)
               );
         oa << methodId;

         Arg<A1,PrimitiveTraits<A1>::val> arg1(a1,false);
         Arg<A2,PrimitiveTraits<A2>::val> arg2(a2,false);
         Arg<A3,PrimitiveTraits<A3>::val> arg3(a3,false);
         Arg<R, PrimitiveTraits<R>::val> retVal(false);

         // Regardless in and out argument
         oa << arg1 << arg2 << arg3;

         _trans.send(oa.buf(),oa.bufSize());

         std::string recvBuf;
         _trans.recv(recvBuf);

         CPstream ia((char*)recvBuf.data(), recvBuf.length());
         
         int ret;
         ia >> ret;
		 if (ret != RPC_OK)
            throw ServiceException("Service error", ret);

		 int resMethodId = -1;
		 ia >> resMethodId;

		 if (methodId != resMethodId)
			 throw ServiceException("Service error", ret);

         ia >> arg1 >> arg2 >> arg3;
         
         if (VoidTraits<R>::val == false )
         {
            ia >> retVal;
         } 
         
         return retVal.ref();
      }

   };

  
   template<typename R, typename A1, typename A2, typename A3>
   invoke_3</*Service,*/R,A1,A2,A3>  invoke(R (Service::*memFunc)(A1,A2,A3))
   {  
      return invoke_3</*Service,*/R,A1,A2,A3>(memFunc,_serviceDef,_trans);
   }

   /////////////////////////////////////////////////////////////////
   // 
   //   4 parameters invoking
   //

   template</*typename Service, */typename R, typename A1, typename A2, typename A3, typename A4>
   class invoke_4 
   {
   private:
      IClientTransport & _trans;
      ServiceDefinition<Service> & _serviceDef_1;
   public:
      typedef R (Service::*MemFunc)(A1,A2,A3,A4);
      MemFunc _memFunc;
      typedef ServiceDefinition<Service> ServiceDef;

      invoke_4(MemFunc memFunc,ServiceDef& serviceDef, 
         IClientTransport& trans        
         ): _memFunc(memFunc),_trans(trans),_serviceDef_1(serviceDef){}

      R operator() (A1 a1, A2 a2, A3 a3, A4 a4)
      {
         CPstream oa;

         // Serialize service name
         oa << _serviceDef_1.getServiceName();
         // Serialize method id
         int methodId = 
            _serviceDef_1.getMethodId(
               IServiceMethod<Service>::getReinterpretFuncAddr((unsigned int)&_memFunc)
               );
         oa << methodId;

         Arg<A1,PrimitiveTraits<A1>::val> arg1(a1,false);
         Arg<A2,PrimitiveTraits<A2>::val> arg2(a2,false);
         Arg<A3,PrimitiveTraits<A3>::val> arg3(a3,false);
         Arg<A4,PrimitiveTraits<A4>::val> arg4(a4,false);
         Arg<R, PrimitiveTraits<R>::val> retVal(false);

         // Regardless in and out argument
         oa << arg1 << arg2 << arg3 << arg4;

         _trans.send(oa.buf(),oa.bufSize());
         
         std::string recvBuf;
         _trans.recv(recvBuf);
         
         CPstream ia((char*)recvBuf.data(), recvBuf.length());
         
         int ret;
         ia >> ret;
		 if (ret != RPC_OK)
            throw ServiceException("Service error", ret);

		 int resMethodId = -1;
		 ia >> resMethodId;

		 if (methodId != resMethodId)
			 throw ServiceException("Service error", ret);

         ia >> arg1 >> arg2 >> arg3 >> arg4;
         
         if (VoidTraits<R>::val == false )
         {
            ia >> retVal;
         } 
         
         return retVal.ref();
      }

   };

  
   template<typename R, typename A1, typename A2, typename A3, typename A4>
   invoke_4</*Service,*/R,A1,A2,A3,A4>  invoke(R (Service::*memFunc)(A1,A2,A3,A4))
   {  
      return invoke_4</*Service,*/R,A1,A2,A3,A4>(memFunc,_serviceDef,_trans);
   }

   /////////////////////////////////////////////////////////////////
   // 
   //   5 parameters invoking
   //

   template</*typename Service, */typename R, typename A1, typename A2, 
      typename A3, typename A4, typename A5>
   class invoke_5 
   {
   private:
      IClientTransport & _trans;
      ServiceDefinition<Service> & _serviceDef_1;
   public:
      typedef R (Service::*MemFunc)(A1,A2,A3,A4,A5);
      MemFunc _memFunc;
      typedef ServiceDefinition<Service> ServiceDef;

      invoke_5(MemFunc memFunc,ServiceDef& serviceDef, 
         IClientTransport& trans        
         ): _memFunc(memFunc),_trans(trans),_serviceDef_1(serviceDef){}

      R operator() (A1 a1, A2 a2, A3 a3, A4 a4, A5 a5)
      {
         CPstream oa;

         // Serialize service name
         oa << _serviceDef_1.getServiceName();
         // Serialize method id
         int methodId = 
            _serviceDef_1.getMethodId(
               IServiceMethod<Service>::getReinterpretFuncAddr((unsigned int)&_memFunc)
               );
         oa << methodId;

         Arg<A1,PrimitiveTraits<A1>::val> arg1(a1,false);
         Arg<A2,PrimitiveTraits<A2>::val> arg2(a2,false);
         Arg<A3,PrimitiveTraits<A3>::val> arg3(a3,false);
         Arg<A4,PrimitiveTraits<A4>::val> arg4(a4,false);
         Arg<A5,PrimitiveTraits<A5>::val> arg5(a5,false);
         Arg<R, PrimitiveTraits<R>::val> retVal(false);

         // Regardless in and out argument
         oa << arg1 << arg2 << arg3 << arg4 << arg5;

         _trans.send(oa.buf(),oa.bufSize());
         
         std::string recvBuf;
         
         _trans.recv(recvBuf);
         
         CPstream ia((char*)recvBuf.data(), recvBuf.length());
         
         int ret;
         ia >> ret;
		 if (ret != RPC_OK)
            throw ServiceException("Service error", ret);

		 int resMethodId = -1;
		 ia >> resMethodId;

		 if (methodId != resMethodId)
			 throw ServiceException("Service error", ret);

         ia >> arg1 >> arg2 >> arg3 >> arg4 >> arg5;
         
         if (VoidTraits<R>::val == false )
         {
            ia >> retVal;
         } 
         
         return retVal.ref();
      }

   };

  
   template<typename R, typename A1, typename A2, 
         typename A3, typename A4, typename A5>
   invoke_5</*Service,*/R,A1,A2,A3,A4,A5>  invoke(R (Service::*memFunc)(A1,A2,A3,A4,A5))
   {  
      return invoke_5</*Service,*/R,A1,A2,A3,A4,A5>(memFunc,_serviceDef,_trans);
   }

   /////////////////////////////////////////////////////////////////
   // 
   //   6 parameters invoking
   //

   template</*typename Service, */typename R, typename A1, typename A2, 
      typename A3, typename A4, typename A5, typename A6>
   class invoke_6 
   {
   private:
      IClientTransport & _trans;
      ServiceDefinition<Service> & _serviceDef_1;
   public:
      typedef R (Service::*MemFunc)(A1,A2,A3,A4,A5,A6);
      MemFunc _memFunc;
      typedef ServiceDefinition<Service> ServiceDef;

      invoke_6(MemFunc memFunc,ServiceDef& serviceDef, 
         IClientTransport& trans        
         ): _memFunc(memFunc),_trans(trans),_serviceDef_1(serviceDef){}

      R operator() (A1 a1, A2 a2, A3 a3, A4 a4, A5 a5, A6 a6)
      {
         CPstream oa;

         // Serialize service name
         oa << _serviceDef_1.getServiceName();
         // Serialize method id
         int methodId = 
            _serviceDef_1.getMethodId(
               IServiceMethod<Service>::getReinterpretFuncAddr((unsigned int)&_memFunc)
               );
         oa << methodId;

         Arg<A1,PrimitiveTraits<A1>::val> arg1(a1,false);
         Arg<A2,PrimitiveTraits<A2>::val> arg2(a2,false);
         Arg<A3,PrimitiveTraits<A3>::val> arg3(a3,false);
         Arg<A4,PrimitiveTraits<A4>::val> arg4(a4,false);
         Arg<A5,PrimitiveTraits<A5>::val> arg5(a5,false);
         Arg<A6,PrimitiveTraits<A6>::val> arg6(a6,false);
         Arg<R, PrimitiveTraits<R>::val> retVal(false);

         // Regardless in and out argument
         oa << arg1 << arg2 << arg3 << arg4 << arg5 << arg6;

         _trans.send(oa.buf(),oa.bufSize());
         
         std::string recvBuf;
         _trans.recv(recvBuf);
         
         CPstream ia((char*)recvBuf.data(), recvBuf.length());
         
         int ret;
         ia >> ret;
		 if (ret != RPC_OK)
            throw ServiceException("Service error", ret);

		 int resMethodId = -1;
		 ia >> resMethodId;

		 if (methodId != resMethodId)
			 throw ServiceException("Service error", ret);

         ia >> arg1 >> arg2 >> arg3 >> arg4 >> arg5 >> arg6;

         if (VoidTraits<R>::val == false )
         {
            ia >> retVal;
         } 
         
         return retVal.ref();
      }

   };

  
   template<typename R, typename A1, typename A2, 
         typename A3, typename A4, typename A5, typename A6>
   invoke_6</*Service,*/R,A1,A2,A3,A4,A5,A6>  invoke(R (Service::*memFunc)(A1,A2,A3,A4,A5,A6))
   {  
      return invoke_6</*Service,*/R,A1,A2,A3,A4,A5,A6>(memFunc,_serviceDef,_trans);
   }

   /////////////////////////////////////////////////////////////////
   // 
   //   7 parameters invoking
   //

   template</*typename Service, */typename R, typename A1, typename A2, 
      typename A3, typename A4, typename A5, typename A6, typename A7>
   class invoke_7 
   {
   private:
      IClientTransport & _trans;
      ServiceDefinition<Service> & _serviceDef_1;
   public:
      typedef R (Service::*MemFunc)(A1,A2,A3,A4,A5,A6,A7);
      MemFunc _memFunc;
      typedef ServiceDefinition<Service> ServiceDef;

      invoke_7(MemFunc memFunc,ServiceDef& serviceDef, 
         IClientTransport& trans        
         ): _memFunc(memFunc),_trans(trans),_serviceDef_1(serviceDef){}

      R operator() (A1 a1, A2 a2, A3 a3, A4 a4, A5 a5, A6 a6, A7 a7)
      {
         CPstream oa;

         // Serialize service name
         oa << _serviceDef_1.getServiceName();
         // Serialize method id
         int methodId = 
            _serviceDef_1.getMethodId(
               IServiceMethod<Service>::getReinterpretFuncAddr((unsigned int)&_memFunc)
               );
         oa << methodId;

         Arg<A1,PrimitiveTraits<A1>::val> arg1(a1,false);
         Arg<A2,PrimitiveTraits<A2>::val> arg2(a2,false);
         Arg<A3,PrimitiveTraits<A3>::val> arg3(a3,false);
         Arg<A4,PrimitiveTraits<A4>::val> arg4(a4,false);
         Arg<A5,PrimitiveTraits<A5>::val> arg5(a5,false);
         Arg<A6,PrimitiveTraits<A6>::val> arg6(a6,false);
         Arg<A7,PrimitiveTraits<A7>::val> arg7(a7,false);
         Arg<R, PrimitiveTraits<R>::val> retVal(false);

         // Regardless in and out argument
         oa << arg1 << arg2 << arg3 << arg4 << arg5 << arg6 << arg7;

         _trans.send(oa.buf(),oa.bufSize());
         
         std::string recvBuf;
         _trans.recv(recvBuf);
        
         CPstream ia((char*)recvBuf.data(), recvBuf.length());
         
         int ret;
         ia >> ret;
		 if (ret != RPC_OK)
            throw ServiceException("Service error", ret);

		 int resMethodId = -1;
		 ia >> resMethodId;

		 if (methodId != resMethodId)
			 throw ServiceException("Service error", ret);

         ia >> arg1 >> arg2 >> arg3 >> arg4 >> arg5 >> arg6 >> arg7;
         
         if (VoidTraits<R>::val == false )
         {
            ia >> retVal;
         } 
         
         return retVal.ref();
      }

   };

  
   template<typename R, typename A1, typename A2, 
         typename A3, typename A4, typename A5, typename A6, typename A7>
   invoke_7</*Service,*/R,A1,A2,A3,A4,A5,A6,A7>  invoke(R (Service::*memFunc)(A1,A2,A3,A4,A5,A6,A7))
   {  
      return invoke_7</*Service,*/R,A1,A2,A3,A4,A5,A6,A7>(memFunc,_serviceDef,_trans);
   }

   /////////////////////////////////////////////////////////////////
   // 
   //   8 parameters invoking
   //

   template</*typename Service, */typename R, typename A1, typename A2, 
      typename A3, typename A4, typename A5, typename A6, typename A7, typename A8>
   class invoke_8 
   {
   private:
      IClientTransport & _trans;
      ServiceDefinition<Service> & _serviceDef_1;
   public:
      typedef R (Service::*MemFunc)(A1,A2,A3,A4,A5,A6,A7,A8);
      MemFunc _memFunc;
      typedef ServiceDefinition<Service> ServiceDef;

      invoke_8(MemFunc memFunc,ServiceDef& serviceDef, 
         IClientTransport& trans        
         ): _memFunc(memFunc),_trans(trans),_serviceDef_1(serviceDef){}

      R operator() (A1 a1, A2 a2, A3 a3, A4 a4, A5 a5, A6 a6, A7 a7, A8 a8)
      {
         CPstream oa;

         // Serialize service name
         oa << _serviceDef_1.getServiceName();
         // Serialize method id
         int methodId = 
            _serviceDef_1.getMethodId(
               IServiceMethod<Service>::getReinterpretFuncAddr((unsigned int)&_memFunc)
               );
         oa << methodId;

         Arg<A1,PrimitiveTraits<A1>::val> arg1(a1,false);
         Arg<A2,PrimitiveTraits<A2>::val> arg2(a2,false);
         Arg<A3,PrimitiveTraits<A3>::val> arg3(a3,false);
         Arg<A4,PrimitiveTraits<A4>::val> arg4(a4,false);
         Arg<A5,PrimitiveTraits<A5>::val> arg5(a5,false);
         Arg<A6,PrimitiveTraits<A6>::val> arg6(a6,false);
         Arg<A7,PrimitiveTraits<A7>::val> arg7(a7,false);
         Arg<A8,PrimitiveTraits<A8>::val> arg8(a8,false);

         Arg<R, PrimitiveTraits<R>::val> retVal(false);

         // Regardless in and out argument
         oa << arg1 << arg2 << arg3 << arg4 << arg5 << arg6 << arg7 << arg8;

         _trans.send(oa.buf(),oa.bufSize());
         
         std::string recvBuf;
         _trans.recv(recvBuf);
         
         CPstream ia((char*)recvBuf.data(), recvBuf.length());
         
         int ret;
         ia >> ret;
		 if (ret != RPC_OK)
            throw ServiceException("Service error", ret);

		 int resMethodId = -1;
		 ia >> resMethodId;

		 if (methodId != resMethodId)
			 throw ServiceException("Service error", ret);

         ia >> arg1 >> arg2 >> arg3 >> arg4 >> arg5 >> arg6 >> arg7 >> arg8;
         
         if (VoidTraits<R>::val == false )
         {
            ia >> retVal;
         } 
         
         return retVal.ref();
      }

   };

  
   template<typename R, typename A1, typename A2, 
         typename A3, typename A4, typename A5, typename A6, typename A7, typename A8>
   invoke_8</*Service,*/R,A1,A2,A3,A4,A5,A6,A7,A8>  invoke(R (Service::*memFunc)(A1,A2,A3,A4,A5,A6,A7,A8))
   {  
      return invoke_8</*Service,*/R,A1,A2,A3,A4,A5,A6,A7,A8>(memFunc,_serviceDef,_trans);
   }

private:
   IClientTransport & _trans;
   ServiceDefinition<Service> & _serviceDef;
};

#endif
