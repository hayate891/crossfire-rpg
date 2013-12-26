/*
 * Crossfire -- cooperative multi-player graphical RPG and adventure game
 *
 * Copyright (c) 1999-2013 Mark Wedel and the Crossfire Development Team
 * Copyright (c) 1992 Frank Tore Johansen
 *
 * Crossfire is free software and comes with ABSOLUTELY NO WARRANTY. You are
 * welcome to redistribute it under certain conditions. For details, see the
 * 'LICENSE' and 'COPYING' files.
 *
 * The authors can be reached via e-mail to crossfire-devel@real-time.com
 */

/**
 * @file gtk-v2/src/map.c
 * Handles map related code in terms of allocation, insertion of new objects,
 * and actual rendering (although the sdl rendering is in the sdl file
 */

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include <gdk/gdkkeysyms.h>
#include <gtk/gtk.h>
#include <png.h>
#include <stdlib.h>
#include <sys/stat.h>

#ifndef WIN32
#include <gdk/gdkx.h>
#include <unistd.h>
#else
#include <gdk/gdkwin32.h>
#include <time.h>
#endif

#include "client-types.h"
#include "image.h"
#include "client.h"
#include "main.h"
#include "mapdata.h"
#include "gtk2proto.h"

static uint8 map_updated = 0;

/*
 * Added for fog of war. Current size of the map structure in memory.
 * We assume a rectangular map so this is the length of one side.
 * command.c needs to know about this so not static
 * FIX ME: Don't assume rectangle
 */

PlayerPosition pl_pos;

GtkWidget *map_drawing_area, *map_notebook;
GdkGC *mapgc;
int map_image_size=DEFAULT_IMAGE_SIZE;
int map_image_half_size=DEFAULT_IMAGE_SIZE/2;
static GdkBitmap *dark1, *dark2, *dark3;
static GdkPixmap *dark;

/*
 * This should really be one of the CONFIG values, or perhaps a checkbox
 * someplace that displays frame rate.
 */
int time_map_redraw=0;

#if WIN32
/**
 *
 * @param tp
 * @param tzp
 * @return 0 indicates success.
 */
int gettimeofday(struct timeval* tp, void* tzp)
{
    DWORD t;
    t = timeGetTime();
    tp->tv_sec = t / 1000;
    tp->tv_usec = t % 1000;
    /* 0 indicates that the call succeeded. */
    return 0;
}
#endif

/**
 * This initializes the stuff we need for the map.
 *
 * @param window_root The client's main playing window.
 */
void map_init(GtkWidget *window_root) {
    map_drawing_area = GTK_WIDGET(gtk_builder_get_object(window_xml,
                "drawingarea_map"));
    map_notebook = GTK_WIDGET(gtk_builder_get_object(window_xml,
                "map_notebook"));

    g_signal_connect ((gpointer) map_drawing_area, "expose_event",
                      G_CALLBACK (on_drawingarea_map_expose_event), NULL);
    g_signal_connect ((gpointer) map_drawing_area, "button_press_event",
                      G_CALLBACK (on_drawingarea_map_button_press_event), NULL);
    g_signal_connect ((gpointer) map_drawing_area, "configure_event",
                      G_CALLBACK (on_drawingarea_map_configure_event), NULL);

#if 0
    gtk_widget_set_size_request (map_drawing_area,
                                 use_config[CONFIG_MAPWIDTH] * map_image_size,
                                 use_config[CONFIG_MAPHEIGHT] * map_image_size);
#endif
    mapgc = gdk_gc_new(map_drawing_area->window);
    gtk_widget_show(map_drawing_area);
    gtk_widget_add_events (map_drawing_area, GDK_BUTTON_PRESS_MASK);

    if (use_config[CONFIG_DISPLAYMODE] == CFG_DM_PIXMAP) {
        int x,y;
        GdkGC   *darkgc;
        /*
         * This is used when drawing with GdkPixmaps.  Create another surface,
         * as well as some light/dark images
         */
        dark = gdk_pixmap_new(map_drawing_area->window, map_image_size, map_image_size, -1);
        gdk_draw_rectangle(dark, map_drawing_area->style->black_gc, TRUE, 0, 0, map_image_size, map_image_size);
        dark1 = gdk_pixmap_new(map_drawing_area->window, map_image_size, map_image_size, 1);
        dark2 = gdk_pixmap_new(map_drawing_area->window, map_image_size, map_image_size, 1);
        dark3 = gdk_pixmap_new(map_drawing_area->window, map_image_size, map_image_size, 1);
        /*
         * We need our own GC here because we are working with single bit depth
         * images
         */
        darkgc = gdk_gc_new(dark1);
        gdk_gc_set_foreground(darkgc, &root_color[NDI_WHITE]);
        /* Clear any garbage values we get when we create the bitmaps */
        gdk_draw_rectangle(dark1, darkgc, TRUE, 0, 0, map_image_size, map_image_size);
        gdk_draw_rectangle(dark2, darkgc, TRUE, 0, 0, map_image_size, map_image_size);
        gdk_draw_rectangle(dark3, darkgc, TRUE, 0, 0, map_image_size, map_image_size);
        gdk_gc_set_foreground(darkgc, &root_color[NDI_BLACK]);

        for (x=0; x<map_image_size; x++) {
            for (y=0; y<map_image_size; y++) {
                /*
                 * We just fill in points every X pixels - dark1 is the
                 * darkest, dark3 is the lightest.  dark1 has 50% of the pixels
                 * filled in, dark2 has 33%, dark3 has 25% The formula's here
                 * are not perfect - dark2 will not match perfectly with an
                 * adjacent dark2 image.  dark3 results in diagonal stripes.
                 * OTOH, these will change depending on the image size.
                 */
                if ((x+y) % 2) {
                    gdk_draw_point(dark1, darkgc, x, y);
                }
                if ((x+y) %3) {
                    gdk_draw_point(dark2, darkgc, x, y);
                }
                if ((x+y) % 4) {
                    gdk_draw_point(dark3, darkgc, x, y);
                }
                /*
                 * dark1 gets filled on 0x01, 0x11, 0x10, only leaving 0x00
                 * empty
                 */
            }
            /*
             * if the row size is even, we put an extra value in count - in
             * this way, the pixels will be even on one line, odd on the next,
             * etc instead of vertical lines - at least for dark1 and dark3
             */
        }
        gdk_gc_unref(darkgc);
    }
#ifdef HAVE_SDL
    else if (use_config[CONFIG_DISPLAYMODE] == CFG_DM_SDL) {
        init_SDL(map_drawing_area,0);
    }
#endif
#ifdef HAVE_OPENGL
    else if (use_config[CONFIG_DISPLAYMODE] == CFG_DM_OPENGL) {
        init_opengl(map_drawing_area);
    }
#endif
}

/**
 * Request a map update from the server. This is to circumvent a bug in some
 * server versions.
 * @todo remove
 */
void reset_map(void)
{
}

/**
 *
 * @param srcx
 * @param srcy
 * @param dstx
 * @param dsty
 * @param clipx
 * @param clipy
 * @param mask
 * @param image
 * @param sizex
 * @param sizey
 */
static void draw_pixmap(int srcx, int srcy, int dstx, int dsty, int clipx, int clipy,
                        void *mask, void *image, int sizex, int sizey)
{
    gdk_gc_set_clip_mask(mapgc, mask);
    gdk_gc_set_clip_origin(mapgc, clipx, clipy);
    gdk_draw_pixmap(map_drawing_area->window, mapgc, image, srcx, srcy, dstx, dsty, sizex, sizey);
}

/**
 *
 * @param dx
 * @param dy
 */
int display_mapscroll(int dx, int dy)
{
#ifdef HAVE_SDL
    if (use_config[CONFIG_DISPLAYMODE]==CFG_DM_SDL) {
        return sdl_mapscroll(dx,dy);
    } else
#endif
        return 0;
}

/**
 * Draw anything in adjacent squares that could smooth on given square
 *
 * @param mx
 * @param my Square to smooth on.
 * You should not call this function to smooth on a 'completely black' square.
 * @param layer Layer to examine (we smooth only one layer at a time)
 * @param picx
 * @param picy Place on the map_drawing_area->window to draw
 */
void drawsmooth (int mx,int my,int layer,int picx,int picy)
{
    static int dx[8]= {0,1,1,1,0,-1,-1,-1};
    static int dy[8]= {-1,-1,0,1,1,1,0,-1};
    static int bweights[8]= {2,0,4,0,8,0,1,0};
    static int cweights[8]= {0,2,0,4,0,8,0,1};
    static int bc_exclude[8]= {
        1+2,/*north exclude northwest (bit0) and northeast(bit1)*/
        0,
        2+4,/*east exclude northeast and southeast*/
        0,
        4+8,/*and so on*/
        0,
        8+1,
        0
    };
    int partdone[8]= {0,0,0,0,0,0,0,0};
    int slevels[8];
    int sfaces[8];
    int i,weight,weightC;
    int emx,emy;
    int smoothface;
    int hasFace = 0;
    for (i=0; i<=layer; i++) {
        hasFace |= the_map.cells[mx][my].heads[i].face;
    }
    if (!hasFace
            || !CAN_SMOOTH(the_map.cells[mx][my], layer)) {
        return;
    }
    for (i=0; i<8; i++) {
        emx=mx+dx[i];
        emy=my+dy[i];
        if ( (emx<0) || (emy<0) || (the_map.x<=emx) || (the_map.y<=emy)) {
            slevels[i]=0;
            sfaces[i]=0; /*black picture*/
        } else if (the_map.cells[emx][emy].smooth[layer]<=the_map.cells[mx][my].smooth[layer]) {
            slevels[i]=0;
            sfaces[i]=0; /*black picture*/
        } else {
            slevels[i]=the_map.cells[emx][emy].smooth[layer];
            sfaces[i]=pixmaps[the_map.cells[emx][emy].heads[layer].face]->smooth_face;
        }
    }
    /*
     * Now we have a list of smoothlevel higher than current square.  There are
     * at most 8 different levels. so... check 8 times for the lowest one (we
     * draw from bottom to top!).
     */
    while (1) {
        int lowest = -1;
        for (i=0; i<8; i++) {
            if ( (slevels[i]>0) && (!partdone[i]) &&
                    ((lowest<0) || (slevels[i]<slevels[lowest]))
               ) {
                lowest=i;
            }
        }
        if (lowest<0) {
            break;    /*no more smooth to do on this square*/
        }
        /*printf ("hey, must smooth something...%d\n",sfaces[lowest]);*/
        /* Here we know 'what' to smooth
         *
         * Calculate the weight for border and weight for corners.  Then
         * 'markdone' the corresponding squares
         *
         * First, the border, which may exclude some corners
         */
        weight=0;
        weightC=15; /*works in backward. remove where there is nothing*/
        /*for (i=0;i<8;i++)
            cornermask[i]=1;*/
        for (i=0; i<8; i++) { /*check all nearby squares*/
            if ( (slevels[i]==slevels[lowest]) &&
                    (sfaces[i]==sfaces[lowest])) {
                partdone[i]=1;
                weight=weight+bweights[i];
                weightC&=~bc_exclude[i];
            } else {
                /*must rmove the weight of a corner if not in smoothing*/
                weightC&=~cweights[i];
            }
        }
        /*We can't do this before since we need the partdone to be adjusted*/
        if (sfaces[lowest]<=0) {
            continue;    /*Can't smooth black*/
        }
        smoothface=sfaces[lowest];
        if (smoothface<=0) {
            continue;  /*picture for smoothing not yet available*/
        }
        /*
         * now, it's quite easy. We must draw using a 32x32 part of the picture
         * smoothface.  This part is located using the 2 weights calculated:
         * (32*weight,0) and (32*weightC,32)
         */
        if ( (!pixmaps[smoothface]->map_image) ||
                (pixmaps[smoothface] == pixmaps[0])) {
            continue;    /*don't have the picture associated*/
        }
        if (weight>0) {
            draw_pixmap(
                weight*map_image_size, 0,
                picx, picy,
                picx-weight*map_image_size, picy,
                pixmaps[smoothface]->map_mask, pixmaps[smoothface]->map_image, map_image_size, map_image_size);
        }
        if (weightC>0) {
            draw_pixmap(
                weightC*map_image_size, map_image_size,
                picx, picy,
                picx-weightC*map_image_size, picy+map_image_size,
                pixmaps[smoothface]->map_mask, pixmaps[smoothface]->map_image, map_image_size, map_image_size);
        }
    } /* while there's some smooth to do */
}

/**
 *
 * @param ax
 * @param ay
 * @param mx
 * @param my
 */
static void display_mapcell(int ax, int ay, int mx, int my)
{
    int layer;

    /* First, we need to black out this space. */
    gdk_draw_rectangle(map_drawing_area->window, map_drawing_area->style->black_gc, TRUE, ax*map_image_size, ay*map_image_size, map_image_size, map_image_size);
    /*
     * Now draw the different layers.  Only draw if using fog of war or the
     * space isn't clear.
     */
    if (use_config[CONFIG_FOGWAR] || !the_map.cells[mx][my].cleared) {
        for (layer=0; layer<MAXLAYERS; layer++) {
            int sx, sy;

            /* draw single-tile faces first */
            int face = mapdata_face(ax, ay, layer);
            if (face > 0 && pixmaps[face]->map_image != NULL) {
                int src_x = pixmaps[face]->map_width - map_image_size;;
                int src_y = pixmaps[face]->map_height - map_image_size;
                int off_x=0, off_y=0;

                /* Normalize the source coordinates - clearly it can't be
                 * be less than zero.  If it is less than zero, this denotes
                 * a 'small' image.  By definition, the bottom right is the
                 * origin of the image (an image 16 pixels high is drawn on the
                 * bottom half of the space, not top), which is why
                 * the offsets are negative of the base values.
                 */
                if (src_x<0) {
                    off_x=-src_x;
                    src_x=0;
                }
                if (src_y<0) {
                    off_y = -src_y;
                    src_y=0;
                }
                draw_pixmap(
                    src_x, src_y,
                    ax*map_image_size + off_x, ay*map_image_size + off_y,
                    ax*map_image_size+map_image_size-pixmaps[face]->map_width,
                    ay*map_image_size+map_image_size-pixmaps[face]->map_height,
                    pixmaps[face]->map_mask, pixmaps[face]->map_image,
                    pixmaps[face]->map_width>map_image_size?map_image_size:pixmaps[face]->map_width,
                    pixmaps[face]->map_height>map_image_size?map_image_size:pixmaps[face]->map_height);

            }
            /*
             * Sometimes, it may happens we need to draw the smooth while there
             * is nothing to draw at that layer (but there was something at
             * lower layers). This is handled here. The else part is to take
             * into account cases where the smooth as already been handled 2
             * code lines before
             */
            if ( use_config[CONFIG_SMOOTH]) {
                drawsmooth (mx, my, layer, ax*map_image_size, ay*map_image_size);
            }

            /* draw big faces last (should overlap other objects) */
            face = mapdata_bigface(ax, ay, layer, &sx, &sy);

            if (face > 0 && pixmaps[face]->map_image != NULL) {
                /*
                 * This is pretty messy, because images are not required to be
                 * an integral multiplier of the image size.  There are really
                 * 4 main variables:
                 * source[xy]: Where within the pixmap to start grabbing pixels.
                 * off[xy]: Offset from space edge on the visible map to start
                 *     drawing pixels.
                 * off[xy] also determines how many pixels to draw
                 *     (map_image_size - off[xy])
                 * clip[xy]: Position of the clipmask.  The position of the
                 *     clipmask is always at the upper left of the image as we
                 *     drawn it on the map, so for any given big image, it will
                 *     have the same values for all the pieces.  However we
                 *     need to re-construct that location based on current
                 *     location.
                 *
                 * For a 32x72 image, it would be drawn like follows:
                 *                  sourcey         offy
                 * top space:       0               24
                 * middle space:    8               0
                 * bottom space:    40              0
                 */
                int dx, dy, sourcex, sourcey, offx, offy, clipx, clipy;

                dx = pixmaps[face]->map_width % map_image_size;
                offx = dx?(map_image_size -dx):0;
                clipx = (ax - sx)*map_image_size + offx;

                if (sx) {
                    sourcex = sx * map_image_size - offx ;
                    offx=0;
                } else {
                    sourcex=0;
                }

                dy = pixmaps[face]->map_height % map_image_size;
                offy = dy?(map_image_size -dy):0;
                clipy = (ay - sy)*map_image_size + offy;

                if (sy) {
                    sourcey = sy * map_image_size - offy;
                    offy=0;
                } else {
                    sourcey=0;
                }

                draw_pixmap(
                    sourcex,  sourcey,
                    ax*map_image_size+offx, ay*map_image_size + offy,
                    clipx, clipy,
                    pixmaps[face]->map_mask, pixmaps[face]->map_image,
                    map_image_size - offx, map_image_size - offy);
            }
        } /* else for processing the layers */
    }
    /*
     * If this is a fog cell, do darkening of the space.  otherwise, process
     * light/darkness - only do those if not a fog cell.
     */
    if (use_config[CONFIG_FOGWAR] && the_map.cells[mx][my].cleared) {
        draw_pixmap(0, 0, ax*map_image_size, ay*map_image_size, ax*map_image_size, ay*map_image_size, dark1, dark, map_image_size, map_image_size);
    } else if (the_map.cells[mx][my].darkness > 192) { /* Full dark */
        gdk_draw_rectangle (map_drawing_area->window, map_drawing_area->style->black_gc,
                            TRUE,map_image_size*ax, map_image_size*ay,
                            map_image_size, map_image_size);
    } else if (the_map.cells[mx][my].darkness> 128) {
        draw_pixmap(0, 0, ax*map_image_size, ay*map_image_size, ax*map_image_size, ay*map_image_size, dark1, dark, map_image_size, map_image_size);
    } else if (the_map.cells[mx][my].darkness> 64) {
        draw_pixmap(0, 0, ax*map_image_size, ay*map_image_size, ax*map_image_size, ay*map_image_size, dark2, dark, map_image_size, map_image_size);
    } else if (the_map.cells[mx][my].darkness> 1) {
        draw_pixmap(0, 0, ax*map_image_size, ay*map_image_size, ax*map_image_size, ay*map_image_size, dark3, dark, map_image_size, map_image_size);
    }
}

/**
 *
 * @param redraw
 */
void gtk_draw_map(int redraw)
{
    int mx, my;
    int x, y;
    struct timeval tv1, tv2,tv3;
    long elapsed1, elapsed2;

    if(!redraw && !map_updated) {
        return;
    }

    if (time_map_redraw) {
        gettimeofday(&tv1, NULL);
    }

    for(x = 0; x < use_config[CONFIG_MAPWIDTH]; x++) {
        for(y = 0; y < use_config[CONFIG_MAPHEIGHT]; y++) {
            /*
             * mx,my represent the spaces on the 'virtual' map (ie, the_map
             * structure).  x and y (from the for loop) represent the visible
             * screen.
             */
            mx = pl_pos.x+x;
            my = pl_pos.y+y;

            if (redraw
                    || the_map.cells[mx][my].need_update
                    || the_map.cells[mx][my].need_resmooth) {
                display_mapcell(x, y, mx, my);
                the_map.cells[mx][my].need_update = 0;
                the_map.cells[mx][my].need_resmooth = 0;
            }
        } /* For y spaces */
    } /* for x spaces */

    if (time_map_redraw) {
        gettimeofday(&tv2, NULL);
    }

    if (time_map_redraw) {
        gettimeofday(&tv3, NULL);
        elapsed1 = (tv2.tv_sec - tv1.tv_sec)*1000000 + (tv2.tv_usec - tv1.tv_usec);
        elapsed2 = (tv3.tv_sec - tv2.tv_sec)*1000000 + (tv3.tv_usec - tv2.tv_usec);
        /*
         * I care about performance for 'long' updates, so put the check in to
         * make these a little more noticable
         */
        if ((elapsed1 + elapsed2)>10000)
            LOG(LOG_INFO,"gtk-v2::sdl_gen_map","gen took %7ld, flip took %7ld, total = %7ld",
                elapsed1, elapsed2, elapsed1 + elapsed2);
    }
}

/**
 * The player has changed maps, so any info we have (for fog of war) is bogus,
 * so clear out all that old info.
 */
void display_map_newmap(void)
{
    reset_map();
}

/**
 * Resize_map_window is a NOOP for the time being - not sure if it will in fact
 * need to do something, since there are scrollbars for the map window now.
 * Note - this is note a window resize request, but rather process the size
 * (in spaces) of the map - is received from server.
 */
void resize_map_window(int x, int y)
{
    /* We do an implicit clear, since after a resize, there may be some
     * left over pixels at the edge which will not get drawn on by map spaces.
     */
    gdk_window_clear(map_drawing_area->window);
    draw_map(TRUE);
}


gboolean
on_drawingarea_map_configure_event     (GtkWidget       *widget,
                                        GdkEventConfigure *event,
                                        gpointer         user_data)
{
    sint16 w = event->width / map_image_size, h=event->height / map_image_size;

    if (w > MAP_MAX_SIZE) {
        w = MAP_MAX_SIZE;
    }
    if (h > MAP_MAX_SIZE) {
        h = MAP_MAX_SIZE;
    }

    /* Only need to do something if the size actually changes in terms
     * of displayable mapspaces.
     */
    if (w!= use_config[CONFIG_MAPWIDTH] || h!=use_config[CONFIG_MAPHEIGHT]) {
        /* We need to set the use_config values, even though we are not really using them,
         * because the setup processing basically expects the values returned from the
         * server to use these values.
         * Likewise, we need to call mapdata_set_size because we may try
         * to do map draws before we get the setup command from the server, and if it
         * is using the old values, that doesn't work quite right.
         */
        use_config[CONFIG_MAPWIDTH] = w;
        use_config[CONFIG_MAPHEIGHT] = h;
        mapdata_set_size(use_config[CONFIG_MAPWIDTH], use_config[CONFIG_MAPHEIGHT]);
        cs_print_string(csocket.fd,
                        "setup mapsize %dx%d", use_config[CONFIG_MAPWIDTH], use_config[CONFIG_MAPHEIGHT]);
    }
    return FALSE;
}


/**
 * Simple routine to put the splash icon in the map window.  Only supported
 * with non-SDL right now.
 */
void draw_splash(void)
{
    static GdkPixmap *splash;
    static int have_init=0;
    GdkBitmap *aboutgdkmask;
    int x,y, w, h;

#include "../../pixmaps/crossfiretitle.xpm"

    if (use_config[CONFIG_DISPLAYMODE]==CFG_DM_PIXMAP) {
        if (!have_init) {
            splash = gdk_pixmap_create_from_xpm_d(map_drawing_area->window,
                                                  &aboutgdkmask, NULL,
                                                  (gchar **)crossfiretitle_xpm);
            have_init=1;
        }
        gdk_window_clear(map_drawing_area->window);
        gdk_drawable_get_size(splash, &w, &h);
        x = (map_drawing_area->allocation.width- w)/2;
        y = (map_drawing_area->allocation.height - h)/2;
        /*
         * Clear the clip mask - it can be left in an inconsistent state from
         * last map redraw.
         */
        gdk_gc_set_clip_mask(mapgc, NULL);
        gdk_draw_pixmap(map_drawing_area->window, mapgc, splash, 0, 0,
                        x, y, w, h);
    }
}

/**
 *
 * @param redraw
 */
void draw_map(int redraw)
{
#ifdef HAVE_SDL
    if (use_config[CONFIG_DISPLAYMODE]==CFG_DM_SDL) {
        sdl_gen_map(redraw);
    } else
#endif
#ifdef HAVE_OPENGL
        if (use_config[CONFIG_DISPLAYMODE]==CFG_DM_OPENGL) {
            opengl_gen_map(redraw);
        } else
#endif
            if (use_config[CONFIG_DISPLAYMODE]==CFG_DM_PIXMAP) {
                if (cpl.input_state == Metaserver_Select) {
                    draw_splash();
                } else {
                    gtk_draw_map(redraw);
                }
            }
}

/**
 *
 * @param widget
 * @param event
 * @param user_data
 * @return FALSE
 */
gboolean
on_drawingarea_map_expose_event        (GtkWidget       *widget,
                                        GdkEventExpose  *event,
                                        gpointer         user_data)
{
    draw_map(TRUE);
    return FALSE;
}

/**
 *
 * @param widget
 * @param event
 * @param user_data
 * @return FALSE
 */
gboolean
on_drawingarea_map_button_press_event  (GtkWidget       *widget,
                                        GdkEventButton  *event,
                                        gpointer         user_data)
{
    int dx, dy, i, x, y, xmidl, xmidh, ymidl, ymidh;

    x=(int)event->x;
    y=(int)event->y;
    dx=(x-2)/map_image_size-(use_config[CONFIG_MAPWIDTH]/2);
    dy=(y-2)/map_image_size-(use_config[CONFIG_MAPHEIGHT]/2);
    xmidl=(use_config[CONFIG_MAPWIDTH]/2) * map_image_size;
    xmidh=(use_config[CONFIG_MAPWIDTH]/2 + 1) * map_image_size;
    ymidl=(use_config[CONFIG_MAPHEIGHT]/2) * map_image_size;
    ymidh=(use_config[CONFIG_MAPHEIGHT]/2 + 1) * map_image_size;

    switch (event->button) {
    case 1:
        look_at(dx,dy);
        break;

    case 2:
    case 3:
        if (x<xmidl) {
            i = 0;
        } else if (x>xmidh) {
            i = 6;
        } else {
            i =3;
        }

        if (y>ymidh) {
            i += 2;
        } else if (y>ymidl) {
            i++;
        }

        if (event->button==2) {
            switch (i) {
            case 0:
                fire_dir (8);
                break;
            case 1:
                fire_dir (7);
                break;
            case 2:
                fire_dir (6);
                break;
            case 3:
                fire_dir (1);
                break;
            case 5:
                fire_dir (5);
                break;
            case 6:
                fire_dir (2);
                break;
            case 7:
                fire_dir (3);
                break;
            case 8:
                fire_dir (4);
                break;
            }
            /* Only want to fire once */
            clear_fire();
        } else switch (i) {
            case 0:
                move_player (8);
                break;
            case 1:
                move_player (7);
                break;
            case 2:
                move_player (6);
                break;
            case 3:
                move_player (1);
                break;
            case 5:
                move_player (5);
                break;
            case 6:
                move_player (2);
                break;
            case 7:
                move_player (3);
                break;
            case 8:
                move_player (4);
                break;
            }
    }
    return FALSE;
}

/**
 * This isn't used - it is basically a prequel - we know we got a map command
 * from the server, but have digested it all yet.  This can be useful if there
 * is info we know we need to store away or the like before it is destroyed,
 * but there isn't anything like that for the gtk client.
 */
void display_map_startupdate(void)
{
}

/**
 * This is called after the map has been all digested.  this should perhaps be
 * removed, and left to being done from from the main event loop.
 *
 * @param redraw If set, force redraw of all tiles.
 * @param notice If set, another call will follow soon.
 */
void display_map_doneupdate(int redraw, int notice)
{
    map_updated |= redraw || !notice;
}
