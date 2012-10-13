/* -*- C++ -*- */

//=============================================================================
/**
 *  @file    URL_Status.h
 *
 *  $Id: URL_Status.h 93639 2011-03-24 13:32:13Z johnnyw $
 *
 *  @author Douglas C. Schmidt <schmidt@cs.wustl.edu>
 */
//=============================================================================


#ifndef _URL_STATUS_H
#define _URL_STATUS_H

#include "ace/config-all.h"

#if !defined (ACE_LACKS_PRAGMA_ONCE)
#pragma once
#endif /* ACE_LACKS_PRAGMA_ONCE */

/**
 * @class URL_Status
 *
 */
class URL_Status
{
public:
  enum STATUS_CODE
  {
    STATUS_OK = 200,
    STATUS_CREATED = 201,
    STATUS_ACCEPTED = 202,
    STATUS_NO_CONTENT = 204,
    STATUS_MOVED_PERMANENTLY = 301,
    STATUS_MOVED_TEMPORARILY = 302,
    STATUS_NOT_MODIFIED = 304,
    STATUS_BAD_REQUEST = 400,
    STATUS_UNAUTHORIZED = 401,
    STATUS_FORBIDDEN = 403,
    STATUS_ITEM_NOT_FOUND = 404,
    STATUS_INTERNAL_SERVER_ERROR = 500,
    STATUS_OP_NOT_IMPLEMENTED = 501,
    STATUS_BAD_GATEWAY = 502,
    STATUS_SERVICE_UNAVAILABLE = 503,
    STATUS_INSUFFICIENT_DATA = 399
  };

  URL_Status (STATUS_CODE = STATUS_INSUFFICIENT_DATA);
  URL_Status (const URL_Status &);

  STATUS_CODE status (void) const;
  void status (int);
  void status (STATUS_CODE);
  int destroy (void);
private:
  STATUS_CODE status_;
};

#endif /* _URL_STATUS_H */
