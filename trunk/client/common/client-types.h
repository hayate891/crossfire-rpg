/*
 * static char *rcsid_client_types_h =
 *   "$Id$";
 */
/*
    Crossfire client, a client program for the crossfire program.

    Copyright (C) 2001 Mark Wedel & Crossfire Development Team

    This program is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 2 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program; if not, write to the Free Software
    Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.

    The author can be reached via e-mail to crossfire-devel@real-time.com
*/

#ifndef CLIENT_TYPES_H
#define CLIENT_TYPES_H
#include <cconfig.h>


/* If using autoconf, use it to pick up the necessary files.  Otherwise,
 * we will draw on includes.h
 */
#include "config.h"
#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>

#ifdef HAVE_SYS_TIME_H
#   include <sys/time.h>
#endif

#include <time.h>

#ifdef HAVE_STRING_H
#   include <string.h>
#endif

#ifdef HAVE_UNISTD_H
#   include <unistd.h>
#endif

#ifdef HAVE_FCNTL_H
#  include <fcntl.h>
#endif

/* Just some handy ones I like to use */
#ifndef FALSE
#define FALSE 0
#endif
#ifndef TRUE
#define TRUE 1
#endif


/* Set of common types used through the program and modules */
typedef unsigned int    uint32;
typedef signed int      sint32;
typedef unsigned short  uint16;
typedef signed short    sint16;
typedef unsigned char   uint8;
typedef signed char     sint8;

#ifndef SOL_TCP
#define SOL_TCP IPPROTO_TCP
#endif


#define MAX_BUF 256
#define BIG_BUF 1024

#endif
