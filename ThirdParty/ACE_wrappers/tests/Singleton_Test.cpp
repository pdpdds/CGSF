
//=============================================================================
/**
 *  @file    Singleton_Test.cpp
 *
 *  $Id: Singleton_Test.cpp 93638 2011-03-24 13:16:05Z johnnyw $
 *
 *    This is a simple test of ACE Singleton
 *
 *
 *  @author Johnny Willemsen  <jwillemsen@remedy.nl>
 */
//=============================================================================


#include "test_config.h"
#include "ace/Singleton.h"



int
run_main (int, ACE_TCHAR *[])
{
  ACE_START_TEST (ACE_TEXT ("Singleton_Test"));

  int *i = ACE_Singleton <int, ACE_SYNCH_RECURSIVE_MUTEX>::instance ();
  *i = 3;
  ACE_Singleton <int, ACE_SYNCH_RECURSIVE_MUTEX>::close ();

  ACE_END_TEST;

  return 0;
}

