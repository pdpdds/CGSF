/* -*- C++ -*- */

//=============================================================================
/**
 *  @file    PMS_Usr.h
 *
 *  $Id: PMS_Usr.h 93651 2011-03-28 08:49:11Z johnnyw $
 *
 *  @author Douglas C. Schmidt
 */
//=============================================================================


#ifndef _PMS_USR_H
#define _PMS_USR_H

#include "PM_Server.h"

/**
 * @class PMS_Usr
 *
 * @brief Provides the client's lookup table abstraction for `Usr' users...
 */
class PMS_Usr : public PM_Server
{

public:
  PMS_Usr (void);

protected:
  virtual int encode (char *packet, int &total_bytes);
  virtual int decode (char *packet, int &total_bytes);
};

#endif /* _PMS_USR_H */
