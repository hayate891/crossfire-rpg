#
# Makefile for crossfire 0.95.0 and beyond
#
# This file will eventually be replaced by a configure script.  But for
# now, this simple makefile should do the job.
#
#   Copyright (C) 1992 Frank Tore Johansen
#   Copyright (C) 2001 Mark Wedel
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
#   The author can be reached via e-mail to mwedel@scruz.net


bin_PROGRAMS = crossfire

noinst_LIBRARIES = libserver.a
crossfire_SOURCES = main.c
libserver_a_SOURCES = \
	alchemy.c\
	apply.c\
	attack.c\
	ban.c\
	build_map.c\
	c_chat.c\
	c_misc.c\
	c_move.c\
	c_new.c\
	c_object.c\
	c_party.c\
	c_range.c\
	c_wiz.c\
	commands.c\
	daemon.c\
	disease.c\
	egoitem.c \
	hiscore.c\
	gods.c\
	init.c\
	login.c\
	monster.c\
	move.c\
	pets.c\
	player.c\
	plugins.c\
	resurrection.c\
	rune.c\
	shop.c\
	skills.c\
	skill_util.c\
	spell_attack.c\
	spell_effect.c\
	spell_util.c\
	swamp.c\
	swap.c\
	time.c\
	timers.c\
	weather.c\
	server.c

AM_CPPFLAGS = -I$(top_srcdir)/include -DDATADIR=\"$(pkgdatadir)\" -DCONFDIR=\"$(pkgconfdir)\" \
	      -DLIBDIR=\"$(pkglibdir)\" -DLOCALDIR=\"$(pkgstatedir)\" \
	      -DPLUGIN_SUFFIX=\".so\"

if HAVE_LIBDL
LIBDL=-ldl
crossfire_LDFLAGS=-export-dynamic
else
LIBDL=
crossfire_LDFLAGS=
endif
crossfire_LDADD = libserver.a ../common/libcross.a ../random_maps/librandom_map.a \
		../socket/libsocket.a $(LIBDL)

../common/libcross.a:
	( cd ../common ; make libcross.a )

../random_maps/librandom_map.a:
	( cd ../random_maps ; make librandom_map.a )

../socket/libsocket.a:
	( cd ../socket ; make libsocket.a )

# Check if everything is installed as it should
installcheck-local:
	$(bindir)/crossfire -o

proto:
	cproto -D__CEXTRACT__ -o ../include/sproto.h $(CPPFLAGS) $(AM_CPPFLAGS) \
		`echo $(SOURCES) " " | $(AWK) -v RS=" " '/.c$$/ { print }'`
	chmod 644 ../include/sproto.h
