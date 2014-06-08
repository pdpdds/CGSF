#ifndef __SERVICE_DEFINITION_TEMPLATE
#define __SERVICE_DEFINITION_TEMPLATE

#include <string>
#include <map>

#include "service_method.h"

template <class Service>
class ServiceDefinitionBase
{
public:
   
   const std::string& getServiceName()
   {
      return _serviceName;
   }
   int getMethodId(unsigned int funcIntAddr)
   {
      std::map<unsigned int,int>::iterator it = _addr_id_map.find(funcIntAddr);
      if (it == _addr_id_map.end() )
         return -1;
      return (it->second);
   }

   IServiceMethod<Service>* getMethodById(int id)
   {
      std::map<int, IServiceMethod<Service> *>::iterator it;
      it = _methodReg.find(id);
      if (it == _methodReg.end())
         return NULL;
      return (it->second);
   }

   ~ServiceDefinitionBase()
   {
      
      std::map<int, IServiceMethod<Service> *>::iterator it = _methodReg.begin();
      while (it != _methodReg.end())
      {
         delete (it->second);
         it++;
      }

   }
protected:
   ServiceDefinitionBase(const std::string& serviceName):_serviceName(serviceName){}
   // key: mem func int address, val: id
   std::map<unsigned int,int> _addr_id_map;
   // key: id, val: IServiceMethod<...>*
   std::map<int, IServiceMethod<Service> *> _methodReg;
   
   typedef std::pair<unsigned int,int> AddrMapPair;
   typedef std::pair<int, IServiceMethod<Service> * > MethodMapPair;

   std::string _serviceName;
};



#define BEGIN_DEFINE_SERVICE(Service) \
template<class Service> \
class ServiceDefinition: public ServiceDefinitionBase<Service> \
{\
public:\
   ServiceDefinition():ServiceDefinitionBase<Service>(#Service)\
   {\
      IServiceMethod<Service>* tmpMethod = NULL;\
      int initId = 0;\
      int curId = 0;

#define EXPOSE_METHOD(Service,method) \
      tmpMethod = getServiceMethod(&Service::method);\
      curId = initId++;\
      _addr_id_map.insert(AddrMapPair(tmpMethod->getMethodIntAddr(),curId));\
      _methodReg.insert(MethodMapPair(curId,tmpMethod));

#define END_DEFINE_SERVICE \
   }\
};


#endif