/* -*- C++ -*- */

//=============================================================================
/**
 *  @file    HT_Client.h
 *
 *  $Id: HT_Client.h 93651 2011-03-28 08:49:11Z johnnyw $
 *
 *  @author Douglas C. Schmidt
 */
//=============================================================================


#ifndef _HT_CLIENT_H
#define _HT_CLIENT_H

#include "Hash_Table.h"

/**
 * @class HT_Client
 *
 * @brief Provides the client's hash table abstraction.
 */
class HT_Client : public Hash_Table
{
public:
  virtual Protocol_Record *insert (const char *key_name,
                                   int max_len = MAXUSERIDNAMELEN);
};

#endif /* _HT_CLIENT_H */
