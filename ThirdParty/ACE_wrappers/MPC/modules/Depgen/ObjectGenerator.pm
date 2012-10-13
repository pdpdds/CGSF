package ObjectGenerator;

# ************************************************************
# Description   : Base class for all Object Generators.
# Author        : Chad Elliott
# Create Date   : 5/23/2003
# $Id: ObjectGenerator.pm 2036 2011-10-06 15:14:09Z johnnyw $
# ************************************************************

# ************************************************************
# Pragmas
# ************************************************************

use strict;

# ************************************************************
# Subroutine Section
# ************************************************************

sub new {
  return bless {}, $_[0];
}


sub process {
  #my($self, $file) = @_;
  return [];
}


1;
