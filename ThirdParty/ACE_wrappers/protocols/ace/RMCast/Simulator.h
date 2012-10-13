// $Id: Simulator.h 91626 2010-09-07 10:59:20Z johnnyw $
// author    : Boris Kolpackov <boris@kolpackov.net>

#ifndef ACE_RMCAST_SIMULATOR_H
#define ACE_RMCAST_SIMULATOR_H

#include "Stack.h"
#include "Protocol.h"
#include "Bits.h"

namespace ACE_RMCast
{
  class Simulator : public Element
  {
  public:
    Simulator ();

  public:
    virtual void
    send (Message_ptr m);

  private:
    Message_ptr hold_;
    Mutex mutex_;
  };
}


#endif  // ACE_RMCAST_SIMULATOR_H
