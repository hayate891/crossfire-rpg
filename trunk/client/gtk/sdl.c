/*
 * static char *rcsid_sdl_c =
 *   "$Id$";
 */

/*
    CrossFire, A Multiplayer game for X-windows

    Copyright (C) 2001 Mark Wedel & Crossfire Development Team
    Copyright (C) 1992 Frank Tore Johansen

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

#include "config.h"
#ifdef HAVE_SDL
#include <client-types.h>
#include <SDL.h>
#include <SDL_image.h>

/* Pick up the gtk headers we need */
#include <gtk/gtk.h>
#include <gdk/gdkx.h>
#include <gdk/gdkkeysyms.h>

#include "gx11.h"
#include <client.h>

/* Actual SDL surface the game view is painted on */
SDL_Surface* mapsurface;
static SDL_Surface* lightmap;
static SDL_Surface* fogmap;


/* Move some of the SDL code to this file here.  This makes it easier to share
 * between the gnome and gtk client.  It also reduces the length of both the gx11.c
 * and gnome.c file.  It also is more readable, as not as many #ifdef SDL.. #endif
 * constructs are needed.
 * Note that there may still be some SDL code in gx11.c - some areas are embedded
 * so much that it is not easy to remove.
 */

/* these should generally be included by the file including this file. */
#include <SDL.h>
#include <SDL_image.h>


static void do_SDL_error( char* SDL_function, char* file, int line)
{
  fprintf( stderr, "ERROR on %s in file %s line %d\n%s\n", SDL_function,
	   file, line, SDL_GetError());
  SDL_Quit();
  exit( 1);
}

/*
 * Set the pixel at (x, y) to the given value
 * NOTE: The surface must be locked before calling this!
 * This function is directly grabbed from the SDL docs.
 * Note this is not currently used, but is useful enough
 * that it should be included.
 */
static void putpixel(SDL_Surface *surface, int x, int y, Uint32 pixel)
{
    int bpp = surface->format->BytesPerPixel;
    /* Here p is the address to the pixel we want to set */
    Uint8 *p = (Uint8 *)surface->pixels + y * surface->pitch + x * bpp;

    switch(bpp) {
    case 1:
        *p = pixel;
        break;

    case 2:
        *(Uint16 *)p = pixel;
        break;

    case 3:
        if(SDL_BYTEORDER == SDL_BIG_ENDIAN) {
            p[0] = (pixel >> 16) & 0xff;
            p[1] = (pixel >> 8) & 0xff;
            p[2] = pixel & 0xff;
	} else {
            p[0] = pixel & 0xff;
            p[1] = (pixel >> 8) & 0xff;
            p[2] = (pixel >> 16) & 0xff;
	}
        break;

    case 4:
        *(Uint32 *)p = pixel;
        break;
    }
}

static void overlay_grid( int re_init, int ax, int ay)
{

  static SDL_Surface* grid_overlay;

  static int first_pass;

  int x= 0;
  int y= 0;
  SDL_Rect dst;
  Uint32 *pixel;
  SDL_PixelFormat* fmt;

  /* Need to convert back to screen coordinates */
  ax-= pl_pos.x;
  ay-= pl_pos.y;
  
  if( re_init == TRUE)
    {
      if( grid_overlay)
	SDL_FreeSurface( grid_overlay);

      first_pass= 0;
      grid_overlay= NULL;
    }

  if( grid_overlay == NULL)
    {
      grid_overlay= SDL_CreateRGBSurface( SDL_HWSURFACE|SDL_SRCALPHA, 
					  use_config[CONFIG_MAPWIDTH]*map_image_size,
					  use_config[CONFIG_MAPHEIGHT]*map_image_size,
					  mapsurface->format->BitsPerPixel,
					  mapsurface->format->Rmask,
					  mapsurface->format->Gmask,
					  mapsurface->format->Bmask,
					  mapsurface->format->Amask);
      if( grid_overlay == NULL)
	do_SDL_error( "CreateRGBSurface", __FILE__, __LINE__);

      grid_overlay= SDL_DisplayFormatAlpha( grid_overlay);

      first_pass= 0;
    }

  /* 
   * If this is our first time drawing the grid, we need to build up the 
   * grid overlay
   */
  if( first_pass== 0)
    {

      /* Red pixels around the edge and along image borders
       * fully transparent pixels everywhere else
       */
      
      fmt= grid_overlay->format;
      for( x= 0; x < map_image_size*use_config[CONFIG_MAPWIDTH]; x++)
	{
	  for( y= 0; y < map_image_size*use_config[CONFIG_MAPHEIGHT]; y++)
	    {
	      /* FIXME: Only works for 32 bit displays right now */
	      pixel= (Uint32*)grid_overlay->pixels+y*grid_overlay->pitch/4+x;

	      if( x == 0 || y == 0 || 
		  ((x % map_image_size) == 0) || ((y % map_image_size) == 0 ) ||
		  y == use_config[CONFIG_MAPHEIGHT]*map_image_size-1 || x == use_config[CONFIG_MAPWIDTH]*map_image_size -1 )
		{
		  *pixel= SDL_MapRGBA( fmt, 255, 0, 0, SDL_ALPHA_OPAQUE);
		}
	      else 
		{
		  *pixel= SDL_MapRGBA( fmt, 0, 0, 0, SDL_ALPHA_TRANSPARENT);
		}
	    }
	}
      first_pass= 1;

      /* 
       * If this is our first pass then we need to overlay the entire grid
       * now. Otherwise we just update the tile we are on
       */
      dst.x= 0;
      dst.y= 0;
      dst.w= map_image_size*use_config[CONFIG_MAPWIDTH];
      dst.h= map_image_size*use_config[CONFIG_MAPHEIGHT];
      SDL_BlitSurface( grid_overlay, NULL, mapsurface, &dst);
    } 
  else 
    {
      dst.x= ax* map_image_size;
      dst.y= ay* map_image_size;
      dst.w= map_image_size;
      dst.h= map_image_size;
      /* One to one pixel mapping of grid and mapsurface so we
       * can share the SDL_Rect
       */
      SDL_BlitSurface( grid_overlay, &dst, mapsurface, &dst);
    }

  return;
}

/*
 * Takes two args, the first is the GtkWindow to draw on, this should always
 * be 'drawingarea'. The second is a boolean, if 0 then the whole
 * SDL system in initialized or reinited if already run once before,
 * if non zero then only the lightmap is rebuilt, if we switch between
 * per-pixel or per-tile lighting 
 */
void init_SDL( GtkWidget* sdl_window, int just_lightmap)
{

    char SDL_windowhack[32];

    if( just_lightmap == 0) {
	g_assert( sdl_window != NULL);
	if( SDL_WasInit( SDL_INIT_VIDEO) != 0) {
	    if( lightmap)
		SDL_FreeSurface( lightmap);
	    if( mapsurface)
		SDL_FreeSurface( mapsurface);
	    SDL_Quit();
	}

	/* 
	 * SDL hack to tell SDL which xwindow to paint onto 
	 */
	sprintf( SDL_windowhack, "SDL_WINDOWID=%ld",
	       GDK_WINDOW_XWINDOW(sdl_window->window) );
	putenv( SDL_windowhack);
      
	if( SDL_Init( SDL_INIT_VIDEO) < 0)
	{
	    fprintf( stderr, "Could not initialize SDL: %s\n", SDL_GetError());
	    gtk_main_quit();
	}

	mapsurface= SDL_SetVideoMode( map_image_size*use_config[CONFIG_MAPWIDTH], map_image_size*use_config[CONFIG_MAPHEIGHT], 0, 
				    SDL_HWSURFACE|SDL_DOUBLEBUF);
      
	if( mapsurface == NULL)
	{
	    do_SDL_error( "SetVideoMode", __FILE__, __LINE__);
	}

	if( fogmap)
	    SDL_FreeSurface( fogmap);

	fogmap= SDL_CreateRGBSurface( SDL_HWSURFACE|SDL_SRCALPHA, map_image_size,
				map_image_size,
				mapsurface->format->BitsPerPixel,
				mapsurface->format->Rmask,
				mapsurface->format->Gmask,
				mapsurface->format->Bmask,
				mapsurface->format->Amask);

	if( fogmap == NULL)
	{
	    do_SDL_error( "SDL_CreateRGBSurface", __FILE__, __LINE__);
	}

	/* 
	 * This is a persurface alpha value, not an alpha channel value.
	 * So this surface doesn't actually need a full alpha channel
	 */
	if( SDL_SetAlpha( fogmap, SDL_SRCALPHA|SDL_RLEACCEL, 128) < 0)
	{
	    do_SDL_error( "SDL_SetAlpha", __FILE__, __LINE__);
	}
    }

    if( just_lightmap != 0 && lightmap)
	SDL_FreeSurface( lightmap);
  
    lightmap= SDL_CreateRGBSurface( SDL_HWSURFACE|SDL_SRCALPHA, map_image_size,
				  map_image_size,
				  mapsurface->format->BitsPerPixel,
				  mapsurface->format->Rmask,
				  mapsurface->format->Gmask,
				  mapsurface->format->Bmask,
				  mapsurface->format->Amask);
    if( lightmap == NULL)
    {
	do_SDL_error( "SDL_CreateRGBSurface", __FILE__, __LINE__);
    }
  
    if(use_config[CONFIG_LIGHTING] != CFG_LT_TILE)
    {
	/* Convert surface to have a full alpha channel if we are doing
	 * per-pixel lighting */
	lightmap= SDL_DisplayFormatAlpha( lightmap);
	if( lightmap == NULL)
	{
	    do_SDL_error( "DisplayFormatAlpha", __FILE__, __LINE__);
	}
    }

    if(use_config[CONFIG_SHOWGRID] == TRUE)
    {
	overlay_grid( TRUE, 0, 0);
    }
}

/* Do the lighting on a per pixel basis.
 * x and y are coordinates on the drawable map surfaces (but in terms of
 * spaces, not pixels).  mx and my are indexes into the
 * the_map.cells[][] array.
 * All the below goes out and figures lighting for each pixel on
 * the space, and creates a surface (with alpha) that is then put on
 * top of the exiting map space.
 *
 * TODO: I think it is possible to greatly speed this up by using
 * pre-generated darkness masks.  Doing all the possibilities 
 * would be 3125 images (5 positions, each with 5 values, 5^5),
 * Doing it based on quadrants would only reduce that to 1024.
 * But I _think_ it may be possible to do this with just 64 images
 * (2^5 + one 90 degree rotation of the same) based on quadrants.
 * ie, do a 16x16 image with the 5 gradiants (0,64,128,255 at the
 * left, and each of those values at the right).  Then do the same
 * idea for top and bottom.  For any single quadrant, you would
 * then merge thse two values (which can be done with a fast blit),
 * corresponding to the right values, and you do the same thing for
 * the other four quadrants.  Note this only works so long as
 * 4 lighting values are used - if more are added, this quickly
 * breaks.  Also, if lighting colored effects are desired,
 * this also doesn't work very well.
 *
 * For now, I've just kept the old logic. MSW 2001-10-09
 */

/* See note below about ALPHA_FUDGE - used to adjust lighting effects some */

#define ALPHA_FUDGE(x)  (2*(x) / 3)
void do_sdl_per_pixel_lighting(int x, int y, int mx, int my)
{

    int dark0, dark1, dark2, dark3, dark4;
    SDL_Rect dst;

    /* I use dark0 -> dark4 in the order to keep it similar to
     * the old code.
     */
    dark0 = the_map.cells[mx][my].darkness;

    if (y-1 < 0 || !the_map.cells[mx][my-1].have_darkness) dark1 = dark0;
    else dark1 = the_map.cells[mx][my-1].darkness;

    if (x+1 >= use_config[CONFIG_MAPWIDTH] || !the_map.cells[mx+1][my].have_darkness) dark2 = dark0;
    else dark2 = the_map.cells[mx+1][my].darkness;

    if (y+1 >= use_config[CONFIG_MAPHEIGHT] || !the_map.cells[mx][my+1].have_darkness) dark3 = dark0;
    else dark3 = the_map.cells[mx][my+1].darkness;

    if (x-1 < 0 || !the_map.cells[mx-1][my].have_darkness) dark4 = dark0;
    else dark4 = the_map.cells[mx-1][my].darkness;

    /* If they are all the same, processing is easy */
    if (dark0 == dark1 && dark0 == dark2 && dark0 == dark3 && dark0 == dark4) {
	dst.x = x * map_image_size;
	dst.y = y * map_image_size;
	dst.w = map_image_size;
	dst.h = map_image_size;

	if (dark0 == 255) {
	    SDL_FillRect(mapsurface,&dst, SDL_MapRGB(mapsurface->format, 0, 0, 0));
	} else if (the_map.cells[mx][my].darkness != 0) {
	    SDL_FillRect(lightmap,NULL, SDL_MapRGBA(lightmap->format, 0, 0, 0, the_map.cells[mx][my].darkness));
	    SDL_BlitSurface(lightmap, NULL, mapsurface, &dst);
	}
	return;
    }


    if (use_config[CONFIG_LIGHTING] == CFG_LT_PIXEL ) {
	/* This almost works as well as the per pixel code below, but does have some various
	 * artifacts in the drawing.  It uses the same logic as the per pixel code below,
	 * bit since SDL does the blit, the alpha handling ends up being different
	 * (I think it ends up being additive).  This results in the darkness being
	 * darker, but you also don't get the smooth effects.  If you divide all the values
	 * by 2 (change ALPHA_FUDGE), the blending is smooth, but now the things are not dark
	 * enough, so the blending aganst solid black spaces does not look good.
	 * The reason this code is of interest is that on my system, it is about 50%
	 * faster than the code below (25 ms to darkness the church in the starting
	 * town vs 50 ms for the code further down)
	 * Setting ALPHA_FUDGE to 2/3 seems to reduce the artifacts described above
	 * to fairly minimal levels, while still keeping things dark enough.
	 * MSW 2001-10-12
	 */

	int i;

	if (dark1 == dark0) {
	    /* If we don't have usable darkness at the top, then this entire region
	    * should be the same value.  Likewise, if the top value and center value
	    * are the same, we can do the entire region.
	    */
	    dst.x=0;
	    dst.y=0;
	    dst.w = map_image_size;
	    dst.h = map_image_half_size;
	    SDL_FillRect(lightmap, &dst, SDL_MapRGBA(lightmap->format, 0, 0, 0, ALPHA_FUDGE(dark0)));
	}
	else for (i=0; i<map_image_half_size; i++) {
	    /* Need to do it line by line */

	    dst.x = 0;
	    dst.y = i;
	    dst.w = map_image_size;
	    dst.h = 1;
	    SDL_FillRect(lightmap, &dst, SDL_MapRGBA(lightmap->format, 0, 0, 0, 
			ALPHA_FUDGE((map_image_half_size - i) * dark1 + i * dark0)/map_image_half_size));

	}
	/* All the following blocks are basically the same as above, just different
	 * darkness areas.
	 */
	if (dark3 == dark0) {
	    dst.x=0;
	    dst.y=map_image_half_size;
	    dst.w = map_image_size;
	    dst.h = map_image_half_size;
	    SDL_FillRect(lightmap, &dst, SDL_MapRGBA(lightmap->format, 0, 0, 0, ALPHA_FUDGE(dark0)));
	}
	else for (i=map_image_half_size; i<map_image_size; i++) {
	    /* Need to do it line by line */

	    dst.x = 0;
	    dst.y = i;
	    dst.w = map_image_size;
	    dst.h = 1;
	    SDL_FillRect(lightmap, &dst, SDL_MapRGBA(lightmap->format, 0, 0, 0, 
			ALPHA_FUDGE(dark0*(map_image_size-i) + dark3*(i-map_image_half_size)) / map_image_half_size));
	}
	/* Blit this to the screen now.  Otherwise, we need to look at the alpha values
	 * and re-average.
	 */

	dst.x= x * map_image_size; 
	dst.y= y * map_image_size;
	SDL_BlitSurface(lightmap, NULL, mapsurface, &dst);

	if (dark4 == dark0) {
	    dst.x=0;
	    dst.y=0;
	    dst.w = map_image_half_size;
	    dst.h = map_image_size;
	    SDL_FillRect(lightmap, &dst, SDL_MapRGBA(lightmap->format, 0, 0, 0, ALPHA_FUDGE(dark0)));
	}
	else for (i=0; i<map_image_half_size; i++) {
	    /* Need to do it line by line */
	    dst.x = i;
	    dst.y = 0;
	    dst.w = 1;
	    dst.h = map_image_size;
	    SDL_FillRect(lightmap, &dst, SDL_MapRGBA(lightmap->format, 0, 0, 0,
			ALPHA_FUDGE(dark4*(map_image_half_size-i) + dark0*i) / map_image_half_size));
	}
	if (dark2 == dark0) {
	    dst.x=map_image_half_size;
	    dst.y=0;
	    dst.w = map_image_half_size;
	    dst.h = map_image_size;
	    SDL_FillRect(lightmap, &dst, SDL_MapRGBA(lightmap->format, 0, 0, 0, ALPHA_FUDGE(dark0)));
	}
	else for (i=map_image_half_size; i<map_image_size; i++) {
	    /* Need to do it line by line */

	    dst.x = i;
	    dst.y = 0;
	    dst.w = 1;
	    dst.h = map_image_size;
	    SDL_FillRect(lightmap, &dst, SDL_MapRGBA(lightmap->format, 0, 0, 0, 
			ALPHA_FUDGE(dark0*(map_image_size-i) + dark2*(i-map_image_half_size)) / map_image_half_size));
	}
	dst.x= x * map_image_size; 
	dst.y= y * map_image_size;
	SDL_BlitSurface(lightmap, NULL, mapsurface, &dst);
    } else if (use_config[CONFIG_LIGHTING] == CFG_LT_PIXEL_BEST ) {
	int dx,dy;
	static  int *darkx=NULL, *darky=NULL,darkx_allocated=0;

	/* Generated stored for the darkx[] array.  Do it dynamically, but
	 * only allocate if the size needs to be expanded to keep performance
	 * better.  darkx could be null in the initial case, but realloc should
	 * just treat that as a malloc (so according to the man page)
	 */
	if (map_image_size > darkx_allocated) {
	    darkx = realloc(darkx, map_image_size * sizeof(int));
	    darky = realloc(darky, map_image_size * sizeof(int));
	    darkx_allocated = map_image_size;
	}
			
	for( dx= 0; dx < map_image_half_size; dx++)
	    darkx[dx]= (dark4*(map_image_half_size-dx) + dark0*dx) / map_image_half_size;
	for( dx= map_image_half_size; dx < map_image_size; dx++)
	    darkx[dx] = (dark0*(map_image_size-dx) + dark2*(dx-map_image_half_size)) / map_image_half_size;

	for( dy= 0; dy < map_image_half_size; dy++)
	    darky[dy]= (dark1*(map_image_half_size-dy) + dark0*dy) / map_image_half_size;
	for( dy= map_image_half_size; dy < map_image_size; dy++)
	    darky[dy] = (dark0*(map_image_size-dy) + dark3*(dy-map_image_half_size)) / map_image_half_size;

	SDL_LockSurface( lightmap);

	for (dx=0; dx<map_image_size; dx++)
	    for (dy=0; dy<map_image_size; dy++) 
		putpixel(lightmap, dx, dy, SDL_MapRGBA(lightmap->format, 0, 0, 0,(darkx[dx] + darky[dy])/2));

	dst.w= map_image_size;
	dst.h= map_image_size;
	dst.x= x * map_image_size; 
	dst.y= y * map_image_size;
	SDL_UnlockSurface( lightmap);
	SDL_BlitSurface(lightmap, NULL, mapsurface, &dst);
    }
}

/* This generates a map in SDL mode.
 * 
 * I had to totally change the logic on how we do this in SDL mode - 
 * to support variable sized images, the old method of generating each
 * space does not work, as one space may spill over to the other.
 * Instead, we first blit the bottom layer, then the layer above
 * that, and so on.  This results in the map being drawn a bit
 * more correctly.  In fact, that logic actually isn't needed, as
 * with the new map commands, we know the offset and size of the
 * images.
 *
 * The logic here only redraws spaces that change.  The logic in the
 * common/commands.c files the odd layers with links for 'big images'.
 * for objects on these layers, we look at the size_x and size_y values
 * to determine the offset from which we should be blitting.
 *
 * Old notes, but left in:
 * The performance here is very good in most cases - about 30 ms (on my system)
 * is used just for my flip at the bottom of the function, drawing only what
 * is needed generally saves a lot of time (<15 ms in most cases) compared to the
 * 80-120 ms usually needed on a 15x15 map.
 */

void sdl_gen_map(int redraw) {
    int mx,my, layer,x,y;
    SDL_Rect dst, src;
    struct timeval tv1, tv2,tv3;
    long elapsed1, elapsed2;

    if (time_map_redraw)
	gettimeofday(&tv1, NULL);

    for( x= 0; x<use_config[CONFIG_MAPWIDTH]; x++) {
	for(y = 0; y<use_config[CONFIG_MAPHEIGHT]; y++) {
	    /* mx,my represent the spaces on the 'virtual' map (ie, the_map structure).
	     * x and y (from the for loop) represent the visable screen.
	     */
	    mx = x + pl_pos.x;
	    my = y + pl_pos.y;

	    /* Don't need to touch this space */
	    if (!redraw && !the_map.cells[mx][my].need_update) continue;

	    /* First, we need to black out this space. */
	    dst.x = x * map_image_size; dst.y = y* map_image_size;
	    dst.w = map_image_size; dst.h = map_image_size;
	    SDL_FillRect(mapsurface, &dst, SDL_MapRGB(mapsurface->format, 0, 0, 0));

	    /* now draw the different layers.  Only draw if using fog of war or the
	     * space isn't clear.
	     */
	    if (use_config[CONFIG_FOGWAR] || !the_map.cells[mx][my].cleared) 
		for (layer=0; layer<MAXLAYERS; layer++) {

		    /* draw the tail first - this seems to get better results */
		    if (the_map.cells[mx][my].tails[layer].face && 
			pixmaps[the_map.cells[mx][my].tails[layer].face]->map_image) {

			/* add one to the size values to take into account the actual width of the space */
			src.x = pixmaps[the_map.cells[mx][my].tails[layer].face]->map_width - 
			    (the_map.cells[mx][my].tails[layer].size_x + 1) * map_image_size;
			src.y = pixmaps[the_map.cells[mx][my].tails[layer].face]->map_height - 
			    (the_map.cells[mx][my].tails[layer].size_y + 1) * map_image_size;
			src.w = map_image_size;
			src.h = map_image_size;
			dst.x = x * map_image_size;
			dst.y = y * map_image_size;
			if (the_map.cells[mx][my].cleared) {
			    if (SDL_BlitSurface(pixmaps[the_map.cells[mx][my].tails[layer].face]->fog_image, 
				&src, mapsurface, &dst))
				    do_SDL_error( "BlitSurface", __FILE__, __LINE__);
			} else {
			    if (SDL_BlitSurface(pixmaps[the_map.cells[mx][my].tails[layer].face]->map_image, 
				&src, mapsurface, &dst))
				    do_SDL_error( "BlitSurface", __FILE__, __LINE__);
			}
		    }
		    /* Draw the head now - logic is pretty much exactly the same
		     * as that for the tail, except we know that we this is at the lower right,
		     * so we don't need to adjust the origin as much.
		     */
		    if (the_map.cells[mx][my].heads[layer].face && 
			pixmaps[the_map.cells[mx][my].heads[layer].face]->map_image) {

			src.x = pixmaps[the_map.cells[mx][my].heads[layer].face]->map_width - map_image_size;
			src.y = pixmaps[the_map.cells[mx][my].heads[layer].face]->map_height - map_image_size;
			src.w = map_image_size;
			src.h = map_image_size;
			dst.x = x * map_image_size;
			dst.y = y * map_image_size;
			if (the_map.cells[mx][my].cleared) {
			    if (SDL_BlitSurface(pixmaps[the_map.cells[mx][my].heads[layer].face]->fog_image, 
				&src, mapsurface, &dst))
				    do_SDL_error( "BlitSurface", __FILE__, __LINE__);
			} else {
			    if (SDL_BlitSurface(pixmaps[the_map.cells[mx][my].heads[layer].face]->map_image, 
				&src, mapsurface, &dst))
				    do_SDL_error( "BlitSurface", __FILE__, __LINE__);
			}
		    }
		} /* else for processing the layers */

	    /* Do final logic for this map space */
	    the_map.cells[mx][my].need_update=0;

#if 0
	    if (the_map.cells[mx][my].cleared == 1 && use_config[CONFIG_FOGWAR]) {
		/* If this is a fogcell, copy over the fogmap */
		dst.x = x * map_image_size;
		dst.y = y * map_image_size;
		SDL_BlitSurface(fogmap, NULL, mapsurface, &dst);
	    }
	    /* Only worry about lighting if it is not a fog cell.  If it is
	     * a fog cell, lighting information is probably bogus.
	     */
	    else 
#endif
	    if (use_config[CONFIG_LIGHTING] == CFG_LT_TILE) {
		dst.x = x * map_image_size;
		dst.y = y * map_image_size;
		dst.w = map_image_size;
		dst.h = map_image_size;

		/* Note - Instead of using a lightmap, I just fillrect
		 * directly onto the map surface - I would think this should be
		 * faster
		 */
		if (the_map.cells[mx][my].darkness == 255) {
		    SDL_FillRect(mapsurface,&dst, SDL_MapRGB(mapsurface->format, 0, 0, 0));
		} else if (the_map.cells[mx][my].darkness != 0) {
		    SDL_SetAlpha(lightmap, SDL_SRCALPHA|SDL_RLEACCEL, the_map.cells[mx][my].darkness);
		    SDL_BlitSurface(lightmap, NULL, mapsurface, &dst);
		}
	    } else if (use_config[CONFIG_LIGHTING] == CFG_LT_PIXEL || use_config[CONFIG_LIGHTING] == CFG_LT_PIXEL_BEST) {
		do_sdl_per_pixel_lighting(x, y, mx, my);
	    }
	} /* For y spaces */
    } /* for x spaces */
    if (time_map_redraw)
	gettimeofday(&tv2, NULL);

    SDL_Flip(mapsurface);

    if (time_map_redraw) {
	gettimeofday(&tv3, NULL);
	elapsed1 = (tv2.tv_sec - tv1.tv_sec)*1000000 + (tv2.tv_usec - tv1.tv_usec);
	elapsed2 = (tv3.tv_sec - tv2.tv_sec)*1000000 + (tv3.tv_usec - tv2.tv_usec);

	/* I care about performance for 'long' updates, so put the check in to make
	 * these a little more noticable */
	if ((elapsed1 + elapsed2)>10000)
	    fprintf(stderr,"sdl_gen_map: gen took %7ld, flip took %7ld, total = %7ld\n",
		    elapsed1, elapsed2, elapsed1 + elapsed2);
    }
} /* sdl_gen_map function */

void sdl_mapscroll(int dx, int dy)
{
    /* a copy of what pngximage does except sdl specfic
     * mapsurface->pitch is the length of a scanline in bytes 
     * including alignment padding
     */

    SDL_LockSurface( mapsurface);
    if( dy < 0) {
	int offset= mapsurface->pitch * (-dy*map_image_size);
	memmove( mapsurface->pixels + offset, mapsurface->pixels, 
		     mapsurface->pitch * (mapsurface->h + dy*map_image_size) );
    }
    else if( dy > 0) {
	int offset= mapsurface->pitch * (dy*map_image_size);
	memmove( mapsurface->pixels,  mapsurface->pixels + offset,
		     mapsurface->pitch * (mapsurface->h - dy*map_image_size) );
    }

    if (dx) {
	int y;
	for( y= 0; y < mapsurface->h; y++) {
	    if( dx < 0) {
		char* start_of_row= mapsurface->pixels + mapsurface->pitch * y;
		int offset= ( mapsurface->format->BytesPerPixel * map_image_size * -dx);
		memmove( start_of_row + offset, start_of_row,
			     mapsurface->pitch - offset);
	    }
	    else {
		char* start_of_row= mapsurface->pixels + mapsurface->pitch * y; 
		int offset= ( mapsurface->format->BytesPerPixel * map_image_size * dx);
		memmove( start_of_row, start_of_row + offset,
			     mapsurface->pitch - offset);
	    }
	}
    }
    SDL_UnlockSurface( mapsurface);
    map_did_scroll= 1;
}

	    

#endif

