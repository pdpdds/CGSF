/* -*- C++ -*- */


//=============================================================================
/**
 *  @file    Reactor_Singleton.h
 *
 *  $Id: Reactor_Singleton.h 93639 2011-03-24 13:32:13Z johnnyw $
 *
 *  @author Doug Schmidt
 */
//=============================================================================


#ifndef _REACTOR_SINGLETON_H
#define _REACTOR_SINGLETON_H

#include "ace/Singleton.h"
#include "ace/Null_Mutex.h"

#if !defined (ACE_LACKS_PRAGMA_ONCE)
# pragma once
#endif /* ACE_LACKS_PRAGMA_ONCE */

#include "ace/Reactor.h"

// Our global Reactor Singleton.
typedef ACE_Singleton<ACE_Reactor, ACE_Null_Mutex> REACTOR;

#endif /* _REACTOR_SINGLETON_H */
