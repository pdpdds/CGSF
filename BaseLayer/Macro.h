#pragma once
#include <assert.h>

#define LOKI_WINDOWS_H

#define SFASSERT(x) assert((x))

#ifndef TRACE
#define TRACE
#endif


#ifndef ASSERT
#define ASSERT(exp) SFASSERT(exp)
#endif

#ifndef ASSERT_VALID
#define ASSERT_VALID(pOb)
#endif

#ifndef VERIFY
#ifdef _DEBUG
#define VERIFY(exp) SFASSERT(exp)
#else
#define VERIFY(exp) exp
#endif
#endif