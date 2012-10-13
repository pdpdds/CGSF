// $Id: Protocol.cpp 91626 2010-09-07 10:59:20Z johnnyw $
// author    : Boris Kolpackov <boris@dre.vanderbilt.edu>

#include "Protocol.hpp"

namespace ACE_TMCast
{
  namespace Protocol
  {
    /*
      namespace
      {
      char const* labels[] = {
      "NONE", "BEGIN", "COMMIT", "ABORT", "COMMITED", "ABORTED"};
      }

      std::string
      tslabel (Protocol::TransactionStatus s)
      {
      return labels[s];
      }

      std::ostream&
      operator << (std::ostream& o, Transaction const& t)
      {
      return o << "{" << t.id << "; " << tslabel (t.status) << "}";
      }
    */
  }
}
