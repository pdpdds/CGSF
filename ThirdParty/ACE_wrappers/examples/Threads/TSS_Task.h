/* -*- C++ -*- */


//=============================================================================
/**
 *  @file    TSS_Task.h
 *
 *  $Id: TSS_Task.h 93639 2011-03-24 13:32:13Z johnnyw $
 *
 *  @author Prashant Jain and Doug Schmidt
 */
//=============================================================================


#include "ace/config-all.h"

#if !defined (ACE_LACKS_PRAGMA_ONCE)
# pragma once
#endif /* ACE_LACKS_PRAGMA_ONCE */

#include "ace/Token.h"
#include "ace/Atomic_Op.h"

class Test_Task
{
public:

  Test_Task (void);
  ~Test_Task (void);

  int open (void *arg);

  static void *svc (void *arg);

  static ACE_Atomic_Op<ACE_Token, int> wait_count_;
  static ACE_Atomic_Op<ACE_Token, int> max_count_;

private:
  static ACE_Atomic_Op<ACE_Token, int> count_;
};
