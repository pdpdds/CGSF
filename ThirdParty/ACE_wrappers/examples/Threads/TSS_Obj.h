
//=============================================================================
/**
 *  @file    TSS_Obj.h
 *
 *  $Id: TSS_Obj.h 93639 2011-03-24 13:32:13Z johnnyw $
 *
 *   This program tests various features of ACE_Thread and the
 *   thread-specific storage variant of <ACE_SingletonEx>.
 *
 *
 *  @author Prashant Jain and Doug Schmidt
 */
//=============================================================================


#include "ace/Atomic_Op.h"
#include "ace/Synch_Traits.h"

#if !defined (ACE_LACKS_PRAGMA_ONCE)
# pragma once
#endif /* ACE_LACKS_PRAGMA_ONCE */

/**
 * @class TSS_Obj
 *
 * @brief This object is stored in thread-specific storage.
 */
class TSS_Obj
{
public:
  TSS_Obj (void);
  ~TSS_Obj (void);

private:

 static ACE_Atomic_Op<ACE_SYNCH_MUTEX, int> count_;
};

