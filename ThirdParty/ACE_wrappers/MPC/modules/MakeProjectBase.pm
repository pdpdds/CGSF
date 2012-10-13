package MakeProjectBase;

# ************************************************************
# Description   : A Make Project base module
# Author        : Chad Elliott
# Create Date   : 1/4/2005
# $Id: MakeProjectBase.pm 2035 2011-10-06 15:05:21Z johnnyw $
# ************************************************************

# ************************************************************
# Pragmas
# ************************************************************

use strict;
use MakePropertyBase;

use vars qw(@ISA);
@ISA = qw(MakePropertyBase);

# ************************************************************
# Subroutine Section
# ************************************************************

sub get_builtin_output {
  #my($self, $input) = @_;
  return ['$@'];
}

sub dollar_special {
  #my $self = shift;
  return 1;
}


sub sort_files {
  #my $self = shift;
  return (defined $ENV{MPC_ALWAYS_SORT});
}


sub project_file_prefix {
  #my $self = shift;
  return 'Makefile.';
}


1;
