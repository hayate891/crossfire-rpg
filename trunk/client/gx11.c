/*
 * static char *rcsid_xio_c =
 *   "$Id$";
 */

/*
    CrossFire, A Multiplayer game for X-windows

    Copyright (C) 2001 Mark Wedel
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

    The author can be reached via e-mail to mwedel@scruz.net
*/

/*
 *
 * This file handles all the windowing stuff.  The idea is
 * that all of it is in one file, so to port to different systems
 * or toolkits, only this file needs to be updated.  All windowing
 * variables (display, gc's, windows, etc), should be stored in
 * this file as statics.
 *
 * This file is largely a combination of the common/xutil.c and server/xio.c
 * file.  While I don't think this was a particulary great interface, the code
 * was there, and I figured it was probably easier to re-use that
 * code instead of writing new code, plus the old code worked.
 *
 */

/* Most functions in this file are private.  Here is a list of
 * the global functions:
 *
 * draw_color_info(int color, char*buf) - draws text in specified color
 * draw_info - draw info in the info window
 * end_windows - used when exiting
 * init_windows - called when starting up
 * load_images - creates the bitmaps and pixmaps (if being used)
 * create_pixmap - creates a pixmap from given file and assigns to
 *		the given face
 * create_xpm - as create_pixmap, but does an XPM image
 * load_additional_images - loads images that have been downloaded
 *	from the server in prior sessions
 *
 * draw_stats(int) - draws the stat window.  Pass 1 to redraw all
 *	stats, not only those that changed
 *
 * draw_message_window(int) - draws the message window.  Pass 1 to redraw
 *	all the bars, not only those that changed.
 *
 * draw_look, draw_inv:  Update the look and inventory windows.
 *
 * NOTE: create_pixmap and create_xpm can be empty functions if the
 * client will always use fonts - in that case, it should never
 * request Bitmap or Pixmap data, and thus not need the create
 * functions above
 *
 * Only functions in this file should be calling functions like
 * draw_stats and draw_message_window with redraw set - functions
 * in other files should always pass 0, because they will never have
 * the information of whether a redraw is needed.
 */


#ifdef HAVE_CONFIG_H
#include "config.h"
#endif
#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#ifdef HAVE_SYS_TIME_H
#include <sys/time.h>
#endif
#include <time.h>
#ifdef HAVE_STRING_H
#include <string.h>
#endif
#ifdef HAVE_UNISTD_H
#include <unistd.h>
#endif

#include "client.h"
/*#include "clientbmap.h"*/
#include "item.h"
#include "pixmaps/crossfiretitle.xpm"

#include <X11/Xlib.h>
#include <X11/Xutil.h>
#include <X11/xpm.h>

/* gtk */
#include <gtk/gtk.h>
#include <gdk/gdkx.h>
#include <gdk/gdkkeysyms.h>

#ifdef HAVE_LIBPNG
#define PNG_GDK
#include "png.c"
#endif

#define MAXPIXMAPNUM 10000
#define GDK_XUTIL

static int image_size=24;

/* All the following are static because these variables should
 * be local only to this file.  Since the idea is to have only
 * this file be replaced for different windowing systems, use of
 * any of these variables anyplace else would not be portable.
 */
typedef enum inventory_show {
  show_all = 0, show_applied = 0x1, show_unapplied = 0x2, show_unpaid = 0x4,
  show_cursed = 0x8, show_magical = 0x10, show_nonmagical = 0x20,
  show_locked = 0x40, show_unlocked=0x80,
  show_mask=0xff
} inventory_show;

#define TYPE_LISTS 9

/*
 *  This is similar obwin, but totally redone for client
 */
typedef struct {
  item *env;		  /* Environment shown in window */
  char title[MAX_BUF];  /* title of item list */
  char old_title[MAX_BUF];  /* previos title (avoid redrawns) */
  
  Window win;		  /* for X-windows */
  GtkWidget *label;
  GtkWidget *weightlabel;
  GtkWidget *maxweightlabel;

  /*  gint invrows;
  gint appliedrows;
  gint unappliedrows;
  gint unpaidrows;
  gint cursedrows;
  gint magicalrows;
  gint nonmagicalrows;
  gint lockedrows;*/

  float pos[TYPE_LISTS];

  GtkWidget *gtk_list[TYPE_LISTS];
  GtkWidget *gtk_lists[TYPE_LISTS];

  GC gc_text;
  GC gc_icon;
  GC gc_status;
  
  uint8 multi_list:1;     /* view is multi type */
  uint8 show_icon:1;	  /* show status icons */
  uint8 show_weight:1;  /* show item's weight */
  
  char format_nw[20];	  /* sprintf-format for text (name and weight) */
  char format_nwl[20];    /* sprintf-format for text (name, weight, limit) */
  char format_n[20];	  /* sprintf-format for text (only name) */
  sint16 text_len;	  /* How wide the text-field is */
  
  sint16 width;	  /* How wide the window is in pixels */
  sint16 height;	  /* How height the window is in pixels */
   
  sint16 item_pos;	  /* The sequence number of the first drawn item */
  sint16 item_used;	  /* How many items actually drawn. (0 - size) */
  
  sint16 size;	  /* How many items there is room to display */
  sint16 *faces;	  /* [size] */
  sint8 *icon1;	  /* status icon : locked */
  sint8 *icon2;	  /* status icon : applied / unpaid */
  sint8 *icon3;	  /* status icon : magic */
  sint8 *icon4;	  /* status icon : damned / cursed */
  char **names;	  /* [size][NAME_LEN] */
  
  /* The scrollbar */
  sint16 bar_length; 	  /* the length of scrollbar in pixels */
  sint16 bar_size;	  /* the current size of scrollbar in pixels */
  sint16 bar_pos;	  /* the starting position of scrollbar in pixels */
  uint32 weight_limit;   /* Weight limit for this list - used for title */
} itemlist;


typedef struct {
  GtkWidget *playername;
  GtkWidget *score;
  GtkWidget *level;
  GtkWidget *hp;
  GtkWidget *sp;
  GtkWidget *gr;
  GtkWidget *Str;
  GtkWidget *Dex;
  GtkWidget *Con;
  GtkWidget *Int;
  GtkWidget *Wis;
  GtkWidget *Cha;
  GtkWidget *Pow;
  GtkWidget *wc;
  GtkWidget *dam;
  GtkWidget *ac;
  GtkWidget *armor;
  GtkWidget *speed;
  GtkWidget *food;
  GtkWidget *skill;
} StatWindow;

typedef struct {
  gint row;
  GtkWidget *list;
} animview;

typedef struct {
  item *item;
  GList *view;
} animobject;
  
static char **gargv;

#define MAX_HISTORY 50
#define MAX_COMMAND_LEN 256
char history[MAX_HISTORY][MAX_COMMAND_LEN];
static int cur_history_position=0, scroll_history_position=0;

GList *anim_inv_list=NULL, *anim_look_list=NULL;

extern int maxfd;
struct timeval timeout;
gint	csocket_fd=0;

 /* Defined in global.h */
#define SCROLLBAR_WIDTH	16	/* +2+2 for border on each side */
#define INFOCHARS 50
#define INFOLINES 36
#define FONTWIDTH 8
#define FONTHEIGHT 13
#define MAX_INFO_WIDTH 80
#define MAXNAMELENGTH 50
#define WINUPPER (-5)
#define WINLOWER 5
#define WINLEFT (-5)
#define WINRIGHT 5

static int gargc;

Display_Mode display_mode = DISPLAY_MODE;

static uint8	
    split_windows=FALSE,
    cache_images=FALSE,
    nopopups=FALSE, 
    splitinfo=FALSE,
    color_inv=FALSE,
    color_text=FALSE,
    tool_tips=FALSE;


/* Don't define this unless you want stability problems */
#undef TRIM_INFO_WINDOW

#ifdef TRIM_INFO_WINDOW
/* Default size of scroll buffers is 100 K */
static int info1_num_chars=0, info2_num_chars=0, info1_max_chars=10000,
    info2_max_chars=10000;
#endif

#define MAXFACES 5
#define MAXPIXMAPNUM 10000
struct MapCell {
  short faces[MAXFACES];
  int count;
};

struct Map {
  struct MapCell cells[11][11];
};

struct PixmapInfo {
  long fg,bg;
  GdkPixmap *gdkpixmap;
  GdkBitmap *gdkmask;
};

struct GtkMap {
  GtkWidget *pixmap;
  GdkPixmap *gdkpixmap;
  GdkBitmap *gdkmask;
  GdkGC *gc;
};


typedef struct {
  int x;
  int y;
} MapPos;


/* vitals window */

typedef struct {
  GtkWidget *bar;
  GtkStyle *style;
  int state;
} Vitals;

static Vitals vitals[4];
static GtkWidget *run_label, *fire_label;
#define SHOW_RESISTS 7
static GtkWidget *resists[SHOW_RESISTS];
static GtkWidget *ckentrytext, *ckeyentrytext, *cmodentrytext, *cnumentrytext;

GdkColor gdk_green =    { 0, 0, 0xcfff, 0 };
GdkColor gdk_red =    { 0, 0xcfff, 0, 0 };
GdkColor gdk_grey = { 0, 0xea60, 0xea60, 0xea60 };
GdkColor gdk_black = { 0, 0, 0, 0 };

GdkColor gdkdiscolor;
static GdkColor map_color[16];
static GdkColor root_color[16];
static GdkPixmap *magicgdkpixmap;
static GdkGC *map_gc;
static GtkWidget *mapvbox;

static struct Map the_map;

static struct PixmapInfo pixmaps[MAXPIXMAPNUM];
#define INFOLINELEN 500
#define XPMGCS 100

static GtkWidget *ccheckbutton1;
static GtkWidget *ccheckbutton2;
static GtkWidget *ccheckbutton3;
static GtkWidget *ccheckbutton4;
static GtkWidget *ccheckbutton5;
static GtkWidget *ccheckbutton6;
static GtkWidget *ccheckbutton7;
static GtkWidget *ccheckbutton8;
static GtkWidget *inv_notebook;

static GtkTooltips *tooltips;

static GtkWidget *dialogtext;
static GtkWidget *dialog_window;
static GtkWidget *drawingarea;
static GdkPixmap *mappixmap=NULL;
static GdkGC *mapgc;

static GtkWidget *cclist;
static gboolean draw_info_freeze1=FALSE, draw_info_freeze2=FALSE;

enum {
    locked_icon = 1, applied_icon, unpaid_icon,
    damned_icon, cursed_icon, magic_icon, close_icon, 
    stipple1_icon, stipple2_icon, max_icons
};


static GtkWidget *entrytext, *counttext;
static gint redraw_needed=FALSE;
static GtkObject *text_hadj,*text_vadj;
static GtkObject *text_hadj2,*text_vadj2;

/*
 * These are used for inventory and look window
 */
static itemlist look_list, inv_list;
static StatWindow statwindow;
/* gtk */
 
static GtkWidget *gtkwin_root,  *gtkwin_info_text, *gtkwin_info_text2;
static GtkWidget *gtkwin_stats, *gtkwin_message, *gtkwin_info, *gtkwin_look, *gtkwin_inv;


static GtkWidget *gtkwin_about = NULL;
static GtkWidget *gtkwin_splash = NULL;
static GtkWidget *gtkwin_chelp = NULL;
static GtkWidget *gtkwin_shelp = NULL;
static GtkWidget *gtkwin_magicmap = NULL;
static GtkWidget *gtkwin_config = NULL;

static char *last_str;

static int pickup_mode = 0;

int updatelock = 0;

/* info win */
#define INFOCHARS 50
#define FONTWIDTH 8

/* this is used for caching the images across runs.  When we get a face
 * command from the server, we check the facecache for that name.  If
 * so, we can then use the num to find out what face number it is on the
 * local side.  the facecachemap does something similar - when the
 * server sends us saying to draw face XXX (server num), the facecacehmap
 * can then be used to determine what XXX is locally.  If not caching,
 * facecachemap is just a 1:1 mapping.
 */
struct FaceCache {
    char    *name;
    uint16  num;
} facecache[MAXPIXMAPNUM];

uint16 facecachemap[MAXPIXMAPNUM], cachelastused=0, cacheloaded=0;

FILE *fcache;

int misses=0,total=0;

#include "xutil.c"

void create_windows (void);


/* Convert xpm memory buffer to xpm data (needed since GTK/GDK doesnt have a
 * function to create from buffer rather than data
*/

char **xpmbuffertodata (char *buffer) {
  char *buf=NULL;
  char **strings=NULL;
  int i=0,q=0,z=0;

  for (i=1; buffer[i]!=';' ;i++) {
    if (buffer[i]=='"') {
      z=0;
      for (i++; buffer[i]!='"';i++) {
        buf=(char *)realloc(buf, (z+2)*sizeof(char));
        buf[z]=buffer[i];
        z++; 
      }
      buf[z]='\0';
      strings=(char **)realloc(strings, (q+2)*sizeof(char *));
      strings[q]=(char *)strdup(buf);    
      q++;
    }
  }
  strings=(char **)realloc(strings, (q+2)*sizeof(char *));
  strings[q]=(char *)NULL;
  free(buf);
  buf=NULL;
  return (strings);
} 

/* free the memeory allocated for the xpm data */

void freexpmdata (char **strings) {
  int q=0;
  for (q=0; strings[q]!=NULL ; q++) {
    free (strings[q]);
  }
  free (strings);
}
	
/* main loop iteration related stuff */
void do_network() {
    fd_set tmp_read, tmp_exceptions;
    int pollret;
    extern int updatelock;

    if (csocket.fd==-1) {
	if (csocket_fd) {
	    gdk_input_remove(csocket_fd);
	    csocket_fd=0;
	    gtk_main_quit();
	}
	return;
    }
  
    if (updatelock < 20) {
	FD_ZERO(&tmp_read);
	FD_ZERO(&tmp_exceptions);
	FD_SET(csocket.fd, &tmp_read);
	FD_SET(csocket.fd, &tmp_exceptions);

	pollret = select(maxfd, &tmp_read, NULL, NULL, &timeout);
	if (pollret==-1) {
	    fprintf(stderr, "Got errno %d on select call.\n", errno);
	}
	else if (FD_ISSET(csocket.fd, &tmp_read)) {
	    DoClient(&csocket);
	}
    } else {
	printf ("locked for network recieves.\n");
    }
}


void event_loop()
{
    gint fleep;
    extern int do_timeout();
    int tag;

    if (MAX_TIME==0) {
	timeout.tv_sec = 0;
	timeout.tv_usec = 0;
    }
    maxfd = csocket.fd + 1;

    if (MAX_TIME!=0) {
	timeout.tv_sec = 0;/* MAX_TIME / 1000000;*/
	timeout.tv_usec = 0;/* MAX_TIME % 1000000;*/
    }

    fleep =  gtk_timeout_add (100,
			  (GtkFunction) do_timeout,
			  NULL);
	
    csocket_fd = gdk_input_add ((gint) csocket.fd,
                              GDK_INPUT_READ,
                              (GdkInputFunction) do_network, &csocket);
    tag = csocket_fd;

    gtk_main();
    gtk_timeout_remove(tag);

    fprintf(stderr,"gtk_main exited, returning from event_loop\n");
}




/* Do the pixmap copy with gc to tile it onto the stack in the cell */

static void gen_draw_face(int face,int x,int y)
{
  gdk_gc_set_clip_mask (mapgc, pixmaps[facecachemap[face]].gdkmask);
  gdk_gc_set_clip_origin (mapgc, image_size*x, image_size*y);
  gdk_window_copy_area (mappixmap, mapgc, image_size*x, image_size*y, pixmaps[facecachemap[face]].gdkpixmap,0,0,image_size,image_size);
}

void end_windows()
{
  free(last_str);
}


/* Animation allocations */

static animview *newanimview() {
  animview *op = malloc (sizeof(animview));
  if (! op) 
    exit(0);
  op->row=0;
  op->list = NULL;
  return op;
}

static animobject *newanimobject() {
  animobject *op = malloc (sizeof(animobject));
  if (! op) 
    exit(0);
  op->view = NULL;
  return op;
}


/* Free allocations for animations */

void freeanimview (gpointer data, gpointer user_data) {
  if (data)
    g_free (data);
}

void freeanimobject (animobject *data, gpointer user_data) {
  if (data)
    g_list_foreach (data->view, freeanimview, 0);
  g_free (data);
}

/* Update the pixmap */

void animateview (animview *data, gint user_data) {
      gtk_clist_set_pixmap (GTK_CLIST (data->list), data->row, 0,
			    pixmaps[facecachemap[user_data]].gdkpixmap,
			    pixmaps[facecachemap[user_data]].gdkmask);
}

/* Run through the animations and update them */

void animate (animobject *data, gpointer user_data) {
  if (data) {
    data->item->last_anim++;
    if (data->item->last_anim>=data->item->anim_speed) {
      data->item->anim_state++;
      if (data->item->anim_state >= animations[data->item->animation_id].num_animations) {
	data->item->anim_state=0;
      }
      data->item->face = animations[data->item->animation_id].faces[data->item->anim_state];
      data->item->last_anim=0;
      g_list_foreach (data->view, (GFunc) animateview, GINT_TO_POINTER((gint)data->item->face));
    }
    
  }
}

/* Run through the lists of animation and do each */

void animate_list () {
  if (anim_inv_list) {
    g_list_foreach     (anim_inv_list, (GFunc) animate, NULL);
  }
  /* If this list needs to be updated, don't try and animated -
   * the contents of the lists are no longer valid.  this should
   * perhaps be done for the inventory list above, but the
   * removal of an animated object with a non animated one within
   * the timeframe if this being called is unlikely.
   */
  if (anim_look_list && !look_list.env->inv_updated) {
    g_list_foreach     (anim_look_list, (GFunc) animate, NULL);
  }
}




/* Handle mouse presses in the game window */


void button_map_event(GtkWidget *widget, GdkEventButton *event) {
  int dx, dy, i, x, y, xmidl, xmidh, ymidl, ymidh;
  
  x=(int)event->x;
  y=(int)event->y;
  dx=(x-2)/image_size-5;
  dy=(y-2)/image_size-5;
  xmidl=5*image_size-5;
  xmidh=6*image_size+5;
  ymidl=5*image_size-5;
  ymidh=6*image_size+5;
  
  switch (event->button) {
  case 1:
    {
      look_at(dx,dy);
    }
    break;
  case 2:
  case 3:
    if (x<xmidl)
      i = 0;
    else if (x>xmidh)
      i = 6;
    else i =3;
    
    if (y>ymidh)
      i += 2;
    else if (y>ymidl)
      i++;
    
    if (event->button==2) {
      switch (i) {
      case 0: fire_dir (8);break;
      case 1: fire_dir (7);break;
      case 2: fire_dir (6);break;
      case 3: fire_dir (1);break;
      case 5: fire_dir (5);break;
      case 6: fire_dir (2);break;
      case 7: fire_dir (3);break;
      case 8: fire_dir (4);break;
      }
      /* Only want to fire once */
      clear_fire();
    }
    else switch (i) {
    case 0: move_player (8);break;
    case 1: move_player (7);break;
    case 2: move_player (6);break;
    case 3: move_player (1);break;
    case 5: move_player (5);break;
    case 6: move_player (2);break;
    case 7: move_player (3);break;
    case 8: move_player (4);break;
    }
  }  
}





/******************************************************************************
 *
 * Code related to face caching.
 *
 *****************************************************************************/

char facecachedir[MAX_BUF];

/* This holds the name we recieve with the 'face' command so we know what
 * to save it as when we actually get the face.
 */
char *facetoname[MAXPIXMAPNUM];

/* Initializes the data for image caching */
static void init_cache_data()
{
    int i;
    char buf[MAX_BUF];
    GtkStyle *style;
#include "pixmaps/question.xpm"


    printf ("Init Cache\n");
    sprintf(buf,"%s/.crossfire/images.xpm", getenv("HOME"));
    
    style = gtk_widget_get_style(gtkwin_root);
    pixmaps[0].gdkpixmap = gdk_pixmap_create_from_xpm_d(gtkwin_root->window,
							&pixmaps[0].gdkmask,
							&style->bg[GTK_STATE_NORMAL],
							(gchar **)question);
    
    pixmaps[0].bg = 0;
    pixmaps[0].fg = 1;
    facetoname[0]=NULL;

    /* Initialize all the images to be of the same value. */
    for (i=1; i<MAXPIXMAPNUM; i++)  {
	pixmaps[i]=pixmaps[0];
	facetoname[i]=NULL;
    }

#ifdef IMAGECACHEDIR
    strcpy(facecachedir, IMAGECACHEDIR);
#else
    sprintf(facecachedir,"%s/.crossfire/images", getenv("HOME"));
#endif

    if (make_path_to_dir(facecachedir)==-1) {
	    fprintf(stderr,"Could not create directory %s, exiting\n", facecachedir);
	    exit(1);
    }

}

/* Deals with command history.  if direction is 0, we are going backwards,
 * if 1, we are moving forward.
 */

void gtk_command_history(int direction)
{
    int i=scroll_history_position;
    if (direction) {
	i--;
	if (i<0) i+=MAX_HISTORY;
	if (i == cur_history_position) return;
    } else {
	i++;
	if (i>=MAX_HISTORY) i = 0;
	if (i == cur_history_position) {
	    /* User has forwarded to what should be current entry - reset it now. */
	    gtk_entry_set_text(GTK_ENTRY(entrytext), "");
	    gtk_entry_set_position(GTK_ENTRY(entrytext), 0);
	    scroll_history_position=cur_history_position;
	    return;
	}
    }

    if (history[i][0] == 0) return;

    scroll_history_position=i;
/*    fprintf(stderr,"resetting postion to %d, data = %s\n", i, history[i]);*/
    gtk_entry_set_text(GTK_ENTRY(entrytext), history[i]);
    gtk_entry_set_position(GTK_ENTRY(entrytext), strlen(history[i]));
    gtk_widget_grab_focus (GTK_WIDGET(entrytext));
    cpl.input_state = Command_Mode;
}

void gtk_complete_command()
{
    gchar *entry_text, *newcommand;
	
    entry_text = gtk_entry_get_text(GTK_ENTRY(entrytext));
    newcommand = complete_command(entry_text);
    /* value differ, so update window */ 
    if (strcmp(entry_text, newcommand)) {
	gtk_entry_set_text(GTK_ENTRY(entrytext), newcommand);
	gtk_entry_set_position(GTK_ENTRY(entrytext), strlen(newcommand));
	/* regrab focus, since we've just updated this */
	gtk_widget_grab_focus (GTK_WIDGET(entrytext));
    }
}


void keyrelfunc(GtkWidget *widget, GdkEventKey *event, GtkWidget *window) {
  
  updatelock=0;
  if (event->keyval>0) {
    if (GTK_WIDGET_HAS_FOCUS (entrytext) /*|| GTK_WIDGET_HAS_FOCUS(counttext)*/ ) {
    } else {
      parse_key_release(XKeysymToKeycode(GDK_DISPLAY(), event->keyval), event->keyval);
      gtk_signal_emit_stop_by_name (GTK_OBJECT(window), "key_release_event") ;
    }
  }
}


void keyfunc(GtkWidget *widget, GdkEventKey *event, GtkWidget *window) {
  char *text;
  updatelock=0;

  if (nopopups) {
    if  (cpl.input_state == Reply_One) {
	text=XKeysymToString(event->keyval);
	send_reply(text);
	cpl.input_state = Playing;
	return;
    }
    else if (cpl.input_state == Reply_Many) {
	gtk_widget_grab_focus (GTK_WIDGET(entrytext));
	return;
    }
  }

  /* Better check for really weirdo keys, X doesnt like keyval 0*/
  if (event->keyval>0) {
    if (GTK_WIDGET_HAS_FOCUS (entrytext) /*|| GTK_WIDGET_HAS_FOCUS(counttext)*/) {
	if (event->keyval == completekeysym) gtk_complete_command();
	if (event->keyval == prevkeysym || event->keyval == nextkeysym) 
	    gtk_command_history(event->keyval==nextkeysym?0:1);
    }  else {
      
      switch(cpl.input_state) {
      case Playing:
	/* Specials - do command history - many times, the player
	 * will want to go the previous command when nothing is entered
	 * in the command window.
	 */
	if (event->keyval == prevkeysym || event->keyval == nextkeysym) {
	    gtk_command_history(event->keyval==nextkeysym?0:1);
	    return;
	}

	if (cpl.run_on) {
	  if (!(event->state & GDK_CONTROL_MASK)) {
/*	    printf ("Run is on while ctrl is not\n");*/
	    gtk_label_set (GTK_LABEL(run_label),"   ");
	    cpl.run_on=0;
	    stop_run();
	  }
	}
	if (cpl.fire_on) {
	  if (!(event->state & GDK_SHIFT_MASK)) {
/*	    printf ("Fire is on while shift is not\n");*/
	    gtk_label_set (GTK_LABEL(fire_label),"   ");
	    cpl.fire_on=0;
	    stop_fire();
	  }
	}
	
	
	parse_key(event->string[0], XKeysymToKeycode(GDK_DISPLAY(), event->keyval), event->keyval);
	gtk_signal_emit_stop_by_name (GTK_OBJECT(window), "key_press_event") ;
	break;
      case Configure_Keys:
	configure_keys(XKeysymToKeycode(GDK_DISPLAY(), event->keyval), event->keyval);
	gtk_signal_emit_stop_by_name (GTK_OBJECT(window), "key_press_event") ;
	break;
      case Command_Mode:
	if (event->keyval == completekeysym) gtk_complete_command();
	if (event->keyval == prevkeysym || event->keyval == nextkeysym) 
	    gtk_command_history(event->keyval==nextkeysym?0:1);
	else gtk_widget_grab_focus (GTK_WIDGET(entrytext));

      case Metaserver_Select:
	gtk_widget_grab_focus (GTK_WIDGET(entrytext));
      break;
      default:
	fprintf(stderr,"Unknown input state: %d\n", cpl.input_state);
      }
      
    }
    
  }
}



/* Event handlers for map drawing area */


/* Create a new backing pixmap of the appropriate size */
static gint
configure_event (GtkWidget *widget, GdkEventConfigure *event)
{
  if (mappixmap) {
    gdk_pixmap_unref(mappixmap);
    gdk_gc_unref(mapgc);
  }
  mappixmap = gdk_pixmap_new(widget->window,
			     widget->allocation.width,
			     widget->allocation.height,
			     -1);
  gdk_draw_rectangle (mappixmap,
		      widget->style->white_gc,
		      TRUE,
		      0, 0,
		      widget->allocation.width,
		      widget->allocation.height);
  mapgc = gdk_gc_new (drawingarea->window);
  
  return TRUE;
}



/* Redraw the screen from the backing pixmap */
static gint
expose_event (GtkWidget *widget, GdkEventExpose *event)
{
  gdk_draw_pixmap(widget->window,
		  widget->style->fg_gc[GTK_WIDGET_STATE (widget)],
		  mappixmap,
		  event->area.x, event->area.y,
		  event->area.x, event->area.y,
		  event->area.width, event->area.height);
  
  return FALSE;
}

/*
 * Sets up player game view window, implemented as a gtk table. Cells are initialized
 * with the bg.xpm pixmap to avoid resizes and to initialize GC's and everything for the
 * actual drawing routines later.
 */





static int get_game_display(GtkWidget *frame) {
  GtkWidget *gtvbox, *gthbox;
  
  gtvbox = gtk_vbox_new (FALSE, 0);
  gtk_container_add (GTK_CONTAINER (frame), gtvbox);
  gthbox = gtk_hbox_new (FALSE, 0);
  gtk_box_pack_start (GTK_BOX (gtvbox), gthbox, FALSE, FALSE, 1);
  
  drawingarea = gtk_drawing_area_new();
  gtk_drawing_area_size(GTK_DRAWING_AREA(drawingarea), image_size*11,image_size*11);
  /* Add mouseclick events to the drawing area */

  gtk_widget_set_events (drawingarea, GDK_BUTTON_PRESS_MASK);

  /* Set up X redraw routine signalling */
  gtk_signal_connect (GTK_OBJECT (drawingarea), "expose_event",
		      (GtkSignalFunc) expose_event, NULL);
  gtk_signal_connect (GTK_OBJECT(drawingarea),"configure_event",
		      (GtkSignalFunc) configure_event, NULL);

  /* Set up handling of mouseclicks in map */
 
  gtk_signal_connect (GTK_OBJECT(drawingarea),
		      "button_press_event",
		      GTK_SIGNAL_FUNC(button_map_event),
		      NULL);

  /* Pack it up and show it */

  gtk_box_pack_start (GTK_BOX (gthbox), drawingarea, FALSE, FALSE, 1);
  
  gtk_widget_show(drawingarea);

  gtk_widget_show(gthbox);
  gtk_widget_show(gtvbox);
  gtk_widget_show (frame);
  return 0;
}



/* ----------------------------------- inventory ---------------------------------------------*/

/*
 * Ugh. Ok, this is a Really Nasty Function From Hell (tm). Dont ask me what it does. 
 * Well, actually, it attempts to keep track of the different inventory object views.
 * This should only be called if an actual object is updated. It handles both the look
 * and inventory object window.
 */


static void draw_list (itemlist *l)
{

  gint tmprow;
  item *tmp;
  animobject *tmpanim=NULL;
  animview *tmpanimview;
  char buf[MAX_BUF];
  char buffer[3][MAX_BUF];
  char *buffers[3];
  gint list;

  /* Is it the inventory or look list? */
  if (l->multi_list) {
    
    /* Ok, inventory list. Animations are handled in client code. First do the nice thing and
     * free all allocated animation lists.
     */

    if (anim_inv_list) {
      g_list_foreach (anim_inv_list, (GFunc) freeanimobject, NULL);
      g_list_free (anim_inv_list);
      anim_inv_list=NULL;
    }
    /* Freeze the CLists to avoid flickering (and to speed up the processing) */
    for (list=0; list < TYPE_LISTS; list++) {
#ifdef GTK_HAVE_FEATURES_1_1_12
      l->pos[list]=GTK_RANGE (GTK_SCROLLED_WINDOW(l->gtk_lists[list])->vscrollbar)->adjustment->value;
#else
      l->pos[list]=GTK_RANGE (GTK_CLIST(l->gtk_list[list])->vscrollbar)->adjustment->value;
#endif
      gtk_clist_freeze (GTK_CLIST(l->gtk_list[list]));
      gtk_clist_clear (GTK_CLIST(l->gtk_list[list]));
    }
  } else {
    if (anim_look_list) {
      g_list_foreach (anim_look_list, (GFunc) freeanimobject, NULL);
      g_list_free (anim_look_list);
      anim_look_list=NULL;
    }
    /* Just freeze the lists and clear them */
#ifdef GTK_HAVE_FEATURES_1_1_12
    l->pos[0]=GTK_RANGE (GTK_SCROLLED_WINDOW(l->gtk_lists[0])->vscrollbar)->adjustment->value;
#else
    l->pos[0]=GTK_RANGE (GTK_CLIST(l->gtk_list[0])->vscrollbar)->adjustment->value;
#endif
    gtk_clist_freeze (GTK_CLIST(l->gtk_list[0]));
    gtk_clist_clear (GTK_CLIST(l->gtk_list[0]));
  }
  
  /* draw title and put stuff in widgets */
  
  if(l->env->weight < 0 || l->show_weight == 0) {
    strcpy(buf, l->title);
    gtk_label_set (GTK_LABEL(l->label), buf);
    gtk_label_set (GTK_LABEL(l->weightlabel), " ");
    gtk_label_set (GTK_LABEL(l->maxweightlabel), " ");
    gtk_widget_draw (l->label, NULL);
    gtk_widget_draw (l->weightlabel, NULL);
    gtk_widget_draw (l->maxweightlabel, NULL);
  }
  else if (!l->weight_limit) {
    strcpy(buf, l->title);
    gtk_label_set (GTK_LABEL(l->label), buf);
    sprintf (buf, "%6.1f",l->env->weight);
    gtk_label_set (GTK_LABEL(l->weightlabel), buf);
    gtk_label_set (GTK_LABEL(l->maxweightlabel), " ");
    gtk_widget_draw (l->label, NULL);
    gtk_widget_draw (l->weightlabel, NULL);
    gtk_widget_draw (l->maxweightlabel, NULL);
  } else {
    strcpy(buf, l->title);
    gtk_label_set (GTK_LABEL(l->label), buf);
    sprintf (buf, "%6.1f",l->env->weight);
    gtk_label_set (GTK_LABEL(l->weightlabel), buf);
    sprintf (buf, "/ %4d",l->weight_limit/1000);
    gtk_label_set (GTK_LABEL(l->maxweightlabel), buf);
    gtk_widget_draw (l->label, NULL);
    gtk_widget_draw (l->weightlabel, NULL);
    gtk_widget_draw (l->maxweightlabel, NULL);
  }

  /* Ok, go through the objects and start appending rows to the lists */

  for(tmp = l->env->inv; tmp ; tmp=tmp->next) {      

    
    strcpy (buffer[0]," "); 
    strcpy (buffer[1], tmp->d_name);

    if (l->show_icon == 0)
      strcat (buffer[1], tmp->flags);
    
    if(tmp->weight < 0 || l->show_weight == 0) {
      strcpy (buffer[2]," "); 
    } else {
      sprintf (buffer[2],"%6.1f" ,tmp->nrof * tmp->weight);
    }
    
    buffers[0] = buffer[0];
    buffers[1] = buffer[1];
    buffers[2] = buffer[2];
    
    if (l->multi_list) {
            
      tmprow = gtk_clist_append (GTK_CLIST (l->gtk_list[0]), buffers);
      /* Set original pixmap */
      gtk_clist_set_pixmap (GTK_CLIST (l->gtk_list[0]), tmprow, 0,
			    pixmaps[facecachemap[tmp->face]].gdkpixmap,
			    pixmaps[facecachemap[tmp->face]].gdkmask); 
      gtk_clist_set_row_data (GTK_CLIST(l->gtk_list[0]), tmprow, tmp);
      if (color_inv) { 
	if (tmp->cursed || tmp->damned) {
	  gtk_clist_set_background (GTK_CLIST(l->gtk_list[0]), tmprow,
				    &root_color[NDI_RED]);
	}
	if (tmp->magical) {
	  gtk_clist_set_background (GTK_CLIST(l->gtk_list[0]), tmprow,
				    &root_color[NDI_BLUE]);
	}
	if ((tmp->cursed || tmp->damned) && tmp->magical) {
	  gtk_clist_set_background (GTK_CLIST(l->gtk_list[0]), tmprow,
				    &root_color[NDI_NAVY]);
	}
      }
      /* If it's an animation, zap in an animation object to the list of
         animations to be done */

      if (tmp->animation_id>0 && tmp->anim_speed) {
	tmpanim = newanimobject();
	tmpanim->item=tmp;
	tmpanimview = newanimview();
	tmpanimview->row=tmprow;
	tmpanimview->list=l->gtk_list[0];
	tmpanim->view = g_list_append (tmpanim->view, tmpanimview);
	anim_inv_list = g_list_append (anim_inv_list, tmpanim);
      }

      if (tmp->applied) {
       	tmprow = gtk_clist_append (GTK_CLIST (l->gtk_list[1]), buffers);
	gtk_clist_set_pixmap (GTK_CLIST (l->gtk_list[1]), tmprow, 0,
			      pixmaps[facecachemap[tmp->face]].gdkpixmap,
			      pixmaps[facecachemap[tmp->face]].gdkmask);
	gtk_clist_set_row_data (GTK_CLIST(l->gtk_list[1]), tmprow, tmp); 
	if (tmp->animation_id>0 && tmp->anim_speed) {
	  tmpanimview = newanimview();
	  tmpanimview->row=tmprow;
	  tmpanimview->list=l->gtk_list[1];
	  tmpanim->view = g_list_append (tmpanim->view, tmpanimview);
	}

      }
      if (!tmp->applied) {
	tmprow = gtk_clist_append (GTK_CLIST (l->gtk_list[2]), buffers);
	gtk_clist_set_pixmap (GTK_CLIST (l->gtk_list[2]), tmprow, 0,
			      pixmaps[facecachemap[tmp->face]].gdkpixmap,
			      pixmaps[facecachemap[tmp->face]].gdkmask);
	gtk_clist_set_row_data (GTK_CLIST(l->gtk_list[2]), tmprow, tmp);
	if (tmp->animation_id>0 && tmp->anim_speed) {
	  tmpanimview = newanimview();
	  tmpanimview->row=tmprow;
	  tmpanimview->list=l->gtk_list[2];
	  tmpanim->view = g_list_append (tmpanim->view, tmpanimview);
	}
	

      }
      if (tmp->unpaid) {
	tmprow = gtk_clist_append (GTK_CLIST (l->gtk_list[3]), buffers);
	gtk_clist_set_pixmap (GTK_CLIST (l->gtk_list[3]), tmprow, 0,
			      pixmaps[facecachemap[tmp->face]].gdkpixmap,
			      pixmaps[facecachemap[tmp->face]].gdkmask);
	gtk_clist_set_row_data (GTK_CLIST(l->gtk_list[3]), tmprow, tmp);
	if (tmp->animation_id>0 && tmp->anim_speed) {
	  tmpanimview = newanimview();
	      tmpanimview->row=tmprow;
	      tmpanimview->list=l->gtk_list[3];
	      tmpanim->view = g_list_append (tmpanim->view, tmpanimview);
	}
	

      }
      if (tmp->cursed || tmp->damned) {
	tmprow = gtk_clist_append (GTK_CLIST (l->gtk_list[4]), buffers);
	gtk_clist_set_pixmap (GTK_CLIST (l->gtk_list[4]), tmprow, 0,
			      pixmaps[facecachemap[tmp->face]].gdkpixmap,
			      pixmaps[facecachemap[tmp->face]].gdkmask);
	gtk_clist_set_row_data (GTK_CLIST(l->gtk_list[4]), tmprow, tmp);
	if (tmp->animation_id>0 && tmp->anim_speed) {
	  tmpanimview = newanimview();
	  tmpanimview->row=tmprow;
	  tmpanimview->list=l->gtk_list[4];
	  tmpanim->view = g_list_append (tmpanim->view, tmpanimview);
	}
	

      }
      if (tmp->magical) {
	tmprow = gtk_clist_append (GTK_CLIST (l->gtk_list[5]), buffers);
	gtk_clist_set_pixmap (GTK_CLIST (l->gtk_list[5]), tmprow, 0,
			      pixmaps[facecachemap[tmp->face]].gdkpixmap,
			      pixmaps[facecachemap[tmp->face]].gdkmask);
	gtk_clist_set_row_data (GTK_CLIST(l->gtk_list[5]), tmprow, tmp);
	if (tmp->animation_id>0 && tmp->anim_speed) {
	  tmpanimview = newanimview();
	  tmpanimview->row=tmprow;
	  tmpanimview->list=l->gtk_list[5];
	  tmpanim->view = g_list_append (tmpanim->view, tmpanimview);
	}
	

      }
      if (!tmp->magical) {
	tmprow = gtk_clist_append (GTK_CLIST (l->gtk_list[6]), buffers);
	gtk_clist_set_pixmap (GTK_CLIST (l->gtk_list[6]), tmprow, 0,
			      pixmaps[facecachemap[tmp->face]].gdkpixmap,
			      pixmaps[facecachemap[tmp->face]].gdkmask);
	gtk_clist_set_row_data (GTK_CLIST(l->gtk_list[6]), tmprow, tmp);
	if (tmp->animation_id>0 && tmp->anim_speed) {
	  tmpanimview = newanimview();
	  tmpanimview->row=tmprow;
	  tmpanimview->list=l->gtk_list[6];
	  tmpanim->view = g_list_append (tmpanim->view, tmpanimview);
	}
	

      }
      if (tmp->locked) {
	tmprow = gtk_clist_append (GTK_CLIST (l->gtk_list[7]), buffers);
	gtk_clist_set_pixmap (GTK_CLIST (l->gtk_list[7]), tmprow, 0,
			      pixmaps[facecachemap[tmp->face]].gdkpixmap,
			      pixmaps[facecachemap[tmp->face]].gdkmask);
	gtk_clist_set_row_data (GTK_CLIST(l->gtk_list[7]), tmprow, tmp);
	if (tmp->animation_id>0 && tmp->anim_speed) {
	  tmpanimview = newanimview();
	  tmpanimview->row=tmprow;
	  tmpanimview->list=l->gtk_list[7];
	  tmpanim->view = g_list_append (tmpanim->view, tmpanimview);
	}
      }
      if (!tmp->locked) {
	tmprow = gtk_clist_append (GTK_CLIST (l->gtk_list[8]), buffers);
	gtk_clist_set_pixmap (GTK_CLIST (l->gtk_list[8]), tmprow, 0,
			      pixmaps[facecachemap[tmp->face]].gdkpixmap,
			      pixmaps[facecachemap[tmp->face]].gdkmask);
	gtk_clist_set_row_data (GTK_CLIST(l->gtk_list[8]), tmprow, tmp);
	if (tmp->animation_id>0 && tmp->anim_speed) {
	  tmpanimview = newanimview();
	  tmpanimview->row=tmprow;
	  tmpanimview->list=l->gtk_list[8];
	  tmpanim->view = g_list_append (tmpanim->view, tmpanimview);
	}
	if (color_inv) { 
	    if (tmp->cursed || tmp->damned) {
		gtk_clist_set_background (GTK_CLIST(l->gtk_list[8]), tmprow,
				    &root_color[NDI_RED]);
	    }
	    if (tmp->magical) {
		gtk_clist_set_background (GTK_CLIST(l->gtk_list[8]), tmprow,
				    &root_color[NDI_BLUE]);
	    }
	    if ((tmp->cursed || tmp->damned) && tmp->magical) {
		gtk_clist_set_background (GTK_CLIST(l->gtk_list[8]), tmprow,
				    &root_color[NDI_NAVY]);
	    }
	}
      }
      
      
    } else {
      tmprow = gtk_clist_append (GTK_CLIST (l->gtk_list[0]), buffers);
      gtk_clist_set_pixmap (GTK_CLIST (l->gtk_list[0]), tmprow, 0,
			    pixmaps[facecachemap[tmp->face]].gdkpixmap,
			    pixmaps[facecachemap[tmp->face]].gdkmask);
      gtk_clist_set_row_data (GTK_CLIST(l->gtk_list[0]), tmprow, tmp);
      if (tmp->animation_id>0 && tmp->anim_speed) {
	tmpanim = newanimobject();
	tmpanim->item=tmp;
	tmpanimview = newanimview();
	tmpanimview->row=tmprow;
	tmpanimview->list=l->gtk_list[0];
	tmpanim->view = g_list_append (tmpanim->view, tmpanimview);
	anim_look_list = g_list_append (anim_look_list, tmpanim);
      }
      if (color_inv) { 
	if (tmp->cursed || tmp->damned) {
	  gtk_clist_set_background (GTK_CLIST(l->gtk_list[0]), tmprow,
				    &root_color[NDI_RED]);
	}
	if (tmp->magical) {
	  gtk_clist_set_background (GTK_CLIST(l->gtk_list[0]), tmprow,
				    &root_color[NDI_BLUE]);
	}
	if ((tmp->cursed || tmp->damned) && tmp->magical) {
	  gtk_clist_set_background (GTK_CLIST(l->gtk_list[0]), tmprow,
				    &root_color[NDI_NAVY]);
	}
      }
      
    }  
  }
  
  /* Ok, stuff is drawn, now replace the scrollbar positioning as far as possible */

  if (l->multi_list) {
    
    for (list=0; list < TYPE_LISTS; list++) {
#ifdef GTK_HAVE_FEATURES_1_1_12
      gtk_adjustment_set_value (GTK_ADJUSTMENT (GTK_RANGE (GTK_SCROLLED_WINDOW(l->gtk_lists[list])->vscrollbar)->adjustment), l->pos[list]);
#else
      gtk_adjustment_set_value (GTK_ADJUSTMENT (GTK_RANGE (GTK_CLIST(l->gtk_list[list])->vscrollbar)->adjustment), l->pos[list]);
#endif
      gtk_clist_thaw (GTK_CLIST(l->gtk_list[list]));
      /*      gtk_widget_draw_children (l->gtk_list[list]);
	      gtk_widget_draw (l->gtk_list[list],NULL);*/
    }
    
  } else {
#ifdef GTK_HAVE_FEATURES_1_1_12
    gtk_adjustment_set_value (GTK_ADJUSTMENT (GTK_RANGE (GTK_SCROLLED_WINDOW(l->gtk_lists[0])->vscrollbar)->adjustment), l->pos[0]);
#else
    gtk_adjustment_set_value (GTK_ADJUSTMENT (GTK_RANGE (GTK_CLIST(l->gtk_list[0])->vscrollbar)->adjustment), l->pos[0]);
#endif
    gtk_clist_thaw (GTK_CLIST(l->gtk_list[0]));
    /*    gtk_widget_draw_children (l->gtk_list[0]);
	  gtk_widget_draw (l->gtk_list[0],NULL);*/
  }

}


/******************************************************************************
 *
 * The functions dealing with the info window follow
 *
 *****************************************************************************/


static void enter_callback(GtkWidget *widget, GtkWidget *entry)
{
    gchar *entry_text;

    /* Next reply will reset this as necessary */
    if (nopopups)
	gtk_entry_set_visibility(GTK_ENTRY(entrytext), TRUE);

    entry_text = gtk_entry_get_text(GTK_ENTRY(entrytext));
	 /*         printf("Entry contents: %s\n", entry_text);*/

    if (cpl.input_state==Metaserver_Select) {
	cpl.input_state = Playing;
	strcpy(cpl.input_text, entry_text);
    } else if (cpl.input_state == Reply_One ||
	       cpl.input_state == Reply_Many) {
	cpl.input_state = Playing;
	strcpy(cpl.input_text, entry_text);
	if (cpl.input_state == Reply_One)
	    cpl.input_text[1] = 0;

        send_reply(cpl.input_text);

    } else {
	cpl.input_state = Playing;
	/* No reason to do anything for a null string */
	if (entry_text[0] != 0) {
	    strncpy(history[cur_history_position], entry_text, MAX_COMMAND_LEN);
	    history[cur_history_position][MAX_COMMAND_LEN] = 0;
	    cur_history_position++;
	    cur_history_position %= MAX_HISTORY;
	    scroll_history_position = cur_history_position;
	    extended_command(entry_text);
	}
    }
    gtk_entry_set_text(GTK_ENTRY(entrytext),"");
    gtk_widget_grab_focus (GTK_WIDGET(gtkwin_info_text));
}

static gboolean
info_text_button_press_event (GtkWidget *widget, GdkEventButton *event,
                              gpointer user_data)
{
  GtkAdjustment *vadj;
  gboolean shifted;
  gfloat v_value;

  vadj = GTK_TEXT (widget)->vadj;
  v_value = vadj->value;

  shifted = (event->state & GDK_SHIFT_MASK) != 0;

  switch (event->button)
  {
    case 4:
      if (shifted)
        v_value -= vadj->page_size;
      else
        v_value -= vadj->step_increment * 5;
      break;

    case 5:
      if (shifted)
        v_value += vadj->page_size;
      else
        v_value += vadj->step_increment * 5;
      break;

    default:
      return FALSE;
  }

  v_value = CLAMP (v_value, vadj->lower, vadj->upper - vadj->page_size);

  gtk_adjustment_set_value (vadj, v_value);

  return TRUE;
}

static int get_info_display(GtkWidget *frame) {
    GtkWidget *box1;
    GtkWidget *box2;
    GtkWidget *tablet;
    GtkWidget *vscrollbar;
    FILE *infile;
    GtkWidget *vpane;

    box1 = gtk_vbox_new (FALSE, 0);
    if (splitinfo) {
	vpane = gtk_vpaned_new();
	gtk_container_add (GTK_CONTAINER (frame), vpane);
	gtk_widget_show(vpane);
	gtk_paned_add2(GTK_PANED(vpane), box1);
    } else {
	gtk_container_add (GTK_CONTAINER (frame), box1);
    }
    gtk_widget_show (box1);
  
    box2 = gtk_vbox_new (FALSE, 3);
    gtk_container_border_width (GTK_CONTAINER (box2), 3);
    gtk_box_pack_start (GTK_BOX (box1), box2, TRUE, TRUE, 0);
    gtk_widget_show (box2);
  
  
    tablet = gtk_table_new (2, 2, FALSE);
    gtk_table_set_row_spacing (GTK_TABLE (tablet), 0, 2);
    gtk_table_set_col_spacing (GTK_TABLE (tablet), 0, 2);
    gtk_box_pack_start (GTK_BOX (box2), tablet, TRUE, TRUE, 0);
    gtk_widget_show (tablet);
  
    text_hadj = gtk_adjustment_new(1, 0, 1, 0.01, 0.1, 40);
    text_vadj = gtk_adjustment_new(1, 0, 1, 0.01, 0.1, 40);

    gtkwin_info_text = gtk_text_new (GTK_ADJUSTMENT(text_hadj),GTK_ADJUSTMENT(text_vadj));
    gtk_text_set_editable (GTK_TEXT (gtkwin_info_text), FALSE);
    gtk_table_attach (GTK_TABLE (tablet), gtkwin_info_text, 0, 1, 0, 1,
		    GTK_EXPAND | GTK_SHRINK | GTK_FILL,
		    GTK_EXPAND | GTK_SHRINK | GTK_FILL, 0, 0);
    gtk_widget_show (gtkwin_info_text);

  
    vscrollbar = gtk_vscrollbar_new (GTK_TEXT (gtkwin_info_text)->vadj);
    gtk_table_attach (GTK_TABLE (tablet), vscrollbar, 1, 2, 0, 1,
		     GTK_FILL, GTK_EXPAND | GTK_SHRINK | GTK_FILL, 0, 0);
    gtk_widget_show (vscrollbar);

    gtk_signal_connect (GTK_OBJECT (gtkwin_info_text), "button_press_event",
                      GTK_SIGNAL_FUNC (info_text_button_press_event),
                      vscrollbar);

    gtk_text_freeze (GTK_TEXT (gtkwin_info_text));
  
    gtk_widget_realize (gtkwin_info_text);

    if (splitinfo) {

	box1 = gtk_vbox_new (FALSE, 0);
	gtk_widget_show (box1);
	gtk_paned_add1(GTK_PANED(vpane), box1);

	tablet = gtk_table_new (2, 2, FALSE);
	gtk_table_set_row_spacing (GTK_TABLE (tablet), 0, 2);
	gtk_table_set_col_spacing (GTK_TABLE (tablet), 0, 2);
	gtk_box_pack_start (GTK_BOX (box1), tablet, TRUE, TRUE, 0);
	gtk_widget_show (tablet);

	text_hadj2 = gtk_adjustment_new(1, 0, 1, 0.01, 0.1, 40);
	text_vadj2 = gtk_adjustment_new(1, 0, 1, 0.01, 0.1, 40);

	gtkwin_info_text2 = gtk_text_new (GTK_ADJUSTMENT(text_hadj2),GTK_ADJUSTMENT(text_vadj2));

	gtk_text_set_editable (GTK_TEXT (gtkwin_info_text2), FALSE);
	gtk_table_attach (GTK_TABLE (tablet), gtkwin_info_text2, 0, 1, 0, 1,
		    GTK_EXPAND | GTK_SHRINK | GTK_FILL,
		    GTK_EXPAND | GTK_SHRINK | GTK_FILL, 0, 0);
	gtk_widget_show (gtkwin_info_text2);
  
	vscrollbar = gtk_vscrollbar_new (GTK_TEXT (gtkwin_info_text2)->vadj);
	gtk_table_attach (GTK_TABLE (tablet), vscrollbar, 1, 2, 0, 1,
		     GTK_FILL, GTK_EXPAND | GTK_SHRINK | GTK_FILL, 0, 0);
	gtk_widget_show (vscrollbar);
	gtk_signal_connect (GTK_OBJECT (gtkwin_info_text2), "button_press_event",
                      GTK_SIGNAL_FUNC (info_text_button_press_event),
                      vscrollbar);

	gtk_widget_realize (gtkwin_info_text2);
    }
  
  infile = fopen("Welcome", "r");
  
  if (infile)
    {
      char buffer[1024];
      int nchars;
      
      while (1)
	{
	  nchars = fread(buffer, 1, 1024, infile);
	  gtk_text_insert (GTK_TEXT (gtkwin_info_text), NULL, NULL,
			   NULL, buffer, nchars);
	  
	  if (nchars < 1024)
	    break;
	}
      
      fclose (infile);
    }
  
  gtk_text_thaw (GTK_TEXT (gtkwin_info_text));
  
  
  entrytext = gtk_entry_new ();
  gtk_signal_connect(GTK_OBJECT(entrytext), "activate",
		     GTK_SIGNAL_FUNC(enter_callback),
		     entrytext);
  gtk_box_pack_start (GTK_BOX (box2),entrytext, FALSE, TRUE, 0);
  GTK_WIDGET_SET_FLAGS (entrytext, GTK_CAN_DEFAULT);
  gtk_widget_grab_default (entrytext);
  gtk_widget_show (entrytext);
  
  return 0;
}

/* Various replies */

static void sendstr(char *sendstr)
{
  gtk_widget_destroy (dialog_window);
  send_reply(sendstr);
  cpl.input_state = Playing;
}


/* This is similar to draw_info below, but doesn't advance to a new
 * line.  Generally, queries use this function to draw the prompt for
 * the name, password, etc.
 */



static void dialog_callback(GtkWidget *dialog)
{
  gchar *dialog_text;
  dialog_text = gtk_entry_get_text(GTK_ENTRY(dialogtext));
  
  gtk_widget_destroy (dialog_window);
  send_reply(dialog_text);
  cpl.input_state = Playing;
}


/* Draw a prompt dialog window */
/* Ok, now this is trying to be smart and decide what sort of dialog is
 * wanted.
 */

void
draw_prompt (const char *str)
{
    GtkWidget *dbox;
    GtkWidget *hbox;
    GtkWidget *dialoglabel;
    GtkWidget *yesbutton, *nobutton;
    GtkWidget *strbutton, *dexbutton, *conbutton, *intbutton, *wisbutton,
	*powbutton, *chabutton;

    gint    found = FALSE;

    if (nopopups)
      {
	draw_info(str, NDI_BLACK);

      }
    else
      {
	  dialog_window = gtk_window_new (GTK_WINDOW_DIALOG);

	  gtk_window_set_policy (GTK_WINDOW (dialog_window), TRUE, TRUE,
				 FALSE);
	  gtk_window_set_title (GTK_WINDOW (dialog_window), "Dialog");
	  gtk_window_set_transient_for (GTK_WINDOW (dialog_window),
					GTK_WINDOW (gtkwin_root));

	  dbox = gtk_vbox_new (FALSE, 6);
	  gtk_container_add (GTK_CONTAINER (dialog_window), dbox);

	  /* Ok, here we start generating the contents */

	  /*  printf ("Last info draw: %s\n", last_str); */
	  while (!found)
	    {
		if (!strcmp (str, ":"))
		  {
		      if (!strcmp (last_str, "What is your name?"))
			{

			    dialoglabel =
				gtk_label_new ("What is your name?");
			    gtk_box_pack_start (GTK_BOX (dbox), dialoglabel,
						FALSE, TRUE, 6);
			    gtk_widget_show (dialoglabel);

			    hbox = gtk_hbox_new (FALSE, 6);
			    dialogtext = gtk_entry_new ();
			    gtk_signal_connect (GTK_OBJECT (dialogtext),
						"activate",
						GTK_SIGNAL_FUNC
						(dialog_callback),
						dialog_window);
			    gtk_box_pack_start (GTK_BOX (hbox), dialogtext,
						TRUE, TRUE, 6);
			    gtk_box_pack_start (GTK_BOX (dbox), hbox, FALSE,
						TRUE, 6);

			    gtk_widget_show (hbox);
			    gtk_widget_show (dialogtext);
			    gtk_widget_grab_focus (dialogtext);
			    found = TRUE;
			    continue;
			}

		      if (!strcmp (last_str, "What is your password?"))
			{

			    dialoglabel =
				gtk_label_new ("What is your password?");
			    gtk_box_pack_start (GTK_BOX (dbox), dialoglabel,
						FALSE, TRUE, 6);
			    gtk_widget_show (dialoglabel);

			    hbox = gtk_hbox_new (FALSE, 6);
			    dialogtext = gtk_entry_new ();
			    gtk_entry_set_visibility (GTK_ENTRY (dialogtext),
						      FALSE);
			    gtk_signal_connect (GTK_OBJECT (dialogtext),
						"activate",
						GTK_SIGNAL_FUNC
						(dialog_callback),
						dialog_window);
			    gtk_box_pack_start (GTK_BOX (hbox), dialogtext,
						TRUE, TRUE, 6);
			    gtk_box_pack_start (GTK_BOX (dbox), hbox, FALSE,
						TRUE, 6);

			    gtk_widget_show (hbox);

			    gtk_widget_show (dialogtext);
			    gtk_widget_grab_focus (dialogtext);
			    found = TRUE;
			    continue;;
			}
		      if (!strcmp
			  (last_str, "Please type your password again."))
			{

			    dialoglabel =
				gtk_label_new
				("Please type your password again.");
			    gtk_box_pack_start (GTK_BOX (dbox), dialoglabel,
						FALSE, TRUE, 6);
			    gtk_widget_show (dialoglabel);

			    hbox = gtk_hbox_new (FALSE, 6);
			    dialogtext = gtk_entry_new ();
			    gtk_entry_set_visibility (GTK_ENTRY (dialogtext),
						      FALSE);
			    gtk_signal_connect (GTK_OBJECT (dialogtext),
						"activate",
						GTK_SIGNAL_FUNC
						(dialog_callback),
						dialog_window);
			    gtk_box_pack_start (GTK_BOX (hbox), dialogtext,
						TRUE, TRUE, 6);
			    gtk_box_pack_start (GTK_BOX (dbox), hbox, FALSE,
						TRUE, 6);

			    gtk_widget_show (hbox);
			    gtk_widget_show (dialogtext);
			    gtk_widget_grab_focus (dialogtext);
			    found = TRUE;
			    continue;
			}
		  }
		/* Ok, tricky ones. */
		if (!strcmp (last_str, "[1-7] [1-7] to swap stats.")
		    || !strncmp (last_str, "Str d", 5)
		    || !strncmp (last_str, "Dex d", 5)
		    || !strncmp (last_str, "Con d", 5)
		    || !strncmp (last_str, "Int d", 5)
		    || !strncmp (last_str, "Wis d", 5)
		    || !strncmp (last_str, "Pow d", 5)
		    || !strncmp (last_str, "Cha d", 5))
		  {

		      dialoglabel =
			  gtk_label_new ("Roll again or exchange ability.");
		      gtk_box_pack_start (GTK_BOX (dbox), dialoglabel, FALSE,
					  TRUE, 6);
		      gtk_widget_show (dialoglabel);

		      hbox = gtk_hbox_new (TRUE, 2);
		      strbutton = gtk_button_new_with_label ("Str");
		      gtk_box_pack_start (GTK_BOX (hbox), strbutton, TRUE,
					  TRUE, 1);
		      gtk_signal_connect_object (GTK_OBJECT (strbutton),
						 "clicked",
						 GTK_SIGNAL_FUNC (sendstr),
						 GINT_TO_POINTER ("1"));


		      dexbutton = gtk_button_new_with_label ("Dex");
		      gtk_box_pack_start (GTK_BOX (hbox), dexbutton, TRUE,
					  TRUE, 1);
		      gtk_signal_connect_object (GTK_OBJECT (dexbutton),
						 "clicked",
						 GTK_SIGNAL_FUNC (sendstr),
						 GINT_TO_POINTER ("2"));

		      conbutton = gtk_button_new_with_label ("Con");
		      gtk_box_pack_start (GTK_BOX (hbox), conbutton, TRUE,
					  TRUE, 1);
		      gtk_signal_connect_object (GTK_OBJECT (conbutton),
						 "clicked",
						 GTK_SIGNAL_FUNC (sendstr),
						 GINT_TO_POINTER ("3"));

		      intbutton = gtk_button_new_with_label ("Int");
		      gtk_box_pack_start (GTK_BOX (hbox), intbutton, TRUE,
					  TRUE, 1);
		      gtk_signal_connect_object (GTK_OBJECT (intbutton),
						 "clicked",
						 GTK_SIGNAL_FUNC (sendstr),
						 GINT_TO_POINTER ("4"));

		      wisbutton = gtk_button_new_with_label ("Wis");
		      gtk_box_pack_start (GTK_BOX (hbox), wisbutton, TRUE,
					  TRUE, 1);
		      gtk_signal_connect_object (GTK_OBJECT (wisbutton),
						 "clicked",
						 GTK_SIGNAL_FUNC (sendstr),
						 GINT_TO_POINTER ("5"));

		      powbutton = gtk_button_new_with_label ("Pow");
		      gtk_box_pack_start (GTK_BOX (hbox), powbutton, TRUE,
					  TRUE, 1);
		      gtk_signal_connect_object (GTK_OBJECT (powbutton),
						 "clicked",
						 GTK_SIGNAL_FUNC (sendstr),
						 GINT_TO_POINTER ("6"));

		      chabutton = gtk_button_new_with_label ("Cha");
		      gtk_box_pack_start (GTK_BOX (hbox), chabutton, TRUE,
					  TRUE, 1);
		      gtk_signal_connect_object (GTK_OBJECT (chabutton),
						 "clicked",
						 GTK_SIGNAL_FUNC (sendstr),
						 GINT_TO_POINTER ("7"));

		      gtk_widget_show (strbutton);
		      gtk_widget_show (dexbutton);
		      gtk_widget_show (conbutton);
		      gtk_widget_show (intbutton);
		      gtk_widget_show (wisbutton);
		      gtk_widget_show (powbutton);
		      gtk_widget_show (chabutton);



		      gtk_box_pack_start (GTK_BOX (dbox), hbox, FALSE, TRUE,
					  6);
		      gtk_widget_show (hbox);

		      hbox = gtk_hbox_new (FALSE, 6);

		      yesbutton = gtk_button_new_with_label ("Roll again");
		      gtk_box_pack_start (GTK_BOX (hbox), yesbutton, TRUE,
					  TRUE, 6);
		      gtk_signal_connect_object (GTK_OBJECT (yesbutton),
						 "clicked",
						 GTK_SIGNAL_FUNC (sendstr),
						 GINT_TO_POINTER ("y"));

		      nobutton = gtk_button_new_with_label ("Keep this");
		      gtk_box_pack_start (GTK_BOX (hbox), nobutton, TRUE,
					  TRUE, 6);
		      gtk_signal_connect_object (GTK_OBJECT (nobutton),
						 "clicked",
						 GTK_SIGNAL_FUNC (sendstr),
						 GINT_TO_POINTER ("n"));

		      gtk_box_pack_start (GTK_BOX (dbox), hbox, FALSE, TRUE,
					  6);

		      gtk_widget_show (yesbutton);
		      gtk_widget_show (nobutton);
		      gtk_widget_show (hbox);

		      found = TRUE;
		      continue;
		  }
		if (!strncmp (last_str, "Str -", 5) ||
		    !strncmp (last_str, "Dex -", 5)
		    || !strncmp (last_str, "Con -", 5)
		    || !strncmp (last_str, "Int -", 5)
		    || !strncmp (last_str, "Wis -", 5)
		    || !strncmp (last_str, "Pow -", 5)
		    || !strncmp (last_str, "Cha -", 5))
		  {


		      dialoglabel =
			  gtk_label_new ("Exchange with which ability?");
		      gtk_box_pack_start (GTK_BOX (dbox), dialoglabel, FALSE,
					  TRUE, 6);
		      gtk_widget_show (dialoglabel);

		      hbox = gtk_hbox_new (TRUE, 2);
		      strbutton = gtk_button_new_with_label ("Str");
		      gtk_box_pack_start (GTK_BOX (hbox), strbutton, TRUE,
					  TRUE, 1);
		      gtk_signal_connect_object (GTK_OBJECT (strbutton),
						 "clicked",
						 GTK_SIGNAL_FUNC (sendstr),
						 GINT_TO_POINTER ("1"));


		      dexbutton = gtk_button_new_with_label ("Dex");
		      gtk_box_pack_start (GTK_BOX (hbox), dexbutton, TRUE,
					  TRUE, 1);
		      gtk_signal_connect_object (GTK_OBJECT (dexbutton),
						 "clicked",
						 GTK_SIGNAL_FUNC (sendstr),
						 GINT_TO_POINTER ("2"));

		      conbutton = gtk_button_new_with_label ("Con");
		      gtk_box_pack_start (GTK_BOX (hbox), conbutton, TRUE,
					  TRUE, 1);
		      gtk_signal_connect_object (GTK_OBJECT (conbutton),
						 "clicked",
						 GTK_SIGNAL_FUNC (sendstr),
						 GINT_TO_POINTER ("3"));

		      intbutton = gtk_button_new_with_label ("Int");
		      gtk_box_pack_start (GTK_BOX (hbox), intbutton, TRUE,
					  TRUE, 1);
		      gtk_signal_connect_object (GTK_OBJECT (intbutton),
						 "clicked",
						 GTK_SIGNAL_FUNC (sendstr),
						 GINT_TO_POINTER ("4"));

		      wisbutton = gtk_button_new_with_label ("Wis");
		      gtk_box_pack_start (GTK_BOX (hbox), wisbutton, TRUE,
					  TRUE, 1);
		      gtk_signal_connect_object (GTK_OBJECT (wisbutton),
						 "clicked",
						 GTK_SIGNAL_FUNC (sendstr),
						 GINT_TO_POINTER ("5"));

		      powbutton = gtk_button_new_with_label ("Pow");
		      gtk_box_pack_start (GTK_BOX (hbox), powbutton, TRUE,
					  TRUE, 1);
		      gtk_signal_connect_object (GTK_OBJECT (powbutton),
						 "clicked",
						 GTK_SIGNAL_FUNC (sendstr),
						 GINT_TO_POINTER ("6"));

		      chabutton = gtk_button_new_with_label ("Cha");
		      gtk_box_pack_start (GTK_BOX (hbox), chabutton, TRUE,
					  TRUE, 1);
		      gtk_signal_connect_object (GTK_OBJECT (chabutton),
						 "clicked",
						 GTK_SIGNAL_FUNC (sendstr),
						 GINT_TO_POINTER ("7"));

		      gtk_widget_show (strbutton);
		      gtk_widget_show (dexbutton);
		      gtk_widget_show (conbutton);
		      gtk_widget_show (intbutton);
		      gtk_widget_show (wisbutton);
		      gtk_widget_show (powbutton);
		      gtk_widget_show (chabutton);


		      gtk_box_pack_start (GTK_BOX (dbox), hbox, FALSE, TRUE,
					  6);
		      gtk_widget_show (hbox);

		      found = TRUE;
		      continue;
		  }

		if (!strncmp (last_str, "Press `d'", 9))
		  {


		      dialoglabel = gtk_label_new ("Choose a character.");
		      gtk_box_pack_start (GTK_BOX (dbox), dialoglabel, FALSE,
					  TRUE, 6);
		      gtk_widget_show (dialoglabel);

		      hbox = gtk_hbox_new (FALSE, 6);

		      yesbutton = gtk_button_new_with_label ("Show next");
		      gtk_box_pack_start (GTK_BOX (hbox), yesbutton, TRUE,
					  TRUE, 6);
		      gtk_signal_connect_object (GTK_OBJECT (yesbutton),
						 "clicked",
						 GTK_SIGNAL_FUNC (sendstr),
						 GINT_TO_POINTER (" "));

		      nobutton = gtk_button_new_with_label ("Keep this");
		      gtk_box_pack_start (GTK_BOX (hbox), nobutton, TRUE,
					  TRUE, 6);
		      gtk_signal_connect_object (GTK_OBJECT (nobutton),
						 "clicked",
						 GTK_SIGNAL_FUNC (sendstr),
						 GINT_TO_POINTER ("d"));

		      gtk_box_pack_start (GTK_BOX (dbox), hbox, FALSE, TRUE,
					  6);

		      gtk_widget_show (yesbutton);
		      gtk_widget_show (nobutton);
		      gtk_widget_show (hbox);

		      found = TRUE;
		      continue;
		  }

		if (!strncmp (str, "Do you want to play", 18))
		  {


		      dialoglabel =
			  gtk_label_new ("Do you want to play again?");
		      gtk_box_pack_start (GTK_BOX (dbox), dialoglabel, FALSE,
					  TRUE, 6);
		      gtk_widget_show (dialoglabel);

		      hbox = gtk_hbox_new (FALSE, 6);

		      yesbutton = gtk_button_new_with_label ("Play again");
		      gtk_box_pack_start (GTK_BOX (hbox), yesbutton, TRUE,
					  TRUE, 6);
		      gtk_signal_connect_object (GTK_OBJECT (yesbutton),
						 "clicked",
						 GTK_SIGNAL_FUNC (sendstr),
						 GINT_TO_POINTER ("a"));

		      nobutton = gtk_button_new_with_label ("Quit");
		      gtk_box_pack_start (GTK_BOX (hbox), nobutton, TRUE,
					  TRUE, 6);
		      gtk_signal_connect_object (GTK_OBJECT (nobutton),
						 "clicked",
						 GTK_SIGNAL_FUNC (sendstr),
						 GINT_TO_POINTER ("q"));

		      gtk_box_pack_start (GTK_BOX (dbox), hbox, FALSE, TRUE,
					  6);

		      gtk_widget_show (yesbutton);
		      gtk_widget_show (nobutton);
		      gtk_widget_show (hbox);

		      found = TRUE;
		      continue;
		  }

		if (!strncmp (str, "Are you sure you want", 21))
		  {


		      dialoglabel =
			  gtk_label_new ("Are you sure you want to quit?");
		      gtk_box_pack_start (GTK_BOX (dbox), dialoglabel, FALSE,
					  TRUE, 6);
		      gtk_widget_show (dialoglabel);

		      hbox = gtk_hbox_new (FALSE, 6);

		      yesbutton = gtk_button_new_with_label ("Yes, quit");
		      gtk_box_pack_start (GTK_BOX (hbox), yesbutton, TRUE,
					  TRUE, 6);
		      gtk_signal_connect_object (GTK_OBJECT (yesbutton),
						 "clicked",
						 GTK_SIGNAL_FUNC (sendstr),
						 GINT_TO_POINTER ("y"));

		      nobutton = gtk_button_new_with_label ("Don't quit");
		      gtk_box_pack_start (GTK_BOX (hbox), nobutton, TRUE,
					  TRUE, 6);
		      gtk_signal_connect_object (GTK_OBJECT (nobutton),
						 "clicked",
						 GTK_SIGNAL_FUNC (sendstr),
						 GINT_TO_POINTER ("n"));

		      gtk_box_pack_start (GTK_BOX (dbox), hbox, FALSE, TRUE,
					  6);

		      gtk_widget_show (yesbutton);
		      gtk_widget_show (nobutton);
		      gtk_widget_show (hbox);

		      found = TRUE;
		      continue;
		  }

		if (!found)
		  {
		      dialoglabel = gtk_label_new (str);
		      gtk_box_pack_start (GTK_BOX (dbox), dialoglabel, FALSE,
					  TRUE, 6);
		      gtk_widget_show (dialoglabel);

		      hbox = gtk_hbox_new (FALSE, 6);
		      dialogtext = gtk_entry_new ();

		      gtk_signal_connect (GTK_OBJECT (dialogtext), "activate",
					  GTK_SIGNAL_FUNC (dialog_callback),
					  dialog_window);
		      gtk_box_pack_start (GTK_BOX (hbox), dialogtext, TRUE,
					  TRUE, 6);
		      gtk_box_pack_start (GTK_BOX (dbox), hbox, FALSE, TRUE,
					  6);

		      gtk_widget_show (hbox);
		      gtk_widget_show (dialogtext);
		      gtk_widget_grab_focus (dialogtext);
		      found = TRUE;
		      continue;
		  }
	    }

	  /* Finished with the contents. */


	  gtk_widget_show (dbox);
	  gtk_widget_show (dialog_window);
      }

}

/* draw_info adds a line to the info window. For speed reasons it will 
 * automatically freeze the info window when adding text to it, set the
 * draw_info_freeze variable true and the actual drawing will take place
 * during the next do_timeout at which point it is unfrozen again. That way
 * we handle massive amounts of text addition with a single gui event, which
 * results in a serious speed improvement for slow client machines (and
 * above all it avoids a gui lockup when the client becomes congested with
 * updates (which is often when you're in the middle of fighting something 
 * serious and not a good time to get slow reaction time)). 
 *
 * MSW 2001-05-25: The removal of input from the text windows should
 * work, and in fact does about 90% of the time.  But that 10% it
 * doesn't, the client crashes.  The error itself is in the gtk library,
 * to hopefully they will fix it someday.  The reason to do this is
 * to keep these buffers a reasonable size so that performance stays
 * good - otherewise, performance slowly degrades.
 */

void draw_info(const char *str, int color) {
    int ncolor = color;
  
    if (ncolor==NDI_WHITE) {
	ncolor=NDI_BLACK;
    }

    strcpy (last_str, str);
    if (splitinfo && color != NDI_BLACK) {
	if (!draw_info_freeze2){
	    gtk_text_freeze (GTK_TEXT (gtkwin_info_text2));
	    draw_info_freeze2=TRUE;
	}
#ifdef TRIM_INFO_WINDOW
	info2_num_chars += strlen(str) + 1;
	/* Limit size of scrollback buffer. To be more efficient, delete a good
	 * blob (5000) characters at a time - in that way, there will be some
	 * time between needing to delete.
	 */
	if (info2_num_chars > info2_max_chars ) {
	    gtk_text_set_point(GTK_TEXT(gtkwin_info_text2),0);
	    gtk_text_forward_delete(GTK_TEXT(gtkwin_info_text2), (info2_num_chars - info2_max_chars) + 5000);
	    info2_num_chars = gtk_text_get_length(GTK_TEXT(gtkwin_info_text2));
	    gtk_text_set_point(GTK_TEXT(gtkwin_info_text2), info2_num_chars);
	    fprintf(stderr,"reduced output buffer2 to %d chars\n", info1_num_chars);
	}
#endif
	gtk_text_insert (GTK_TEXT (gtkwin_info_text2), NULL, &root_color[ncolor], NULL, str , -1);
	gtk_text_insert (GTK_TEXT (gtkwin_info_text2), NULL, &root_color[ncolor], NULL, "\n" , -1);

    } else {
	/* all nootes in the above section apply here also */
	if (!draw_info_freeze1){
	    gtk_text_freeze (GTK_TEXT (gtkwin_info_text));
	    draw_info_freeze1=TRUE;
	}
#ifdef TRIM_INFO_WINDOW
	info1_num_chars += strlen(str) + 1;
	if (info1_num_chars > info1_max_chars ) {
	    gtk_text_set_point(GTK_TEXT(gtkwin_info_text),0);
	    gtk_text_forward_delete(GTK_TEXT(gtkwin_info_text), (info1_num_chars - info1_max_chars) + 5000);
	    info1_num_chars = gtk_text_get_length(GTK_TEXT(gtkwin_info_text));
	    gtk_text_set_point(GTK_TEXT(gtkwin_info_text), info1_num_chars);
	}
#endif
	gtk_text_insert (GTK_TEXT (gtkwin_info_text), NULL, &root_color[ncolor], NULL, str , -1);
	gtk_text_insert (GTK_TEXT (gtkwin_info_text), NULL, &root_color[ncolor], NULL, "\n" , -1);
    }
}


void draw_color_info(int colr, const char *buf){

  if (color_text){

    draw_info(buf,colr);

  }
  else{
    draw_info("==========================================",NDI_BLACK);
    draw_info(buf,NDI_BLACK);
    draw_info("==========================================",NDI_BLACK);
  }
}




/***********************************************************************
 *
 * Stats window functions follow
 *
 ***********************************************************************/

static int get_stats_display(GtkWidget *frame) {
  /*  GtkWidget *label_playername;*/
  GtkWidget *stats_vbox;
  GtkWidget *stats_box_1;
  GtkWidget *stats_box_2;
  GtkWidget *stats_box_4;
  GtkWidget *stats_box_5;
  GtkWidget *stats_box_6;
  GtkWidget *stats_box_7;
  

      stats_vbox = gtk_vbox_new (FALSE, 0);
      /* 1st row */
      stats_box_1 = gtk_hbox_new (FALSE, 0);

      statwindow.playername = gtk_label_new("Player: ");
      gtk_box_pack_start (GTK_BOX (stats_box_1), statwindow.playername, FALSE, FALSE, 5);
      gtk_widget_show (statwindow.playername);

      gtk_box_pack_start (GTK_BOX (stats_vbox), stats_box_1, FALSE, FALSE, 0);
      gtk_widget_show (stats_box_1);

      /* 2nd row */

      stats_box_2 = gtk_hbox_new (FALSE, 0);
      statwindow.score = gtk_label_new("Score: 0");
      gtk_box_pack_start (GTK_BOX (stats_box_2), statwindow.score, FALSE, FALSE, 5);
      gtk_widget_show (statwindow.score);

      statwindow.level = gtk_label_new("Level: 0");
      gtk_box_pack_start (GTK_BOX (stats_box_2), statwindow.level, FALSE, FALSE, 5);
      gtk_widget_show (statwindow.level);

      gtk_box_pack_start (GTK_BOX (stats_vbox), stats_box_2, FALSE, FALSE, 0);
      gtk_widget_show (stats_box_2);
      

      /* 4th row */
      stats_box_4 = gtk_hbox_new (FALSE, 0);

      statwindow.Str = gtk_label_new("S 0");
      gtk_box_pack_start (GTK_BOX (stats_box_4), statwindow.Str, FALSE, FALSE, 5);
      gtk_widget_show (statwindow.Str);

      statwindow.Dex = gtk_label_new("D 0");
      gtk_box_pack_start (GTK_BOX (stats_box_4), statwindow.Dex, FALSE, FALSE, 5);
      gtk_widget_show (statwindow.Dex);

      statwindow.Con = gtk_label_new("Co 0");
      gtk_box_pack_start (GTK_BOX (stats_box_4), statwindow.Con, FALSE, FALSE, 5);
      gtk_widget_show (statwindow.Con);

      statwindow.Int = gtk_label_new("I 0");
      gtk_box_pack_start (GTK_BOX (stats_box_4), statwindow.Int, FALSE, FALSE, 5);
      gtk_widget_show (statwindow.Int);

      statwindow.Wis = gtk_label_new("W 0");
      gtk_box_pack_start (GTK_BOX (stats_box_4), statwindow.Wis, FALSE, FALSE, 5);
      gtk_widget_show (statwindow.Wis);

      statwindow.Pow = gtk_label_new("P 0");
      gtk_box_pack_start (GTK_BOX (stats_box_4), statwindow.Pow, FALSE, FALSE, 5);
      gtk_widget_show (statwindow.Pow);

      statwindow.Cha = gtk_label_new("Ch 0");
      gtk_box_pack_start (GTK_BOX (stats_box_4), statwindow.Cha, FALSE, FALSE, 5);
      gtk_widget_show (statwindow.Cha);

      gtk_box_pack_start (GTK_BOX (stats_vbox), stats_box_4, FALSE, FALSE, 0);
      gtk_widget_show (stats_box_4);

      /* 5th row */

      stats_box_5 = gtk_hbox_new (FALSE, 0);

      statwindow.wc = gtk_label_new("Wc: 0");
      gtk_box_pack_start (GTK_BOX (stats_box_5), statwindow.wc, FALSE, FALSE, 5);
      gtk_widget_show (statwindow.wc);

      statwindow.dam = gtk_label_new("Dam: 0");
      gtk_box_pack_start (GTK_BOX (stats_box_5), statwindow.dam, FALSE, FALSE, 5);
      gtk_widget_show (statwindow.dam);

      statwindow.ac = gtk_label_new("Ac: 0");
      gtk_box_pack_start (GTK_BOX (stats_box_5), statwindow.ac, FALSE, FALSE, 5);
      gtk_widget_show (statwindow.ac);

      statwindow.armor = gtk_label_new("Armor: 0");
      gtk_box_pack_start (GTK_BOX (stats_box_5), statwindow.armor, FALSE, FALSE, 5);
      gtk_widget_show (statwindow.armor);

      gtk_box_pack_start (GTK_BOX (stats_vbox), stats_box_5, FALSE, FALSE, 0);
      gtk_widget_show (stats_box_5);

      /* 6th row */

      stats_box_6 = gtk_hbox_new (FALSE, 0);

      statwindow.speed = gtk_label_new("Speed: 0");
      gtk_box_pack_start (GTK_BOX (stats_box_6), statwindow.speed, FALSE, FALSE, 5);
      gtk_widget_show (statwindow.speed);

      gtk_box_pack_start (GTK_BOX (stats_vbox), stats_box_6, FALSE, FALSE, 0);
      gtk_widget_show (stats_box_6);

      /* 7th row */

      stats_box_7 = gtk_hbox_new (FALSE, 0);

      statwindow.skill = gtk_label_new("Skill: 0");
      gtk_box_pack_start (GTK_BOX (stats_box_7), statwindow.skill, FALSE, FALSE, 5);
      gtk_widget_show (statwindow.skill);


      gtk_box_pack_start (GTK_BOX (stats_vbox), stats_box_7, FALSE, FALSE, 0);
      gtk_widget_show (stats_box_7);



      gtk_container_add (GTK_CONTAINER (frame), stats_vbox);
      gtk_widget_show (stats_vbox);


   return 0;
}

/* This draws the stats window.  If redraw is true, it means
 * we need to redraw the entire thing, and not just do an
 * updated.
 */

void draw_stats(int redraw) {
  float weap_sp;
  char buff[MAX_BUF];

  static Stats last_stats = {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0};
  static char last_name[MAX_BUF]="", last_range[MAX_BUF]="";

  if (updatelock < 25) {
    updatelock++;
    if (strcmp(cpl.title, last_name) || redraw) {
      strcpy(last_name,cpl.title);
      strcpy(buff,cpl.title);
      gtk_label_set (GTK_LABEL(statwindow.playername), cpl.title);
      gtk_widget_draw (statwindow.playername, NULL);
    }
    
    if(redraw || cpl.stats.exp!=last_stats.exp) {
      last_stats.exp = cpl.stats.exp;
      sprintf(buff,"Score: %5d",cpl.stats.exp);
      gtk_label_set (GTK_LABEL(statwindow.score), buff);
      gtk_widget_draw (statwindow.score, NULL);
    }
    
    if(redraw || cpl.stats.level!=last_stats.level) {
      last_stats.level = cpl.stats.level;
      sprintf(buff,"Level: %d",cpl.stats.level);
      gtk_label_set (GTK_LABEL(statwindow.level), buff);
      gtk_widget_draw (statwindow.level, NULL);
    }
    
    if(redraw || 
       cpl.stats.hp!=last_stats.hp || cpl.stats.maxhp!=last_stats.maxhp) {
      last_stats.hp=cpl.stats.hp;
      last_stats.maxhp=cpl.stats.maxhp;
      sprintf(buff,"Hp: %d/%d",cpl.stats.hp, cpl.stats.maxhp);
      gtk_label_set (GTK_LABEL(statwindow.hp), buff);
      gtk_widget_draw (statwindow.hp, NULL);
    }
    
    if(redraw || 
       cpl.stats.sp!=last_stats.sp || cpl.stats.maxsp!=last_stats.maxsp) {
      last_stats.sp=cpl.stats.sp;
      last_stats.maxsp=cpl.stats.maxsp;
      sprintf(buff,"Sp: %d/%d",cpl.stats.sp, cpl.stats.maxsp);
      gtk_label_set (GTK_LABEL(statwindow.sp), buff);
      gtk_widget_draw (statwindow.sp, NULL);
    }
    
    if(redraw || 
       cpl.stats.grace!=last_stats.grace || cpl.stats.maxgrace!=last_stats.maxgrace) {
      last_stats.grace=cpl.stats.grace;
      last_stats.maxgrace=cpl.stats.maxgrace;
      sprintf(buff,"Gr: %d/%d",cpl.stats.grace, cpl.stats.maxgrace);
      gtk_label_set (GTK_LABEL(statwindow.gr), buff);
      gtk_widget_draw (statwindow.gr, NULL);
    }
    
    if(redraw || cpl.stats.Str!=last_stats.Str) {
      last_stats.Str=cpl.stats.Str;
      sprintf(buff,"S%2d",cpl.stats.Str);
      gtk_label_set (GTK_LABEL(statwindow.Str), buff);
      gtk_widget_draw (statwindow.Str, NULL);
    }
    
    if(redraw || cpl.stats.Dex!=last_stats.Dex) {
      last_stats.Dex=cpl.stats.Dex;
      sprintf(buff,"D%2d",cpl.stats.Dex);
      gtk_label_set (GTK_LABEL(statwindow.Dex), buff);
      gtk_widget_draw (statwindow.Dex, NULL);
    }
    
    if(redraw || cpl.stats.Con!=last_stats.Con) {
      last_stats.Con=cpl.stats.Con;
      sprintf(buff,"Co%2d",cpl.stats.Con);
      gtk_label_set (GTK_LABEL(statwindow.Con), buff);
      gtk_widget_draw (statwindow.Con, NULL);
    }
    
    if(redraw || cpl.stats.Int!=last_stats.Int) {
      last_stats.Int=cpl.stats.Int;
      sprintf(buff,"I%2d",cpl.stats.Int);
      gtk_label_set (GTK_LABEL(statwindow.Int), buff);
      gtk_widget_draw (statwindow.Int, NULL);
    }
    
    if(redraw || cpl.stats.Wis!=last_stats.Wis) {
      last_stats.Wis=cpl.stats.Wis;
      sprintf(buff,"W%2d",cpl.stats.Wis);
      gtk_label_set (GTK_LABEL(statwindow.Wis), buff);
      gtk_widget_draw (statwindow.Wis, NULL);
    }
    
    if(redraw || cpl.stats.Pow!=last_stats.Pow) {
      last_stats.Pow=cpl.stats.Pow;
      sprintf(buff,"P%2d",cpl.stats.Pow);
      gtk_label_set (GTK_LABEL(statwindow.Pow), buff);
      gtk_widget_draw (statwindow.Pow, NULL);
    }
    
    if(redraw || cpl.stats.Cha!=last_stats.Cha) {
      last_stats.Cha=cpl.stats.Cha;
      sprintf(buff,"Ch%2d",cpl.stats.Cha);
      gtk_label_set (GTK_LABEL(statwindow.Cha), buff);
      gtk_widget_draw (statwindow.Cha, NULL);
    }
    
    if(redraw || cpl.stats.wc!=last_stats.wc) {
      last_stats.wc=cpl.stats.wc;
      sprintf(buff,"Wc%3d",cpl.stats.wc);
      gtk_label_set (GTK_LABEL(statwindow.wc), buff);
      gtk_widget_draw (statwindow.wc, NULL);
    }
    
    if(redraw || cpl.stats.dam!=last_stats.dam) {
      last_stats.dam=cpl.stats.dam;
      sprintf(buff,"Dam%3d",cpl.stats.dam);
      gtk_label_set (GTK_LABEL(statwindow.dam), buff);
      gtk_widget_draw (statwindow.dam, NULL);
    }
    
    if(redraw || cpl.stats.ac!=last_stats.ac) {
      last_stats.ac=cpl.stats.ac;
      sprintf(buff,"Ac%3d",cpl.stats.ac);
      gtk_label_set (GTK_LABEL(statwindow.ac), buff);
      gtk_widget_draw (statwindow.ac, NULL);
    }
    
    if(redraw || cpl.stats.resists[0]!=last_stats.resists[0]) {
      last_stats.resists[0]=cpl.stats.resists[0];
      sprintf(buff,"Arm%3d",cpl.stats.resists[0]);
      gtk_label_set (GTK_LABEL(statwindow.armor), buff);
      gtk_widget_draw (statwindow.armor, NULL);
    }
    
    if(redraw || cpl.stats.speed!=last_stats.speed ||
       cpl.stats.weapon_sp != last_stats.weapon_sp) {
      last_stats.speed=cpl.stats.speed;
      last_stats.weapon_sp=cpl.stats.weapon_sp;
      weap_sp = (float) cpl.stats.speed/((float)cpl.stats.weapon_sp);
      sprintf(buff,"Speed: %3.2f (%1.2f)",(float)cpl.stats.speed/FLOAT_MULTF,weap_sp);
      gtk_label_set (GTK_LABEL(statwindow.speed), buff);
      gtk_widget_draw (statwindow.speed, NULL);
    }
    
    if(redraw || cpl.stats.food!=last_stats.food) {
      last_stats.food=cpl.stats.food;
      sprintf(buff,"Food: %3d",cpl.stats.food);
      gtk_label_set (GTK_LABEL(statwindow.food), buff);
      gtk_widget_draw (statwindow.food, NULL);
    }
    
    if(redraw || strcmp(cpl.range, last_range)) {
      strcpy(last_range, cpl.range);
      sprintf(buff,cpl.range);
      gtk_label_set (GTK_LABEL(statwindow.skill), buff);
      gtk_widget_draw (statwindow.skill, NULL);
    }
  }
}


/***********************************************************************
*
* Handles the message window
*
***********************************************************************/


void create_stat_bar (GtkWidget *mtable, gint row, gchar *label, gint bar, GtkWidget **plabel) {
  /*  GtkWidget *plabel;*/

  *plabel = gtk_label_new (label);
  gtk_table_attach (GTK_TABLE (mtable), *plabel, 0, 1, row, row+1,/*GTK_FILL |*/ GTK_EXPAND,GTK_FILL | GTK_EXPAND,0,0);
  gtk_widget_show (*plabel);

  vitals[bar].bar = gtk_progress_bar_new ();
  gtk_table_attach(GTK_TABLE(mtable), vitals[bar].bar, 0,1,row+1,row+2,GTK_FILL | GTK_EXPAND, 0 ,3,0);
  gtk_widget_set_usize (vitals[bar].bar,100,15);

    
  gtk_widget_show (vitals[bar].bar);



  vitals[bar].state=1;

  vitals[bar].style = gtk_style_new ();
  vitals[bar].style->bg[GTK_STATE_PRELIGHT] = gdk_green;
  gtk_widget_set_style (vitals[bar].bar, vitals[bar].style);
}


static int get_message_display(GtkWidget *frame) {
  GtkWidget *plabel;
  GtkWidget *mtable;
  GtkWidget *vbox;
  int i;

  vbox = gtk_vbox_new (TRUE, 0);
  gtk_container_add (GTK_CONTAINER(frame),vbox);

  mtable = gtk_table_new (2,9,FALSE);
  gtk_box_pack_start (GTK_BOX(vbox),mtable,TRUE,FALSE,0);



  create_stat_bar (mtable, 1,"Hp: 0",0, &statwindow.hp);

  create_stat_bar (mtable, 3,"Mana: 0",1, &statwindow.sp);

  create_stat_bar (mtable, 5,"Grace: 0",2, &statwindow.gr);

  create_stat_bar (mtable, 7,"Food: 0",3, &statwindow.food);


  plabel = gtk_label_new ("Status");
  gtk_table_attach(GTK_TABLE(mtable), plabel, 1,2,1,2,GTK_FILL | GTK_EXPAND,GTK_FILL | GTK_EXPAND,0,0);
  gtk_widget_show (plabel);

  fire_label = gtk_label_new ("    ");
  gtk_table_attach(GTK_TABLE(mtable), fire_label, 1,2,2,3,GTK_FILL | GTK_EXPAND,GTK_FILL | GTK_EXPAND,0,0);
  gtk_widget_show (fire_label);

  run_label = gtk_label_new ("   ");
  gtk_table_attach(GTK_TABLE(mtable), run_label, 1,2,3,4,GTK_FILL | GTK_EXPAND,GTK_FILL | GTK_EXPAND,0,0);
  gtk_widget_show (run_label);

  for (i=0; i<SHOW_RESISTS; i++) {
    resists[i] = gtk_label_new("          ");
    gtk_table_attach(GTK_TABLE(mtable), resists[i], 1,2,4+i, 5+i,GTK_FILL | GTK_EXPAND,GTK_FILL | GTK_EXPAND,0,0);
    gtk_widget_show (resists[i]);
  }
			  

  gtk_progress_bar_update (GTK_PROGRESS_BAR (vitals[0].bar), 1);
  gtk_progress_bar_update (GTK_PROGRESS_BAR (vitals[1].bar), 1);
  gtk_progress_bar_update (GTK_PROGRESS_BAR (vitals[2].bar), 1);
  gtk_progress_bar_update (GTK_PROGRESS_BAR (vitals[3].bar), 1);
  gtk_style_unref (vitals[0].style); 
  gtk_style_unref (vitals[1].style); 
  gtk_style_unref (vitals[2].style); 
  gtk_style_unref (vitals[3].style); 
  

  gtk_widget_show (mtable);
  gtk_widget_show (vbox);
   return 0;
}

#define MAX_BARS_MESSAGE 80

static void draw_stat_bar(int bar_pos, float bar, int is_alert)
{
 if (vitals[bar_pos].state!=is_alert) {
    if (is_alert) {
      vitals[bar_pos].style = gtk_style_new ();
      vitals[bar_pos].style->bg[GTK_STATE_PRELIGHT] = gdk_red;
      gtk_widget_set_style (vitals[bar_pos].bar, vitals[bar_pos].style);
      gtk_style_unref (vitals[bar_pos].style); 
      vitals[bar_pos].state=is_alert;
    }
    else {
      vitals[bar_pos].style = gtk_style_new ();
      vitals[bar_pos].style->bg[GTK_STATE_PRELIGHT] = gdk_green;
      gtk_widget_set_style (vitals[bar_pos].bar, vitals[bar_pos].style);
      gtk_style_unref (vitals[bar_pos].style);
      vitals[bar_pos].state=0;
    }
  }
 /* if (bar==0) {
   bar=(float)0.01;
 }*/
 gtk_progress_bar_update (GTK_PROGRESS_BAR (vitals[bar_pos].bar),bar );
 gtk_widget_draw (vitals[bar_pos].bar, NULL);
}

/* This updates the status bars.  If redraw, then redraw them
 * even if they have not changed
 */

void draw_message_window(int redraw) {
    float bar;
    int is_alert,flags;
    static uint16 scrollsize_hp=0, scrollsize_sp=0, scrollsize_food=0,
	scrollsize_grace=0;
    static uint8 scrollhp_alert=FALSE, scrollsp_alert=FALSE,
	scrollfood_alert=FALSE, scrollgrace_alert=FALSE;

    if (updatelock < 25) {
	updatelock++;
	/* draw hp bar */
	if(cpl.stats.maxhp>0)
	{
	    bar=(float)cpl.stats.hp/cpl.stats.maxhp;
	    if(bar<=0)
		bar=(float)0.01;
	    is_alert=(cpl.stats.hp <= cpl.stats.maxhp/4);
	}
	else
	{
	    bar=(float)0.01;
	    is_alert=0;
	}

	if (redraw || scrollsize_hp!=bar || scrollhp_alert!=is_alert)
	    draw_stat_bar(0, bar, is_alert);

	scrollsize_hp=bar;
	scrollhp_alert=is_alert;

	/* draw sp bar.  spellpoints can go above max
	 * spellpoints via supercharging with the transferrance spell,
	 * or taking off items that raise max spellpoints.
	 */
	if (cpl.stats.sp>cpl.stats.maxsp)
	    bar=(float)1;
	else
	    bar=(float)cpl.stats.sp/cpl.stats.maxsp;
	if(bar<=0) 
	    bar=(float)0.01;

	is_alert=(cpl.stats.sp <= cpl.stats.maxsp/4);

	if (redraw || scrollsize_sp!=bar || scrollsp_alert!=is_alert)
	    draw_stat_bar(1, bar, is_alert);

	scrollsize_sp=bar;
	scrollsp_alert=is_alert;

	/* draw grace bar. grace can go above max or below min */
	if (cpl.stats.grace>cpl.stats.maxgrace)
	    bar = MAX_BARS_MESSAGE;
	else
	    bar=(float)cpl.stats.grace/cpl.stats.maxgrace;
	if(bar<=0)
	    bar=(float)0.01;

	if (bar>1.0) {
	    bar=(float)1.0;
	}
	is_alert=(cpl.stats.grace <= cpl.stats.maxgrace/4);


	if (redraw || scrollsize_grace!=bar || scrollgrace_alert!=is_alert)
	    draw_stat_bar(2, bar, is_alert);

	scrollsize_grace=bar;
	scrollgrace_alert=is_alert;
  
	/* draw food bar */
	bar=(float)cpl.stats.food/999;
	if(bar<=0) 
	    bar=(float)0.01;
	is_alert=(cpl.stats.food <= 999/4);

	if (redraw || scrollsize_food!=bar || scrollfood_alert!=is_alert)
	    draw_stat_bar(3, bar, is_alert);

	scrollsize_food=bar;
	scrollfood_alert=is_alert;

	flags = cpl.stats.flags;

	if (redraw || cpl.stats.resist_change) {
	    int i,j=0;
	    char buf[40];

	    cpl.stats.resist_change=0;
	    for (i=0; i<NUM_RESISTS; i++) {
		if (cpl.stats.resists[i]) {
		    sprintf(buf,"%-10s %+4d",
			resists_name[i], cpl.stats.resists[i]);
		    gtk_label_set(GTK_LABEL(resists[j]), buf);
		    gtk_widget_draw(resists[j], NULL);
		    j++;
		    if (j >= SHOW_RESISTS) break;
		}
	    }
	    /* Erase old/unused resistances */
	    while (j<SHOW_RESISTS) {
		gtk_label_set(GTK_LABEL(resists[j]), "              ");
		gtk_widget_draw(resists[j], NULL);
		j++;
	    }
	} /* if we draw the resists */
    }
    else {
	/*    printf ("WARNING -- RACE. Frozen updates until updatelock is cleared!\n");*/
    }
}


 


/****************************************************************************
 *
 * Inventory window functions follow
 *
 ****************************************************************************/


/*
 * draw_all_list clears a window and after that draws all objects 
 * and a scrollbar
 */
static void draw_all_list(itemlist *l)
{
    int i;

    strcpy (l->old_title, "");

    for(i=0; i<l->size; i++) {
	copy_name(l->names[i], "");
	l->faces[i] = 0;
	l->icon1[i] = 0;
	l->icon2[i] = 0;
	l->icon3[i] = 0;
	l->icon4[i] = 0;
    }

    l->bar_size = 1;    /* so scroll bar is drawn */
    draw_list (l);

}

void open_container (item *op) 
{
  look_list.env = op;
  sprintf (look_list.title, "%s:", op->d_name);
  draw_list (&look_list);
}

void close_container (item *op) 
{
  if (look_list.env != cpl.below) {
    client_send_apply (look_list.env->tag);
    look_list.env = cpl.below;
    strcpy (look_list.title, "You see:");
    draw_list (&look_list);
  }
}


/* Handle mouse presses in the lists */
#ifdef GTK_HAVE_FEATURES_1_1_12
static void list_button_event (GtkWidget *gtklist, gint row, gint column, GdkEventButton *event, itemlist *l)
{
  item *tmp;
  if (event->button==1) {
      tmp = gtk_clist_get_row_data (GTK_CLIST(gtklist), row); 
      gtk_clist_unselect_row (GTK_CLIST(gtklist), row, 0);
      if (event->state & GDK_SHIFT_MASK)
	toggle_locked(tmp);
      else
	client_send_examine (tmp->tag);     

  }
  if (event->button==2) {
      tmp = gtk_clist_get_row_data (GTK_CLIST(gtklist), row); 

      gtk_clist_unselect_row (GTK_CLIST(gtklist), row, 0);
      if (event->state & GDK_SHIFT_MASK)
	send_mark_obj(tmp);
      else
	client_send_apply (tmp->tag);

  }
  if (event->button==3) {

    tmp = gtk_clist_get_row_data (GTK_CLIST(gtklist), row);
    gtk_clist_unselect_row (GTK_CLIST(gtklist), row, 0);
    
    if (tmp->locked) {
      draw_info ("This item is locked.",NDI_BLACK);
    } else if (l == &inv_list) {
      cpl.count = gtk_spin_button_get_value_as_int (GTK_SPIN_BUTTON(counttext));
      client_send_move (look_list.env->tag, tmp->tag, cpl.count);
      if (nopopups) {
	gtk_spin_button_set_value(GTK_SPIN_BUTTON(counttext),0.0);
        cpl.count=0;
      }
    }
    else {
      cpl.count = gtk_spin_button_get_value_as_int (GTK_SPIN_BUTTON(counttext));
      client_send_move (inv_list.env->tag, tmp->tag, cpl.count);
      cpl.count=0;
      
    }
    
  }
  
}
#else
static void list_button_event (GtkWidget *gtklist, GdkEventButton *event, itemlist *l)
{
  GList *node;
  item *tmp;
  if (event->type==GDK_BUTTON_PRESS && event->button==1) {
    if (GTK_CLIST(gtklist)->selection) {
      node =  GTK_CLIST(gtklist)->selection;
      tmp = gtk_clist_get_row_data (GTK_CLIST(gtklist), (gint)node->data); 
      gtk_clist_unselect_row (GTK_CLIST(gtklist), (gint)node->data, 0);
      if (event->state & GDK_SHIFT_MASK)
	toggle_locked(tmp);
      else
	client_send_examine (tmp->tag);     
    }
  }
  if (event->type==GDK_BUTTON_PRESS && event->button==2) {
    if (GTK_CLIST(gtklist)->selection) {
      node =  GTK_CLIST(gtklist)->selection;
      tmp = gtk_clist_get_row_data (GTK_CLIST(gtklist), (gint)node->data); 

      gtk_clist_unselect_row (GTK_CLIST(gtklist), (gint)node->data, 0);
      if (event->state & GDK_SHIFT_MASK)
	send_mark_obj(tmp);
      else
	client_send_apply (tmp->tag);
    }
  }
  if (event->type==GDK_BUTTON_PRESS && event->button==3) {
    if (GTK_CLIST(gtklist)->selection) {
      node =  GTK_CLIST(gtklist)->selection;
      tmp = gtk_clist_get_row_data (GTK_CLIST(gtklist), (gint)node->data);
      gtk_clist_unselect_row (GTK_CLIST(gtklist), (gint)node->data, 0);

      if (tmp->locked) {
	draw_info ("This item is locked.",NDI_BLACK);
      } else if (l == &inv_list) {
	cpl.count = gtk_spin_button_get_value_as_int (GTK_SPIN_BUTTON(counttext));
	client_send_move (look_list.env->tag, tmp->tag, cpl.count);
	if (nopopups) {
	    gtk_spin_button_set_value(GTK_SPIN_BUTTON(counttext),0.0);
	    cpl.count=0;
	}1
      }
      else {
	cpl.count = gtk_spin_button_get_value_as_int (GTK_SPIN_BUTTON(counttext));
	client_send_move (inv_list.env->tag, tmp->tag, cpl.count);
      cpl.count=0;
      }
    }
    
  }
  
}
#endif

static void resize_notebook_event (GtkWidget *widget, GtkAllocation *event) {
    int i, newwidth;
    static int oldwidth=0;

    newwidth = GTK_CLIST(inv_list.gtk_list[0])->clist_window_width - image_size - 75;

    if (newwidth != oldwidth) {
	oldwidth = newwidth;
	for (i=0; i<TYPE_LISTS; i++) {
	    gtk_clist_set_column_width (GTK_CLIST(inv_list.gtk_list[i]), 0, image_size);
	    gtk_clist_set_column_width (GTK_CLIST(inv_list.gtk_list[i]), 1, newwidth);
	    gtk_clist_set_column_width (GTK_CLIST(inv_list.gtk_list[i]), 2, 50);
	}
	gtk_clist_set_column_width (GTK_CLIST(look_list.gtk_list[0]), 0, image_size);
	gtk_clist_set_column_width (GTK_CLIST(look_list.gtk_list[0]), 1, newwidth);
	gtk_clist_set_column_width (GTK_CLIST(look_list.gtk_list[0]), 2, 50);

    }
}

void count_callback(GtkWidget *widget, GtkWidget *entry)
       {
         gchar *count_text;
         count_text = gtk_entry_get_text(GTK_ENTRY(counttext));
	 /*         printf("Entry contents: %s\n", count_text);*/
	 cpl.count = atoi (count_text);
	 /*	 gtk_entry_set_text(GTK_ENTRY(counttext),"");*/
	 gtk_widget_grab_focus (GTK_WIDGET(gtkwin_info_text)); 
       }


/* Create tabbed notebook page */
#ifdef GTK_HAVE_FEATURES_1_1_12
void create_notebook_page (GtkWidget *notebook, GtkWidget **list, GtkWidget **lists, gchar **label) {
#else
void create_notebook_page (GtkWidget *notebook, GtkWidget **list, gchar **label) {
#endif
  GtkWidget *vbox1;
  GtkStyle *liststyle, *tabstyle;
  GdkPixmap *labelgdkpixmap;
  GdkBitmap *labelgdkmask;
  GtkWidget *tablabel;

  gchar *titles[] ={"?","Name","Weight"};	   

  tabstyle = gtk_widget_get_style(gtkwin_root);

  labelgdkpixmap = gdk_pixmap_create_from_xpm_d(gtkwin_root->window,
					    &labelgdkmask,
					    &tabstyle->bg[GTK_STATE_NORMAL],
					    (gchar **) label );

 
  tablabel = gtk_pixmap_new (labelgdkpixmap, labelgdkmask);
  gtk_widget_show (tablabel);

  vbox1 = gtk_vbox_new(FALSE, 0);
  gtk_notebook_append_page (GTK_NOTEBOOK (notebook), vbox1, tablabel);
#ifdef GTK_HAVE_FEATURES_1_1_12
  *lists = gtk_scrolled_window_new (0,0);
#endif
  *list = gtk_clist_new_with_titles (3, titles);

  gtk_clist_set_column_width (GTK_CLIST(*list), 0, image_size);
  gtk_clist_set_column_width (GTK_CLIST(*list), 1, 150);
  gtk_clist_set_column_width (GTK_CLIST(*list), 2, 50);
  /* Since the program will automatically adjust these, any changes
   * the user makes can get obliterated, so just don't let the user
   * make changes.
   */
  gtk_clist_set_column_resizeable(GTK_CLIST(*list), 0, FALSE);
  gtk_clist_set_column_resizeable(GTK_CLIST(*list), 1, FALSE);
  gtk_clist_set_column_resizeable(GTK_CLIST(*list), 2, FALSE);

  gtk_clist_set_selection_mode (GTK_CLIST(*list) , GTK_SELECTION_SINGLE);
  gtk_clist_set_row_height (GTK_CLIST(*list), image_size); 
#ifdef GTK_HAVE_FEATURES_1_1_12
  gtk_scrolled_window_set_policy (GTK_SCROLLED_WINDOW(*lists),
				  GTK_POLICY_AUTOMATIC, GTK_POLICY_AUTOMATIC);
#else 
  gtk_clist_set_policy (GTK_CLIST (*list), GTK_POLICY_AUTOMATIC,
			GTK_POLICY_AUTOMATIC);
#endif  
  liststyle = gtk_style_new ();
  liststyle->bg[GTK_STATE_SELECTED] = gdk_grey;
  liststyle->fg[GTK_STATE_SELECTED] = gdk_black;
  gtk_widget_set_style (*list, liststyle);
#ifdef GTK_HAVE_FEATURES_1_1_12
  gtk_clist_set_button_actions (GTK_CLIST(*list),
				1,
				GTK_BUTTON_SELECTS);
  gtk_clist_set_button_actions (GTK_CLIST(*list),
				2,
				GTK_BUTTON_SELECTS);
  gtk_signal_connect (GTK_OBJECT(*list),
		      "select_row",
		      GTK_SIGNAL_FUNC(list_button_event),
		      &inv_list);
#else
  gtk_signal_connect_after (GTK_OBJECT(*list),
                              "button_press_event",
                              GTK_SIGNAL_FUNC(list_button_event),
                              &inv_list);
#endif
  gtk_widget_show (*list);
#ifdef GTK_HAVE_FEATURES_1_1_12
  gtk_container_add (GTK_CONTAINER (*lists), *list);
  gtk_box_pack_start (GTK_BOX(vbox1),*lists, TRUE, TRUE, 0);
  gtk_widget_show (*lists);
#else
  gtk_box_pack_start (GTK_BOX(vbox1),*list, TRUE, TRUE, 0);
#endif

  gtk_signal_connect (GTK_OBJECT(*list),"size-allocate",
		      (GtkSignalFunc) resize_notebook_event, NULL);

  gtk_widget_show (vbox1);
}


static int get_inv_display(GtkWidget *frame)
{
  /*  GtkWidget *vbox1;*/
#include "pixmaps/all.xpm" 
#include "pixmaps/hand.xpm" 
#include "pixmaps/hand2.xpm" 
#include "pixmaps/coin.xpm"
#include "pixmaps/skull.xpm"
#include "pixmaps/mag.xpm"
#include "pixmaps/nonmag.xpm"
#include "pixmaps/lock.xpm"
#include "pixmaps/unlock.xpm"
  
  GtkWidget *vbox2;
  GtkWidget *hbox1;
  GtkWidget *invlabel;
  GtkAdjustment *adj;

  strcpy (inv_list.title, "Inventory:");
  inv_list.env = cpl.ob;
  inv_list.show_weight = 1;
  inv_list.weight_limit=0;
  
  vbox2 = gtk_vbox_new(FALSE, 0); /* separation here */
  
  gtk_container_add (GTK_CONTAINER(frame), vbox2); 

  hbox1 = gtk_hbox_new(FALSE, 2);
  gtk_box_pack_start (GTK_BOX(vbox2),hbox1, FALSE, FALSE, 0);
  gtk_widget_show (hbox1);


  inv_list.label = gtk_label_new ("Inventory:");
  gtk_box_pack_start (GTK_BOX(hbox1),inv_list.label, TRUE, FALSE, 2);
  gtk_widget_show (inv_list.label);

  inv_list.weightlabel = gtk_label_new ("0");
  gtk_box_pack_start (GTK_BOX(hbox1),inv_list.weightlabel, TRUE, FALSE, 2);
  gtk_widget_show (inv_list.weightlabel);


  inv_list.maxweightlabel = gtk_label_new ("0");
  gtk_box_pack_start (GTK_BOX(hbox1),inv_list.maxweightlabel, TRUE, FALSE, 2);
  gtk_widget_show (inv_list.maxweightlabel);

  invlabel = gtk_label_new ("Count:");
  gtk_box_pack_start (GTK_BOX(hbox1),invlabel, FALSE, FALSE, 5);
  gtk_widget_show (invlabel);

  adj = (GtkAdjustment *) gtk_adjustment_new (0.0, 0.0, 100000.0, 1.0,
                                                  100.0, 0.0);
  counttext = gtk_spin_button_new (adj, 1.0, 0);

  gtk_spin_button_set_wrap (GTK_SPIN_BUTTON (counttext), FALSE);
  gtk_widget_set_usize (counttext, 65, 0);
  gtk_spin_button_set_update_policy (GTK_SPIN_BUTTON (counttext),
				     GTK_UPDATE_ALWAYS);
   gtk_signal_connect(GTK_OBJECT(counttext), "activate",
		     GTK_SIGNAL_FUNC(count_callback),
		     counttext);


  gtk_box_pack_start (GTK_BOX (hbox1),counttext, FALSE, FALSE, 0);

  gtk_widget_show (counttext);
  gtk_tooltips_set_tip (tooltips, counttext, "This sets the number of items you wish to pickup or drop. You can also use the keys 0-9 to set it.", NULL);

  inv_notebook = gtk_notebook_new ();
  gtk_notebook_set_tab_pos (GTK_NOTEBOOK (inv_notebook), GTK_POS_TOP );


  gtk_box_pack_start (GTK_BOX(vbox2),inv_notebook, TRUE, TRUE, 0);

#ifdef GTK_HAVE_FEATURES_1_1_12
  create_notebook_page (inv_notebook, &inv_list.gtk_list[0], &inv_list.gtk_lists[0], all_xpm); 
  create_notebook_page (inv_notebook, &inv_list.gtk_list[1], &inv_list.gtk_lists[1], hand_xpm); 
  create_notebook_page (inv_notebook, &inv_list.gtk_list[2], &inv_list.gtk_lists[2], hand2_xpm); 
  create_notebook_page (inv_notebook, &inv_list.gtk_list[3], &inv_list.gtk_lists[3], coin_xpm);
  create_notebook_page (inv_notebook, &inv_list.gtk_list[4], &inv_list.gtk_lists[4], skull_xpm);
  create_notebook_page (inv_notebook, &inv_list.gtk_list[5], &inv_list.gtk_lists[5], mag_xpm);
  create_notebook_page (inv_notebook, &inv_list.gtk_list[6], &inv_list.gtk_lists[6], nonmag_xpm);
  create_notebook_page (inv_notebook, &inv_list.gtk_list[7], &inv_list.gtk_lists[7], lock_xpm);
  create_notebook_page (inv_notebook, &inv_list.gtk_list[8], &inv_list.gtk_lists[8], unlock_xpm);

#else
  create_notebook_page (inv_notebook, &inv_list.gtk_list[0], all_xpm); 
  create_notebook_page (inv_notebook, &inv_list.gtk_list[1], hand_xpm); 
  create_notebook_page (inv_notebook, &inv_list.gtk_list[2], hand2_xpm); 
  create_notebook_page (inv_notebook, &inv_list.gtk_list[3], coin_xpm);
  create_notebook_page (inv_notebook, &inv_list.gtk_list[4], skull_xpm);
  create_notebook_page (inv_notebook, &inv_list.gtk_list[5], mag_xpm);
  create_notebook_page (inv_notebook, &inv_list.gtk_list[6], nonmag_xpm);
  create_notebook_page (inv_notebook, &inv_list.gtk_list[7], lock_xpm);
  create_notebook_page (inv_notebook, &inv_list.gtk_list[8], unlock_xpm);
#endif
  gtk_widget_show (vbox2);
  gtk_widget_show (inv_notebook);

  inv_list.multi_list=1;
  draw_all_list(&inv_list);
 
    return 0;
}

static int get_look_display(GtkWidget *frame) 
{
  GtkWidget *vbox1;
  GtkWidget *hbox1;
  GtkWidget *closebutton;
  GtkStyle *liststyle;
  
  /*  gchar *test[] ={"testamer","testa","test"};*/
  gchar *titles[] ={"?","Name","Weight"};
  
  look_list.env = cpl.below;
  strcpy (look_list.title, "You see:");
  look_list.show_weight = 1;
  look_list.weight_limit = 0;
    

  vbox1 = gtk_vbox_new(FALSE, 0);/*separation here*/
  gtk_container_add (GTK_CONTAINER(frame), vbox1);

  hbox1 = gtk_hbox_new(FALSE, 2);
  gtk_box_pack_start (GTK_BOX(vbox1),hbox1, FALSE, FALSE, 0);
  gtk_widget_show (hbox1);

  closebutton = gtk_button_new_with_label ("Close");
  gtk_signal_connect_object (GTK_OBJECT (closebutton), "clicked",
			       GTK_SIGNAL_FUNC(close_container),
			       NULL);
  gtk_box_pack_start (GTK_BOX(hbox1),closebutton, FALSE, FALSE, 2);
  gtk_widget_show (closebutton);
  gtk_tooltips_set_tip (tooltips,closebutton , "This will close an item if you have one open.", NULL);

  look_list.label = gtk_label_new ("You see:");
  gtk_box_pack_start (GTK_BOX(hbox1),look_list.label, TRUE, FALSE, 2);
  gtk_widget_show (look_list.label);

  look_list.weightlabel = gtk_label_new ("0");
  gtk_box_pack_start (GTK_BOX(hbox1),look_list.weightlabel, TRUE, FALSE, 2);
  gtk_widget_show (look_list.weightlabel);

  look_list.maxweightlabel = gtk_label_new ("0");
  gtk_box_pack_start (GTK_BOX(hbox1),look_list.maxweightlabel, TRUE, FALSE, 2);
  gtk_widget_show (look_list.maxweightlabel);

#ifdef GTK_HAVE_FEATURES_1_1_12
  look_list.gtk_lists[0] = gtk_scrolled_window_new (0,0);
#endif
  look_list.gtk_list[0] = gtk_clist_new_with_titles (3,titles);;
  gtk_clist_set_column_width (GTK_CLIST(look_list.gtk_list[0]), 0, image_size);
  gtk_clist_set_column_width (GTK_CLIST(look_list.gtk_list[0]), 1, 150);
  gtk_clist_set_column_width (GTK_CLIST(look_list.gtk_list[0]), 2, 50);
  gtk_clist_set_selection_mode (GTK_CLIST(look_list.gtk_list[0]) , GTK_SELECTION_SINGLE);
  gtk_clist_set_row_height (GTK_CLIST(look_list.gtk_list[0]), image_size); 
#ifdef GTK_HAVE_FEATURES_1_1_12
  gtk_scrolled_window_set_policy (GTK_SCROLLED_WINDOW(look_list.gtk_lists[0]),
				  GTK_POLICY_AUTOMATIC, GTK_POLICY_AUTOMATIC);
#else
  gtk_clist_set_policy (GTK_CLIST (look_list.gtk_list[0]), GTK_POLICY_AUTOMATIC,
			GTK_POLICY_AUTOMATIC);
#endif
 liststyle = gtk_style_new ();
  liststyle->bg[GTK_STATE_SELECTED] = gdk_grey;
  liststyle->fg[GTK_STATE_SELECTED] = gdk_black;
  gtk_widget_set_style (look_list.gtk_list[0], liststyle);

#ifdef GTK_HAVE_FEATURES_1_1_12
  gtk_clist_set_button_actions (GTK_CLIST(look_list.gtk_list[0]),
				1,
				GTK_BUTTON_SELECTS);
  gtk_clist_set_button_actions (GTK_CLIST(look_list.gtk_list[0]),
				2,
				GTK_BUTTON_SELECTS);
  gtk_signal_connect (GTK_OBJECT(look_list.gtk_list[0]),
		      "select_row",
		      GTK_SIGNAL_FUNC(list_button_event),
		      &look_list);
#else
  gtk_signal_connect_after (GTK_OBJECT(look_list.gtk_list[0]),
                              "button_press_event",
                              GTK_SIGNAL_FUNC(list_button_event),
                              &look_list);
#endif

  gtk_widget_show (look_list.gtk_list[0]);
#ifdef GTK_HAVE_FEATURES_1_1_12
  gtk_container_add (GTK_CONTAINER (look_list.gtk_lists[0]), look_list.gtk_list[0]);
  gtk_box_pack_start (GTK_BOX(vbox1),look_list.gtk_lists[0], TRUE, TRUE, 0);
  gtk_widget_show (look_list.gtk_lists[0]);
#else
  gtk_box_pack_start (GTK_BOX(vbox1),look_list.gtk_list[0], TRUE, TRUE, 0);
#endif
  gtk_widget_show (vbox1);
  look_list.multi_list=0;
  draw_all_list(&look_list);

    return 0;
}


/*
 *  draw_lists() redraws inventory and look windows when necessary
 */
void draw_lists ()
{
  if (inv_list.env->inv_updated) {

    draw_list (&inv_list);
    inv_list.env->inv_updated = 0;
  } else {
    if (look_list.env->inv_updated) {
      draw_list (&look_list);
      look_list.env->inv_updated = 0;
    }
  }
}

void set_show_icon (char *s)
{
    if (s == NULL || *s == 0 || strncmp ("inventory", s, strlen(s)) == 0) {
	inv_list.show_icon = ! inv_list.show_icon; /* toggle */
	draw_all_list(&inv_list);
    } else if (strncmp ("look", s, strlen(s)) == 0) {
	look_list.show_icon = ! look_list.show_icon; /* toggle */
	draw_all_list(&look_list);
    }
}

void set_show_weight (char *s)
{
    if (s == NULL || *s == 0 || strncmp ("inventory", s, strlen(s)) == 0) {
	inv_list.show_weight = ! inv_list.show_weight; /* toggle */
	draw_list (&inv_list);
    } else if (strncmp ("look", s, strlen(s)) == 0) {
	look_list.show_weight = ! look_list.show_weight; /* toggle */
	draw_list (&look_list);
    }
}

void aboutdialog(GtkWidget *widget) {
#include "help/about.h"
  GtkWidget *vbox;
  GtkWidget *hbox;
  GtkWidget *aboutlabel;
  GtkWidget *vscrollbar;
  GtkWidget *aboutbutton;
  GtkWidget *aboutgtkpixmap;
  GdkPixmap *aboutgdkpixmap;
  GdkBitmap *aboutgdkmask;

  GtkStyle *style;
  /*  gchar *text="GTK Crossfire Client\nGTK porting by David Sundqvist\nOriginal client code by Mark Wedel and others\nLogo and configure scripts by Raphael Quinet\nThis software is licensed according to the terms set forth\nin the GNU General Public License\n";*/

  if(!gtkwin_about) {
    
    gtkwin_about = gtk_window_new (GTK_WINDOW_DIALOG);
    gtk_window_position (GTK_WINDOW (gtkwin_about), GTK_WIN_POS_CENTER);
    gtk_widget_set_usize (gtkwin_about,500,210);
    gtk_window_set_title (GTK_WINDOW (gtkwin_about), "About Crossfire");

    gtk_signal_connect (GTK_OBJECT (gtkwin_about), "destroy", GTK_SIGNAL_FUNC(gtk_widget_destroyed), &gtkwin_about);
    
    gtk_container_border_width (GTK_CONTAINER (gtkwin_about), 0);
    vbox = gtk_vbox_new(FALSE, 2);
    gtk_container_add (GTK_CONTAINER(gtkwin_about),vbox);
    style = gtk_widget_get_style(gtkwin_about);
    gtk_widget_realize(gtkwin_about);
    aboutgdkpixmap = gdk_pixmap_create_from_xpm_d(gtkwin_about->window,
						  &aboutgdkmask,
						  &style->bg[GTK_STATE_NORMAL],
						  (gchar **)crossfiretitle);
    aboutgtkpixmap= gtk_pixmap_new (aboutgdkpixmap, aboutgdkmask);
    gtk_box_pack_start (GTK_BOX (vbox),aboutgtkpixmap, FALSE, TRUE, 0);
    gtk_widget_show (aboutgtkpixmap);

    hbox = gtk_hbox_new(FALSE, 2);
    gtk_box_pack_start (GTK_BOX (vbox), hbox, TRUE, TRUE, 0);

    aboutlabel = gtk_text_new (NULL, NULL);
    gtk_text_set_editable (GTK_TEXT (aboutlabel), FALSE);
    gtk_box_pack_start (GTK_BOX (hbox),aboutlabel, TRUE, TRUE, 0);
    gtk_widget_show (aboutlabel);

    vscrollbar = gtk_vscrollbar_new (GTK_TEXT (aboutlabel)->vadj);
    gtk_box_pack_start (GTK_BOX (hbox),vscrollbar, FALSE, FALSE, 0);
 
    gtk_widget_show (vscrollbar);
    gtk_widget_show (hbox);

    hbox = gtk_hbox_new(FALSE, 2);
    
    aboutbutton = gtk_button_new_with_label ("Close");
    gtk_signal_connect_object (GTK_OBJECT (aboutbutton), "clicked",
			       GTK_SIGNAL_FUNC(gtk_widget_destroy),
			       GTK_OBJECT (gtkwin_about));
    gtk_box_pack_start (GTK_BOX (hbox), aboutbutton, TRUE, FALSE, 0);
    gtk_box_pack_start (GTK_BOX (vbox), hbox, FALSE, FALSE, 0);
    gtk_widget_show (aboutbutton);
    gtk_widget_show (hbox);

    gtk_widget_show (vbox);
    gtk_widget_show (gtkwin_about);
    gtk_text_insert (GTK_TEXT (aboutlabel), NULL, &aboutlabel->style->black, NULL, text , -1);    
  }
  else { 
    gdk_window_raise (gtkwin_about->window);
  }
}

/* Ok, here it sets the config and saves it. This is sorta dangerous, and I'm not sure
 * if it's actually possible to do dynamic reconfiguration of everything this way. Something may
 * blow up in our faces.
 */

void applyconfig () {
  int sound;
  if (GTK_TOGGLE_BUTTON (ccheckbutton1)->active)  {
    cache_images=TRUE;
  } else {
    cache_images=FALSE;
  }
  if (GTK_TOGGLE_BUTTON (ccheckbutton2)->active) {
    if (!split_windows) {
      gtk_widget_destroy(gtkwin_root);
      split_windows=TRUE;
	create_windows();
	display_map_doneupdate();
	draw_stats (1);
      draw_all_list(&inv_list);
      draw_all_list(&look_list);
    }
  } else {
    if (split_windows) {
      gtk_widget_destroy(gtkwin_root);
      gtk_widget_destroy(gtkwin_info);
      gtk_widget_destroy(gtkwin_stats);
      gtk_widget_destroy(gtkwin_message);
      gtk_widget_destroy(gtkwin_inv);
      gtk_widget_destroy(gtkwin_look);
      split_windows=FALSE;
      create_windows();
      display_map_doneupdate();
      draw_stats (1);
      draw_all_list(&inv_list);
      draw_all_list(&look_list);
    }
  }
  if (GTK_TOGGLE_BUTTON (ccheckbutton3)->active) {
    if (nosound) {
      nosound=FALSE;
      sound = init_sounds();
      if (sound<0)
	cs_write_string(csocket.fd,"setsound 0", 10);
      else
	cs_write_string(csocket.fd,"setsound 1", 10);
    }
  } else {
    if (!nosound) {
      nosound=TRUE;
    }
  }
  if (GTK_TOGGLE_BUTTON (ccheckbutton4)->active)   {
    if (!color_inv) {
      color_inv=TRUE;
      draw_all_list(&inv_list);
      draw_all_list(&look_list);
    }
  } else {
    if (color_inv) {
      color_inv=FALSE;
      draw_all_list(&inv_list);
      draw_all_list(&look_list);
    }
  }
  if (GTK_TOGGLE_BUTTON (ccheckbutton5)->active)   {
    if (!color_text) {
      color_text=TRUE;
    }
  } else {
    if (color_text) {
      color_text=FALSE;
    }
  }
  if (GTK_TOGGLE_BUTTON (ccheckbutton6)->active)   {
    if (!tool_tips) {
      gtk_tooltips_enable(tooltips);
      tool_tips=TRUE;
    }
  } else {
    if (tool_tips) {
       gtk_tooltips_disable(tooltips);
      tool_tips=FALSE;
    }
  }
  if (GTK_TOGGLE_BUTTON (ccheckbutton7)->active)   {
    if (!splitinfo) {
      gtk_tooltips_enable(tooltips);
      splitinfo=TRUE;
    }
  } else {
    if (splitinfo) {
       gtk_tooltips_disable(tooltips);
      splitinfo=FALSE;
    }
  }
  if (GTK_TOGGLE_BUTTON (ccheckbutton8)->active)   {
    if (!nopopups) {
      gtk_tooltips_enable(tooltips);
      nopopups=TRUE;
    }
  } else {
    if (nopopups) {
       gtk_tooltips_disable(tooltips);
      nopopups=FALSE;
    }
  }
}

/* Ok, here it sets the config and saves it. This is sorta dangerous, and I'm not sure
 * if it's actually possible to do dynamic reconfiguration of everything this way.
 */

void saveconfig () {
  int sound;
  if (GTK_TOGGLE_BUTTON (ccheckbutton1)->active) {
    cache_images=TRUE;
  } else {
    cache_images=FALSE;
  }
  if (GTK_TOGGLE_BUTTON (ccheckbutton2)->active) {
    if (!split_windows) {
      gtk_widget_destroy(gtkwin_root);
      split_windows=TRUE;
      create_windows();
      display_map_doneupdate();
      draw_stats (1);
      draw_all_list(&inv_list);
      draw_all_list(&look_list);
      
    }
  } else {
    if (split_windows) {
      gtk_widget_destroy(gtkwin_root);
      gtk_widget_destroy(gtkwin_info);
      gtk_widget_destroy(gtkwin_stats);
      gtk_widget_destroy(gtkwin_message);
      gtk_widget_destroy(gtkwin_inv);
      gtk_widget_destroy(gtkwin_look);
      split_windows=FALSE;
      create_windows();
      display_map_doneupdate();
      draw_stats (1);
      draw_all_list(&inv_list);
      draw_all_list(&look_list);
      
      
    }
  }
  if (GTK_TOGGLE_BUTTON (ccheckbutton3)->active) {
    if (nosound) {
      nosound=FALSE;
      sound = init_sounds();
      if (sound<0)
	cs_write_string(csocket.fd,"setsound 0", 10);
      else
	cs_write_string(csocket.fd,"setsound 1", 10);
    }
  } else {
    if (!nosound) {
      nosound=TRUE;
    }
  }
  if (GTK_TOGGLE_BUTTON (ccheckbutton4)->active)  {
    if (!color_inv) {
      color_inv=TRUE;
      draw_all_list(&inv_list);
      draw_all_list(&look_list);

    }
  } else {
    if (color_inv) {
      color_inv=FALSE;
      draw_all_list(&inv_list);
      draw_all_list(&look_list);
    }
  }
  if (GTK_TOGGLE_BUTTON (ccheckbutton5)->active)   {
    if (!color_text) {
      color_text=TRUE;
    }
  } else {
    if (color_text) {
      color_text=FALSE;
    }
  }
  if (GTK_TOGGLE_BUTTON (ccheckbutton6)->active)   {
    if (!tool_tips) {
      gtk_tooltips_enable(tooltips);
      tool_tips=TRUE;
    }
  } else {
    if (tool_tips) {
      gtk_tooltips_disable(tooltips);
      tool_tips=FALSE;
    }
  }
  if (GTK_TOGGLE_BUTTON (ccheckbutton7)->active)   {
    if (!splitinfo) {
      gtk_tooltips_enable(tooltips);
      splitinfo=TRUE;
    }
  } else {
    if (splitinfo) {
      gtk_tooltips_disable(tooltips);
      splitinfo=FALSE;
    }
  }
  if (GTK_TOGGLE_BUTTON (ccheckbutton8)->active)   {
    if (!nopopups) {
      gtk_tooltips_enable(tooltips);
      nopopups=TRUE;
    }
  } else {
    if (nopopups) {
      gtk_tooltips_disable(tooltips);
      nopopups=FALSE;
    }
  }
  save_defaults();
}

/*void cknumentry_callback (GtkWidget *widget, GdkEventKey *event, GtkWidget *window) {
  gtk_entry_set_text (GTK_ENTRY(ckeyentrytext),  XKeysymToString(event->keyval));
  gtk_signal_emit_stop_by_name (GTK_OBJECT(window), "key_press_event"); 
  }*/



static void ckeyentry_callback (GtkWidget *widget, GdkEventKey *event, GtkWidget *window) {
  /*  configure_keys(XKeysymToKeycode(GDK_DISPLAY(), event->keyval), event->keyval);*/
  gtk_entry_set_text (GTK_ENTRY(ckeyentrytext),  XKeysymToString(event->keyval));
  
  switch (event->state) {
  case GDK_CONTROL_MASK:
    gtk_entry_set_text (GTK_ENTRY(cmodentrytext),  "R");
    break;
  case GDK_SHIFT_MASK:
    gtk_entry_set_text (GTK_ENTRY(cmodentrytext),  "F");
    break;
  default:
    gtk_entry_set_text (GTK_ENTRY(cmodentrytext),  "A");
  }
  /*  XKeysymToString(event->keyval);*/
  gtk_signal_emit_stop_by_name (GTK_OBJECT(window), "key_press_event"); 
}

/*void ckeyentry_callback (GtkWidget *entry) {
  KeySym keysym;
  gchar *key;
  key=gtk_entry_get_text(GTK_ENTRY(entry));
  printf ("Text: %s\n", key);
  keysym = XStringToKeysym(key);
  printf ("Keysym: %i\n", keysym);

  }
*/
void ckeyclear () {
  gtk_label_set (GTK_LABEL(cnumentrytext), "0"); 
  gtk_entry_set_text (GTK_ENTRY(ckeyentrytext), "Press key to bind here"); 
  /*  gtk_entry_set_text (GTK_ENTRY(cknumentrytext), ""); */
  gtk_entry_set_text (GTK_ENTRY(cmodentrytext), ""); 
  gtk_entry_set_text (GTK_ENTRY(ckentrytext), ""); 
}
#ifdef GTK_HAVE_FEATURES_1_1_12
void cclist_button_event(GtkWidget *gtklist, gint row, gint column, GdkEventButton *event) {
  gchar *buf;
  if (event->button==1) {
    gtk_clist_get_text (GTK_CLIST(cclist), row, 0, &buf); 
    gtk_label_set (GTK_LABEL(cnumentrytext), buf); 
    gtk_clist_get_text (GTK_CLIST(cclist), row, 1, &buf); 
    gtk_entry_set_text (GTK_ENTRY(ckeyentrytext), buf); 
    gtk_clist_get_text (GTK_CLIST(cclist), row, 3, &buf); 
    gtk_entry_set_text (GTK_ENTRY(cmodentrytext), buf); 
    gtk_clist_get_text (GTK_CLIST(cclist), row, 4, &buf); 
    gtk_entry_set_text (GTK_ENTRY(ckentrytext), buf); 
  } 
}
#else
void cclist_button_event(GtkWidget *gtklist, GdkEventButton *event) {
  gchar *buf;
  GList *node;

  node =  GTK_CLIST(gtklist)->selection;
  if (node) {
    if (event->type==GDK_BUTTON_PRESS && event->button==1) {
      gtk_clist_get_text (GTK_CLIST(cclist), (gint)node->data, 0, &buf); 
      gtk_label_set (GTK_LABEL(cnumentrytext), buf); 
      gtk_clist_get_text (GTK_CLIST(cclist), (gint)node->data, 1, &buf); 
      gtk_entry_set_text (GTK_ENTRY(ckeyentrytext), buf); 
      /*      gtk_clist_get_text (GTK_CLIST(cclist), (gint)node->data, 2, &buf); 
	      gtk_entry_set_text (GTK_ENTRY(cknumentrytext), buf); */
      gtk_clist_get_text (GTK_CLIST(cclist), (gint)node->data, 3, &buf); 
      gtk_entry_set_text (GTK_ENTRY(cmodentrytext), buf); 
      gtk_clist_get_text (GTK_CLIST(cclist), (gint)node->data, 4, &buf); 
      gtk_entry_set_text (GTK_ENTRY(ckentrytext), buf); 
    } 
  } else {
    gtk_label_set (GTK_LABEL(cnumentrytext), "0"); 
    gtk_entry_set_text (GTK_ENTRY(ckeyentrytext), "Press key to bind here"); 
    /*    gtk_entry_set_text (GTK_ENTRY(cknumentrytext), "");*/ 
    gtk_entry_set_text (GTK_ENTRY(cmodentrytext), ""); 
    gtk_entry_set_text (GTK_ENTRY(ckentrytext), ""); 
  }
}
#endif

/*void draw_keybindings (GtkWidget *keylist) {*/
void draw_keybindings (GtkWidget *keylist) {
  int i, count=1;
  Key_Entry *key;
  int allbindings=0;
  /*  static char buf[MAX_BUF];*/
  char buff[MAX_BUF];
  int bi=0;
  char buffer[5][MAX_BUF];
  char *buffers[5];
  gint tmprow; 

  gtk_clist_clear (GTK_CLIST(keylist));
     for (i=0; i<=MAX_KEYCODE; i++) {
      for (key=keys[i]; key!=NULL; key =key->next) {
	if (key->flags & KEYF_STANDARD && !allbindings) continue;
	bi=0;
	
	if ((key->flags & KEYF_MODIFIERS) == KEYF_MODIFIERS)
	  buff[bi++] ='A';
	else {
	  if (key->flags & KEYF_NORMAL)
	    buff[bi++] ='N';
	  if (key->flags & KEYF_FIRE)
	    buff[bi++] ='F';
	  if (key->flags & KEYF_RUN)
	    buff[bi++] ='R';
	}
	if (key->flags & KEYF_EDIT)
	  buff[bi++] ='E';
	if (key->flags & KEYF_STANDARD)
	  buff[bi++] ='S';
	
	buff[bi]='\0';
	
	if(key->keysym == NoSymbol) {
	  /*	  sprintf(buf, "key (null) (%i) %s %s",
		  kc,buff, key->command);
	  */
	}
	  else {
	    sprintf(buffer[0], "%i",count);
	    sprintf(buffer[1], "%s", XKeysymToString(key->keysym));
	    sprintf(buffer[2], "%i",i);
	    sprintf(buffer[3], "%s",buff);
	    sprintf(buffer[4], "%s", key->command);
	    buffers[0] = buffer[0];
	    buffers[1] = buffer[1];
	    buffers[2] = buffer[2];
	    buffers[3] = buffer[3];
	    buffers[4] = buffer[4];
	    tmprow = gtk_clist_append (GTK_CLIST (keylist), buffers);
	  }
	
	/*	sprintf(buf,"%3d %s",count,  get_key_info(key,i,0));
		draw_info(buf,NDI_BLACK);*/
	count++;
      }
    }
}
    
void bind_callback (GtkWidget *gtklist, GdkEventButton *event) {
  KeySym keysym;
  gchar *entry_text;
  gchar *cpnext;
  KeyCode k;
  gchar *mod="";
  char buf[MAX_BUF];
  /*  int flags=0;*/
  /*  int standard=1;
      
      if (standard) standard=KEYF_STANDARD;
      else standard=0;*/
  bind_flags = KEYF_MODIFIERS;
  
  if ((bind_flags & KEYF_MODIFIERS)==KEYF_MODIFIERS) {
    bind_flags &= ~KEYF_MODIFIERS;
    mod=gtk_entry_get_text (GTK_ENTRY(cmodentrytext));
    if (!strcmp(mod, "F")) {
      bind_flags |= KEYF_FIRE;
    }
    if (!strcmp(mod, "R")) {
      bind_flags |= KEYF_RUN;
      }
    if (!strcmp(mod, "A")) {
      bind_flags |= KEYF_MODIFIERS;
    }
  }
  cpnext = gtk_entry_get_text (GTK_ENTRY(ckentrytext));
  entry_text = gtk_entry_get_text (GTK_ENTRY(ckeyentrytext));
  keysym = XStringToKeysym(entry_text);
  k = XKeysymToKeycode(GDK_DISPLAY(), keysym);
  insert_key(keysym, k,  bind_flags, cpnext);
  save_keys();
  draw_keybindings (cclist);
  sprintf(buf, "Binded to key '%s' (%i)", XKeysymToString(keysym), (int)k);
  draw_info(buf,NDI_BLACK);
}

void ckeyunbind (GtkWidget *gtklist, GdkEventButton *event) {
  gchar *buf;
  GList *node;
  node =  GTK_CLIST(cclist)->selection;
  if (node) {
    gtk_clist_get_text (GTK_CLIST(cclist), (gint)node->data, 0, &buf); 

    unbind_key(buf);
    draw_keybindings (cclist);

  }
}

void disconnect(GtkWidget *widget) {
    close(csocket.fd);
    csocket.fd = -1;
    if (csocket_fd) {
	gdk_input_remove(csocket_fd);
	csocket_fd=0;
	gtk_main_quit();
    }
}

/*
 *  GUI Config dialog. 
 *
 *
 */

void configdialog(GtkWidget *widget) {
  GtkWidget *vbox;
  GtkWidget *hbox;
  /*  GtkWidget *configtext;*/
  /*  GtkStyle *style;*/

  GtkWidget *tablabel;
  GtkWidget *notebook;
  GtkWidget *vbox1;
  GtkWidget *vbox2;
  GtkWidget *hbox1;
  GtkWidget *applybutton;
  GtkWidget *cancelbutton;
  GtkWidget *savebutton;
  GtkWidget *frame1;

  GtkWidget *ehbox;
  GtkWidget *clabel1, *clabel2, *clabel4, *clabel5, *cb1, *cb2, *cb3;
#ifdef GTK_HAVE_FEATURES_1_1_12
  GtkWidget *cclists;
#endif 

  gchar *titles[] ={"#","Key","(#)","Mods","Command"};	   
  /* If the window isnt already up (in which case it's just raised) */

  if(!gtkwin_config) {
    
    gtkwin_config = gtk_window_new (GTK_WINDOW_DIALOG);
    gtk_window_position (GTK_WINDOW (gtkwin_config), GTK_WIN_POS_CENTER);
    gtk_widget_set_usize (gtkwin_config,450,360);
    gtk_window_set_title (GTK_WINDOW (gtkwin_config), "Crossfire Configure");
    gtk_window_set_policy (GTK_WINDOW (gtkwin_config), TRUE, TRUE, FALSE);

    gtk_signal_connect (GTK_OBJECT (gtkwin_config), "destroy", GTK_SIGNAL_FUNC(gtk_widget_destroyed), &gtkwin_config);
    
    gtk_container_border_width (GTK_CONTAINER (gtkwin_config), 0);
    vbox = gtk_vbox_new(FALSE, 2);
    gtk_container_add (GTK_CONTAINER(gtkwin_config),vbox);
    hbox = gtk_hbox_new(FALSE, 2);
    gtk_box_pack_start (GTK_BOX (vbox), hbox, TRUE, TRUE, 0);

    notebook = gtk_notebook_new ();
    gtk_notebook_set_tab_pos (GTK_NOTEBOOK (notebook), GTK_POS_TOP );
    gtk_box_pack_start (GTK_BOX(hbox),notebook, TRUE, TRUE, 0);

    tablabel = gtk_label_new ("General");
    gtk_widget_show (tablabel);
    vbox2 = gtk_vbox_new(FALSE, 0);
  

    gtk_notebook_append_page (GTK_NOTEBOOK (notebook), vbox2, tablabel);
    frame1 = gtk_frame_new("General options");  
    gtk_frame_set_shadow_type (GTK_FRAME(frame1), GTK_SHADOW_ETCHED_IN);
    gtk_box_pack_start (GTK_BOX (vbox2), frame1, TRUE, TRUE, 0);
    vbox1 = gtk_vbox_new(FALSE, 0);
    gtk_container_add (GTK_CONTAINER(frame1), vbox1);
    /* Add the checkbuttons to the notebook and set them according to current settings */
    ccheckbutton1 = gtk_check_button_new_with_label ("Cache Images" );
    gtk_box_pack_start(GTK_BOX(vbox1),ccheckbutton1, FALSE, FALSE, 0);
    if (cache_images) {
      gtk_toggle_button_set_state(GTK_TOGGLE_BUTTON(ccheckbutton1), TRUE);
    } else {
      gtk_toggle_button_set_state(GTK_TOGGLE_BUTTON(ccheckbutton1), FALSE);
    }

    ccheckbutton2 = gtk_check_button_new_with_label ("Split Windows" );
    gtk_box_pack_start(GTK_BOX(vbox1),ccheckbutton2, FALSE, FALSE, 0);
    if (split_windows) {
      gtk_toggle_button_set_state(GTK_TOGGLE_BUTTON(ccheckbutton2), TRUE);
    } else {
      gtk_toggle_button_set_state(GTK_TOGGLE_BUTTON(ccheckbutton2), FALSE);
    }

    ccheckbutton3 = gtk_check_button_new_with_label ("Sound" );
    gtk_box_pack_start(GTK_BOX(vbox1),ccheckbutton3, FALSE, FALSE, 0);
    if (nosound) {
      gtk_toggle_button_set_state(GTK_TOGGLE_BUTTON(ccheckbutton3), FALSE);
    } else {
      gtk_toggle_button_set_state(GTK_TOGGLE_BUTTON(ccheckbutton3), TRUE);
    }

    ccheckbutton4 = gtk_check_button_new_with_label ("Color invlists" );
    gtk_box_pack_start(GTK_BOX(vbox1),ccheckbutton4, FALSE, FALSE, 0);
    if (color_inv) {
      gtk_toggle_button_set_state(GTK_TOGGLE_BUTTON(ccheckbutton4), TRUE);
    } else {
      gtk_toggle_button_set_state(GTK_TOGGLE_BUTTON(ccheckbutton4), FALSE);
    }

    ccheckbutton5 = gtk_check_button_new_with_label ("Color info text" );
    gtk_box_pack_start(GTK_BOX(vbox1),ccheckbutton5, FALSE, FALSE, 0);
    if (color_text) {
      gtk_toggle_button_set_state(GTK_TOGGLE_BUTTON(ccheckbutton5), TRUE);
    } else {
      gtk_toggle_button_set_state(GTK_TOGGLE_BUTTON(ccheckbutton5), FALSE);
    }

    ccheckbutton6 = gtk_check_button_new_with_label ("Show tooltips" );
    gtk_box_pack_start(GTK_BOX(vbox1),ccheckbutton6, FALSE, FALSE, 0);
    if (tool_tips) {
      gtk_toggle_button_set_state(GTK_TOGGLE_BUTTON(ccheckbutton6), TRUE);
    } else {
      gtk_toggle_button_set_state(GTK_TOGGLE_BUTTON(ccheckbutton6), FALSE);
    }

    ccheckbutton7 = gtk_check_button_new_with_label ("Split Information Window\n(Takes effect next run)" );
    gtk_box_pack_start(GTK_BOX(vbox1),ccheckbutton7, FALSE, FALSE, 0);
    if (splitinfo) {
      gtk_toggle_button_set_state(GTK_TOGGLE_BUTTON(ccheckbutton7), TRUE);
    } else {
      gtk_toggle_button_set_state(GTK_TOGGLE_BUTTON(ccheckbutton7), FALSE);
    }

    ccheckbutton8 = gtk_check_button_new_with_label ("No popup windows" );
    gtk_box_pack_start(GTK_BOX(vbox1),ccheckbutton8, FALSE, FALSE, 0);
    if (nopopups) {
      gtk_toggle_button_set_state(GTK_TOGGLE_BUTTON(ccheckbutton8), TRUE);
    } else {
      gtk_toggle_button_set_state(GTK_TOGGLE_BUTTON(ccheckbutton8), FALSE);
    }

    gtk_widget_show (ccheckbutton1);
    gtk_widget_show (ccheckbutton2);
    gtk_widget_show (ccheckbutton3);
    gtk_widget_show (ccheckbutton4);
    gtk_widget_show (ccheckbutton5);
    gtk_widget_show (ccheckbutton6);
    gtk_widget_show (ccheckbutton7);
    gtk_widget_show (ccheckbutton8);

    gtk_widget_show (vbox1);
    gtk_widget_show (frame1);
    gtk_widget_show (vbox2);
 

    tablabel = gtk_label_new ("Keybindings");
    gtk_widget_show (tablabel);
    vbox2 = gtk_vbox_new(FALSE, 0);
    gtk_notebook_append_page (GTK_NOTEBOOK (notebook), vbox2, tablabel);    
    frame1 = gtk_frame_new("Keybindings");  
    gtk_frame_set_shadow_type (GTK_FRAME(frame1), GTK_SHADOW_ETCHED_IN);
    gtk_box_pack_start (GTK_BOX (vbox2), frame1, TRUE, TRUE, 0);
    vbox1 = gtk_vbox_new(FALSE, 0);
    gtk_container_add (GTK_CONTAINER(frame1), vbox1);
#ifdef GTK_HAVE_FEATURES_1_1_12
    cclists = gtk_scrolled_window_new (0,0);
#endif 
    cclist = gtk_clist_new_with_titles (5, titles);

    gtk_clist_set_column_width (GTK_CLIST(cclist), 0, 20);
    gtk_clist_set_column_width (GTK_CLIST(cclist), 1, 50);
    gtk_clist_set_column_width (GTK_CLIST(cclist), 2, 20);
    gtk_clist_set_column_width (GTK_CLIST(cclist), 3, 40);
    gtk_clist_set_column_width (GTK_CLIST(cclist), 4, 245);
    gtk_clist_set_selection_mode (GTK_CLIST(cclist) , GTK_SELECTION_SINGLE);

#ifdef GTK_HAVE_FEATURES_1_1_12
    gtk_scrolled_window_set_policy (GTK_SCROLLED_WINDOW(cclists),
				    GTK_POLICY_AUTOMATIC, GTK_POLICY_AUTOMATIC);
    gtk_container_add (GTK_CONTAINER (cclists), cclist);
    gtk_box_pack_start (GTK_BOX(vbox1),cclists, TRUE, TRUE, 0);
#else
    gtk_clist_set_policy (GTK_CLIST (cclist), GTK_POLICY_AUTOMATIC,
			  GTK_POLICY_AUTOMATIC);
    gtk_box_pack_start (GTK_BOX(vbox1),cclist, TRUE, TRUE, 0);
#endif
    draw_keybindings (cclist);
    
#ifdef GTK_HAVE_FEATURES_1_1_12
    gtk_signal_connect_after (GTK_OBJECT(cclist),
                              "select_row",
                              GTK_SIGNAL_FUNC(cclist_button_event),
                              NULL);
#else
    gtk_signal_connect_after (GTK_OBJECT(cclist),
                              "button_press_event",
                              GTK_SIGNAL_FUNC(cclist_button_event),
                              NULL);
#endif

    gtk_widget_show(cclist);
#ifdef GTK_HAVE_FEATURES_1_1_12
 gtk_widget_show(cclists);
#endif  
    
    ehbox=gtk_hbox_new(FALSE, 0);


    clabel1 =  gtk_label_new ("Binding #:");
    gtk_box_pack_start (GTK_BOX (ehbox),clabel1, FALSE, TRUE, 2);
    gtk_widget_show (clabel1);

    cnumentrytext = gtk_label_new ("0");
    gtk_box_pack_start (GTK_BOX (ehbox),cnumentrytext, FALSE, TRUE, 2);
    gtk_widget_set_usize (cnumentrytext, 25, 0);
    gtk_widget_show (cnumentrytext);

    clabel2 =  gtk_label_new ("Key:");
    gtk_box_pack_start (GTK_BOX (ehbox),clabel2, FALSE, TRUE, 2);
    gtk_widget_show (clabel2);

    ckeyentrytext = gtk_entry_new ();
    gtk_box_pack_start (GTK_BOX (ehbox),ckeyentrytext, TRUE, TRUE, 2);
    gtk_widget_set_usize (ckeyentrytext, 110, 0);
    gtk_signal_connect(GTK_OBJECT(ckeyentrytext), "key_press_event",
		       GTK_SIGNAL_FUNC(ckeyentry_callback),
		       ckeyentrytext);
    /*  gtk_signal_connect(GTK_OBJECT(ckeyentrytext), "activate",
		       GTK_SIGNAL_FUNC(ckeyentry_callback),
		       ckeyentrytext);*/
    gtk_widget_show (ckeyentrytext);
     gtk_entry_set_text (GTK_ENTRY(ckeyentrytext),  "Press key to bind here");

    /*    clabel3 =  gtk_label_new ("Key #:");
    gtk_box_pack_start (GTK_BOX (ehbox),clabel3, FALSE, TRUE, 2);
    gtk_widget_show (clabel3);

    cknumentrytext = gtk_entry_new ();
    gtk_box_pack_start (GTK_BOX (ehbox),cknumentrytext, FALSE, TRUE, 2);
   gtk_signal_connect(GTK_OBJECT(cknumentrytext), "key_press_event",
		       GTK_SIGNAL_FUNC(cknumentry_callback),
		       cknumentrytext);
    gtk_widget_set_usize (cknumentrytext, 35, 0);
    gtk_widget_show (cknumentrytext);*/

    clabel4 =  gtk_label_new ("Mods:");
    gtk_box_pack_start (GTK_BOX (ehbox),clabel4, FALSE, TRUE, 2);
    gtk_widget_show (clabel4);

    cmodentrytext = gtk_entry_new ();
    gtk_box_pack_start (GTK_BOX (ehbox),cmodentrytext, FALSE, TRUE, 2);
    gtk_widget_set_usize (cmodentrytext, 45, 0);
    gtk_widget_show (cmodentrytext);


    gtk_box_pack_start (GTK_BOX (vbox1),ehbox, FALSE, TRUE, 2);

    gtk_widget_show (ehbox);

    ehbox=gtk_hbox_new(FALSE, 0);

    clabel5 =  gtk_label_new ("Command:");
    gtk_box_pack_start (GTK_BOX (ehbox),clabel5, FALSE, TRUE, 2);
    gtk_widget_show (clabel5);

    ckentrytext = gtk_entry_new ();
    gtk_box_pack_start (GTK_BOX (ehbox),ckentrytext, TRUE, TRUE, 2);
    gtk_widget_show (ckentrytext);


    gtk_box_pack_start (GTK_BOX (vbox1),ehbox, FALSE, TRUE, 2);
    
    gtk_widget_show (ehbox);

    ehbox=gtk_hbox_new(TRUE, 0);


    cb1 = gtk_button_new_with_label ("Unbind");
    gtk_box_pack_start (GTK_BOX (ehbox),cb1, FALSE, TRUE, 4);
    /*gtk_widget_set_usize (cb1, 45, 0);*/
    gtk_signal_connect_object (GTK_OBJECT (cb1), "clicked",
			       GTK_SIGNAL_FUNC(ckeyunbind),
			       NULL);
    gtk_widget_show (cb1);
    
    cb2 = gtk_button_new_with_label ("Bind");
    gtk_box_pack_start (GTK_BOX (ehbox),cb2, FALSE, TRUE, 4);
    gtk_signal_connect_object (GTK_OBJECT (cb2), "clicked",
			       GTK_SIGNAL_FUNC(bind_callback),
			       NULL);
    /*  gtk_widget_set_usize (cb2, 45, 0);*/
    gtk_widget_show (cb2);

    cb3 = gtk_button_new_with_label ("Clear");
    gtk_box_pack_start (GTK_BOX (ehbox),cb3, FALSE, TRUE, 4);
    /*    gtk_widget_set_usize (cb2, 45, 0);*/
    gtk_signal_connect_object (GTK_OBJECT (cb3), "clicked",
			       GTK_SIGNAL_FUNC(ckeyclear),
			       NULL);
    gtk_widget_show (cb3);
    gtk_box_pack_start (GTK_BOX (vbox1),ehbox, FALSE, TRUE, 2);

    gtk_widget_show (ehbox);


    gtk_widget_show (vbox1);
    gtk_widget_show (frame1);
    gtk_widget_show (vbox2);


    gtk_widget_show (notebook);
    gtk_widget_show (hbox);

    /* And give some options to actually do something with our new nifty configuration */

    hbox1 = gtk_hbox_new(TRUE, 0);
    gtk_box_pack_start(GTK_BOX(vbox), hbox1, FALSE, FALSE, 6);
    savebutton = gtk_button_new_with_label("Save");
    gtk_signal_connect_object (GTK_OBJECT (savebutton), "clicked",
			       GTK_SIGNAL_FUNC(saveconfig),
			       NULL);
    gtk_box_pack_start(GTK_BOX(hbox1), savebutton, FALSE, TRUE, 4);

    applybutton = gtk_button_new_with_label("Apply");
    gtk_signal_connect_object (GTK_OBJECT (applybutton), "clicked",
			       GTK_SIGNAL_FUNC(applyconfig),
			       NULL);
    gtk_box_pack_start(GTK_BOX(hbox1), applybutton, FALSE, TRUE, 4);

    cancelbutton = gtk_button_new_with_label("Close");
    gtk_signal_connect_object (GTK_OBJECT (cancelbutton), "clicked",
			       GTK_SIGNAL_FUNC(gtk_widget_destroy),
			       GTK_OBJECT (gtkwin_config));
 
    gtk_box_pack_start(GTK_BOX(hbox1), cancelbutton, FALSE, TRUE, 4);
    gtk_widget_show(savebutton);
    gtk_widget_show(applybutton);
    gtk_widget_show(cancelbutton);

    gtk_widget_show (hbox1);
    gtk_widget_show (vbox);
    gtk_widget_show (gtkwin_config);
  }
  else { 
    gdk_window_raise (gtkwin_config->window);
  }
}

/* Ok, simplistic help system. Just put the text file up in a scrollable window */

void chelpdialog(GtkWidget *widget) {
#include "help/chelp.h"
  GtkWidget *vbox;
  GtkWidget *hbox;
  GtkWidget *chelptext;
  GtkWidget *helpbutton;
  GtkWidget *vscrollbar;
  /*  GtkStyle *style;*/


  if(!gtkwin_chelp) {
    
    gtkwin_chelp = gtk_window_new (GTK_WINDOW_DIALOG);
    gtk_window_position (GTK_WINDOW (gtkwin_chelp), GTK_WIN_POS_CENTER);
    gtk_widget_set_usize (gtkwin_chelp,400,300);
    gtk_window_set_title (GTK_WINDOW (gtkwin_chelp), "Crossfire Client Help");
    gtk_window_set_policy (GTK_WINDOW (gtkwin_chelp), TRUE, TRUE, FALSE);

    gtk_signal_connect (GTK_OBJECT (gtkwin_chelp), "destroy", GTK_SIGNAL_FUNC(gtk_widget_destroyed), &gtkwin_chelp);
    
    gtk_container_border_width (GTK_CONTAINER (gtkwin_chelp), 0);
    vbox = gtk_vbox_new(FALSE, 2);
    gtk_container_add (GTK_CONTAINER(gtkwin_chelp),vbox);
    hbox = gtk_hbox_new(FALSE, 2);
    gtk_box_pack_start (GTK_BOX (vbox), hbox, TRUE, TRUE, 0);

    chelptext = gtk_text_new (NULL, NULL);
    gtk_text_set_editable (GTK_TEXT (chelptext), FALSE);
    gtk_box_pack_start (GTK_BOX (hbox),chelptext, TRUE, TRUE, 0);
    gtk_widget_show (chelptext);

    vscrollbar = gtk_vscrollbar_new (GTK_TEXT (chelptext)->vadj);
    gtk_box_pack_start (GTK_BOX (hbox),vscrollbar, FALSE, FALSE, 0);
 
    gtk_widget_show (vscrollbar);
    gtk_widget_show (hbox);

    hbox = gtk_hbox_new(FALSE, 2);
    
    helpbutton = gtk_button_new_with_label ("Close");
    gtk_signal_connect_object (GTK_OBJECT (helpbutton), "clicked",
			       GTK_SIGNAL_FUNC(gtk_widget_destroy),
			       GTK_OBJECT (gtkwin_chelp));
    gtk_box_pack_start (GTK_BOX (hbox), helpbutton, TRUE, FALSE, 0);
    gtk_box_pack_start (GTK_BOX (vbox), hbox, FALSE, FALSE, 0);
    gtk_widget_show (helpbutton);
    gtk_widget_show (hbox);

    gtk_widget_show (vbox);
    gtk_widget_show (gtkwin_chelp);
    gtk_text_insert (GTK_TEXT (chelptext), NULL, &chelptext->style->black, NULL, text , -1);    
  }
  else { 
    gdk_window_raise (gtkwin_chelp->window);
  }
}

/* Same simplistic help system. Serverside help this time. */

void shelpdialog(GtkWidget *widget) {
#include "help/shelp.h"
  GtkWidget *vbox;
  GtkWidget *hbox;
  GtkWidget *shelptext;
  GtkWidget *helpbutton;
  GtkWidget *vscrollbar;
  /*  GtkStyle *style;*/

  if(!gtkwin_shelp) {
    
    gtkwin_shelp = gtk_window_new (GTK_WINDOW_DIALOG);
    gtk_window_position (GTK_WINDOW (gtkwin_shelp), GTK_WIN_POS_CENTER);
    gtk_widget_set_usize (gtkwin_shelp,400,300);
    gtk_window_set_title (GTK_WINDOW (gtkwin_shelp), "Crossfire Server Help");
    gtk_window_set_policy (GTK_WINDOW (gtkwin_shelp), TRUE, TRUE, FALSE);

    gtk_signal_connect (GTK_OBJECT (gtkwin_shelp), "destroy", GTK_SIGNAL_FUNC(gtk_widget_destroyed), &gtkwin_shelp);
    
    gtk_container_border_width (GTK_CONTAINER (gtkwin_shelp), 0);
    vbox = gtk_vbox_new(FALSE, 2);
    gtk_container_add (GTK_CONTAINER(gtkwin_shelp),vbox);
    hbox = gtk_hbox_new(FALSE, 2);
    gtk_box_pack_start (GTK_BOX (vbox), hbox, TRUE, TRUE, 0);

    shelptext = gtk_text_new (NULL, NULL);
    gtk_text_set_editable (GTK_TEXT (shelptext), FALSE);
    gtk_box_pack_start (GTK_BOX (hbox),shelptext, TRUE, TRUE, 0);
    gtk_widget_show (shelptext);

    vscrollbar = gtk_vscrollbar_new (GTK_TEXT (shelptext)->vadj);
    gtk_box_pack_start (GTK_BOX (hbox),vscrollbar, FALSE, FALSE, 0);
 
    gtk_widget_show (vscrollbar);
    gtk_widget_show (hbox);

    hbox = gtk_hbox_new(FALSE, 2);
    
    helpbutton = gtk_button_new_with_label ("Close");
    gtk_signal_connect_object (GTK_OBJECT (helpbutton), "clicked",
			       GTK_SIGNAL_FUNC(gtk_widget_destroy),
			       GTK_OBJECT (gtkwin_shelp));
    gtk_box_pack_start (GTK_BOX (hbox), helpbutton, TRUE, FALSE, 0);
    gtk_box_pack_start (GTK_BOX (vbox), hbox, FALSE, FALSE, 0);
    gtk_widget_show (helpbutton);
    gtk_widget_show (hbox);

    gtk_widget_show (vbox);
    gtk_widget_show (gtkwin_shelp);
    gtk_text_insert (GTK_TEXT (shelptext), NULL, &shelptext->style->black, NULL, text , -1);
  }
  else { 
    gdk_window_raise (gtkwin_shelp->window);
  }
}


/* Various routines for setting modes by menu choices. */

void menu_pickup0 () {
  pickup_mode = 0;
  send_command("pickup 0", -1, 0);
}

void menu_pickup1 () {
  pickup_mode = 1;
  send_command("pickup 1", -1, 0);
}

void menu_pickup2 () {
  pickup_mode = 2;
  send_command("pickup 2", -1, 0);
}

void menu_pickup3 () {
  pickup_mode = 3;
  send_command("pickup 3", -1, 0);
}

void menu_pickup4 () {
  pickup_mode = 4;
  send_command("pickup 4", -1, 0);
}

void menu_pickup5 () {
  pickup_mode = 5;
  send_command("pickup 5", -1, 0);
  
}

void menu_pickup6 () {
  pickup_mode = 6;
  send_command("pickup 6", -1, 0);
}

void menu_pickup7 () {
  pickup_mode = 7;
  send_command("pickup 7", -1, 0);
}

void menu_pickup10 () {
  pickup_mode = 10;
  send_command("pickup 10", -1, 0);
}



void menu_who () {
  extended_command("who");
}

void menu_apply () {
  extended_command("apply");
}

void menu_cast () {
  gtk_entry_set_text(GTK_ENTRY(entrytext),"cast ");
  gtk_widget_grab_focus (GTK_WIDGET(entrytext));
}

void menu_search () {
  extended_command("search");
}

void menu_disarm () {
  extended_command("disarm");
}


void menu_spells () {
  char buf[MAX_BUF];
  int i;
  for (i=0; i<25 ; i++) {
    sprintf(buf,"Range: spell (%s)", cpl.spells[cpl.ready_spell]);

    /*    strcpy (buf, cpl.spells[i]);*/
    printf ("Spell: %s\n", cpl.spells[cpl.ready_spell]);
  }
}

void menu_clear () {
  guint size;
  
  size = gtk_text_get_length(GTK_TEXT (gtkwin_info_text));
  gtk_text_freeze (GTK_TEXT (gtkwin_info_text));
  gtk_text_set_point(GTK_TEXT (gtkwin_info_text), 0);
  gtk_text_forward_delete (GTK_TEXT (gtkwin_info_text), size );
  gtk_text_thaw (GTK_TEXT (gtkwin_info_text));

  size = gtk_text_get_length(GTK_TEXT (gtkwin_info_text2));
  gtk_text_freeze (GTK_TEXT (gtkwin_info_text2));
  gtk_text_set_point(GTK_TEXT (gtkwin_info_text2), 0);
  gtk_text_forward_delete (GTK_TEXT (gtkwin_info_text2), size );
  gtk_text_thaw (GTK_TEXT (gtkwin_info_text2));
}

void sexit()
{
    extended_command("quit");
}

/* get_menu_display 
 * This sets up menus
 */

static int get_menu_display (GtkWidget *box) {
  GtkWidget *filemenu;
  GtkWidget *actionmenu;
  GtkWidget *pickupmenu;
  GtkWidget *clientmenu;
  GtkWidget *helpmenu;
  GtkWidget *menu_bar;
  GtkWidget *root_filemenu;
  GtkWidget *root_helpmenu;
  GtkWidget *root_actionmenu;
  /*  GtkWidget *sub_pickupmenu;*/
  GtkWidget *root_clientmenu;
  GtkWidget *menu_items;
  GtkWidget *pickup_menu_item;
  GSList *pickupgroup;


  /* Init the menu-widget, and remember -- never
   * gtk_show_widget() the menu widget!! 
   * This is the menu that holds the menu items, the one that
   * will pop up when you click on the "Root Menu" in the app */
  filemenu = gtk_menu_new();
  
  /* Next we make a little loop that makes three menu-entries for "test-menu".
   * Notice the call to gtk_menu_append.  Here we are adding a list of
   * menu items to our menu.  Normally, we'd also catch the "clicked"
   * signal on each of the menu items and setup a callback for it,
   * but it's omitted here to save space. */

#ifdef GTK_HAVE_FEATURES_1_1_12
  menu_items = gtk_tearoff_menu_item_new ();
  gtk_menu_append (GTK_MENU (filemenu), menu_items);
  gtk_widget_show (menu_items);
#endif

  menu_items = gtk_menu_item_new_with_label("Save config");
  gtk_menu_append(GTK_MENU (filemenu), menu_items);   
  gtk_signal_connect_object(GTK_OBJECT(menu_items), "activate",
			    GTK_SIGNAL_FUNC(save_defaults), NULL);
  gtk_widget_show(menu_items);

  menu_items = gtk_menu_item_new_with_label("Save window positions");
  gtk_menu_append(GTK_MENU (filemenu), menu_items);   
  gtk_signal_connect_object(GTK_OBJECT(menu_items), "activate",
			    GTK_SIGNAL_FUNC(save_winpos), NULL);
  gtk_widget_show(menu_items);

  menu_items = gtk_menu_item_new ();
  gtk_menu_append(GTK_MENU (filemenu), menu_items);   
  gtk_widget_show(menu_items);


  menu_items = gtk_menu_item_new_with_label("Quit character");
  gtk_menu_append(GTK_MENU (filemenu), menu_items);   
  gtk_signal_connect_object(GTK_OBJECT(menu_items), "activate",
			    GTK_SIGNAL_FUNC(sexit), NULL);
  gtk_widget_show(menu_items);

  menu_items = gtk_menu_item_new_with_label("Quit client");
  gtk_menu_append(GTK_MENU (filemenu), menu_items);   
  gtk_signal_connect_object(GTK_OBJECT(menu_items), "activate",
			    GTK_SIGNAL_FUNC(exit), NULL);
  gtk_widget_show(menu_items);

  /* This is the root menu, and will be the label
   * displayed on the menu bar.  There won't be a signal handler attached,
   * as it only pops up the rest of the menu when pressed. */
  root_filemenu = gtk_menu_item_new_with_label("File");
  
  gtk_widget_show(root_filemenu);
  
  /* Now we specify that we want our newly created "menu" to be the menu
   * for the "root menu" */
  gtk_menu_item_set_submenu(GTK_MENU_ITEM (root_filemenu), filemenu);

 /* Do the clientmenu */

  clientmenu = gtk_menu_new();
  
  /*  menu_items = gtk_menu_item_new_with_label("Navigator");
  gtk_menu_append(GTK_MENU (clientmenu), menu_items);   
  gtk_signal_connect_object(GTK_OBJECT(menu_items), "activate",
			    GTK_SIGNAL_FUNC(navbut), NULL);
			    gtk_widget_show(menu_items);*/

#ifdef GTK_HAVE_FEATURES_1_1_12
  menu_items = gtk_tearoff_menu_item_new ();
  gtk_menu_append (GTK_MENU (clientmenu), menu_items);
  gtk_widget_show (menu_items);
#endif

  menu_items = gtk_menu_item_new_with_label("Clear info");
  gtk_menu_append(GTK_MENU (clientmenu), menu_items);   
  gtk_signal_connect_object(GTK_OBJECT(menu_items), "activate",
			    GTK_SIGNAL_FUNC(menu_clear), NULL);
  gtk_widget_show(menu_items);


  menu_items = gtk_menu_item_new_with_label("Spells");
  gtk_menu_append(GTK_MENU (clientmenu), menu_items);   
  gtk_signal_connect_object(GTK_OBJECT(menu_items), "activate",
			    GTK_SIGNAL_FUNC(menu_spells), NULL);
  gtk_widget_show(menu_items);

  menu_items = gtk_menu_item_new_with_label("Configure");
  gtk_menu_append(GTK_MENU (clientmenu), menu_items);   
  gtk_signal_connect_object(GTK_OBJECT(menu_items), "activate",
			    GTK_SIGNAL_FUNC(configdialog), NULL);
  gtk_widget_show(menu_items);


  menu_items = gtk_menu_item_new_with_label("Disconnect");
  gtk_menu_append(GTK_MENU (clientmenu), menu_items);   
  gtk_signal_connect_object(GTK_OBJECT(menu_items), "activate",
			    GTK_SIGNAL_FUNC(disconnect), NULL);
  gtk_widget_show(menu_items);


  root_clientmenu = gtk_menu_item_new_with_label("Client");
  
  gtk_widget_show(root_clientmenu);
  gtk_menu_item_set_submenu(GTK_MENU_ITEM (root_clientmenu), clientmenu);

  /* Do the actionmenu */

  actionmenu = gtk_menu_new();

#ifdef GTK_HAVE_FEATURES_1_1_12
  menu_items = gtk_tearoff_menu_item_new ();
  gtk_menu_append (GTK_MENU (actionmenu), menu_items);
  gtk_widget_show (menu_items);
#endif

  menu_items = gtk_menu_item_new_with_label("Who");
  gtk_menu_append(GTK_MENU (actionmenu), menu_items);   
  gtk_signal_connect_object(GTK_OBJECT(menu_items), "activate",
			    GTK_SIGNAL_FUNC(menu_who), NULL);
  gtk_widget_show(menu_items);

  menu_items = gtk_menu_item_new_with_label("Cast...");
  gtk_menu_append(GTK_MENU (actionmenu), menu_items);   
  gtk_signal_connect_object(GTK_OBJECT(menu_items), "activate",
			    GTK_SIGNAL_FUNC(menu_cast), NULL);
  gtk_widget_show(menu_items);

  menu_items = gtk_menu_item_new_with_label("Apply");
  gtk_menu_append(GTK_MENU (actionmenu), menu_items);   
  gtk_signal_connect_object(GTK_OBJECT(menu_items), "activate",
			    GTK_SIGNAL_FUNC(menu_apply), NULL);
  gtk_widget_show(menu_items);

  pickup_menu_item = gtk_menu_item_new_with_label("Pickup");
  gtk_menu_append(GTK_MENU (actionmenu), pickup_menu_item);
  /*  gtk_signal_connect_object(GTK_OBJECT(pickup_menu_item), "activate",
			    GTK_SIGNAL_FUNC(menu_apply), NULL);*/
  gtk_widget_show(pickup_menu_item);

  menu_items = gtk_menu_item_new_with_label("Search");
  gtk_menu_append(GTK_MENU (actionmenu), menu_items);   
  gtk_signal_connect_object(GTK_OBJECT(menu_items), "activate",
			    GTK_SIGNAL_FUNC(menu_search), NULL);
  gtk_widget_show(menu_items);

  menu_items = gtk_menu_item_new_with_label("Disarm");
  gtk_menu_append(GTK_MENU (actionmenu), menu_items);   
  gtk_signal_connect_object(GTK_OBJECT(menu_items), "activate",
			    GTK_SIGNAL_FUNC(menu_disarm), NULL);
  gtk_widget_show(menu_items);


  root_actionmenu = gtk_menu_item_new_with_label("Action");
  
  gtk_widget_show(root_actionmenu);
  gtk_menu_item_set_submenu(GTK_MENU_ITEM (root_actionmenu), actionmenu);

 /* Do the submenu */

  pickupmenu = gtk_menu_new();
  
  /*  This allows you to change your pickup status. Eight different modes for pick up exist: ``don't pick up'',``pick up 1
item'', ``pick up 1 item and stop'', ``stop before picking up'', ``pick up all items'', pick up all items and stop'',
``pick up all magic items'', ``pick up all coins and gems''. Whenever you move over a pile of stuff your pickup*/
  pickupgroup=NULL;

#ifdef GTK_HAVE_FEATURES_1_1_12
  menu_items = gtk_tearoff_menu_item_new ();
  gtk_menu_append (GTK_MENU (pickupmenu), menu_items);
  gtk_widget_show (menu_items);
#endif

  menu_items = gtk_radio_menu_item_new_with_label(pickupgroup, "Don't pick up");
  pickupgroup = gtk_radio_menu_item_group (GTK_RADIO_MENU_ITEM (menu_items));
  gtk_check_menu_item_set_show_toggle (GTK_CHECK_MENU_ITEM (menu_items), TRUE);
  gtk_menu_append(GTK_MENU (pickupmenu), menu_items);   
  gtk_signal_connect_object(GTK_OBJECT(menu_items), "activate",
			    GTK_SIGNAL_FUNC(menu_pickup0), NULL);
  gtk_widget_show(menu_items);

  menu_items = gtk_radio_menu_item_new_with_label(pickupgroup, "Pick up 1 item");
  pickupgroup = gtk_radio_menu_item_group (GTK_RADIO_MENU_ITEM (menu_items));
  gtk_check_menu_item_set_show_toggle (GTK_CHECK_MENU_ITEM (menu_items), TRUE);
  gtk_menu_append(GTK_MENU (pickupmenu), menu_items);   
  gtk_signal_connect_object(GTK_OBJECT(menu_items), "activate",
			    GTK_SIGNAL_FUNC(menu_pickup1), NULL);
  gtk_widget_show(menu_items);


  menu_items = gtk_radio_menu_item_new_with_label(pickupgroup, "Pick up 1 item and stop");
  pickupgroup = gtk_radio_menu_item_group (GTK_RADIO_MENU_ITEM (menu_items));
  gtk_check_menu_item_set_show_toggle (GTK_CHECK_MENU_ITEM (menu_items), TRUE);
  gtk_menu_append(GTK_MENU (pickupmenu), menu_items);   
  gtk_signal_connect_object(GTK_OBJECT(menu_items), "activate",
			    GTK_SIGNAL_FUNC(menu_pickup2), NULL);
  gtk_widget_show(menu_items);

  menu_items = gtk_radio_menu_item_new_with_label(pickupgroup, "Stop before picking up.");
  pickupgroup = gtk_radio_menu_item_group (GTK_RADIO_MENU_ITEM (menu_items));
  gtk_check_menu_item_set_show_toggle (GTK_CHECK_MENU_ITEM (menu_items), TRUE);
  gtk_menu_append(GTK_MENU (pickupmenu), menu_items);   
  gtk_signal_connect_object(GTK_OBJECT(menu_items), "activate",
			    GTK_SIGNAL_FUNC(menu_pickup3), NULL);
  gtk_widget_show(menu_items);

  menu_items = gtk_radio_menu_item_new_with_label(pickupgroup, "Pick up all items.");
  pickupgroup = gtk_radio_menu_item_group (GTK_RADIO_MENU_ITEM (menu_items));
  gtk_check_menu_item_set_show_toggle (GTK_CHECK_MENU_ITEM (menu_items), TRUE);
  gtk_menu_append(GTK_MENU (pickupmenu), menu_items);   
  gtk_signal_connect_object(GTK_OBJECT(menu_items), "activate",
			    GTK_SIGNAL_FUNC(menu_pickup4), NULL);
  gtk_widget_show(menu_items);

  menu_items = gtk_radio_menu_item_new_with_label(pickupgroup, "Pick up all items and stop.");
  pickupgroup = gtk_radio_menu_item_group (GTK_RADIO_MENU_ITEM (menu_items));
  gtk_check_menu_item_set_show_toggle (GTK_CHECK_MENU_ITEM (menu_items), TRUE);
  gtk_menu_append(GTK_MENU (pickupmenu), menu_items);   
  gtk_signal_connect_object(GTK_OBJECT(menu_items), "activate",
			    GTK_SIGNAL_FUNC(menu_pickup5), NULL);
  gtk_widget_show(menu_items);

  menu_items = gtk_radio_menu_item_new_with_label(pickupgroup, "Pick up all magic items.");
  pickupgroup = gtk_radio_menu_item_group (GTK_RADIO_MENU_ITEM (menu_items));
  gtk_check_menu_item_set_show_toggle (GTK_CHECK_MENU_ITEM (menu_items), TRUE);
  gtk_menu_append(GTK_MENU (pickupmenu), menu_items);   
  gtk_signal_connect_object(GTK_OBJECT(menu_items), "activate",
			    GTK_SIGNAL_FUNC(menu_pickup6), NULL);
  gtk_widget_show(menu_items);

  menu_items = gtk_radio_menu_item_new_with_label(pickupgroup, "Pick up all coins and gems.");
  pickupgroup = gtk_radio_menu_item_group (GTK_RADIO_MENU_ITEM (menu_items));
  gtk_check_menu_item_set_show_toggle (GTK_CHECK_MENU_ITEM (menu_items), TRUE);
  gtk_menu_append(GTK_MENU (pickupmenu), menu_items);   
  gtk_signal_connect_object(GTK_OBJECT(menu_items), "activate",
			    GTK_SIGNAL_FUNC(menu_pickup7), NULL);
  gtk_widget_show(menu_items);

  menu_items = gtk_radio_menu_item_new_with_label(pickupgroup, "Pickup silver and higher value/weight.");
  pickupgroup = gtk_radio_menu_item_group (GTK_RADIO_MENU_ITEM (menu_items));
  gtk_check_menu_item_set_show_toggle (GTK_CHECK_MENU_ITEM (menu_items), TRUE);
  gtk_menu_append(GTK_MENU (pickupmenu), menu_items);   
  gtk_signal_connect_object(GTK_OBJECT(menu_items), "activate",
			    GTK_SIGNAL_FUNC(menu_pickup10), NULL);
  gtk_widget_show(menu_items);


  /*  sub_pickupmenu = gtk_menu_item_new_with_label("Action");
  
  gtk_widget_show(sub_pickupmenu);*/
  gtk_menu_item_set_submenu(GTK_MENU_ITEM (pickup_menu_item), pickupmenu);

 
  /*Do the helpmenu */
  helpmenu = gtk_menu_new();
  
#ifdef GTK_HAVE_FEATURES_1_1_12
  menu_items = gtk_tearoff_menu_item_new ();
  gtk_menu_append (GTK_MENU (helpmenu), menu_items);
  gtk_widget_show (menu_items);
#endif

  menu_items = gtk_menu_item_new_with_label("About");
  gtk_menu_append(GTK_MENU (helpmenu), menu_items);   
  gtk_signal_connect_object(GTK_OBJECT(menu_items), "activate",
			    GTK_SIGNAL_FUNC(aboutdialog), NULL);
  gtk_widget_show(menu_items);

  menu_items = gtk_menu_item_new ();
  gtk_menu_append(GTK_MENU (helpmenu), menu_items);   
  gtk_widget_show(menu_items);


  menu_items = gtk_menu_item_new_with_label("Client help");
  gtk_menu_append(GTK_MENU (helpmenu), menu_items);   
  gtk_signal_connect_object(GTK_OBJECT(menu_items), "activate",
			    GTK_SIGNAL_FUNC(chelpdialog), NULL);
  gtk_widget_show(menu_items);

  menu_items = gtk_menu_item_new_with_label("Server help");
  gtk_menu_append(GTK_MENU (helpmenu), menu_items);   
  gtk_signal_connect_object(GTK_OBJECT(menu_items), "activate",
			    GTK_SIGNAL_FUNC(shelpdialog), NULL);
  gtk_widget_show(menu_items);



  root_helpmenu = gtk_menu_item_new_with_label("Help");
  
  gtk_widget_show(root_helpmenu);
  gtk_menu_item_set_submenu(GTK_MENU_ITEM (root_helpmenu), helpmenu);

  /* Create a menu-bar to hold the menus and add it to our main window */
 

  menu_bar = gtk_menu_bar_new();
  gtk_box_pack_start(GTK_BOX(box), menu_bar, FALSE, FALSE, 2);
  gtk_widget_show(menu_bar);
  
  /* Create a button to which to attach menu as a popup */

  /* And finally we append the menu-item to the menu-bar -- this is the
   * "root" menu-item I have been raving about =) */
  gtk_menu_bar_append(GTK_MENU_BAR (menu_bar), root_filemenu);
  gtk_menu_bar_append(GTK_MENU_BAR (menu_bar), root_clientmenu);
  gtk_menu_bar_append(GTK_MENU_BAR (menu_bar), root_actionmenu);
  gtk_menu_item_right_justify (GTK_MENU_ITEM(root_helpmenu));
  gtk_menu_bar_append(GTK_MENU_BAR (menu_bar), root_helpmenu);
  
  /* always display the window as the last step so it all splashes on
   * the screen at once. */
  
  return 0;
}



/* get_root_display: 
 * this sets up the root window (or none, if in split
 * windows mode, and also scans for any Xdefaults.  Right now, only
 * splitwindow and image are used.  image is the display
 * mechanism to use.  I thought having one type that is set
 * to font, xpm, or pixmap was better than using xpm and pixmap
 * resources with on/off values (which gets pretty weird
 * if one of this is set to off.
 */


/* Create the splash window at startup */

void create_splash() {
  GtkWidget *vbox;

  GtkWidget *aboutgtkpixmap;
  GdkPixmap *aboutgdkpixmap;
  GdkBitmap *aboutgdkmask;

  GtkStyle *style;

 gtkwin_splash = gtk_window_new (GTK_WINDOW_DIALOG);
 gtk_window_position (GTK_WINDOW (gtkwin_splash), GTK_WIN_POS_CENTER);
 gtk_widget_set_usize (gtkwin_splash,346,87);
 gtk_window_set_title (GTK_WINDOW (gtkwin_splash), "Welcome to Crossfire");
 gtk_signal_connect (GTK_OBJECT (gtkwin_splash), "destroy", GTK_SIGNAL_FUNC(gtk_widget_destroyed), &gtkwin_splash);
    
 gtk_container_border_width (GTK_CONTAINER (gtkwin_splash), 0);
 vbox = gtk_vbox_new(FALSE, 0);
 gtk_container_add (GTK_CONTAINER(gtkwin_splash),vbox);
 style = gtk_widget_get_style(gtkwin_splash);
 gtk_widget_realize(gtkwin_splash);
 aboutgdkpixmap = gdk_pixmap_create_from_xpm_d(gtkwin_splash->window,
					       &aboutgdkmask,
						  &style->bg[GTK_STATE_NORMAL],
					       (gchar **)crossfiretitle);
 aboutgtkpixmap= gtk_pixmap_new (aboutgdkpixmap, aboutgdkmask);
 gtk_box_pack_start (GTK_BOX (vbox),aboutgtkpixmap, FALSE, TRUE, 0);
 gtk_widget_show (aboutgtkpixmap);
 

 gtk_widget_show (vbox);
 gtk_widget_show (gtkwin_splash);

 
 while ( gtk_events_pending() ) {
   gtk_main_iteration();
 }
 sleep (1);
 while ( gtk_events_pending() ) {
   gtk_main_iteration();
 }

}


void destroy_splash() {
  gtk_widget_destroy(gtkwin_splash);
}

/* Error handlers removed.  Right now, there is nothing for
 * the client to do if it gets a fatal error - it doesn't have
 * any information to save.  And we might as well let the standard
 * X11 error handler handle non fatal errors.
 */

 
void create_windows() {
  GtkWidget *rootvbox;
  GtkWidget *frame;
  GtkWidget *mhpaned;
  GtkWidget *ghpaned;
  GtkWidget *gvpaned;
  GtkWidget *vpaned;
  int i;

  tooltips = gtk_tooltips_new();

  if (split_windows==FALSE) {  
    gtkwin_root = gtk_window_new (GTK_WINDOW_TOPLEVEL);
    gtk_widget_set_events (gtkwin_root, GDK_KEY_RELEASE_MASK);
    gtk_widget_set_uposition (gtkwin_root, 0, 0);
    gtk_widget_set_usize (gtkwin_root,636+(image_size*11),336+(image_size*11));
    gtk_window_set_title (GTK_WINDOW (gtkwin_root), "Crossfire GTK Client");
    gtk_signal_connect (GTK_OBJECT (gtkwin_root), "destroy", GTK_SIGNAL_FUNC(gtk_widget_destroyed), &gtkwin_root);
    
    gtk_container_border_width (GTK_CONTAINER (gtkwin_root), 0);

    /* Alloc colors.  colorname[] comes from xutil.c */
    for (i=0; i<=12; i++ ) {
	if ( !gdk_color_parse(colorname[i], &root_color[i])) {
	    printf ("cparse failed (%s)\n",colorname[i]);
	}
	if ( !gdk_color_alloc (gtk_widget_get_colormap (gtkwin_root), &root_color[i])) {
	    printf ("calloc failed\n");
	}
    }
    
    /* menu / windows division */
    rootvbox = gtk_vbox_new(FALSE, 0);
    gtk_container_add (GTK_CONTAINER (gtkwin_root), rootvbox);
    gtk_widget_show (rootvbox);
    
    get_menu_display(rootvbox);
    
    /* first horizontal division. inv+obj on left, rest on right */
    
    mhpaned = gtk_hpaned_new ();
    /*  gtk_container_add (GTK_CONTAINER (gtkwin_root), mhpaned);*/
    gtk_box_pack_start (GTK_BOX (rootvbox), mhpaned, TRUE, TRUE, 0);
    gtk_container_border_width (GTK_CONTAINER(mhpaned), 5);
    gtk_widget_show (mhpaned);

    /* Divisior game+stats | text */
    
    ghpaned = gtk_hpaned_new ();
    gtk_paned_add2 (GTK_PANED (mhpaned), ghpaned);
    
    /* text frame */
    
    frame = gtk_frame_new (NULL);
    gtk_frame_set_shadow_type (GTK_FRAME(frame), GTK_SHADOW_ETCHED_IN);
    gtk_widget_set_usize (frame, 300, 400);
    gtk_paned_add2 (GTK_PANED (ghpaned), frame);
    
    get_info_display (frame);
    gtk_widget_show (frame);
    /* game & statbars below, stats above */
    
    gvpaned = gtk_vpaned_new ();
    gtk_paned_add1 (GTK_PANED (ghpaned), gvpaned);
    
    /* game - statbars */
    
    vpaned = gtk_vpaned_new ();
    gtk_paned_add2 (GTK_PANED (gvpaned), vpaned);
    
    
    /* Statbars frame */
    frame = gtk_frame_new (NULL);
    gtk_frame_set_shadow_type (GTK_FRAME(frame), GTK_SHADOW_ETCHED_IN);
    gtk_widget_set_usize (frame, (image_size*11)+6, 100);
    gtk_paned_add2 (GTK_PANED (vpaned), frame);
    
    get_message_display(frame);
    
    gtk_widget_show (frame);
    
    /* Game frame */
    frame = gtk_frame_new (NULL);
    gtk_frame_set_shadow_type (GTK_FRAME(frame), GTK_SHADOW_ETCHED_IN);
    gtk_widget_set_usize (frame, (image_size*11)+6, (image_size*11)+6);
    gtk_paned_add1 (GTK_PANED (vpaned), frame);
    
    get_game_display (frame);
    
    gtk_widget_show (frame);
    
    /* stats frame */
    frame = gtk_frame_new (NULL);
    gtk_frame_set_shadow_type (GTK_FRAME(frame), GTK_SHADOW_ETCHED_IN);
    gtk_widget_set_usize (frame, (image_size*11)+6, 110);
    gtk_paned_add1 (GTK_PANED (gvpaned), frame);
    get_stats_display (frame);
    
    gtk_widget_show (frame);
    
    gtk_widget_show (vpaned);
    gtk_widget_show (gvpaned);
    
    vpaned = gtk_vpaned_new ();
    gtk_paned_add1 (GTK_PANED (mhpaned), vpaned);
    
    /* inventory frame */
    frame = gtk_frame_new (NULL);
    gtk_frame_set_shadow_type (GTK_FRAME(frame), GTK_SHADOW_ETCHED_IN);
    gtk_widget_set_usize (frame, 270, 400);
    gtk_paned_add1 (GTK_PANED (vpaned), frame);

    get_inv_display (frame);
    
    gtk_widget_show (frame);
    
    /* look frame */
    frame = gtk_frame_new (NULL);
    gtk_frame_set_shadow_type (GTK_FRAME(frame), GTK_SHADOW_ETCHED_IN);
    gtk_widget_set_usize (frame, 270, 200);
    gtk_paned_add2 (GTK_PANED (vpaned), frame);
    
    get_look_display (frame);
    
    gtk_widget_show (frame);
    
    gtk_widget_show (vpaned);
    
    gtk_widget_show (ghpaned);
    
    gtk_widget_show (mhpaned);
    

    /* Connect signals */
    
    gtk_signal_connect_object (GTK_OBJECT (gtkwin_root), "key_press_event",
			       GTK_SIGNAL_FUNC(keyfunc), GTK_OBJECT(gtkwin_root));
    gtk_signal_connect_object (GTK_OBJECT (gtkwin_root), "key_release_event",
			       GTK_SIGNAL_FUNC(keyrelfunc), GTK_OBJECT(gtkwin_root));
    gtk_widget_show (gtkwin_root);
    

  } else { /* split window mode */

 
  /* game window */

    gtkwin_root = gtk_window_new (GTK_WINDOW_TOPLEVEL);
    gtk_widget_set_events (gtkwin_root, GDK_KEY_RELEASE_MASK);
    gtk_widget_set_uposition (gtkwin_root, 300, 160);
    gtk_widget_set_usize (gtkwin_root,(image_size*11)+6,(image_size*11)+6);
    gtk_window_set_title (GTK_WINDOW (gtkwin_root), "Crossfire - view");
    gtk_window_set_policy (GTK_WINDOW (gtkwin_root), TRUE, TRUE, FALSE);
    gtk_signal_connect (GTK_OBJECT (gtkwin_root), "destroy", GTK_SIGNAL_FUNC(gtk_widget_destroyed), &gtkwin_root);
    
    gtk_container_border_width (GTK_CONTAINER (gtkwin_root), 0);
    
 
    rootvbox = gtk_vbox_new(FALSE, 0);
    gtk_container_add (GTK_CONTAINER (gtkwin_root), rootvbox);
   
    gtk_widget_realize (rootvbox);
    
    gtk_widget_realize (gtkwin_root);
    

 
    gtk_widget_show (rootvbox);   
    gtk_widget_show (gtkwin_root);
    gtk_widget_draw (gtkwin_root,NULL);
    gtk_widget_draw (rootvbox,NULL);

    get_game_display(rootvbox);     



  /* Stats and menu window */
    gtkwin_stats = gtk_window_new (GTK_WINDOW_TOPLEVEL);
    gtk_widget_set_events (gtkwin_stats, GDK_KEY_RELEASE_MASK);
    gtk_widget_set_uposition (gtkwin_stats, 300, 0);
    gtk_widget_set_usize (gtkwin_stats,(image_size*11)+6,140);
    gtk_window_set_title (GTK_WINDOW (gtkwin_stats), "Crossfire GTK Client");
    gtk_window_set_policy (GTK_WINDOW (gtkwin_stats), TRUE, TRUE, FALSE);
    gtk_signal_connect (GTK_OBJECT (gtkwin_stats), "destroy", GTK_SIGNAL_FUNC(gtk_widget_destroyed), &gtkwin_stats);
    
    gtk_container_border_width (GTK_CONTAINER (gtkwin_stats), 0);
    

    rootvbox = gtk_vbox_new(FALSE, 0);
    gtk_container_add (GTK_CONTAINER (gtkwin_stats), rootvbox);
    gtk_widget_show (rootvbox);
    
    get_menu_display(rootvbox);
    get_stats_display (rootvbox);
    gtk_widget_realize (gtkwin_stats);
    gdk_window_set_group (gtkwin_stats->window, gtkwin_root->window);
    gtk_widget_show (gtkwin_stats);
    
 
   /* info window - text and messages */
    gtkwin_info = gtk_window_new (GTK_WINDOW_TOPLEVEL);
    gtk_widget_set_events (gtkwin_info, GDK_KEY_RELEASE_MASK);
    gtk_widget_set_uposition (gtkwin_info, 570, 0);
    gtk_widget_set_usize (gtkwin_info,400,600);
    gtk_window_set_title (GTK_WINDOW (gtkwin_info), "Crossfire - info");
     gtk_window_set_policy (GTK_WINDOW (gtkwin_info), TRUE, TRUE, FALSE);
    gtk_signal_connect (GTK_OBJECT (gtkwin_info), "destroy", GTK_SIGNAL_FUNC(gtk_widget_destroyed), &gtkwin_info);
    
    gtk_container_border_width (GTK_CONTAINER (gtkwin_info), 0);
    
    /* Alloc colors - not entirely necessary, really, since GTK should do this */
    /* colorname[] comes from xutil.c */
    for (i=0; i<=12; i++ ) {
	if ( !gdk_color_parse(colorname[i], &root_color[i])) {
	    printf ("cparse failed (%s)\n",colorname[i]);
	}
	if ( !gdk_color_alloc (gtk_widget_get_colormap (gtkwin_info), &root_color[i])) {
	    printf ("calloc failed\n");
	}
    }

    rootvbox = gtk_vbox_new(FALSE, 0);
    gtk_container_add (GTK_CONTAINER (gtkwin_info), rootvbox);
    gtk_widget_show (rootvbox);
    
    get_info_display(rootvbox);

    gtk_widget_show (gtkwin_info);
    gtk_widget_realize (gtkwin_info);
    gdk_window_set_group (gtkwin_info->window, gtkwin_root->window);

 /* statbars window */
    gtkwin_message = gtk_window_new (GTK_WINDOW_TOPLEVEL);
    gtk_widget_set_events (gtkwin_message, GDK_KEY_RELEASE_MASK);
    gtk_widget_set_uposition (gtkwin_message, 300, 450);
    gtk_widget_set_usize (gtkwin_message,(image_size*11)+6,170);
    gtk_window_set_title (GTK_WINDOW (gtkwin_message), "Crossfire - vitals");
 gtk_window_set_policy (GTK_WINDOW (gtkwin_message), TRUE, TRUE, FALSE);
    gtk_signal_connect (GTK_OBJECT (gtkwin_message), "destroy", GTK_SIGNAL_FUNC(gtk_widget_destroyed), &gtkwin_message);
    
    gtk_container_border_width (GTK_CONTAINER (gtkwin_message), 0);
    
    
    rootvbox = gtk_vbox_new(FALSE, 0);
    gtk_container_add (GTK_CONTAINER (gtkwin_message), rootvbox);
    gtk_widget_show (rootvbox);
    
    get_message_display(rootvbox);
    
    gtk_widget_show (gtkwin_message);
    gtk_widget_realize (gtkwin_message);
    gdk_window_set_group (gtkwin_message->window, gtkwin_root->window);

 /* inventory window */
    gtkwin_inv = gtk_window_new (GTK_WINDOW_TOPLEVEL);
    gtk_widget_set_events (gtkwin_inv, GDK_KEY_RELEASE_MASK);
    gtk_widget_set_uposition (gtkwin_inv, 0, 0);
    gtk_widget_set_usize (gtkwin_inv,290,400);
    gtk_window_set_title (GTK_WINDOW (gtkwin_inv), "Crossfire - inventory");
    gtk_window_set_policy (GTK_WINDOW (gtkwin_inv), TRUE, TRUE, FALSE);
    gtk_signal_connect (GTK_OBJECT (gtkwin_inv), "destroy", GTK_SIGNAL_FUNC(gtk_widget_destroyed), &gtkwin_inv);
    
    gtk_container_border_width (GTK_CONTAINER (gtkwin_inv), 0);
    
    
    rootvbox = gtk_vbox_new(FALSE, 0);
    gtk_container_add (GTK_CONTAINER (gtkwin_inv), rootvbox);
    gtk_widget_show (rootvbox);
    
    get_inv_display(rootvbox);
    
    gtk_widget_show (gtkwin_inv);
    gtk_widget_realize (gtkwin_inv);
    gdk_window_set_group (gtkwin_inv->window, gtkwin_root->window);
 /* look window */
    gtkwin_look = gtk_window_new (GTK_WINDOW_TOPLEVEL);
    gtk_widget_set_events (gtkwin_look, GDK_KEY_RELEASE_MASK);
    gtk_widget_set_uposition (gtkwin_look, 0, 420);
    gtk_widget_set_usize (gtkwin_look,290,150);
    gtk_window_set_title (GTK_WINDOW (gtkwin_look), "Crossfire - look");
    gtk_window_set_policy (GTK_WINDOW (gtkwin_look), TRUE, TRUE, FALSE);
    gtk_signal_connect (GTK_OBJECT (gtkwin_look), "destroy", GTK_SIGNAL_FUNC(gtk_widget_destroyed), &gtkwin_look);
    
    gtk_container_border_width (GTK_CONTAINER (gtkwin_look), 0);
    
    
    rootvbox = gtk_vbox_new(FALSE, 0);
    gtk_container_add (GTK_CONTAINER (gtkwin_look), rootvbox);
    gtk_widget_show (rootvbox);
    
    get_look_display(rootvbox);
    
    gtk_widget_show (gtkwin_look);
    gtk_widget_realize (gtkwin_look);
    gdk_window_set_group (gtkwin_look->window, gtkwin_root->window);
    /* Setup key events */
    
    gtk_signal_connect_object (GTK_OBJECT (gtkwin_message), "key_press_event",
			       GTK_SIGNAL_FUNC(keyfunc), GTK_OBJECT(gtkwin_message));
    gtk_signal_connect_object (GTK_OBJECT (gtkwin_message), "key_release_event",
			       GTK_SIGNAL_FUNC(keyrelfunc), GTK_OBJECT(gtkwin_message));
  
    gtk_signal_connect_object (GTK_OBJECT (gtkwin_root), "key_press_event",
			       GTK_SIGNAL_FUNC(keyfunc), GTK_OBJECT(gtkwin_root));
    gtk_signal_connect_object (GTK_OBJECT (gtkwin_root), "key_release_event",
			       GTK_SIGNAL_FUNC(keyrelfunc), GTK_OBJECT(gtkwin_root));

    gtk_signal_connect_object (GTK_OBJECT (gtkwin_info), "key_press_event",
			       GTK_SIGNAL_FUNC(keyfunc), GTK_OBJECT(gtkwin_info));
    gtk_signal_connect_object (GTK_OBJECT (gtkwin_info), "key_release_event",
			       GTK_SIGNAL_FUNC(keyrelfunc), GTK_OBJECT(gtkwin_info));

    gtk_signal_connect_object (GTK_OBJECT (gtkwin_look), "key_press_event",
			       GTK_SIGNAL_FUNC(keyfunc), GTK_OBJECT(gtkwin_look));
    gtk_signal_connect_object (GTK_OBJECT (gtkwin_look), "key_release_event",
			       GTK_SIGNAL_FUNC(keyrelfunc), GTK_OBJECT(gtkwin_look));

    gtk_signal_connect_object (GTK_OBJECT (gtkwin_inv), "key_press_event",
			       GTK_SIGNAL_FUNC(keyfunc), GTK_OBJECT(gtkwin_inv));
    gtk_signal_connect_object (GTK_OBJECT (gtkwin_inv), "key_release_event",
			       GTK_SIGNAL_FUNC(keyrelfunc), GTK_OBJECT(gtkwin_inv));

    gtk_signal_connect_object (GTK_OBJECT (gtkwin_stats), "key_press_event",
			       GTK_SIGNAL_FUNC(keyfunc), GTK_OBJECT(gtkwin_stats));
    gtk_signal_connect_object (GTK_OBJECT (gtkwin_stats), "key_release_event",
			       GTK_SIGNAL_FUNC(keyrelfunc), GTK_OBJECT(gtkwin_stats));
    set_window_pos();
  }
  gtk_tooltips_set_delay(tooltips, 1000 );
  if (tool_tips) {
    gtk_tooltips_enable(tooltips);
  }
}

int sync_display = 0;
static int get_root_display(char *display_name,int gargc, char **gargv) {
  gtk_init (&gargc,&gargv);
  last_str=malloc(32767);

  create_splash();
  create_windows();
  
  return 0;
}
 
/* null procedures. gtk does this for us. */


void set_weight_limit (uint32 wlim)
{
    inv_list.weight_limit = wlim;
}


void set_scroll(char *s)
{
}

void draw_all_info()
{
}

void resize_win_info()
{
}


int get_info_width()
{
    return 40;	/* would be better to return some real info here  - I'll have to look at it later
		 * to see how easy it is to get that */
}



/***********************************************************************
 *
 * Here is the start of event handling functions
 *
 ***********************************************************************/



/* This function handles the reading of the X Events and then
 * doing the appropriate action.  For most input events, it is calling
 * another function.
 *
 * It can also call display functions to make sure the information is
 * correct - in this way, updates will not be done so often (like
 * for every ITEM command received), but less frequently but still
 * update the display fully.  All the functions above are optimized to
 * only draw stuff that needs drawing.  So calling them a lot is not
 * going to draw stuff too much.
 */

void do_clearlock () {
}

void x_set_echo() {
  if (nopopups) {
    gtk_entry_set_visibility(GTK_ENTRY(entrytext), !cpl.no_echo);
  }
}

int do_timeout() {

  updatelock=0;
  if (draw_info_freeze1) {
    gtk_text_thaw (GTK_TEXT (gtkwin_info_text));
    gtk_adjustment_set_value(GTK_ADJUSTMENT(text_vadj), GTK_ADJUSTMENT(text_vadj)->upper-GTK_ADJUSTMENT(text_vadj)->page_size);
    gtk_text_set_adjustments(GTK_TEXT (gtkwin_info_text),GTK_ADJUSTMENT(text_hadj),GTK_ADJUSTMENT(text_vadj));
    draw_info_freeze1=FALSE;
  }
  if (draw_info_freeze2) {
    gtk_text_thaw (GTK_TEXT (gtkwin_info_text2));
    gtk_adjustment_set_value(GTK_ADJUSTMENT(text_vadj2), GTK_ADJUSTMENT(text_vadj2)->upper-GTK_ADJUSTMENT(text_vadj2)->page_size);
    gtk_text_set_adjustments(GTK_TEXT (gtkwin_info_text2),GTK_ADJUSTMENT(text_hadj2),GTK_ADJUSTMENT(text_vadj2));
    draw_info_freeze2=FALSE;
  }
  if (redraw_needed) {
    display_map_doneupdate();
    draw_all_list(&inv_list);
    draw_all_list(&look_list);
    redraw_needed=FALSE;
  }
  if (!inv_list.env->inv_updated) {
    animate_list();
  }
  if (cpl.showmagic) magic_map_flash_pos();
  draw_lists();
  return TRUE;
}




/* Here are the old Xutil commands needed. */
/* ----------------------------------------------------------------------------*/


void display_newbitmap(long face,long fg,long bg,char *buf)
{
}

/* This function draws the magic map in the game window.  I guess if
 * we wanted to get clever, we could open up some other window or
 * something.
 *
 * A lot of this code was taken from server/xio.c  But being all
 * the map data has been figured, it tends to be much simpler.
 */
void draw_magic_map()
{

  int x=0;
  int y=0;
 
  GtkWidget *hbox;
  GtkWidget *closebutton;
  GtkStyle *style;

  static GtkWidget *magicgtkpixmap;
 

  static GdkBitmap *magicgdkmask;
 
 

  if (!cpl.magicmap) {
    draw_info ("You have yet to cast magic map.",NDI_BLACK);
    return;
  }
  
  if(!gtkwin_magicmap) {
    
    gtkwin_magicmap = gtk_window_new (GTK_WINDOW_DIALOG);
    gtk_window_position (GTK_WINDOW (gtkwin_magicmap), GTK_WIN_POS_CENTER);
    gtk_widget_set_usize (gtkwin_magicmap,264,300);
    gtk_window_set_title (GTK_WINDOW (gtkwin_magicmap), "Magic map");
    gtk_window_set_policy (GTK_WINDOW (gtkwin_magicmap), FALSE, FALSE, FALSE);
    
    gtk_signal_connect (GTK_OBJECT (gtkwin_magicmap), "destroy", GTK_SIGNAL_FUNC(gtk_widget_destroyed), &gtkwin_magicmap);

    mapvbox = gtk_vbox_new(FALSE, 0);
    gtk_widget_set_usize (mapvbox,264,300);
    gtk_container_add (GTK_CONTAINER(gtkwin_magicmap),mapvbox);

    style = gtk_widget_get_style(gtkwin_magicmap);
    gtk_widget_realize(mapvbox);


    magicgdkpixmap = gdk_pixmap_new(gtkwin_magicmap->window,
				    264,
				    264,
				    -1);
    magicgtkpixmap= gtk_pixmap_new (magicgdkpixmap, magicgdkmask);
    gtk_box_pack_start (GTK_BOX (mapvbox),magicgtkpixmap, FALSE, FALSE, 0);
    gtk_widget_show (magicgtkpixmap);
    
    hbox = gtk_hbox_new(FALSE, 2);
    
    closebutton = gtk_button_new_with_label ("Close");
    gtk_signal_connect_object (GTK_OBJECT (closebutton), "clicked",
			       GTK_SIGNAL_FUNC(gtk_widget_destroy),
			       GTK_OBJECT (gtkwin_magicmap));
    gtk_box_pack_start (GTK_BOX (hbox), closebutton, TRUE, FALSE, 0);
    gtk_box_pack_start (GTK_BOX (mapvbox), hbox, FALSE, FALSE, 0);
    gtk_widget_show (closebutton);
    gtk_widget_show (hbox);

    gtk_widget_show (mapvbox);
    gtk_widget_show (gtkwin_magicmap);
    
    
    gdk_color_parse("Black", &map_color[0]);
    gdk_color_alloc (gtk_widget_get_colormap (magicgtkpixmap), &map_color[0]);
    gdk_color_parse("White", &map_color[1]);
    gdk_color_alloc (gtk_widget_get_colormap (magicgtkpixmap), &map_color[1]);
    gdk_color_parse("Navy", &map_color[2]);
    gdk_color_alloc (gtk_widget_get_colormap (magicgtkpixmap), &map_color[2]);
    gdk_color_parse("Red", &map_color[3]);
    gdk_color_alloc (gtk_widget_get_colormap (magicgtkpixmap), &map_color[3]);
    gdk_color_parse("Orange", &map_color[4]);
    gdk_color_alloc (gtk_widget_get_colormap (magicgtkpixmap), &map_color[4]);
    gdk_color_parse("DodgerBlue", &map_color[5]);
    gdk_color_alloc (gtk_widget_get_colormap (magicgtkpixmap), &map_color[5]);
    gdk_color_parse("DarkOrange2", &map_color[6]);
    gdk_color_alloc (gtk_widget_get_colormap (magicgtkpixmap), &map_color[6]);
    gdk_color_parse("SeaGreen", &map_color[7]);
    gdk_color_alloc (gtk_widget_get_colormap (magicgtkpixmap), &map_color[7]);
    gdk_color_parse("DarkSeaGreen", &map_color[8]);
    gdk_color_alloc (gtk_widget_get_colormap (magicgtkpixmap), &map_color[8]);
    gdk_color_parse("Grey50", &map_color[9]);
    gdk_color_alloc (gtk_widget_get_colormap (magicgtkpixmap), &map_color[9]);
    gdk_color_parse("Sienna", &map_color[10]);
    gdk_color_alloc (gtk_widget_get_colormap (magicgtkpixmap), &map_color[10]);
    gdk_color_parse("Gold", &map_color[11]);
    gdk_color_alloc (gtk_widget_get_colormap (magicgtkpixmap), &map_color[11]);
    gdk_color_parse("Khaki", &map_color[12]);
    gdk_color_alloc (gtk_widget_get_colormap (magicgtkpixmap), &map_color[12]);


    map_gc = gdk_gc_new (magicgdkpixmap);


  gdk_gc_set_foreground (map_gc, &map_color[0]);
   gdk_draw_rectangle (magicgdkpixmap, map_gc,	       
		       TRUE,
		       0,
		       0,
		       264,
		       264);
    cpl.mapxres = (262)/cpl.mmapx;
    cpl.mapyres = (262)/cpl.mmapy;
    if (cpl.mapxres < 1 || cpl.mapyres<1) {
      fprintf(stderr,"magic map resolution less than 1, map is %dx%d\n",
	      cpl.mmapx, cpl.mmapy);
      return;
    }
    /* In theory, cpl.mapxres and cpl.mapyres do not have to be the same.  However,
     * it probably makes sense to keep them the same value.
     * Need to take the smaller value.
     */
    if (cpl.mapxres>cpl.mapyres) cpl.mapxres=cpl.mapyres;
    else cpl.mapyres=cpl.mapxres;
    
    if (cpl.mapxres>24) {
      cpl.mapxres=24;
      cpl.mapyres=24;
    }
    /* this is keeping the same unpacking scheme that the server uses
     * to pack it up.
     */
    for (y = 0; y < cpl.mmapy; y++) {
      for (x = 0; x < cpl.mmapx; x++) {
	uint8 val = cpl.magicmap[y*cpl.mmapx + x];

    gdk_gc_set_foreground (map_gc, &map_color[val&FACE_COLOR_MASK]);

	gdk_draw_rectangle (magicgdkpixmap, map_gc,
			    TRUE,
			    2+cpl.mapxres*x,
			    2+cpl.mapyres*y,
			    cpl.mapxres,
			    cpl.mapyres);
      } /* Saw into this space */
    }
    /*    gdk_gc_destroy (map_gc);*/
    gtk_widget_draw (mapvbox,NULL);
  }
  
  else { 
    /* ------------------ There is already a magic map up - replace it ---------*/

    gdk_window_raise (gtkwin_magicmap->window);
    /* --------------------------- */
 
   gdk_gc_set_foreground (map_gc, &map_color[0]);
   gdk_draw_rectangle (magicgdkpixmap, map_gc,	       
		       TRUE,
		       0,
		       0,
		       264,
		       264);
   
    cpl.mapxres = (262)/cpl.mmapx;
    cpl.mapyres = (262)/cpl.mmapy;
    if (cpl.mapxres < 1 || cpl.mapyres<1) {
      fprintf(stderr,"magic map resolution less than 1, map is %dx%d\n",
	      cpl.mmapx, cpl.mmapy);
      return;
    }

    if (cpl.mapxres>cpl.mapyres) cpl.mapxres=cpl.mapyres;
    else cpl.mapyres=cpl.mapxres;
    


    if (cpl.mapxres>24) {
      cpl.mapxres=24;
      cpl.mapyres=24;
    }



    for (y = 0; y < cpl.mmapy; y++) {
      for (x = 0; x < cpl.mmapx; x++) {
	uint8 val = cpl.magicmap[y*cpl.mmapx + x];

    gdk_gc_set_foreground (map_gc, &map_color[val&FACE_COLOR_MASK]);

	gdk_draw_rectangle (magicgdkpixmap, map_gc,
			    TRUE,
			    2+cpl.mapxres*x,
			    2+cpl.mapyres*y,
			    cpl.mapxres,
			    cpl.mapyres);
	
      } 

    }
 gtk_widget_draw (mapvbox,NULL);
    /*---------------------------*/
  }
}

/* Basically, this just flashes the player position on the magic map */

void magic_map_flash_pos()
{
  if (!cpl.showmagic) return;
  if (!gtkwin_magicmap) return;
  cpl.showmagic ^=2;
  if (cpl.showmagic & 2) {
    gdk_gc_set_foreground (map_gc, &map_color[0]);
  } else {
    gdk_gc_set_foreground (map_gc, &map_color[1]);
  }
  gdk_draw_rectangle (magicgdkpixmap, map_gc,
		      TRUE,
		      2+cpl.mapxres*cpl.pmapx,
		      2+cpl.mapyres*cpl.pmapy,
		      cpl.mapxres,
		      cpl.mapyres);
  gtk_widget_draw (mapvbox,NULL);
}

/* Gets a specified windows coordinates.  This function is pretty much
 * an exact copy out of the server.
 */

static void get_window_coord(GtkWidget *win,
                 int *x,int *y,
                 int *wx,int *wy,
                 int *w,int *h)
{
  int tmp;
  gdk_window_get_geometry (win->window, x, y, w, h, &tmp);
  gdk_window_get_origin (win->window, wx, wy);
}


void save_winpos()
{
  char savename[MAX_BUF],buf[MAX_BUF];
  FILE    *fp;
  int	    x,y,w,h,wx,wy;
  
  if (!split_windows) {
    draw_info("You can only save window positions in split windows mode", NDI_BLUE);
	return;
  }
  sprintf(savename,"%s/.crossfire/winpos", getenv("HOME"));
  if (!(fp=fopen(savename,"w"))) {
    sprintf(buf,"Unable to open %s, window positions not saved",savename);
    draw_info(buf,NDI_BLUE);
    return;
    }
  /* This is a bit simpler than what the server was doing - it has
   * some code to handle goofy window managers which I am not sure
   * is still needed.
   */
  get_window_coord(gtkwin_root, &x,&y, &wx,&wy,&w,&h);
  fprintf(fp,"win_game: %d %d %d %d\n", wx,wy, w, h);
  get_window_coord(gtkwin_stats, &x,&y, &wx,&wy,&w,&h);
  fprintf(fp,"win_stats: %d %d %d %d\n", wx,wy, w, h);
  get_window_coord(gtkwin_info, &x,&y, &wx,&wy,&w,&h);
  fprintf(fp,"win_info: %d %d %d %d\n", wx,wy, w, h);
  get_window_coord(gtkwin_inv, &x,&y, &wx,&wy,&w,&h);
  fprintf(fp,"win_inv: %d %d %d %d\n", wx,wy, w, h);
  get_window_coord(gtkwin_look, &x,&y, &wx,&wy,&w,&h);
  fprintf(fp,"win_look: %d %d %d %d\n", wx,wy, w, h);
  get_window_coord(gtkwin_message, &x,&y, &wx,&wy,&w,&h);
  fprintf(fp,"win_message: %d %d %d %d\n", wx,wy, w, h);
  fclose(fp);
  sprintf(buf,"Window positions saved to %s",savename);
  draw_info(buf,NDI_BLUE);
}

void command_show (char *params)
{
    if(!params)  {
	/* Shouldn't need to get current page, but next_page call is not wrapping
	 * like the docs claim it should.
	 */
	if (gtk_notebook_get_current_page(GTK_NOTEBOOK(inv_notebook))==8)
	    gtk_notebook_set_page(GTK_NOTEBOOK(inv_notebook), 0);
	else 
	    gtk_notebook_next_page(GTK_NOTEBOOK(inv_notebook));

    } else if (!strncmp(params, "all", strlen(params)))
	gtk_notebook_set_page(GTK_NOTEBOOK(inv_notebook), 0);
    else if (!strncmp(params, "applied", strlen(params)))
	gtk_notebook_set_page(GTK_NOTEBOOK(inv_notebook), 1);

    else if (!strncmp(params, "unapplied", strlen(params)))
	gtk_notebook_set_page(GTK_NOTEBOOK(inv_notebook), 2);

    else if (!strncmp(params, "unpaid", strlen(params)))
	gtk_notebook_set_page(GTK_NOTEBOOK(inv_notebook), 3);

    else if (!strncmp(params, "cursed", strlen(params)))
	gtk_notebook_set_page(GTK_NOTEBOOK(inv_notebook), 4);

    else if (!strncmp(params, "magical", strlen(params)))
	gtk_notebook_set_page(GTK_NOTEBOOK(inv_notebook), 5);

    else if (!strncmp(params, "nonmagical", strlen(params)))
	gtk_notebook_set_page(GTK_NOTEBOOK(inv_notebook), 6);

    else if (!strncmp(params, "locked", strlen(params)))
	gtk_notebook_set_page(GTK_NOTEBOOK(inv_notebook), 7);

    else if (!strncmp(params, "unlocked", strlen(params)))
	gtk_notebook_set_page(GTK_NOTEBOOK(inv_notebook), 8);


}

/* Reads in the winpos file created by the above function and sets the
 * the window positions appropriately.
 */
void set_window_pos()
{
  gint wx=0;
  gint wy=0;
  gint w=0;
  gint h=0;

    char buf[MAX_BUF],*cp;
    FILE *fp;

    if (!split_windows) return;

    sprintf(buf,"%s/.crossfire/winpos", getenv("HOME"));
    if (!(fp=fopen(buf,"r"))) return;

    while(fgets(buf,MAX_BUF-1, fp)!=NULL) {
	buf[MAX_BUF-1]='\0';
	if (!(cp=strchr(buf,' '))) continue;
	*cp++='\0';
	if (sscanf(cp,"%d %d %d %d",&wx,&wy,&w,&h)!=4)
	    continue;
	if (!strcmp(buf,"win_game:")) {
	  gtk_widget_set_uposition (gtkwin_root, wx, wy);
	  gtk_widget_set_usize (gtkwin_root, w, h);
	}
	if (!strcmp(buf,"win_stats:")) {
	  gtk_widget_set_uposition (gtkwin_stats, wx, wy);
	  gtk_widget_set_usize (gtkwin_stats, w, h);
	}
	if (!strcmp(buf,"win_info:")) {
	  gtk_widget_set_uposition (gtkwin_info, wx, wy);
	  gtk_widget_set_usize (gtkwin_info, w, h);
	}
	if (!strcmp(buf,"win_inv:")) {
	  gtk_widget_set_uposition (gtkwin_inv, wx, wy);
	  gtk_widget_set_usize (gtkwin_inv, w, h);
	}
	if (!strcmp(buf,"win_look:")) {
	  gtk_widget_set_uposition (gtkwin_look, wx, wy);
	  gtk_widget_set_usize (gtkwin_look, w, h);
	}
	if (!strcmp(buf,"win_message:")) {
	  gtk_widget_set_uposition (gtkwin_message, wx, wy);
	  gtk_widget_set_usize (gtkwin_message, w, h);
	}
    }
}




/***********************************************************************
 *
 * Here starts the X11 init functions.  It will call other
 * functions that were grouped previously by window
 *
 ***********************************************************************/

/* Usage routine.  All clients should support server, port and
 * display options, with -pix and -xpm also suggested.  -split
 * does not need to be supported - it is in this copy because
 * the old code supported it.
 */

static void usage(char *progname)
{
    puts("Usage of cfclient:\n\n");
    puts("-server <name>   - Connect to <name> instead of localhost.");
    puts("-port <number>   - Use port <number> instead of the standard port number");
    puts("-display <name>  - Use <name> instead if DISPLAY environment variable.\n");
    puts("-split           - Use split windows.");
    puts("-echo            - Echo the bound commands");
#ifdef Xpm_Pix
    puts("-xpm             - Use color pixmaps (XPM) for display.");
#endif
#ifdef HAVE_LIBPNG
    puts("-png             - Use png images for display.");
#endif
    puts("-showicon        - Print status icons in inventory window");
    puts("-scrolllines <number>    - number of lines for scrollback");
    puts("-sync            - Synchronize on display");
    puts("-help            - Display this message.");
    puts("-cache           - Cache images for future use.");
    puts("-nocache         - Do not cache images (default action).");
    puts("-nosound         - Disable sound output.");
    puts("-updatekeycodes  - Update the saved bindings for this keyboard.");
    puts("-keepcache       - Keep already cached images even if server has different ones.");
    puts("-pngfile <name>  - Use <name> for source of images");
    puts("-nopopups        - Don't use pop up windows for input");
    puts("-splitinfo       - Use two information windows, segregated by information type.");
    exit(0);
}

/* init_windows:  This initiliazes all the windows - it is an
 * interface routine.  The command line arguments are passed to
 * this function to interpert.  Note that it is not in fact
 * required to parse anything, but doing at least -server and
 * -port would be a good idea.
 *
 * This function returns 0 on success, nonzero on failure.
 */

int init_windows(int argc, char **argv)
{
    int on_arg=1;
    char *display_name="";
    load_defaults(); 

    for (on_arg=1; on_arg<argc; on_arg++) {
	if (!strcmp(argv[on_arg],"-display")) {
	    if (++on_arg == argc) {
		fprintf(stderr,"-display requires a display name\n");
		return 1;
	    }
	    display_name = argv[on_arg];
	    continue;
	}
	if (strcmp(argv[on_arg],"-sync")==0) {
	    sync_display = 1;
	    continue;
	}
	if (!strcmp(argv[on_arg],"-port")) {
	    if (++on_arg == argc) {
		fprintf(stderr,"-port requires a port number\n");
		return 1;
	    }
	    port_num = atoi(argv[on_arg]);
	    continue;
	}
	if (!strcmp(argv[on_arg],"-server")) {
	    if (++on_arg == argc) {
		fprintf(stderr,"-server requires a host name\n");
		return 1;
	    }
	    server = argv[on_arg];
	    continue;
	}
	if (!strcmp(argv[on_arg],"-pngfile")) {
	    if (++on_arg == argc) {
		fprintf(stderr,"-pngfile requires a file name\n");
		return 1;
	    }
	    image_file = argv[on_arg];
	    continue;
	}
	if (!strcmp(argv[on_arg],"-xpm")) {
	    display_mode = Xpm_Display;
	    continue;
	}
        if (!strcmp(argv[on_arg],"-png")) {
            display_mode = Png_Display;
	}
	else if (!strcmp(argv[on_arg],"-cache")) {
	    cache_images= TRUE;
	    continue;
	}
	else if (!strcmp(argv[on_arg],"-nocache")) {
	    cache_images= FALSE;
	    continue;
	}
	else if (!strcmp(argv[on_arg],"-split")) {
	    split_windows=TRUE;
	    continue;
	}
	else if (!strcmp(argv[on_arg],"-nosplit")) {
	    split_windows=FALSE;
	    continue;
	}
	else if (!strcmp(argv[on_arg],"-showicon")) {
	    inv_list.show_icon = TRUE;
	    continue;
	}
	else if (!strcmp(argv[on_arg],"-echo")) {
	    cpl.echo_bindings=TRUE;
	    continue;
	}
	else if (!strcmp(argv[on_arg],"-help")) {
	    usage(argv[0]);
	    continue;
	}
	else if (!strcmp(argv[on_arg],"-nosound")) {
	    nosound=TRUE;
	    continue;
	}
	else if (!strcmp(argv[on_arg],"-updatekeycodes")) {
	    updatekeycodes=TRUE;
	    continue;
	}
	else if (!strcmp(argv[on_arg],"-keepcache")) {
	    keepcache=TRUE;
	    continue;
	}
	else if (!strcmp(argv[on_arg],"-nopopups")) {
	    nopopups=TRUE;
	    continue;
	}
	else if (!strcmp(argv[on_arg],"-splitinfo")) {
	    splitinfo=TRUE;
	    continue;
	}
	else {
	    fprintf(stderr,"Do not understand option %s\n", argv[on_arg]);
	    usage(argv[0]);
	    return 1;
	}
    }
    /* Moving processing and setting of display attributes down here.
     * This is because a default display mode may also require special
     * handling.
     * This is basically same as x11.c, but we must have xpm mode and we don't
     * have pixmap mode.
     */
    if (display_mode == Png_Display) {
#ifndef HAVE_LIBPNG
	fprintf(stderr,"Client not configured with Png display mode enabled\n");
	fprintf(stderr,"Will try using xpm display mode\n");
	display_mode = Xpm_Display;
#else
	image_size=32;
#endif
    }
    if (display_mode == Xpm_Display) {
	    image_size=24;
    }

    /* Finished parsing all the command line options.  Now start
     * working on the display.
     */
    gargc=argc;
    gargv=argv;
    for (on_arg=0; on_arg<MAXPIXMAPNUM; on_arg++)
	facecachemap[on_arg]=on_arg;

    for (on_arg = 0; on_arg<MAX_HISTORY; on_arg++)
	history[on_arg][0]=0;

    if (get_root_display(display_name,gargc,gargv))
		return 1;

    if (display_mode == Png_Display) {
	gdk_rgb_init();
    }

    init_keys();
    if (cache_images) init_cache_data();
    if (image_file[0] != '\0') ReadImages();
    destroy_splash();

    return 0;
}


void display_map_clearcell(long x,long y)
{
  the_map.cells[x][y].count = 0;
}

void display_map_addbelow(long x,long y,long face)
{
  the_map.cells[x][y].faces[the_map.cells[x][y].count] = face&0xFFFF;
  the_map.cells[x][y].count ++;
}

/* Draw the tiled pixmap tiles in the mapcell */

void display_mapcell_pixmap(int ax,int ay)
{
  int k;

  gdk_draw_rectangle (mappixmap, 
		      drawingarea->style->mid_gc[0],
		      TRUE,
		      image_size*ax,
		      image_size*ay,
		      image_size,
		      image_size);

  for(k=the_map.cells[ax][ay].count-1;k>-1;k--) {
    gen_draw_face(the_map.cells[ax][ay].faces[k], ax,ay);
  }
}


int display_usebitmaps()
{
  return display_mode == Pix_Display;
}
int display_usexpm()
{
  return display_mode == Xpm_Display;
}
int display_usepng()
{
  return display_mode == Png_Display;
}

int display_willcache()
{
    return cache_images;
}

/* Do the map drawing */

void display_map_doneupdate()
{
  int ax,ay;
  /*  GdkGC *black_gc;*/

  if (updatelock < 30) {
  updatelock++;

  /* draw black on all non-visible squares, and tile pixmaps on the others */
  for(ax=0;ax<11;ax++) {
    for(ay=0;ay<11;ay++) { 
      if (the_map.cells[ax][ay].count==0) {
	/*	black_gc = gtkmap[ax][ay].pixmap->style->black_gc;*/
	gdk_draw_rectangle (mappixmap, drawingarea->style->black_gc,
			    TRUE,
			    image_size*ax,
			    image_size*ay,
			    image_size,
			    image_size);
	continue;
      } 
      display_mapcell_pixmap(ax,ay);
    }
  }
  gdk_draw_pixmap(drawingarea->window,
		  drawingarea->style->fg_gc[GTK_WIDGET_STATE (drawingarea)],
		  mappixmap,
		  0, 0,
		  0, 0,
		  image_size*11,image_size*11);
  /*
  gtk_widget_draw (table, NULL);
  */
  }
  else {
    /*    printf ("WARNING - Frozen updates until updatelock is cleared!\n");*/
  }
}

void display_mapscroll(int dx,int dy)
{
  int x,y;
  struct Map newmap;
  
  for(x=0;x<11;x++) {
    for(y=0;y<11;y++) {
      newmap.cells[x][y].count = 0;
      if (x+dx < 0 || x+dx >= 11)
      continue;
      if (y+dy < 0 || y+dy >= 11)
      continue;
	memcpy((char*)&(newmap.cells[x][y]), (char*)&(the_map.cells[x+dx][y+dy]),
	       sizeof(struct MapCell));
    }
  }
  memcpy((char*)&the_map,(char*)&newmap,sizeof(struct Map));

}


/* This is based a lot on the xpm function below */
/* There is no good way to load the data directly to a pixmap -
 * even some function which would seem to do the job just hide the
 * writing to a temp function further down (Imlib_inlined_png_to_image
 * does this).  As such, we might as well just do it at the top level - plus
 * if we are caching, at least we only write the file once then.
 */
void display_newpng(long face,char *buf,long buflen)
{
#ifdef HAVE_LIBPNG
    char    *filename;

    FILE *tmpfile;

    if (cache_images) {
	if (facetoname[face]==NULL) {
	    fprintf(stderr,"Caching images, but name for %ld not set\n", face);
	}
	filename = facetoname[face];
	if ((tmpfile = fopen(filename,"w"))==NULL) {
	    fprintf(stderr,"Can not open %s for writing\n", filename);
	}
	else {
	    fwrite(buf, buflen, 1, tmpfile);
	    fclose(tmpfile);
	}
    }

    if (png_to_gdkpixmap(gtkwin_root->window, buf, buflen,
			 &pixmaps[face].gdkpixmap, &pixmaps[face].gdkmask,
			 gtk_widget_get_colormap(gtkwin_root))) {
	    fprintf(stderr,"Got error on png_to_gdkpixmap\n");
    }

    if (cache_images) {
	if (facetoname[face]) {
	    free(facetoname[face]);
	    facetoname[face]=NULL;
	}
    }
#endif
}

void display_newpixmap(long face,char *buf,long buflen)
{
  FILE *tmpfile;

  GtkStyle *style;
  gchar **xpmbuffer;
  
  
  if (cache_images) {
    if (facetoname[face]==NULL) {
      fprintf(stderr,"Caching images, but name for %ld not set\n", face);
    }
    else if ((tmpfile = fopen(facetoname[face],"w"))==NULL) {
      fprintf(stderr,"Can not open %s for writing\n", facetoname[face]);
    }
    else {
      fprintf(tmpfile,"%s",buf);
      fclose(tmpfile);
    }
    style = gtk_widget_get_style(gtkwin_root);

   xpmbuffer=xpmbuffertodata(buf);
    pixmaps[face].gdkpixmap = gdk_pixmap_create_from_xpm_d(gtkwin_root->window,
							 &pixmaps[face].gdkmask,
							 &style->bg[GTK_STATE_NORMAL],
							 (gchar **) xpmbuffer );
    

    freexpmdata (xpmbuffer);

    redraw_needed=TRUE;

  } else {

    style = gtk_widget_get_style(gtkwin_root);

    xpmbuffer=xpmbuffertodata(buf);
    pixmaps[face].gdkpixmap = gdk_pixmap_create_from_xpm_d(gtkwin_root->window,
							 &pixmaps[face].gdkmask,
							 &style->bg[GTK_STATE_NORMAL],
							 (gchar **) xpmbuffer );
    

    freexpmdata (xpmbuffer);
  }

  if (facetoname[face] && cache_images) {
    free(facetoname[face]);
    facetoname[face]=NULL;
  }
}

void display_map_startupdate()
{
}

char *get_metaserver()
{
    cpl.input_state = Metaserver_Select;


    while(cpl.input_state==Metaserver_Select) {
	if (gtk_events_pending())
	    gtk_main_iteration();
	usleep(10*1000);    /* 10 milliseconds */
    }
    return cpl.input_text;
}

/* We can now connect to different servers, so we need to clear out
 * any old images.  We try to free the data also to prevent memory
 * leaks.
 * This could be more clever, ie, if we're caching images and go to
 * a new server and get a name, we should try to re-arrange our cache
 * or the like.
 */
 
void reset_image_data()
{
    int i;

    for (i=1; i<MAXPIXMAPNUM; i++) {
	if (pixmaps[i].gdkpixmap && (pixmaps[i].gdkpixmap!=pixmaps[0].gdkpixmap)) {
	    gdk_pixmap_unref(pixmaps[i].gdkpixmap);
	    pixmaps[i].gdkpixmap=NULL;
	    if (pixmaps[i].gdkmask) {
		gdk_pixmap_unref(pixmaps[i].gdkmask);
		pixmaps[i].gdkmask=NULL;
	    }
	}
	if (cache_images && facetoname[i]!=NULL) {
	    free(facetoname[i]);
	    facetoname[i]=NULL;
	}
    }
    memset(&the_map, 0, sizeof(struct Map));
    look_list.env=cpl.below;
}

