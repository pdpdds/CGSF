package CDT7WorkspaceCreator;

# ************************************************************
# Description   : A CDT7 Workspace Creator (Eclipse 3.6)
# Author        : Adam Mitz, Object Computing, Inc.
# Create Date   : 10/04/2010
# $Id: CDT7WorkspaceCreator.pm 2035 2011-10-06 15:05:21Z johnnyw $
# ************************************************************

# ************************************************************
# Pragmas
# ************************************************************

use strict;
use CDT7ProjectCreator;
use CDT6WorkspaceCreator;

use vars qw(@ISA);
@ISA = qw(CDT6WorkspaceCreator);

# ************************************************************
# Subroutine Section
# ************************************************************

sub get_cdt_version {
  return '7';
}

1;
