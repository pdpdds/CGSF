#ifndef __SERVICE_ERR_H
#define __SERVICE_ERR_H

#include  <stdexcept>
#include  <string>
#include <sstream>

using namespace std;


class ServiceException : public exception
{
public:
   ServiceException() throw( ) : exception(), _errCode(0)
   {}
   ServiceException(const ServiceException& right) throw( )
      : exception(right), _reason(right.what()), _errCode(right._errCode)
   {}

   ServiceException& operator=(const ServiceException& right) throw( )
   {
      _reason = right.what();
      _errCode = right._errCode;
      return *this;
   }

   virtual ~ServiceException( ) throw( ){}
   virtual const char *what( ) const throw( ) { return _reason.c_str(); }
public:
   ServiceException (const string & errInfo, int errCode): exception(), _errCode(errCode)
   {
      stringstream strm;
      strm << "Error:"<<errInfo<<", Error Code:"<< errCode;
      _reason = strm.str();
   }
   
   ServiceException (const string & errInfo): exception(), _errCode(0)
   {
      stringstream strm;
      strm << "Error:"<<errInfo;
      _reason = strm.str();
   }

   int getErrCode() {
      return _errCode;
   }

private:
   string _reason;
   int _errCode;

};

#endif