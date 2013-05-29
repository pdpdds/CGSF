#include "stdafx.h"
#include "CrashHandler.h"
#include <stdio.h>
#include <conio.h>
#include <stdlib.h>
#include <new.h>
#include <signal.h>
#include <exception>
#include <sys/stat.h>
#include <psapi.h>
#include <rtcapi.h>
#include <Shellapi.h>
#include <dbghelp.h>

#include <float.h>
void sigfpe_test()
{ 
    // Code taken from http://www.devx.com/cplus/Article/34993/1954

    //Set the x86 floating-point control word according to what
    //exceptions you want to trap. 
    _clearfp(); //Always call _clearfp before setting the control
    //word
    //Because the second parameter in the following call is 0, it
    //only returns the floating-point control word
    unsigned int cw; 
    _controlfp_s(&cw, 0, 0); //Get the default control
    //word
    //Set the exception masks off for exceptions that you want to
    //trap.  When a mask bit is set, the corresponding floating-point
    //exception is //blocked from being generating.
    cw &=~(EM_OVERFLOW|EM_UNDERFLOW|EM_ZERODIVIDE|
        EM_DENORMAL|EM_INVALID);
    //For any bit in the second parameter (mask) that is 1, the 
    //corresponding bit in the first parameter is used to update
    //the control word.  
    unsigned int cwOriginal;
    _controlfp_s(&cwOriginal, cw, MCW_EM); //Set it.
    //MCW_EM is defined in float.h.
    //Restore the original value when done:
    //_controlfp(cwOriginal, MCW_EM);

    // Divide by zero

    float a = 1;
    float b = 0;
    float c = a/b;
    c; 
}

#define BIG_NUMBER 0x1fffffff
#pragma warning(disable: 4717) // avoid C4717 warning
int RecurseAlloc() 
{
    int *pi = new int[BIG_NUMBER];
    pi;
    RecurseAlloc();
    return 0;
}

class CDerived;
class CBase
{
public:
    CBase(CDerived *derived): m_pDerived(derived) {};
    ~CBase();
    virtual void function(void) = 0;

    CDerived * m_pDerived;
};

#pragma warning(disable:4355)
class CDerived : public CBase
{
public:
    CDerived() : CBase(this) {};   // C4355
    virtual void function(void) {};
};

CBase::~CBase()
{
    m_pDerived -> function();
}

void main()
{
	CCrashHandler ch;
	ch.SetProcessExceptionHandlers();
	ch.SetThreadExceptionHandlers();
	
	printf("Choose an exception type:\n");
	printf("0 - SEH exception\n");
	printf("1 - terminate\n");
	printf("2 - unexpected\n");
	printf("3 - pure virtual method call\n");
	printf("4 - invalid parameter\n");
	printf("5 - new operator fault\n");	
	printf("6 - SIGABRT\n");
	printf("7 - SIGFPE\n");
	printf("8 - SIGILL\n");
	printf("9 - SIGINT\n");
	printf("10 - SIGSEGV\n");
	printf("11 - SIGTERM\n");
	printf("12 - RaiseException\n");
	printf("13 - throw C++ typed exception\n");
	printf("Your choice >  ");

	int ExceptionType = 0;
	scanf_s("%d", &ExceptionType);

	switch(ExceptionType)
    {
    case 0: // SEH
        {
            // Access violation
            int *p = 0;
#pragma warning(disable : 6011)   // warning C6011: Dereferencing NULL pointer 'p'
            *p = 0;
#pragma warning(default : 6011)   
        }
        break;
    case 1: // terminate
        {
            // Call terminate
            terminate();
        }
        break;
    case 2: // unexpected
        {
            // Call unexpected
            unexpected();
        }
        break;
    case 3: // pure virtual method call
        {
            // pure virtual method call
            CDerived derived;
        }
        break;
    case 4: // invalid parameter
        {      
            char* formatString;
            // Call printf_s with invalid parameters.
            formatString = NULL;
#pragma warning(disable : 6387)   // warning C6387: 'argument 1' might be '0': this does not adhere to the specification for the function 'printf'
            printf(formatString);
#pragma warning(default : 6387)   

        }
        break;
    case 5: // new operator fault
        {
            // Cause memory allocation error
            RecurseAlloc();
        }
        break;
    case 6: // SIGABRT 
        {
            // Call abort
            abort();
        }
        break;
    case 7: // SIGFPE
        {
            // floating point exception ( /fp:except compiler option)
            sigfpe_test();            
        }    
		break;
    case 8: // SIGILL 
        {
            raise(SIGILL);              
        }    
		break;
    case 9: // SIGINT 
        {
            raise(SIGINT);              
        }    
		break;
    case 10: // SIGSEGV 
        {
            raise(SIGSEGV);              
        }    
		break;
    case 11: // SIGTERM
        {
            raise(SIGTERM);            
        }
		break;
    case 12: // RaiseException 
        {
            // Raise noncontinuable software exception
            RaiseException(123, EXCEPTION_NONCONTINUABLE, 0, NULL);        
        }
        break;
    case 13: // throw 
        {
            // Throw typed C++ exception.
            throw 13;
        }
        break;
    default:
        {
            printf("Unknown exception type specified.");       
			_getch();
        }
        break;
    }
	
}


