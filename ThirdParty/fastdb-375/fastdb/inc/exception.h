//-< EXCEPTION.H >---------------------------------------------------*--------*
// FastDB                    Version 1.0         (c) 1999  GARRET    *     ?  *
// (Main Memory Database Management System)                          *   /\|  *
//                                                                   *  /  \  *
//                          Created:     3-Oct-99 Sebastiano Suraci  * / [] \ *
//                          Last update: 5-Oct-99 K.A. Knizhnik      * GARRET *
//-------------------------------------------------------------------*--------*
// Database exception 
//-------------------------------------------------------------------*--------*

#ifndef __EXCEPTION_H__
#define __EXCEPTION_H__

#include <exception>

BEGIN_FASTDB_NAMESPACE

#ifdef FASTDB_DLL
class __declspec(dllexport)  std::exception;
#endif

class FASTDB_DLL_ENTRY dbException  : public std::exception
{
  protected:
    int   err_code;
    char* msg; 
    int   arg;
    
  public:
    dbException(int p_err_code, char const* p_msg = NULL, int p_arg = 0);
    dbException(dbException const& ex);
    
    virtual~dbException() throw (); 
    
    virtual const char *what() const throw();

    int   getErrCode() const { return err_code; }
    char* getMsg() const     { return msg; }
    long  getArg() const     { return arg; }
};

END_FASTDB_NAMESPACE

#endif
