/* -*- C++ -*- */

//=============================================================================
/**
 *  @file    SL_Client.h
 *
 *  $Id: SL_Client.h 93651 2011-03-28 08:49:11Z johnnyw $
 *
 *  @author Douglas C. Schmidt
 */
//=============================================================================


#ifndef _SL_CLIENT_H
#define _SL_CLIENT_H

#include "Single_Lookup.h"

/**
 * @class SL_Client
 *
 * @brief Provides the client's single user lookup table abstraction.
 */
class SL_Client : public Single_Lookup
{

public:
  SL_Client (const char *key_name);
  virtual Protocol_Record *insert (const char *key_name,
                                   int max_len = MAXUSERIDNAMELEN);
};

#endif /* _SL_CLIENT_H */
