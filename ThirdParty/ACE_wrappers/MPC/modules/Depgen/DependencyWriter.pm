package DependencyWriter;

# ************************************************************
# Description   : Base class for all Dependency Writers.
# Author        : Chad Elliott
# Create Date   : 2/10/2002
# $Id: DependencyWriter.pm 2036 2011-10-06 15:14:09Z johnnyw $
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
  #my($self, $objects, $files) = @_;
  return '';
}


1;
