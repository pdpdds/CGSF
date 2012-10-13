// $Id: AuthenticationBase.cpp 90891 2010-06-28 09:55:39Z mcorino $

#include "ace/INet/AuthenticationBase.h"

#if !defined (__ACE_INLINE__)
#include "ace/INet/AuthenticationBase.inl"
#endif

ACE_BEGIN_VERSIONED_NAMESPACE_DECL

namespace ACE
{
  namespace INet
  {
    AuthenticationBase::~AuthenticationBase () {}

    AuthenticatorBase::~AuthenticatorBase () {}
  }
}

ACE_END_VERSIONED_NAMESPACE_DECL
