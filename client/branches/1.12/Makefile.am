#
# Makefile for crossfire client
#
#   Copyright (C) 2005 Mark Wedel & Crossfire Development Team
#
#   This program is free software; you can redistribute it and/or modify
#   it under the terms of the GNU General Public License as published by
#   the Free Software Foundation; either version 2 of the License, or
#   (at your option) any later version.
#
#   This program is distributed in the hope that it will be useful,
#   but WITHOUT ANY WARRANTY; without even the implied warranty of
#   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
#   GNU General Public License for more details.
#
#   You should have received a copy of the GNU General Public License
#   along with this program; if not, write to the Free Software
#   Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
#
#   The author can be reached via e-mail to crossfire-devel@real-time.com

SUBDIRS=@CF_SUBDIRS@
ACLOCAL_AMFLAGS = -I macros --install

EXTRA_DIST = \
	ChangeLog		\
	COPYING			\
	License			\
	Makefile.in		\
	NOTES			\
	README			\
	TODO			\
	aclocal.m4		\
	configure		\
	configure.ac		\
	crossfire-client.spec

proto:
	@for dir in $(SUBDIRS); \
	    do \
	    echo "making proto in $$dir..."; \
	    (cd $$dir;  $(MAKE) $(MFLAGS) 'MFLAGS=$(MFLAGS)' proto; ) \
	done;

dox:
	@ doxygen 2>doxygen.err

