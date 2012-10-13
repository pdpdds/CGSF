/* -*- C++ -*- */

//=============================================================================
/**
 *  @file    PMS_Flo.h
 *
 *  $Id: PMS_Flo.h 93651 2011-03-28 08:49:11Z johnnyw $
 *
 *  @author Douglas C. Schmidt
 */
//=============================================================================


#ifndef _PMS_FLO_H
#define _PMS_FLO_H

#include "PM_Server.h"

/**
 * @class PMS_Flo
 *
 * @brief Provides the server's lookup table abstraction for `flo' users...
 */
class PMS_Flo : public PM_Server
{

public:
  PMS_Flo (void);

protected:
  virtual int encode (char *packet, int &total_bytes);
  virtual int decode (char *packet, int &total_bytes);
};

#endif /* _PMS_FLO_H */
