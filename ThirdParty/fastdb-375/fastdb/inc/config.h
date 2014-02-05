//-< CONFIG.H >------------------------------------------------------*--------*
// FastDB                    Version 1.0         (c) 1999  GARRET    *     ?  *
// (Post Relational Database Management System)                      *   /\|  *
//                                                                   *  /  \  *
//                          Created:     21-Jan-2004  K.A. Knizhnik  * / [] \ *
//                          Last update: 21-Jan-2004  K.A. Knizhnik  * GARRET *
//-------------------------------------------------------------------*--------*
// FastDB configuration definitions
//-------------------------------------------------------------------*--------*

#ifndef __CONFIG_H__
#define __CONFIG_H__

#ifdef HAVE_CONFIG_H
#include "acconfig.h"

#else // not HAVE_CONFIG_H

#if (defined(_WIN32) || defined(_WINCE)) && (!defined(_CRT_SECURE_NO_DEPRECATE))
#define _CRT_SECURE_NO_DEPRECATE true
#endif

#ifdef VXWORKS
#ifndef UNIX
#define UNIX 1
#endif
#endif // VXWORKS

// USE_LOCALE_SETTINGS - use C locale for string comparison operations
#define USE_LOCALE_SETTINGS 1

// FASTDB_DEBUG - debug level
//    - DEBUG_NONE - disable any checking and tracing (except asserts)
//    - DEBUG_CHECK - disable trace message
//    - DEBUG_TRACE_IMPORTANT - enable trace of important messages 
//    - DEBUG_TRACE_ALL - enable trace of all messages 
#define FASTDB_DEBUG DEBUG_TRACE_IMPORTANT
//#define FASTDB_DEBUG DEBUG_TRACE_ALL

// USE_NAMESPACES - place FastDB classes in separate namespace  
// #define USE_NAMESPACES 1

// SECURE_SERVER - enable authentication for remote logins: keep user/password table, 
//   check password for remote logins
//#define SECURE_SERVER 1

// USE_QUEUE_MANAGER - use queue manager for internal HTTP server. 
//   This manager will manage pool of threads assigned for client connections. 
//   Otherwise all requests will be executed sequnetially in the main loop.
//#define USE_QUEUE_MANAGER 1


// FASTDB_DLL - create fastdb.dll
//#define FASTDB_DLL 1


// THROW_EXCEPTION_ON_ERROR - throw C++ exception in case of database error instead of abort()
#ifndef _WINCE
#define THROW_EXCEPTION_ON_ERROR 1
#endif


//IGNORE_CASE - perform all string comparisons as case insensitive
//#define IGNORE_CASE 1

//USE_STD_STRING - accept std::string class as table field type
#define USE_STD_STRING 1

//AUTOINCREMENT_SUPPORT - support autoincrement fields 
//  (database built with this flag will be incompatible with database built without it)
#define AUTOINCREMENT_SUPPORT 1

//CLONE_IDENTIFIERS - do not storef addresses of string constants in symbol table.
//  This option is needed if DLL library using FastDB can be unloaded.
//#define CLONE_IDENTIFIERS 1 

#ifdef VXWORKS
#define DISKLESS_CONFIGURATION 1
#endif // VXWORKS
//DISKLESS_CONFIGURATION - only in-memory temporary database
//#define DISKLESS_CONFIGURATION 1

// RECTANGLE_DIMENSION - dimension of built-in rectangle type
#define RECTANGLE_DIMENSION 2

// RECTANGLE_COORDINATE_TYPE - type of rectanlge's coordinates
#define RECTANGLE_COORDINATE_TYPE int
//#define RECTANGLE_COORDINATE_TYPE double

// RECTANGLE_AREA_TYPE - type of rectanlge's area
#define RECTANGLE_AREA_TYPE db_int8
//#define RECTANGLE_AREA_TYPE double

//SET_NULL_DACL - use NULL DACL security descriptor for all synchronization objects.
//#define SET_NULL_DACL 1

//INT8_IS_DEFINED - int8 type is defined at your system, in this case you should use db_int8 type instead
//#define INT8_IS_DEFINED 1

#ifdef VXWORKS
#define USE_POSIX_SEMAPHORES 1
#endif // VXWORKS
//USE_POSIX_SEMAPHORES use POSIX sem_* sempahores instead of SysV semaphores
//     instead of SysV semaphores and shared memory
//#define USE_POSIX_SEMAPHORES 1

// Use process shared pthread mutexes (PTHREAD_PROCESS_SHARED) instead of Sys-V semaphores.
// It provides 3-5 times increase of performance in case of large number of concurrent transactions.
// Please notice that process shared semaphreos are not supported by some OSes, for example by Mac OS-X
//#define USE_SHARED_PTHREAD_MUTEX 1
#if defined(__QNX__) || defined(__linux__)
#define USE_SHARED_PTHREAD_MUTEX 1
#endif

//USE_POSIX_MMAP - if 1 then use Posix mmap for mapping database file and monitor, 
//  if 0 - use SysV IPC shmem for allocating memory for database file and monitor,
//  if not defined - use mmap for mapping database file and shmem for allocating monitor
//#define USE_POSIX_MMAP 0

//REPLICATION_SUPPORT - fault tolerant version of FastDB
//#define REPLICATION_SUPPORT 1

#ifdef VXWORKS
#define NO_MMAP 1
#endif
//NO_MMAP do not use OS mappig of file on virtual memory. FastDB will track modification of
//  pages itself and save dirty pages in the file. If USE_POSIX_MMAP=0, memory for database is
//  allocated using shmat() and database can be shared by several processes, 
//  otherwise - using valloc() and database can be accessed only by one process. 
//#define NO_MMAP 1

//FUZZY_CHECKPOINT allows to increase application performance, by performing writing to the file in 
//  in a separate thread. In this case commit will not be blocked until all changes will be actually written to the disk,
//  write requests will be just linked into the queue from which them will be taken by writting thread and proceeded
//  in the same order as in commit. This apporach provides daatbase consistency in case of fault, but results
//  of several recently committed transaction can be lost in this case.
//  Fuzzy checkpointing works only in conjunction with NO_MMAP option, it means that data sharing is not allowed
//  in this case - database can be accessed only by one application.
//#define FUZZY_CHECKPOINT 1


//USE_MFC - use MFC (include "afx.h" instead of "windows.h")
//#define USE_MFC 1

//USE_ATL - use Microsoft ATL 
//#define USE_ATL 1

// LARGE_DATABASE_SUPPORT - support databases with size larger than 4Gb.
// If this macro is defined FastDB maximal database size is limited by 1 terrabyte.
// Defnining this macro increase iniitial database size and object index size (because
// 64-bit offsetss are used instead fo 4-byte offsets). Also FastDB library compiled with 
// LARGE_DATABASE_SUPPOR will not be able to work with database created by library
// built without LARGE_DATABASE_SUPPOR and visa versa. 
#if !defined(_WIN32) || defined(_WIN64) // most unixes are now 64-bit, while 32-bit windows is still quite popular
#define LARGE_DATABASE_SUPPORT 
#endif

// dbDatabaseOidBits number of bits used to represent object identifier. 
// So the number of objects in database is limited by 2**dbDatabaseOidBits.
// Default value of this parameter is 32, so maximal number of objects is limited by 2**32.
// The value of this parameter is used to estimate size of bitmap, so do not assign unnecessarily 
// large values (for example 64)
//#define dbDatabaseOidBits 33

// Do not use member templates
#if defined(__SUNPRO_CC_COMPAT) && __SUNPRO_CC_COMPAT == 4
#define  NO_MEMBER_TEMPLATES
#endif

// Automatically detect and recover crash of one or more database clients.
// FastDB will start separate watchdog threads which will check if other processes working with database are 
// alive.
// It is not recommended to use this mode because there are many cases when it works incorrectly.
// Preferable way of handling process failure is using of RECOVERABLE_CRITICAL_SECTION.
//#define AUTO_DETECT_PROCESS_CRASH 1

// Use recoverable critical section (operating system is able to release 
// critical section in case of crash of the process entered this critical section)
//#define RECOVERABLE_CRITICAL_SECTION 1

// Do not flush chages to the disk during transaction commit. This option will greatly increase performance
// but can cause database corruption in case of crash
//#define NO_FLUSH_ON_COMMIT 1

// dbDatabaseOidBits number of bits used to represent object identifier. 
// So the number of objects in database is limited by 2**dbDatabaseOidBits.
// Default value of this parameter is 32, so maximal number of objects is limited by 2**32.
// Support of larger number of objects is possible only at 64-bit OS
// The value of this parameter is used to estimate size of bitmap, so do not assign unnecessarily 
// large values (for example 64)
//#define dbDatabaseOidBits 33

// dbDatabaseOffsetBits number of bits used to represent object offset in the storage
// So the maximal database size is limited by 2**dbDatabaseOidBits.
// Default value of this parameter is 32, so maximal number of objects is limited by 2**32.
// Support of larger database size only at 64-bit OS
// The value of this parameter is used to estimate size of bitmap, so do not assign unnecessarily 
// large values (for example 64)
//#define dbDatabaseOffsetBits 38

// USE_REGEX - use regex library. When this macro is defined FastDB provides
// MATCH command. GNU regex or compatible library and headers should be available.
//#define USE_REGEX true

// Calling convention used for CLI callback functions
// You should use stdcall convention if you want to use CSharp CLI API
//#define CLI_CALLBACK_CC __stdcall

// SOCK_LINGER - set SO_LINGER option. Value of SO_LINGER is specified using LINGER_TIME
// #define SOCK_LINGER 1


// Use reentrant version of localtime
#if !defined(__APPLE__) && !defined(_WIN32) && !defined(VXWORKS)
#define HAVE_LOCALTIME_R 1
#endif

// Use reentrant version of localtime
#if !defined(__APPLE__) && !defined(_WIN32) && !defined(_AIX) && !defined(HAVE_GETHOSTBYNAME_R) && !defined(__NetBSD__) && !defined(VXWORKS)
#define HAVE_GETHOSTBYNAME_R 1
#endif

// Protect replica from unintended access (data corruption)
//#define PROTECT_DATABASE 1

// Wait acknowledgment that transaction is delivered to all slave nodes
//#define SYNCHRONOUS_REPLICATION true

// PAD_HEADER - add pad field to the database header to make it possible to transfer 
// file between systems with different alignment rules
// This pad field was unconditionally inserted in database header in 3.17 version
// of Perst which breaks backward compatibility. Because of the customers concerns
// in 3.39 version of Perst the "pad" field was wrapped with the following condition
// #if dbDatabaseOffsetBits > 32 && defined(ALIGN_HEADER)
// which in turn breaks compatibility with database created with 3.17-3.38 versions.
// Finally in 3.44 the condition was repalced with 
// #if (dbDatabaseOffsetBits > 32 && defined(ALIGN_HEADER)) || defined(PAD_HEADER)
// So you should uncomment definition of PAD_HEADER to preserve compatibility with databases 
// created by FastDB versions 3.17-3.38.
//#define PAD_HEADER

// ALIGN_HEADER - align each field of dbHeader class on its size
// to make it possible to transfer file between systems with different alignment rules
//#define ALIGN_HEADER true

// DO_NOT_REUSE_OID_WITHIN_SESSION - do not reuse OID of deallocated objects in the current session
//#define DO_NOT_REUSE_OID_WITHIN_SESSION true


// Use Doug Lea version of malloc instead of standard malloc
//#define USE_DLMALLOC true

// Invoke dbDatabase::handleError instead of assert for internal errors.
// #define HANDLE_ASSERTION_FAILURES

// Profile query execution
//#define PROFILE

// Store information about lock owners
#ifndef DEBUG_LOCKS
#define DEBUG_LOCKS 0
#endif

#ifndef STLPORT_WITH_VC6
#define STLPORT_WITH_VC6 0
#endif

#ifndef HAVE_ZLIB
#define HAVE_ZLIB 0
#endif

#endif // not HAVE_CONFIG_H
#endif

