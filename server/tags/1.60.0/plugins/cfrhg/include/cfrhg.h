/*****************************************************************************/
/* Template for version 2.0 plugins.                                         */
/* Contact: yann.chachkoff@myrealbox.com                                     */
/*****************************************************************************/
/* That code is placed under the GNU General Public Licence (GPL)            */
/* (C)2001-2005 by Chachkoff Yann (Feel free to deliver your complaints)     */
/*****************************************************************************/
/*  CrossFire, A Multiplayer game for X-windows                              */
/*                                                                           */
/*  Copyright (C) 2000 Mark Wedel                                            */
/*  Copyright (C) 1992 Frank Tore Johansen                                   */
/*                                                                           */
/*  This program is free software; you can redistribute it and/or modify     */
/*  it under the terms of the GNU General Public License as published by     */
/*  the Free Software Foundation; either version 2 of the License, or        */
/*  (at your option) any later version.                                      */
/*                                                                           */
/*  This program is distributed in the hope that it will be useful,          */
/*  but WITHOUT ANY WARRANTY; without even the implied warranty of           */
/*  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the            */
/*  GNU General Public License for more details.                             */
/*                                                                           */
/*  You should have received a copy of the GNU General Public License        */
/*  along with this program; if not, write to the Free Software              */
/*  Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.                */
/*                                                                           */
/*****************************************************************************/

/**
 * @file
 * This file is part of the @ref plugin_rhg "random house generation plugin".
 * See this page for more information.
 * @ingroup plugin_rhg
 * @{
 */

#ifndef PLUGIN_cfrhg_H
#define PLUGIN_cfrhg_H

#define PLUGIN_NAME "cfrhg"
#define PLUGIN_VERSION "Random house generator plugin version 1.0"

#ifndef __CEXTRACT__
#include <plugin.h>
#endif

#undef MODULEAPI
#ifdef WIN32
# ifdef PYTHON_PLUGIN_EXPORTS
#  define MODULEAPI __declspec(dllexport)
# else
#  define MODULEAPI __declspec(dllimport)
# endif
#elif HAVE_VISIBILITY
# define MODULEAPI __attribute__((visibility("default")))
#else
# define MODULEAPI
#endif

#include <plugin_common.h>
#include <cfrhg.h>

extern f_plug_api gethook;

/*@}*/

#endif /* PLUGIN_cfrhg_H */
