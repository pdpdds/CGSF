//-< FASTDB.H >------------------------------------------------------*--------*
// FastDB                    Version 1.0         (c) 1999  GARRET    *     ?  *
// (Main Memory Database Management System)                          *   /\|  *
//                                                                   *  /  \  *
//                          Created:     20-Nov-98    K.A. Knizhnik  * / [] \ *
//                          Last update: 10-Dec-98    K.A. Knizhnik  * GARRET *
//-------------------------------------------------------------------*--------*
// Main header file

//-------------------------------------------------------------------*--------*

#ifndef __FASTDB_H__
#define __FASTDB_H__

#define FASTDB_MAJOR_VERSION 3
#define FASTDB_MINOR_VERSION 73
#define FASTDB_VERSION (FASTDB_MAJOR_VERSION*100 + FASTDB_MINOR_VERSION)

#include "database.h"
#include "array.h"
#include "query.h"
#include "cursor.h"
#include "datetime.h"
#include "container.h"

#ifdef THROW_EXCEPTION_ON_ERROR
#include "exception.h"
#endif

#endif
