/* -*- C++ -*- */

//=============================================================================
/**
 *  @file    SMR_Client.h
 *
 *  $Id: SMR_Client.h 93651 2011-03-28 08:49:11Z johnnyw $
 *
 *  @author Douglas C. Schmidt
 */
//=============================================================================


#ifndef _SMR_CLIENT_H
#define _SMR_CLIENT_H

#include "SM_Client.h"

class SMR_Client : public SM_Client
{
public:
  SMR_Client (short port_number);
  virtual ~SMR_Client (void);
};

#endif /* _SMR_CLIENT_H */
