#                                               -*- Autoconf -*-
# Process this file with autoconf to produce a configure script.

AC_PREREQ([2.63])
AC_INIT([fastdb], [3.53], [knizhnik@garret.ru])
AC_CONFIG_SRCDIR([src/database.cpp])
AC_CONFIG_AUX_DIR([config])
AC_CONFIG_HEADER([inc/acconfig.h])

CFLAGS="${CFLAGS} -Wno-invalid-offsetof -Wall"

# Checks for programs.
AC_PROG_CXX
AC_PROG_CPP
AC_PROG_YACC
AC_PROG_CC
AC_PROG_LEX
AC_PROG_INSTALL
AC_PROG_LN_S
AC_PROG_LIBTOOL

# Checks for libraries.
AC_CHECK_LIB([m], [ceil])
AC_CHECK_LIB(
   [rt], 
   [clock_gettime], 
   [AC_DEFINE([HAVE_CLOCK_GETTIME], [1], [define whether to use clock_gettime()])], 
)

# Checks for header files.
AC_FUNC_ALLOCA
AC_CHECK_HEADERS([arpa/inet.h fcntl.h limits.h locale.h malloc.h netdb.h netinet/in.h stddef.h stdlib.h string.h strings.h sys/ioctl.h sys/param.h sys/socket.h sys/time.h unistd.h])
AC_CANONICAL_HOST
AM_INIT_AUTOMAKE([fastdb], [3.53])

# Checks for typedefs, structures, and compiler characteristics.
AC_HEADER_STDBOOL
AC_C_INLINE
AC_TYPE_SIZE_T
AC_TYPE_SSIZE_T
AC_CHECK_TYPES([ptrdiff_t])

# Checks for library functions.
AC_FUNC_ERROR_AT_LINE
AC_FUNC_MKTIME
AC_FUNC_MMAP
AC_FUNC_STRCOLL
AC_FUNC_STRTOD

if test "x${cross_compiling}" != "xyes" ; then
   AC_FUNC_MALLOC
   AC_FUNC_REALLOC
fi


# ---------------------------------------------------------------------------- #

# Debug options
AC_MSG_CHECKING([whether compiling with debug options enabled])
AC_ARG_ENABLE(
   [debug],
   [AS_HELP_STRING([--enable-debug], [compile with debug options enabled])],
   [want_debug=${enableval}],
   [
      if test "x${want_debug}" = "x" ; then
         want_debug="no"
      fi
   ]
)

case ${want_debug} in
yes)
   AC_MSG_RESULT(yes)
   CFLAGS="-O0 -g ${CFLAGS} -DDEBUG"
   ;;
no) 
   AC_MSG_RESULT(no)
   CFLAGS="${CFLAGS} -DNDEBUG -O2 -fno-strict-aliasing"
   AC_DEFINE([HANDLE_ASSERTION_FAILURES], [1], [call dbDatabase::handleError instead of assert()])
   ;;
*)
   AC_MSG_FAILURE([invalid argument for --enable-debug: "$want_debug"])
   ;;
esac


# ---------------------------------------------------------------------------- #

# Thread options
AC_MSG_CHECKING([whether compiling with multithread support])
AC_ARG_ENABLE(
   [multithread],
   [AS_HELP_STRING([--disable-multithread], [compile for single threaded applications only])],
   [want_multithread=${enableval}],
   [
      if test "x${want_multithread}" = "x" ; then
         want_multithread="yes"
      fi
   ]
)

case ${want_multithread} in
yes)
   AC_MSG_RESULT(yes)
   AC_CHECK_LIB([pthread], [pthread_create], [], [AC_MSG_FAILURE([missing pthread library])])
   ;;
no) 
   AC_MSG_RESULT(no)
   AC_DEFINE([NO_PTHREADS], [1], [indicates that pthread lib isn't required])
   ;;
*)
   AC_MSG_FAILURE([invalid argument for --disable-multithread: "$want_multithread"])
   ;;
esac

AM_CONDITIONAL([NO_PTHREADS], [test "x${want_multithread}" = "xno"])


# ---------------------------------------------------------------------------- #

# fault tolerance options
AC_MSG_CHECKING([whether compiling with fault tolerant option])
AC_ARG_ENABLE(
   [fault-tolerant],
   [AS_HELP_STRING([--enable-fault-tolerant], [compile with fault tolerant option enabled])],
   [want_fault_tolerant=${enableval}],
   [
      if test "x${want_fault_tolerant}" = "x" ; then
         want_fault_tolerant="no"
      fi
   ]
)

case ${want_fault_tolerant} in
yes)
   AC_MSG_RESULT(yes)
   AC_DEFINE([REPLICATION_SUPPORT], [1], [indicates that replication support is enabled])
   ;;
no) 
   AC_MSG_RESULT(no)
   ;;
*)
   AC_MSG_FAILURE([invalid argument for --enable-fault-tolerant: "$want_fault_tolerant"])
   ;;
esac


# ---------------------------------------------------------------------------- #

# check for locale settings
AC_MSG_CHECKING([whether compiling with support for C locale settings])
AC_ARG_ENABLE(
   [locale],
   [AS_HELP_STRING([--disable-locale], [disable use of C locale for string comparison operations])],
   [want_locale_settings=${enableval}],
   [
      if test "x${want_locale_settings}" = "x" ; then
         want_locale_settings="yes"
      fi
   ]
)

case ${want_locale_settings} in
yes)
   AC_MSG_RESULT(yes)
   AC_DEFINE([USE_LOCALE_SETTINGS], [1], [defines whether to use C locale for string comparison operations])
   ;;
no) 
   AC_MSG_RESULT(no)
   ;;
*)
   AC_MSG_FAILURE([invalid argument for --enable-locale: "$want_locale_settings"])
   ;;
esac


# ---------------------------------------------------------------------------- #

# check for trace level
AC_MSG_CHECKING([for debug trace level])
AC_ARG_WITH(
   [debug-level],
   [AS_HELP_STRING([--with-debug-level=lev],
   [sets internal debug level to one among "all", "important", "check", "none"])],
   [debug_level=${withval}],
   [
      if test "x${debug_level}" = "x" ; then
         debug_level="important"
      fi
   ]
)

case $debug_level in
all)
   AC_DEFINE([FASTDB_DEBUG], [DEBUG_TRACE_ALL], [all checks and messages enabled])
   ;;
important)
   AC_DEFINE([FASTDB_DEBUG], [DEBUG_TRACE_IMPORTANT], [all checks and trace of important message only])
   ;;
check)
   AC_DEFINE([FASTDB_DEBUG], [DEBUG_CEHCK], [all checks but no messages])
   ;;
none)
   AC_DEFINE([FASTDB_DEBUG], [DEBUG_NONE], [no checks and no messages])
   ;;
*)
   AC_MSG_FAILURE([invalid argument for --with-debug-level: "$debug_level"])
   ;;
esac

AC_MSG_RESULT($debug_level)


# ---------------------------------------------------------------------------- #

# namespace options
AC_MSG_CHECKING([whether to place fastdb classes in separate namespaces])
AC_ARG_ENABLE(
   [separate-ns],
   [AS_HELP_STRING([--enable-separate-ns], [place fastdb classes in separate namespaces])],
   [want_separate_namespace=${enableval}],
   [
      if test "x${want_separate_namespace}" = "x" ; then
         want_separate_namespace="no"
      fi
   ]
)

case ${want_separate_namespace} in
yes)
   AC_MSG_RESULT(yes)
   AC_DEFINE([USE_NAMESPACES], [1], [indicates that fastdb classes must be places in seperate namespaces])
   ;;
no) 
   AC_MSG_RESULT(no)
   ;;
*)
   AC_MSG_FAILURE([invalid argument for --enable-separate-ns: "$want_separate_namespace"])
   ;;
esac


# ---------------------------------------------------------------------------- #

# secure server options
AC_MSG_CHECKING([whether to compile with authentication support])
AC_ARG_ENABLE(
   [authentication-support],
   [AS_HELP_STRING([--enable-authentication-support], [enable authentication for remote logins (keep user/pass table)])],
   [want_authentication_support=${enableval}],
   [
      if test "x${want_authentication_support}" = "x" ; then
         want_authentication_support="no"
      fi
   ]
)

case ${want_authentication_support} in
yes)
   AC_MSG_RESULT(yes)
   AC_DEFINE([SECURE_SERVER], [1], [enable support for authentication])
   ;;
no) 
   AC_MSG_RESULT(no)
   ;;
*)
   AC_MSG_FAILURE([invalid argument for --enable-authentication-support: "$want_authentication_support"])
   ;;
esac


# ---------------------------------------------------------------------------- #

# internal http server options
AC_MSG_CHECKING([whether to enable queue manager for internal http server])
AC_ARG_ENABLE(
   [http-queue-manager],
   [AS_HELP_STRING([--enable-http-queue-manager], [enable queue manager for internal http server])],
   [want_http_queue_mgr=${enableval}],
   [
      if test "x${want_http_queue_mgr}" = "x" ; then
         want_http_queue_mgr="no"
      fi
   ]
)

case ${want_http_queue_mgr} in
yes)
   AC_MSG_RESULT(yes)
   AC_DEFINE([USE_QUEUE_MANAGER], [1], [enable support for internal http server queue manager])
   ;;
no)
   AC_MSG_RESULT(no)
   ;;
*)
   AC_MSG_FAILURE([invalid argument for --enable-http-queue-manager: "$want_http_queue_mgr"])
   ;;
esac


# ---------------------------------------------------------------------------- #

# exception throwing options
AC_MSG_CHECKING([whether to throw exceptions on error instead of abort()])
AC_ARG_ENABLE(
   [cpp-exceptions],
   [AS_HELP_STRING([--enable-cpp-exceptions], [enable throwing C++ exceptions on error instead of abort])],
   [want_cpp_exceptions=${enableval}],
   [
      if test "x${want_cpp_exceptions}" = "x" ; then
         want_cpp_exceptions="yes"
      fi
   ]
)

case ${want_cpp_exceptions} in
yes)
   AC_MSG_RESULT(yes)
   AC_DEFINE([THROW_EXCEPTION_ON_ERROR], [1], [enable throwing C++ exceptions on error])
   ;;
no)
   AC_MSG_RESULT(no)
   ;;
*)
   AC_MSG_FAILURE([invalid argument for --enable-cpp-exceptions: "$want_cpp_exceptions"])
   ;;
esac


# ---------------------------------------------------------------------------- #

# case sensitiveness options
AC_MSG_CHECKING([whether to ignore case for all string comparisons])
AC_ARG_ENABLE(
   [case-sensitiveness],
   [AS_HELP_STRING([--disable-case-sensitiveness], [ignore case when performing string comparisons])],
   [want_case_sensitive=${enableval}],
   [
      if test "x${want_case_sensitive}" = "x" ; then
         want_case_sensitive="yes"
      fi
   ]
)

case ${want_case_sensitive} in
yes)
   AC_MSG_RESULT(no)
   ;;
no)
   AC_MSG_RESULT(yes)
   AC_DEFINE([IGNORE_CASE], [1], [carry on all string comparisons as case insensitive])
   ;;
*)
   AC_MSG_FAILURE([invalid argument for --disable-case-sensitiveness: "$want_case_sensitive"])
   ;;
esac


# ---------------------------------------------------------------------------- #

# STL string support
AC_MSG_CHECKING([whether STL string support is enbled])
AC_ARG_ENABLE(
   [stl-support],
   [AS_HELP_STRING([--disable-stl-support], [disable support for STL strings])],
   [want_stl_support=${enableval}],
   [
      if test "x${want_stl_support}" = "x" ; then
         want_stl_support="yes"
      fi
   ]
)

case ${want_stl_support} in
yes)
   AC_MSG_RESULT(yes)
   AC_DEFINE([USE_STD_STRING], [1], [enable support for STL strings])
   ;;
no)
   AC_MSG_RESULT(no)
   ;;
*)
   AC_MSG_FAILURE([invalid argument for --disable-stl-support: "$want_stl_support"])
   ;;
esac


# ---------------------------------------------------------------------------- #

# diskless option
AC_MSG_CHECKING([whether configuring for diskless systems])
AC_ARG_ENABLE(
   [diskless],
   [AS_HELP_STRING([--enable-diskless], [configure for diskeless systems: only in-memory temporary database])],
   [want_diskless=${enableval}],
   [
      if test "x${want_diskless}" = "x" ; then
         want_diskless="no"
      fi
   ]
)

case ${want_diskless} in
yes)
   AC_MSG_RESULT(yes)
   AC_DEFINE([DISKLESS_CONFIGURATION], [1], [define diskless mode operations (database not persistent)])
   ;;
no)
   AC_MSG_RESULT(no)
   ;;
*)
   AC_MSG_FAILURE([invalid argument for --enable-diskless: "$want_diskless"])
   ;;
esac


# ---------------------------------------------------------------------------- #

# auto increment option
AC_MSG_CHECKING([whether to enable autoincrement fields support])
AC_ARG_ENABLE(
   [autoincrement],
   [AS_HELP_STRING([--disable-autoincrement], [disable support for autoincrement fields])],
   [want_autoincrement=${enableval}],
   [
      if test "x${want_autoincrement}" = "x" ; then
         want_autoincrement="yes"
      fi
   ]
)

case ${want_autoincrement} in
yes)
   AC_MSG_RESULT(yes)
   AC_DEFINE([AUTOINCREMENT_SUPPORT], [1], [define autoincrement fileds support])
   ;;
no)
   AC_MSG_RESULT(no)
   ;;
*)
   AC_MSG_FAILURE([invalid argument for --disable-autoincrement: "$want_autoincrement"])
   ;;
esac


# ---------------------------------------------------------------------------- #

# rectangle dimension option
AC_MSG_CHECKING([for rectangle dimension])
AC_ARG_WITH(
   [rectangle-dimension],
   [AS_HELP_STRING([--with-rectangle-dimension], [specify rectangle dimension])],
   [rectangle_dimension=${withval}],
   [
      if test "x${rectangle_dimension}" = "x" ; then
         rectangle_dimension="2"
      fi
   ]
)

AC_MSG_RESULT($rectangle_dimension)
AC_DEFINE_UNQUOTED([RECTANGLE_DIMENSION], [$rectangle_dimension], [specify rectangle dimension])


# ---------------------------------------------------------------------------- #

# rectangle coordinate type
AC_MSG_CHECKING([for rectangle coordinate type])
AC_ARG_WITH(
   [rectangle-coordinate],
   [AS_HELP_STRING([--with-rectangle-coordinate], [specify rectangle coordinate type])],
   [rectangle_coordinate_type=${withval}],
   [
      if test "x${rectangle_coordinate_type}" = "x" ; then
         rectangle_coordinate_type="int"
      fi
   ]
)

AC_MSG_RESULT($rectangle_coordinate_type)
AC_DEFINE_UNQUOTED([RECTANGLE_COORDINATE_TYPE], [$rectangle_coordinate_type], [specify rectangle coordinate type (int|double|...)])


# ---------------------------------------------------------------------------- #

# rectangle area type
AC_MSG_CHECKING([for rectangle area type])
AC_ARG_WITH(
   [rectangle-area],
   [AS_HELP_STRING([--with-rectangle-area], [specify rectangle area type])],
   [rectangle_area_type=${withval}],
   [
      if test "x${rectangle_area_type}" = "x" ; then
         rectangle_area_type="db_int8"
      fi
   ]
)

AC_MSG_RESULT($rectangle_area_type)
AC_DEFINE_UNQUOTED([RECTANGLE_AREA_TYPE], [$rectangle_area_type], [specify rectangle area type (db_int8|double|...)])


# ---------------------------------------------------------------------------- #

# POSIX options
AC_MSG_CHECKING([whether to enable POSIX semaphores support])
AC_ARG_ENABLE(
   [posix-sem],
   [AS_HELP_STRING([--enable-posix-sem], [enable support for POSIX semaphores instead of SysV ones])],
   [want_posix_sem=${enableval}],
   [
      if test "x${want_posix_sem}" = "x" ; then
         want_posix_sem="no"
      fi
   ]
)

case ${want_posix_sem} in
yes)
   AC_MSG_RESULT(yes)
   AC_DEFINE([USE_POSIX_SEMAPHORES], [1], [define to use POSIX sempahores instead of SysV ones])
   ;;
no)
   AC_MSG_RESULT(no)
   ;;
*)
   AC_MSG_FAILURE([invalid argument for --enable-posix-sem: "$want_posix_sem"])
   ;;
esac


AC_MSG_CHECKING([whether to enable POSIX mmap support])
AC_ARG_ENABLE(
   [posix-mmap],
   [AS_HELP_STRING([--enable-posix-mmap], [enable support for POSIX mmap instead of SysV shmem])],
   [want_posix_mmap=${enableval}],
   [
      if test "x${want_posix_mmap}" = "x" ; then
         want_posix_mmap="nd"
      fi
   ]
)

case ${want_posix_mmap} in
yes)
   AC_MSG_RESULT(yes)
   AC_DEFINE([USE_POSIX_MMAP], [1], [define to use POSIX mmap instead of SysV shmem])
   ;;
no)
   AC_MSG_RESULT(no)
   AC_DEFINE([USE_POSIX_MMAP], [0], [define to use shmem for DB and monitor])
   ;;
nd)
   AC_MSG_RESULT(nd)
   ;;
*)
   AC_MSG_FAILURE([invalid argument for --enable-posix-mmap: "$want_posix_mmap"])
   ;;
esac


# ---------------------------------------------------------------------------- #

# mmap option
AC_MSG_CHECKING([whether to disable mmap usage])
AC_ARG_ENABLE(
   [mmap],
   [AS_HELP_STRING([--disable-mmap], [disable mmap usage ])],
   [want_mmap=${enableval}],
   [
      if test "x${want_mmap}" = "x" ; then
         want_mmap="yes"
      fi
   ]
)

case ${want_mmap} in
yes)
   AC_MSG_RESULT(yes)
   ;;
no)
   AC_MSG_RESULT(no)
   AC_DEFINE([NO_MMAP], [1], [define not to use mmap])
   ;;
*)
   AC_MSG_FAILURE([invalid argument for --disable-mmap: "$want_mmap"])
   ;;
esac


# ---------------------------------------------------------------------------- #

# fuzzy checpoint option
AC_MSG_CHECKING([whether configuring with fuzzy checkpoint option enabled])
AC_ARG_ENABLE(
   [fuzzy-checkpoint],
   [AS_HELP_STRING([--enable-fuzzy-checkpoint], [enable fuzzy checkpoint: queue DB commits])],
   [want_fuzzy_checkpoint=${enableval}],
   [
      if test "x${want_fuzzy_checkpoint}" = "x" ; then
         want_fuzzy_checkpoint="no"
      fi
   ]
)

case ${want_fuzzy_checkpoint} in
yes)
   AC_MSG_RESULT(yes)
   AC_DEFINE([FUZZY_CHECKPOINT], [1], [define to use a separate thread for committing DB changes)])
   ;;
no)
   AC_MSG_RESULT(no)
   ;;
*)
   AC_MSG_FAILURE([invalid argument for --enable-fuzzy_checkpoint: "$want_fuzzy_checkpoint"])
   ;;
esac


# ---------------------------------------------------------------------------- #

# critical section recovery option
AC_MSG_CHECKING([whether configuring for recoverable critical sections])
AC_ARG_ENABLE(
   [recoverable-critical-sections],
   [AS_HELP_STRING([--enable-recoverable-critical-sections], [release critical section on process crash])],
   [want_recoverable_critical_sections=${enableval}],
   [
      if test "x${want_recoverable_critical_sections}" = "x" ; then
         want_recoverable_critical_sections="no"
      fi
   ]
)

case ${want_recoverable_critical_sections} in
yes)
   AC_MSG_RESULT(yes)
   AC_DEFINE([RECOVERABLE_CRITICAL_SECTION], [1], [define to release critical sections on process crash)])
   ;;
no)
   AC_MSG_RESULT(no)
   ;;
*)
   AC_MSG_FAILURE([invalid argument for --enable-recoverable-critical-sections: "$want_recoverable_critical_sections"])
   ;;
esac


# ---------------------------------------------------------------------------- #

# flush option
AC_MSG_CHECKING([whether to disable DB flush on commit])
AC_ARG_ENABLE(
   [flush-commit],
   [AS_HELP_STRING([--disable-flush-commit], [disable flush on DB commit (not fault tolerant!)])],
   [want_flush_commit=${enableval}],
   [
      if test "x${want_flush_commit}" = "x" ; then
         want_flush_commit="yes"
      fi
   ]
)

case ${want_flush_commit} in
yes)
   AC_MSG_RESULT(yes)
   ;;
no)
   AC_MSG_RESULT(no)
   AC_DEFINE([NO_FLUSH_ON_COMMIT], [1], [define not to flush DB on commit (not fault tolerant!)])
   ;;
*)
   AC_MSG_FAILURE([invalid argument for --disable-flush-commit: "$want_flush_commit"])
   ;;
esac


# ---------------------------------------------------------------------------- #

# database Oid bits option
AC_MSG_CHECKING([for database OID bits])
AC_ARG_WITH(
   [oid-bits],
   [AS_HELP_STRING([--with-oid-bits], [specify the OID bit size])],
   [oid_bits=${withval}],
   [
      if test "x${oid_bits}" = "x" ; then
         oid_bits="32"
      fi
   ]
)

AC_MSG_RESULT($oid_bits)
AC_DEFINE_UNQUOTED([dbDatabaseOidBits], [$oid_bits], [define database OID bit size])

AC_MSG_CHECKING([for database offset bits])
AC_ARG_WITH(
   [offset-bits],
   [AS_HELP_STRING([--with-offset-bits], [specify the offset bit size])],
   [offset_bits=${withval}],
   [
      if test "x${offset_bits}" = "x" ; then
         offset_bits="32"
      fi
   ]
)

AC_MSG_RESULT($offset_bits)
AC_DEFINE_UNQUOTED([dbDatabaseOffsetBits], [$offset_bits], [define database offset bit size])


# ---------------------------------------------------------------------------- #

# Thread options
AC_MSG_CHECKING([whether compiling with regex support])
AC_ARG_WITH(
   [regex],
   [AS_HELP_STRING([--with-regex], [compile with regex support])],
   [with_regex=${withval}],
   [
      if test "x${with_regex}" = "x" ; then
         with_regex="no"
      fi
   ]
)

case ${with_regex} in
yes)
   AC_MSG_RESULT(yes)
   AC_CHECK_HEADER(
      [regex.h],
      [AC_DEFINE([USE_REGEX], [true], [indicates to use regex library])],
      [AC_MSG_FAILURE([missing regex headers])]
   )
   ;;
no) 
   AC_MSG_RESULT(no)
   ;;
*)
   AC_MSG_FAILURE([invalid argument for --with-regex: "$with_regex"])
   ;;
esac


# ---------------------------------------------------------------------------- #

# SOCK LINGER option
AC_MSG_CHECKING([whether enabling the SO_LINGER option])
AC_ARG_ENABLE(
   [socket-linger],
   [AS_HELP_STRING([--enable-socket-linger], [enable the SO_LINGER option])],
   [want_fuzzy_checkpoint=${enableval}],
   [
      if test "x${want_so_linger}" = "x" ; then
         want_so_linger="no"
      fi
   ]
)

case ${want_so_linger} in
yes)
   AC_MSG_RESULT(yes)
   AC_DEFINE([SOCK_LINGER], [1], [define to use the SO_LINGER option)])
   ;;
no)
   AC_MSG_RESULT(no)
   ;;
*)
   AC_MSG_FAILURE([invalid argument for --enable-socket-linger: "$want_so_linger"])
   ;;
esac


# ---------------------------------------------------------------------------- #

# protection option
AC_MSG_CHECKING([whether compiling with DB protection option enabled])
AC_ARG_ENABLE(
   [db-protection],
   [AS_HELP_STRING([--enable-db-protection], [use mprotect() to protect DB memory from accidental corruption])],
   [want_db_protection=${enableval}],
   [
      if test "x${want_db_protection}" = "x" ; then
         want_db_protection="no"
      fi
   ]
)

case ${want_db_protection} in
yes)
   AC_MSG_RESULT(yes)
   AC_DEFINE([PROTECT_DATABASE], [1], [define to use mprotect() to prevent accidantal DB corruption)])
   ;;
no)
   AC_MSG_RESULT(no)
   ;;
*)
   AC_MSG_FAILURE([invalid argument for --enable-db-protection: "$want_db_protection"])
   ;;
esac


# ---------------------------------------------------------------------------- #

# replication option
AC_MSG_CHECKING([whether compiling with synchronous replication support])
AC_ARG_ENABLE(
   [sync-replication],
   [AS_HELP_STRING([--enable-sync-replication], [wait ACK from all slave nodes on commit])],
   [want_sync_replication=${enableval}],
   [
      if test "x${want_sync_replication}" = "x" ; then
         want_sync_replication="no"
      fi
   ]
)

case ${want_sync_replication} in
yes)
   AC_MSG_RESULT(yes)
   AC_DEFINE([SYNCHRONOUS_REPLICATION], [true], [define to wait ACK from all slave nodes on commit)])
   ;;
no)
   AC_MSG_RESULT(no)
   ;;
*)
   AC_MSG_FAILURE([invalid argument for --enable-sync-replication: "$want_sync_replication"])
   ;;
esac


# ---------------------------------------------------------------------------- #

# interworking option
AC_MSG_CHECKING([support for exchanging data between systems with different alignment rules])
AC_ARG_ENABLE(
   [align-indep],
   [AS_HELP_STRING([--enable-align-indep], [allow exchanging data between systems with different alignment rules])],
   [want_align_indep=${enableval}],
   [
      if test "x${want_align_indep}" = "x" ; then
         want_align_indep="no"
      fi
   ]
)

case ${want_align_indep} in
yes)
   AC_MSG_RESULT(yes)

   AC_MSG_CHECKING([wether compiling keeping compatibility with DB version 3.17-3.38])
   AC_ARG_ENABLE(
      [align-compat],
      [AS_HELP_STRING([--enable-align-compat], [keep compatibility with DB version 3.17-3.38])],
      [want_align_compat=${enableval}],
      [
         if test "x${want_align_compat}" = "x" ; then
            want_align_compat="no"
         fi
      ]
   )
   
   if test "x$want_align_compat" = "xyes" ; then
      AC_DEFINE([PAD_HEADER], [1], [keep compatibility with DB version 3.17-3.38])
   fi

   AC_DEFINE([ALIGN_HEADER], [true], [align each field of dbHeader class on its page boudary])
   ;;
no)
   AC_MSG_RESULT(no)
   ;;
*)
   AC_MSG_FAILURE([invalid argument for --enable-sync-replication: "$want_align_indep"])
   ;;
esac


# ---------------------------------------------------------------------------- #

# OID reuse option
AC_MSG_CHECKING([whether to reuse OIDs])
AC_ARG_ENABLE(
   [oid-reuse],
   [AS_HELP_STRING([--disable-oid-reuse], [disable OID reuse within the same session])],
   [want_oid_reuse=${enableval}],
   [
      if test "x${want_oid_reuse}" = "x" ; then
         want_oid_reuse="yes"
      fi
   ]
)

case ${want_oid_reuse} in
yes)
   AC_MSG_RESULT(yes)
   ;;
no)
   AC_MSG_RESULT(no)
   AC_DEFINE([DO_NOT_REUSE_OID_WITHIN_SESSION], [true], [disable OID reuse within the same session])
   ;;
*)
   AC_MSG_FAILURE([invalid argument for --disable-oid-reuse: "$want_oid_reuse"])
   ;;
esac


# ---------------------------------------------------------------------------- #

# malloc option
AC_MSG_CHECKING([whether using Doug Lea version of malloc])
AC_ARG_ENABLE(
   [dl-malloc],
   [AS_HELP_STRING([--enable-dl-malloc], [use Doug Lea version of malloc instead of standard malloc])],
   [want_dl_malloc=${enableval}],
   [
      if test "x${want_dl_malloc}" = "x" ; then
         want_dl_malloc="no"
      fi
   ]
)

case ${want_dl_malloc} in
yes)
   AC_MSG_RESULT(yes)
   AC_DEFINE([USE_DLMALLOC], [true], [use Doug Lea version of malloc])
   ;;
no)
   AC_MSG_RESULT(no)
   ;;
*)
   AC_MSG_FAILURE([invalid argument for --enable-dl-malloc: "$want_dl_malloc"])
   ;;
esac


# ---------------------------------------------------------------------------- #

CXXFLAGS="${CFLAGS}"
AC_CHECK_FUNCS([ftruncate gethostbyname gethostbyname_r gethostname getpagesize gettimeofday inet_ntoa localtime_r memmove memset munmap pow regcomp select setlocale socket sqrt strchr strdup strerror strrchr strstr uname])

AC_CONFIG_FILES([Makefile])
AC_OUTPUT(
   src/Makefile
   examples/Makefile
)

