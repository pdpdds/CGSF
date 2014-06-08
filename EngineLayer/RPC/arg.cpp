#include "stdafx.h"
#include "arg.hpp"

template <class A, bool IsPrimitive> int Arg<A, IsPrimitive>::s_iRegisterNo = 
	CPstream::_RegisterClass_ (Arg<A, IsPrimitive>::CreateObject) ; 

template <class A> 
int Arg <A&,false> ::s_iRegisterNo = 
	CPstream::_RegisterClass_ (Arg <A&,false>::CreateObject) ; 

template <class A> 
int Arg <A*,false> ::s_iRegisterNo = 
	CPstream::_RegisterClass_ (Arg <A*,false>::CreateObject) ; 

template <class A> 
int Arg <A*,true> ::s_iRegisterNo = 
	CPstream::_RegisterClass_ (Arg <A*,true>::CreateObject) ; 

template <class A> 
int Arg <A&,true> ::s_iRegisterNo = 
	CPstream::_RegisterClass_ (Arg <A&,true>::CreateObject) ; 
/*
template <class A> 
int Arg <A*,true> ::s_iRegisterNo = 
	CPstream::_RegisterClass_ (Arg <A*,true>::CreateObject) ; 
*/

//template <> 
int Arg <char*,true> ::s_iRegisterNo = 
	CPstream::_RegisterClass_ (Arg <char*,true>::CreateObject) ; 

//template <> 
int Arg <void,true> ::s_iRegisterNo = 
	CPstream::_RegisterClass_ (Arg<void,true>::CreateObject) ; 