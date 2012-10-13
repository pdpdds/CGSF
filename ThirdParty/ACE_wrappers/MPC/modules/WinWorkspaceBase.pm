package WinWorkspaceBase;

# ************************************************************
# Description   : A Windows base module for Workspace Creators
# Author        : Chad Elliott
# Create Date   : 2/26/2007
# $Id: WinWorkspaceBase.pm 2035 2011-10-06 15:05:21Z johnnyw $
# ************************************************************

# ************************************************************
# Pragmas
# ************************************************************

use strict;
use WinPropertyBase;

use vars qw(@ISA);
@ISA = qw(WinPropertyBase);

# ************************************************************
# Subroutine Section
# ************************************************************

sub crlf {
  return $_[0]->windows_crlf();
}


sub convert_slashes {
  #my $self = shift;
  return 1;
}

sub case_insensitive {
  #my $self = shift;
  return 1;
}


1;
