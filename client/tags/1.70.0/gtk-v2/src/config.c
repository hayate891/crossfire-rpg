const char * const rcsid_gtk2_config_c =
    "$Id$";
/*
    Crossfire client, a client program for the crossfire program.

    Copyright (C) 2005-2011 Mark Wedel & Crossfire Development Team

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

    The author can be reached via e-mail to crossfire@metalforge.org
*/

/**
 * @file gtk-v2/src/config.c
 * Covers configuration issues.
 */

#ifdef HAVE_CONFIG_H
#  include <config.h>
#endif

#include <gtk/gtk.h>
#include <glade/glade.h>
#include <ctype.h>

#include "client.h"

#include "main.h"
#include "image.h"
#include "gtk2proto.h"

#include <dirent.h>

#ifdef MINGW
int alphasort(const struct dirent **a, const struct dirent **b)
{
    return strcoll((*a)->d_name, (*b)->d_name);
}

int scandir(const char *dir, struct dirent ***namelist,
            int (*select)(const struct dirent *),
            int (*compar)(const struct dirent **, const struct dirent **)) {
    DIR *d;
    struct dirent *entry;
    register int i=0;
    size_t entrysize;

    if((d = opendir(dir)) == NULL)
        return -1;

    *namelist = NULL;
    while ((entry = readdir(d)) != NULL)
    {
        if (select == NULL || (select != NULL && (*select)(entry)))
        {
            *namelist = (struct dirent **)realloc((void *)(*namelist),
                                                  (size_t)((i + 1) * sizeof(struct dirent *)));
            if (*namelist == NULL) return -1;
            entrysize = sizeof(struct dirent) - sizeof(entry->d_name) + strlen(entry->d_name) + 1;
            (*namelist)[i] = (struct dirent *)malloc(entrysize);
            if ((*namelist)[i] == NULL) return -1;
            memcpy((*namelist)[i], entry, entrysize);
            i++;
        }
    }
    if (closedir(d)) return -1;
    if (i == 0) return -1;
    if (compar != NULL)
        qsort((void *)(*namelist), (size_t)i, sizeof(struct dirent *),
              (int(*)(const void*, const void*))compar);
    
    return i;
}
#endif

extern char window_xml_file[MAX_BUF];  /* Name of the .glade layout in use. */

GtkWidget *config_window, *config_spinbutton_cwindow, *config_button_echo,
    *config_button_fasttcp, *config_button_timestamp, *config_button_grad_color, *config_button_foodbeep,
    *config_button_sound, *config_button_cache, *config_button_download,
    *config_button_fog, *config_spinbutton_iconscale, *config_spinbutton_mapscale,
    *config_spinbutton_mapwidth, *config_spinbutton_mapheight,
    *config_button_smoothing, *config_combobox_displaymode,
    *config_combobox_faceset, *config_combobox_lighting,
    *config_combobox_theme, *config_combobox_glade;

/* This is the string names that correspond to the numberic id's in client.h */

static char *theme = "Standard";
static char *themedir = "themes";
static char *gladedir = "glade-gtk2";

static const char * const display_modes[] = {"Pixmap", "SDL", "OpenGL"};

/**
 * Sets up player-specific client and layout rc files and handles loading of a
 * client theme if one is selected.  First, the player-specific rc files are
 * added to the GTK rc default files list.  ${HOME}/.crossfire/gtkrc is added
 * first.  All client sessions are affected by this rc file if it exists.
 * Next, ${HOME}/.crossfire/[layout].gtkrc is added, where [layout] is the
 * name of the .glade file that is loaded.  IE. If gtk-v2.glade is loaded,
 * [layout] is "gtk-v2".  This sets up the possibility for a player to make a
 * layout-specific rc file.  Finally, if the client theme is not "None", the
 * client theme file is added.  In most cases, the player-specific files are
 * probably not going to exist, so the theme system will continue to work the
 * way it always has.  The player will have to "do something" to get the extra
 * functionality.  At some point, conceptually the client itself could be
 * enhanced to allow it to save some basic settings to either or both of the
 * player-specific rc files.
 *
 * @param reload
 * If true, user has changed theme after initial startup.  In this mode, we
 * need to call the routines that store away private theme data.  When program
 * is starting up, this is false, because all the widgets haven't been realized
 * yet, and the initialize routines will get the theme data at that time.
 */
static char **default_files=NULL;
void init_theme()
{
    char path[MAX_BUF];
    char xml_basename[MAX_BUF];
    char **tmp;
    char *cp;
    int i;

    /*
     * The GTK man page says copy of this data should be made, so do that.
     */
    tmp = gtk_rc_get_default_files();
    i = 0;
    while (tmp[i]) {
        i++;
    }
    /*
     * Add two more GTK rc files that may be used by a player to customize
     * the client appearance in general, or to customize the appearance
     * of a specific .glade layout.  Allocate pointers to the local copy
     * of the entire list.
     */
    i += 2;
    default_files = malloc(sizeof(char*) * (i+1));
    /*
     * Copy in GTK's default list which probably contains system paths
     * like <SYSCONFDIR>/gtk-2.0/gtkrc and user-specific files like
     * ${HOME}/.gtkrc, or even LANGuage-specific ones like
     * ${HOME}/.gtkrc.en, etc.
     */
    i=0;
    while (tmp[i]) {
        default_files[i] = strdup(tmp[i]);
        i++;
    }
    /*
     * Add a player-specific gtkrc to the list of default rc files.  This
     * file is probably reserved for player use, though in all liklihood
     * will not get used that much.  Still, it makes it easy for someone
     * to make their own theme without having to have access to the
     * system-wide theme folder.  This is the lowest priority client rc
     * file as either a <layout>.gtkrc file or a client-configured theme
     * settings can over-ride it.
     */
    snprintf(path, sizeof(path), "%s/.crossfire/gtkrc", getenv("HOME"));
    default_files[i] = strdup(path);
    i++;
    /*
     * Add a UI layout-specific rc file to the list of default list.  It
     * seems reasonable to allow client code to have access to this file
     * to make some basic changes to fonts, via a graphical interface.
     * Truncate window_xml_file to remove a .extension if one exists, so
     * that the window positions file can be created with a .gtkrc suffix.
     * This is a mid-priority client rc file as its settings supersede the
     * client gtkrc file, but are overridden by a client-configured theme.
     */
    strncpy(xml_basename, window_xml_file, MAX_BUF);
    cp = strrchr(xml_basename, '.');
    if (cp)
        cp[0] = 0;
    snprintf(path, sizeof(path),
             "%s/.crossfire/%s.gtkrc", getenv("HOME"), xml_basename);
    CONVERT_FILESPEC_TO_OS_FORMAT(path);
    default_files[i] = strdup(path);
    i++;
    /*
     * Mark the end of the list of default rc files.
     */
    default_files[i] = NULL;
}

void load_theme(int reload) {
    char path[MAX_BUF];
    int i;

    /*
     * Whether or not this is default and initial run, we want to register
     * the modified rc search path list, so GTK needs to get the changes.
     * It is necessary to reset the the list each time through here each
     * theme change grows the list.  Only one theme should be in the list
     * at a time.
     */
    gtk_rc_set_default_files(default_files);

    /*
     * If a client-configured theme has been selected (something other than
     * "None"), then add it to the list of GTK rc files to process.  Since
     * this file is added last, it takes priority over both the gtkrc and
     * <layout>.gtkrc files.  Remember, strcmp returns zero on a match, and
     * a theme file should not be registered if "None" is selected.
     */
    if (strcmp(theme, "None")) {
        snprintf(path, MAX_BUF, "%s/%s/%s", CF_DATADIR, themedir, theme);
        /*
         * Check for existence of the client theme file.  Unfortunately, at
         * initial run time, the window may not be realized yet, so the
         * message cannot be sent to the user directly.  It doesn't hurt to
         * add the path even if the file isn't there, but the player might
         * still want to know something is wrong since they picked a theme.
         */
        if (access(path, R_OK) == -1)
            LOG(LOG_ERROR, "config.c::load_theme",
                "Unable to find theme file %s", path);

        gtk_rc_add_default_file(path);
    }

    /*
     * Require GTK to reparse and rebind all the widget data.
     */
    gtk_rc_reparse_all_for_settings(
                                    gtk_settings_get_for_screen(gdk_screen_get_default()), TRUE);
    gtk_rc_reset_styles(
                        gtk_settings_get_for_screen(gdk_screen_get_default()));
    /*
     * Call client functions to reparse the custom widgets it controls.
     */
    info_get_styles();
    inventory_get_styles();
    stats_get_styles();
    spell_get_styles();
    update_spell_information();
    /*
     * Set inv_updated to force a redraw - otherwise it will not
     * necessarily bind the lists with the new widgets.
     */
    cpl.below->inv_updated = 1;
    cpl.ob->inv_updated = 1;
    draw_lists();
    draw_stats(TRUE);
    draw_message_window(TRUE);
}

/**
 * This function processes the user saved settings file and establishes the
 * configuration of the client.
 */
void load_defaults(void)
{
    char path[MAX_BUF],inbuf[MAX_BUF],*cp;
    FILE *fp;
    int i, val;

    /* Copy over the want values to use values now */
    for (i=0; i<CONFIG_NUMS; i++) {
        use_config[i] = want_config[i];
    }

    snprintf(path, sizeof(path), "%s/.crossfire/gdefaults2", getenv("HOME"));
    CONVERT_FILESPEC_TO_OS_FORMAT(path);
    if ((fp=fopen(path,"r"))==NULL) return;
    while (fgets(inbuf, MAX_BUF-1, fp)) {
        inbuf[MAX_BUF-1]='\0';
        inbuf[strlen(inbuf)-1]='\0';    /* kill newline */

        if (inbuf[0]=='#') continue;
        /* Skip any setting line that does not contain a colon character */
        if (!(cp=strchr(inbuf,':'))) continue;
        *cp='\0';
        cp+=2;      /* colon, space, then value */

        val = -1;
        if (isdigit(*cp)) val=atoi(cp);
        else if (!strcmp(cp,"True")) val = TRUE;
        else if (!strcmp(cp,"False")) val = FALSE;

        for (i=1; i<CONFIG_NUMS; i++) {
            if (!strcmp(config_names[i], inbuf)) {
                if (val == -1) {
                    LOG(LOG_WARNING, "config.c::load_defaults",
                        "Invalid value/line: %s: %s", inbuf, cp);
                } else {
                    want_config[i] = val;
                }
                break;  /* Found a match - won't find another */
            }
        }
        /* We found a match in the loop above, so do not do anything more */
        if (i < CONFIG_NUMS) continue;

        /*
         * Legacy - now use the map_width and map_height values Don't do sanity
         * checking - that will be done below
         */
        if (!strcmp(inbuf,"mapsize")) {
            if (sscanf(cp,"%hdx%hd", &want_config[CONFIG_MAPWIDTH], &want_config[CONFIG_MAPHEIGHT])!=2) {
                LOG(LOG_WARNING, "config.c::load_defaults",
                    "Malformed mapsize option in gdefaults2.  Ignoring");
            }
        }
        else if (!strcmp(inbuf, "server")) {
            server = strdup_local(cp);  /* memory leak ! */
            continue;
        }
        else if (!strcmp(inbuf, "theme")) {
            theme = strdup_local(cp);   /* memory leak ! */
            continue;
        }
        else if (!strcmp(inbuf, "window_layout")) {
            strncpy(window_xml_file, cp, MAX_BUF-1);
            continue;
        }
        else if (!strcmp(inbuf, "nopopups")) {
            /* Changed name from nopopups to popups, so inverse value */
            want_config[CONFIG_POPUPS] = !val;
            continue;
        }
        else if (!strcmp(inbuf, "nosplash")) {
            want_config[CONFIG_SPLASH] = !val;
            continue;
        }
        else if (!strcmp(inbuf, "splash")) {
            want_config[CONFIG_SPLASH] = val;
            continue;
        }
        else if (!strcmp(inbuf, "faceset")) {
            face_info.want_faceset = strdup_local(cp);  /* memory leak ! */
            continue;
        }
        /* legacy, as this is now just saved as 'lighting' */
        else if (!strcmp(inbuf, "per_tile_lighting")) {
            if (val) want_config[CONFIG_LIGHTING] = CFG_LT_TILE;
        }
        else if (!strcmp(inbuf, "per_pixel_lighting")) {
            if (val) want_config[CONFIG_LIGHTING] = CFG_LT_PIXEL;
        }
        else if (!strcmp(inbuf, "resists")) {
            if (val) want_config[CONFIG_RESISTS] = val;
        }
        else if (!strcmp(inbuf, "sdl")) {
            if (val) want_config[CONFIG_DISPLAYMODE] = CFG_DM_SDL;
        }
        else LOG(LOG_WARNING, "config.c::load_defaults",
                 "Unknown line in gdefaults2: %s %s", inbuf, cp);
    }
    fclose(fp);
    /*
     * Make sure some of the values entered are sane - since a user can edit
     * the defaults file directly, they could put bogus values in
     */
    if (want_config[CONFIG_ICONSCALE]< 25 || want_config[CONFIG_ICONSCALE]>200) {
        LOG(LOG_WARNING, "config.c::load_defaults",
            "Ignoring iconscale value read from gdefaults2 file.\n"
            "Invalid iconscale range (%d), valid range for -iconscale "
            "is 25 through 200", want_config[CONFIG_ICONSCALE]);
        want_config[CONFIG_ICONSCALE] = use_config[CONFIG_ICONSCALE];
    }
    if (want_config[CONFIG_MAPSCALE]< 25 || want_config[CONFIG_MAPSCALE]>200) {
        LOG(LOG_WARNING, "config.c::load_defaults",
            "ignoring mapscale value read for gdefaults2 file.\n"
            "Invalid mapscale range (%d), valid range for -iconscale "
            "is 25 through 200", want_config[CONFIG_MAPSCALE]);
        want_config[CONFIG_MAPSCALE] = use_config[CONFIG_MAPSCALE];
    }
    if (!want_config[CONFIG_LIGHTING]) {
        LOG(LOG_WARNING, "config.c::load_defaults",
            "No lighting mechanism selected - will not use darkness code");
        want_config[CONFIG_DARKNESS] = FALSE;
    }
    if (want_config[CONFIG_RESISTS] > 2) {
        LOG(LOG_WARNING, "config.c::load_defaults",
            "ignoring resists display value read for gdafaults file.\n"
            "Invalid value (%d), must be one value of 0, 1 or 2.",
            want_config[CONFIG_RESISTS]);
        want_config[CONFIG_RESISTS] = 0;
    }

    /* Make sure the map size os OK */
    if (want_config[CONFIG_MAPWIDTH] < 9 || want_config[CONFIG_MAPWIDTH] > MAP_MAX_SIZE) {
        LOG(LOG_WARNING, "config.c::load_defaults", "Invalid map width (%d) "
            "option in gdefaults2. Valid range is 9 to %d",
            want_config[CONFIG_MAPWIDTH], MAP_MAX_SIZE);
        want_config[CONFIG_MAPWIDTH] = use_config[CONFIG_MAPWIDTH];
    }
    if (want_config[CONFIG_MAPHEIGHT] < 9 || want_config[CONFIG_MAPHEIGHT] > MAP_MAX_SIZE) {
        LOG(LOG_WARNING, "config.c::load_defaults", "Invalid map height (%d) "
            "option in gdefaults2. Valid range is 9 to %d",
            want_config[CONFIG_MAPHEIGHT], MAP_MAX_SIZE);
        want_config[CONFIG_MAPHEIGHT] = use_config[CONFIG_MAPHEIGHT];
    }

#if !defined(HAVE_OPENGL)
    if (want_config[CONFIG_DISPLAYMODE] == CFG_DM_OPENGL) {
        want_config[CONFIG_DISPLAYMODE] = CFG_DM_PIXMAP;
        LOG(LOG_ERROR, "config.c::load_defaults", "Display mode is set to OpenGL, but client "
            "is not compiled with OpenGL support.  Reverting to pixmap mode.");
    }
#endif

#if !defined(HAVE_SDL)
    if (want_config[CONFIG_DISPLAYMODE] == CFG_DM_SDL) {
        want_config[CONFIG_DISPLAYMODE] = CFG_DM_PIXMAP;
        LOG(LOG_ERROR, "config.c::load_defaults", "Display mode is set to SDL, but client "
            "is not compiled with SDL support.  Reverting to pixmap mode.");
    }
#endif


    /* Now copy over the values just loaded */
    for (i=0; i<CONFIG_NUMS; i++) {
        use_config[i] = want_config[i];
    }

    image_size = DEFAULT_IMAGE_SIZE * use_config[CONFIG_ICONSCALE] / 100;
    map_image_size = DEFAULT_IMAGE_SIZE * use_config[CONFIG_MAPSCALE] / 100;
    map_image_half_size = DEFAULT_IMAGE_SIZE * use_config[CONFIG_MAPSCALE] / 200;
    /*inv_list.show_icon = use_config[CONFIG_SHOWICON];*/
}

/**
 * This function saves user settings chosen using the configuration popup
 * dialog.
 */
void save_defaults(void)
{
    char path[MAX_BUF],buf[MAX_BUF];
    FILE *fp;
    int i;

    snprintf(path, sizeof(path), "%s/.crossfire/gdefaults2", getenv("HOME"));
    CONVERT_FILESPEC_TO_OS_FORMAT(path);
    if (make_path_to_file(path)==-1) {
        LOG(LOG_ERROR, "config.c::save_defaults","Could not create %s", path);
        return;
    }
    if ((fp=fopen(path,"w"))==NULL) {
        LOG(LOG_ERROR, "config.c::save_defaults", "Could not open %s", path);
        return;
    }
    fprintf(fp,"# crossfire-client-gtk2 automatically generates this file.\n");
    fprintf(fp,"# Manual editing is allowed, but the client may be a bit\n");
    fprintf(fp,"# finicky about the keys and values.  Comparisons are case\n");
    fprintf(fp,"# sensitive.  'True' and 'False' are the proper case, but\n");
    fprintf(fp,"# have been replaced with 1 and 0 respectively.\n#\n");
    fprintf(fp,"server: %s\n", server);
    fprintf(fp,"theme: %s\n", theme);
    fprintf(fp,"faceset: %s\n", face_info.want_faceset);
    fprintf(fp,"window_layout: %s\n", window_xml_file);
    /*
     * This isn't quite as good as before, as instead of saving things as
     * 'True' or 'False', it is just 1 or 0.  However, for the most part, the
     * user isn't going to be editing the file directly.
     */
    for (i=1; i < CONFIG_NUMS; i++) {
        fprintf(fp,"%s: %d\n", config_names[i], want_config[i]);
    }

    fclose(fp);
    snprintf(buf, sizeof(buf), "Defaults saved to %s",path);
    draw_ext_info(NDI_BLUE, MSG_TYPE_CLIENT, MSG_TYPE_CLIENT_CONFIG, buf);
}

/**
 * Set up the configuration popup dialog using libglade and the XML layout file
 * loaded from main.c and connect various signals associated with this dialog.
 */
void config_init(GtkWidget *window_root)
{
    static int has_init=0;
    GladeXML *xml_tree;
    GtkWidget *widget;
    int count, i;

    has_init=1;

    config_window = glade_xml_get_widget(dialog_xml, "config_window");
    xml_tree = glade_get_widget_tree(GTK_WIDGET(config_window));

    config_spinbutton_cwindow =
        glade_xml_get_widget(xml_tree, "config_spinbutton_cwindow");
    config_button_echo =
        glade_xml_get_widget(xml_tree, "config_button_echo");
    config_button_fasttcp =
        glade_xml_get_widget(xml_tree, "config_button_fasttcp");
    config_button_timestamp =
        glade_xml_get_widget(xml_tree, "config_button_timestamp");
    config_button_grad_color =
        glade_xml_get_widget(xml_tree, "config_button_grad_color");
    config_button_foodbeep =
        glade_xml_get_widget(xml_tree, "config_button_foodbeep");
    config_button_sound =
        glade_xml_get_widget(xml_tree, "config_button_sound");
    config_button_cache =
        glade_xml_get_widget(xml_tree, "config_button_cache");
    config_button_download =
        glade_xml_get_widget(xml_tree, "config_button_download");
    config_button_fog =
        glade_xml_get_widget(xml_tree, "config_button_fog");
    config_button_smoothing =
        glade_xml_get_widget(xml_tree, "config_button_smoothing");
    config_spinbutton_iconscale =
        glade_xml_get_widget(xml_tree, "config_spinbutton_iconscale");
    config_spinbutton_mapscale =
        glade_xml_get_widget(xml_tree, "config_spinbutton_mapscale");
    config_spinbutton_mapwidth =
        glade_xml_get_widget(xml_tree, "config_spinbutton_mapwidth");
    config_spinbutton_mapheight =
        glade_xml_get_widget(xml_tree, "config_spinbutton_mapheight");
    config_combobox_displaymode =
        glade_xml_get_widget(xml_tree, "config_combobox_displaymode");
    config_combobox_faceset =
        glade_xml_get_widget(xml_tree, "config_combobox_faceset");
    config_combobox_lighting =
        glade_xml_get_widget(xml_tree, "config_combobox_lighting");
    config_combobox_theme =
        glade_xml_get_widget(xml_tree, "config_combobox_theme");
    config_combobox_glade =
        glade_xml_get_widget(xml_tree, "config_combobox_glade");

    g_signal_connect((gpointer) config_window, "delete_event",
        G_CALLBACK(gtk_widget_hide_on_delete), NULL);

    widget = glade_xml_get_widget(xml_tree, "config_button_save");
    g_signal_connect((gpointer) widget, "clicked",
        G_CALLBACK(on_config_button_save_clicked), NULL);

    widget = glade_xml_get_widget(xml_tree, "config_button_apply");
    g_signal_connect((gpointer) widget, "clicked",
        G_CALLBACK(on_config_button_apply_clicked), NULL);

    widget = glade_xml_get_widget(xml_tree, "config_button_close");
    g_signal_connect((gpointer) widget, "clicked",
        G_CALLBACK(on_config_button_close_clicked), NULL);

    /*
     * Display mode combo box setup.  First, remove all entries, then populate
     * with what options are available based on what was compiled in.
     */
    count =  gtk_tree_model_iter_n_children(
                    gtk_combo_box_get_model(GTK_COMBO_BOX(config_combobox_displaymode)), NULL);
    for (i=0; i < count; i++)
        gtk_combo_box_remove_text(GTK_COMBO_BOX(config_combobox_displaymode), 0);

#ifdef HAVE_OPENGL
        gtk_combo_box_append_text(GTK_COMBO_BOX(config_combobox_displaymode),  "OpenGL");
#endif

#ifdef HAVE_SDL
        gtk_combo_box_append_text(GTK_COMBO_BOX(config_combobox_displaymode),  "SDL");
#endif
        gtk_combo_box_append_text(GTK_COMBO_BOX(config_combobox_displaymode),  "Pixmap");

}

/**
 * This function is used by scandir below to get only the directory entries
 * needed.  In the case of themes, simply skip dot files.
 *
 * @param d
 * the dirent entry from scandir.
 *
 * function returns 1 if the file is a valid theme file name, 0 for files
 * that are not.
 */
static int scandir_theme_filter(const struct dirent *d)
{
    if (d->d_name[0] == '.') return 0;
    return 1;
}

/**
 * This function is used by scandir below to get only the directory entries
 * needed.  In the case of glade files, skip all files that do not end with
 * ".glade" and the default glade XML file that defines auxilliary dialogs.
 *
 * @param d
 * the dirent entry from scandir.
 *
 * function returns 1 if the file is a valid glade XML file name, 0 for files
 * that are not.
 */
static int scandir_glade_filter(const struct dirent *d)
{
    char *token = NULL;
    char *extok = NULL;
    char delim[] = ".";
    char exten[] = "glade";
    char parse[MAX_BUF] = "";

    strncpy(parse, d->d_name, MAX_BUF);
    token = strtok(parse, delim);
    while (token) {
        extok = token;
        token = strtok(NULL, delim);
    }
    if (extok && strncmp(exten, extok, strlen(exten)) == 0) {
        if (strncmp(parse, DIALOG_XML_FILENAME, strlen(parse)) == 0)
            return 0;
        return 1;
    }
    return 0;
}

/**
 * Load a control with entries created from a directory that has files suited
 * to a particular function.  IE. themes, and UI layout files.
 *
 * This sees what themes or layouts are in the directory and puts them in the
 * pulldown menu for the selection box.  The presumption is made that these
 * files won't change during run time, so we only need to do this once.
 *
 * @param combobox
 * a glade combobox widget to be filled with filenames.
 *
 * @param active
 * a pointer to a string that is the active combobox item.
 *
 * @param want_none
 * 1 if a "None" entry is added to the filename list; 0 if not.
 *
 * @param subdir
 * the subdirectory that contains filenames to add to the combobox list.
 *
 * @param scandir_filter
 * a pointer to a function that is called by scandir() to filter filenames to
 * add to the combobox.  The function returns 1 if the filename is valid for
 * addition to the combobox list.
 *
 */
static void fill_combobox_from_datadir(GtkWidget *combobox, char *active,
    uint64 want_none, char *subdir, int (*scandir_filter) ())
{
    int             count, i;
    GtkTreeModel    *model;
    gchar           *buf;
    GtkTreeIter     iter;

    model = gtk_combo_box_get_model(GTK_COMBO_BOX(combobox));
    count =  gtk_tree_model_iter_n_children(model, NULL);
    /*
     * If count is 0, the combo box control has not been initialized yet, so
     * fill it with the appropriate selections now.
     */
    if (count == 0) {
        char path[MAX_BUF];
        struct dirent **files;
        int done_none=0;

        snprintf(path, MAX_BUF, "%s/%s", CF_DATADIR, subdir);

        count = scandir(path, &files, *scandir_filter, alphasort);
        LOG(LOG_DEBUG, "config.c::fill_combobox_from_datadir",
            "found %d files in %s\n", count, path);

        for (i=0; i<count; i++) {
            /*
             * If a 'None' entry is desired, and if an entry that falls after
             * 'None' is found, and, if 'None' has not already been added,
             * insert it now.
             */
            if (!done_none && want_none &&
                strcmp(files[i]->d_name, "None") > 0) {
                    gtk_combo_box_append_text(GTK_COMBO_BOX(combobox), "None");
                    done_none=1;
            }

            gtk_combo_box_append_text(GTK_COMBO_BOX(combobox), files[i]->d_name);
        }
        /* Set this for below */
        count =  gtk_tree_model_iter_n_children(model, NULL);
    }
    /*
     * The block belows causes the matching combobox item to be selected.  Set
     * it irregardless of whether this is the first time this is run or not.
     */
    for (i=0; i < count; i++) {
        if (!gtk_tree_model_iter_nth_child(model, &iter, NULL, i)) {
            LOG(LOG_ERROR, "config.c::fill_combobox_from_datadir",
                "Unable to get combo box iter\n");
            break;
        }
        gtk_tree_model_get(model, &iter, 0, &buf, -1);
        if (!strcasecmp(active, buf)) {
            gtk_combo_box_set_active(GTK_COMBO_BOX(combobox), i);
            g_free(buf);
            break;
        }
        g_free(buf);
    }
}

/*
 * Setup_config_window sets the buttons, combos, etc, to the state that matches
 * the want_config[] values.
 */
static void setup_config_window(void)
{
    int count, i;
    GtkTreeModel    *model;
    gchar   *buf;
    GtkTreeIter iter;

    gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(config_button_echo),
                                 want_config[CONFIG_ECHO]);

    gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(config_button_fasttcp),
                                 want_config[CONFIG_FASTTCP]);

    gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(config_button_timestamp),
                                 want_config[CONFIG_TIMESTAMP]);

    gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(config_button_grad_color),
                                 want_config[CONFIG_GRAD_COLOR]);

    gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(config_button_foodbeep),
                                 want_config[CONFIG_FOODBEEP]);

    gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(config_button_sound),
                                 want_config[CONFIG_SOUND]);

    gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(config_button_cache),
                                 want_config[CONFIG_CACHE]);

    gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(config_button_download),
                                 want_config[CONFIG_DOWNLOAD]);

    gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(config_button_fog),
                                 want_config[CONFIG_FOGWAR]);

    gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(config_button_smoothing),
                                 want_config[CONFIG_SMOOTH]);

    gtk_spin_button_set_value(GTK_SPIN_BUTTON(config_spinbutton_cwindow),
                              (float)want_config[CONFIG_CWINDOW]);

    gtk_spin_button_set_value(GTK_SPIN_BUTTON(config_spinbutton_iconscale),
                              (float)want_config[CONFIG_ICONSCALE]);

    gtk_spin_button_set_value(GTK_SPIN_BUTTON(config_spinbutton_mapscale),
                              (float)want_config[CONFIG_MAPSCALE]);

    gtk_spin_button_set_value(GTK_SPIN_BUTTON(config_spinbutton_mapwidth),
                              (float)want_config[CONFIG_MAPWIDTH]);

    gtk_spin_button_set_value(GTK_SPIN_BUTTON(config_spinbutton_mapheight),
                              (float)want_config[CONFIG_MAPHEIGHT]);
    /*
     * Face set combo box setup.
     * Remove all the entries currently in the combo box
     */
    model = gtk_combo_box_get_model(GTK_COMBO_BOX(config_combobox_faceset));
    count =  gtk_tree_model_iter_n_children(model, NULL);

    for (i=0; i < count; i++)
        gtk_combo_box_remove_text(GTK_COMBO_BOX(config_combobox_faceset), 0);

    /* If we have real faceset info from the server, use it */
    if (face_info.have_faceset_info) {
        for (i=0; i<MAX_FACE_SETS; i++)
            if (face_info.facesets[i].fullname)
                gtk_combo_box_append_text(GTK_COMBO_BOX(config_combobox_faceset),
                          face_info.facesets[i].fullname);
    } else {
        gtk_combo_box_append_text(GTK_COMBO_BOX(config_combobox_faceset),  "Standard");
        gtk_combo_box_append_text(GTK_COMBO_BOX(config_combobox_faceset),  "Classic");
    }
    count =  gtk_tree_model_iter_n_children(model, NULL);

    for (i=0; i < count; i++) {
        if (!gtk_tree_model_iter_nth_child(model, &iter, NULL, i)) {
            LOG(LOG_ERROR, "config.c::setup_config_window",
                "Unable to get faceset iter\n");
            break;
        }
        gtk_tree_model_get(model, &iter, 0, &buf, -1);

        if (face_info.want_faceset && !strcasecmp(face_info.want_faceset, buf)) {
            gtk_combo_box_set_active(GTK_COMBO_BOX(config_combobox_faceset), i);
            g_free(buf);
            break;
        }
        g_free(buf);
    }

    if (sizeof(display_modes) < want_config[CONFIG_DISPLAYMODE]) {
        LOG(LOG_ERROR, "config.c::setup_config_window",
            "Player display mode not in display_modes range\n");
    } else {
        /*
         * We want to set up the boxes to match current settings for things
         * like displaymode.
         */
        model = gtk_combo_box_get_model(GTK_COMBO_BOX(config_combobox_displaymode));
        count =  gtk_tree_model_iter_n_children(model, NULL);
        for (i=0; i < count; i++) {
            if (!gtk_tree_model_iter_nth_child(model, &iter, NULL, i)) {
                LOG(LOG_ERROR, "config.c::setup_config_window",
                    "Unable to get faceset iter\n");
                break;
            }
            gtk_tree_model_get(model, &iter, 0, &buf, -1);
            if (!strcasecmp(display_modes[want_config[CONFIG_DISPLAYMODE]], buf)) {
                gtk_combo_box_set_active(GTK_COMBO_BOX(config_combobox_displaymode), i);
                g_free(buf);
                break;
            }
            g_free(buf);
        }
    }

    /*
     * We want to set up the boxes to match current settings for things like
     * lighting.  A bit of a hack to hardcode the strings below.
     */
    model = gtk_combo_box_get_model(GTK_COMBO_BOX(config_combobox_lighting));
    count =  gtk_tree_model_iter_n_children(model, NULL);
    for (i=0; i < count; i++) {
        if (!gtk_tree_model_iter_nth_child(model, &iter, NULL, i)) {
            LOG(LOG_ERROR, "config.c::setup_config_window",
                "Unable to get lighting iter\n");
            break;
        }
        gtk_tree_model_get(model, &iter, 0, &buf, -1);
        if ((want_config[CONFIG_LIGHTING] == CFG_LT_TILE && !strcasecmp(buf, "Per Tile")) ||
            (want_config[CONFIG_LIGHTING] == CFG_LT_PIXEL && !strcasecmp(buf, "Fast Per Pixel")) ||
            (want_config[CONFIG_LIGHTING] == CFG_LT_PIXEL_BEST && !strcasecmp(buf, "Best Per Pixel")) ||
            (want_config[CONFIG_LIGHTING] == CFG_LT_NONE && !strcasecmp(buf, "None"))) {
                gtk_combo_box_set_active(GTK_COMBO_BOX(config_combobox_lighting), i);
                g_free(buf);
                break;
        }
        g_free(buf);
    }
    /*
     * Use the filenames of files found in the themes subdirectory of the
     * crossfire-client data directory as the selectable items in a combo box
     * selector control.  Specify the current default, and that a "None" entry
     * needs to be added also.
     */
    fill_combobox_from_datadir(config_combobox_theme, theme, 1, themedir,
        &scandir_theme_filter);
    /*
     * Use the filenames of files found in the glade-gtk subdirectory of the
     * crossfire-client data directory as the selectable items in a combo box
     * selector control.  Specify the current default, and that no "None" entry
     * is desired.
     */
    fill_combobox_from_datadir(config_combobox_glade, window_xml_file, 0,
        gladedir, &scandir_glade_filter);
}

#define IS_DIFFERENT(TYPE) (want_config[TYPE] != use_config[TYPE])

/**
 * This is basically the opposite of setup_config_window() above - instead of
 * setting the display state appropriately, we read the display state and
 * update the want_config values.
 */
static void read_config_window(void)
{
    gchar   *buf;

    want_config[CONFIG_ECHO] = gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(config_button_echo));
    want_config[CONFIG_FASTTCP] = gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(config_button_fasttcp));
    want_config[CONFIG_TIMESTAMP] = gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(config_button_timestamp));
    want_config[CONFIG_GRAD_COLOR] = gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(config_button_grad_color));
    want_config[CONFIG_FOODBEEP] = gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(config_button_foodbeep));
    want_config[CONFIG_SOUND] = gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(config_button_sound));
    want_config[CONFIG_CACHE] = gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(config_button_cache));
    want_config[CONFIG_DOWNLOAD] = gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(config_button_download));
    want_config[CONFIG_FOGWAR] = gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(config_button_fog));
    want_config[CONFIG_SMOOTH] = gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(config_button_smoothing));
    want_config[CONFIG_CWINDOW] = gtk_spin_button_get_value_as_int(GTK_SPIN_BUTTON(config_spinbutton_cwindow));
    want_config[CONFIG_ICONSCALE] = gtk_spin_button_get_value_as_int(GTK_SPIN_BUTTON(config_spinbutton_iconscale));
    want_config[CONFIG_MAPSCALE] = gtk_spin_button_get_value_as_int(GTK_SPIN_BUTTON(config_spinbutton_mapscale));
    want_config[CONFIG_MAPWIDTH] = gtk_spin_button_get_value_as_int(GTK_SPIN_BUTTON(config_spinbutton_mapwidth));
    want_config[CONFIG_MAPHEIGHT] = gtk_spin_button_get_value_as_int(GTK_SPIN_BUTTON(config_spinbutton_mapheight));

    buf = gtk_combo_box_get_active_text(GTK_COMBO_BOX(config_combobox_faceset));

    /*
     * We may be able to eliminate the extra strdup/free, but I'm not 100% sure
     * that we are guaranteed that glib won't implement them through its
     * own/different malloc library.
     */
    if (buf) {
        free(face_info.want_faceset);
        face_info.want_faceset = strdup_local(buf);
        g_free(buf);
    }

    buf = gtk_combo_box_get_active_text(GTK_COMBO_BOX(config_combobox_displaymode));
    if (buf) {
        if (!strcasecmp(buf,"OpenGL")) want_config[CONFIG_DISPLAYMODE] = CFG_DM_OPENGL;
        else if (!strcasecmp(buf,"SDL")) want_config[CONFIG_DISPLAYMODE] = CFG_DM_SDL;
        else if (!strcasecmp(buf,"Pixmap")) want_config[CONFIG_DISPLAYMODE] = CFG_DM_PIXMAP;
        g_free(buf);
    }

    buf = gtk_combo_box_get_active_text(GTK_COMBO_BOX(config_combobox_lighting));
    if (buf) {
        if (!strcasecmp(buf, "Per Tile")) want_config[CONFIG_LIGHTING] = CFG_LT_TILE;
        else if (!strcasecmp(buf, "Fast Per Pixel")) want_config[CONFIG_LIGHTING] = CFG_LT_PIXEL;
        else if (!strcasecmp(buf, "Best Per Pixel")) want_config[CONFIG_LIGHTING] = CFG_LT_PIXEL_BEST;
        else if (!strcasecmp(buf, "None")) want_config[CONFIG_LIGHTING] = CFG_LT_NONE;
        if  (want_config[CONFIG_LIGHTING] != CFG_LT_NONE) {
            want_config[CONFIG_DARKNESS] = 1;
            use_config[CONFIG_DARKNESS] = 1;
        }

        g_free(buf);
    }
    /*
     * Load up the theme.  A problem is we don't really know if theme has been
     * allocated or is just pointing as a static string.  So we lose a few
     * bytes and just don't free the data.
     */
    buf = gtk_combo_box_get_active_text(GTK_COMBO_BOX(config_combobox_theme));
    if (buf) {
        if (strcmp(buf, theme)) {
            theme = buf;
            load_theme(TRUE);
        } else {
            g_free(buf);
        }
    } else {
        theme = "None";
    }
    /*
     * Load up the layout and set the combobox control to point to the loaded
     * default value.
     */
    buf = gtk_combo_box_get_active_text(GTK_COMBO_BOX(config_combobox_glade));
    if (buf && strcmp(buf, window_xml_file)) {
        strncpy(window_xml_file, buf, MAX_BUF);
    }
    /*
     * Some values can take effect right now, others not.  Code below handles
     * these cases - largely grabbed from gtk/config.c
     */
    if (IS_DIFFERENT(CONFIG_SOUND)) {
        int tmp;
        if (want_config[CONFIG_SOUND]) {
            tmp = init_sounds();
            if (csocket.fd)
                cs_print_string(csocket.fd, "setup sound %d", tmp >= 0);
        } else {
            if (csocket.fd)
                cs_print_string(csocket.fd, "setup sound 0");
        }
        use_config[CONFIG_SOUND] = want_config[CONFIG_SOUND];
    }
    if (IS_DIFFERENT(CONFIG_FASTTCP)) {
#ifdef TCP_NODELAY
#ifndef WIN32
        int q = want_config[CONFIG_FASTTCP];

        if (csocket.fd && setsockopt(csocket.fd, SOL_TCP, TCP_NODELAY, &q, sizeof(q)) == -1)
            perror("TCP_NODELAY");
#else
        int q = want_config[CONFIG_FASTTCP];

        if (csocket.fd && setsockopt(csocket.fd, SOL_TCP, TCP_NODELAY, ( const char* )&q, sizeof(q)) == -1)
            perror("TCP_NODELAY");
#endif
#endif
        use_config[CONFIG_FASTTCP] = want_config[CONFIG_FASTTCP];
    }

    if (IS_DIFFERENT(CONFIG_LIGHTING)) {
#ifdef HAVE_SDL
        if (use_config[CONFIG_DISPLAYMODE]==CFG_DM_SDL)
            /* This is done to make the 'lightmap' in the proper format */
            init_SDL( NULL, 1);
#endif
    }
    /*
     * Nothing to do, but we can switch immediately without problems.  do force
     * a redraw
     */
    if (IS_DIFFERENT(CONFIG_GRAD_COLOR)) {
        use_config[CONFIG_GRAD_COLOR] = want_config[CONFIG_GRAD_COLOR];
        draw_stats(TRUE);
    }
}

/**
 * Defines the behavior invoked when the configuration dialog save button is
 * pressed.
 *
 * @param button
 * @param user_data
 */
void
on_config_button_save_clicked          (GtkButton       *button,
                                        gpointer         user_data)
{
    read_config_window();
    save_defaults();
}

/**
 * Defines the behavior invoked when the configuration dialog apply button is
 * pressed.
 *
 * @param button
 * @param user_data
 */
void
on_config_button_apply_clicked         (GtkButton       *button,
                                        gpointer         user_data)
{
    read_config_window();
}

/**
 * Defines the behavior invoked when the configuration dialog close button is
 * pressed.
 *
 * @param button
 * @param user_data
 */
void
on_config_button_close_clicked         (GtkButton       *button,
                                        gpointer         user_data)
{
    gtk_widget_hide(config_window);
}

/**
 * Defines the behavior invoked when the configuration dialog is activated.
 *
 * @param menuitem
 * @param user_data
 */
void
on_configure_activate                 (GtkMenuItem     *menuitem,
                                        gpointer         user_data)
{
    gtk_widget_show(config_window);
    setup_config_window();
}

/**
 * Handles saving of the window positions when the Client | Save Window
 * Position menu item is executed.  All hpaned and vpaned widget settings have
 * the information required, and the code automatically works for Glade XML
 * layouts that follow the glade-2's default widget naming convention.
 */
void save_winpos(void)
{
    char savename[MAX_BUF];
    char buf[MAX_BUF];
    char *cp;
    FILE *save;
    int  x,y,w,h,wx,wy;
    extern  GtkWidget *window_root;
    GList *pane_list, *list_loop;
    GladeXML *xml_tree;
    GtkWidget *widget;

    /*
     * Truncate window_xml_file to remove a .extension if one exists, so that
     * the window positions file can be created with a different .extension.
     * this helps keep the length of the file name more reasonable.
     */
    strncpy(buf, window_xml_file, MAX_BUF);
    cp = strrchr(buf, '.');
    if (cp)
      cp[0] = 0;

    snprintf(savename, sizeof(savename), "%s/.crossfire/%s.pos", getenv("HOME"), buf);
    CONVERT_FILESPEC_TO_OS_FORMAT(savename);
    if (!(save = fopen(savename, "w"))) {
        snprintf(buf, sizeof(buf), "Cannot open %s - window positions not saved!", savename);
        draw_ext_info(NDI_RED, MSG_TYPE_CLIENT, MSG_TYPE_CLIENT_ERROR, buf);
        return;
    }
    get_window_coord(window_root, &x,&y, &wx,&wy,&w,&h);
    fprintf(save,"window_root: +%d+%dx%dx%d\n", wx, wy, w, h);

    xml_tree = glade_get_widget_tree(GTK_WIDGET(window_root));
    /*
     * Iterate through all widgets whose names begin with hpaned_* and vpaned_*
     * to save the widget name and the position of the pane divider.  Widgets
     * are dynamically found and processed so long as the .glade file designer
     * adheres to the naming conventions that Glade Designer uses.
     */
    pane_list = glade_xml_get_widget_prefix(xml_tree, "hpaned_");
    for (list_loop = pane_list; list_loop; list_loop = g_list_next(list_loop)) {
        widget = list_loop->data;
        fprintf(save, "%s: %d\n", glade_get_widget_name(widget),
            gtk_paned_get_position(GTK_PANED(widget)));
    }
    g_list_free(pane_list);

    pane_list = glade_xml_get_widget_prefix(xml_tree, "vpaned_");
    for (list_loop = pane_list; list_loop; list_loop = g_list_next(list_loop)) {
        widget = list_loop->data;
        fprintf(save, "%s: %d\n", glade_get_widget_name(widget),
            gtk_paned_get_position(GTK_PANED(widget)));
    }
    g_list_free(pane_list);

    fclose(save);
    snprintf(buf, sizeof(buf), "Window positions saved to %s", savename);
    draw_ext_info(NDI_BLUE, MSG_TYPE_CLIENT, MSG_TYPE_CLIENT_CONFIG, buf);
}

/**
 * Handles saving of the window positions when the Client | Save Window
 * Position menu item is activated.
 *
 * @param menuitem
 * @param user_data
 */
void
on_save_window_position_activate       (GtkMenuItem     *menuitem,
                                        gpointer         user_data)
{
    save_winpos();
    /*
     * The following prevents multiple saves per menu activation.
     */
    g_signal_stop_emission_by_name(GTK_OBJECT(menuitem), "activate");
}

/**
 * Retrieves saved window positions saved with the Client | Save Window
 * Position menu item.
 *
 * @param window_root The client's main window.
 */
void load_window_positions(GtkWidget *window_root)
{
    char loadname[MAX_BUF];
    char buf[MAX_BUF];
    char *cp;
    GladeXML *xml_tree;
    GtkWidget *widget;
    FILE    *load;

    /*
     * Truncate window_xml_file to remove a .extension if one exists, so that
     * the window positions file can be created with a different .extension.
     * this helps keep the length of the file name more reasonable.
     */
    strncpy(buf, window_xml_file, MAX_BUF);
    cp = strrchr(buf, '.');
    if (cp)
      cp[0] = 0;

    snprintf(loadname, sizeof(loadname), "%s/.crossfire/%s.pos", getenv("HOME"), buf);
    CONVERT_FILESPEC_TO_OS_FORMAT(loadname);
    if (!(load = fopen(loadname, "r"))) {
        snprintf(buf, sizeof(buf),
            "Cannot open %s: Using default window positions.", loadname);
        draw_ext_info(NDI_RED, MSG_TYPE_CLIENT, MSG_TYPE_CLIENT_CONFIG, buf);
        return;
    }
    else {
        snprintf(buf, sizeof(buf), "Loading window positions from %s", loadname);
        draw_ext_info(NDI_BLUE, MSG_TYPE_CLIENT, MSG_TYPE_CLIENT_CONFIG, buf);
    }

    xml_tree = glade_get_widget_tree(GTK_WIDGET (window_root));

    while(fgets(buf, MAX_BUF-1, load)!=NULL) {
        if ((cp=strchr(buf,':'))!=NULL) {
            *cp=0;
            cp++;
            while(isspace(*cp)) cp++;

            if (!strcmp(buf,"window_root")) {
                int x,y,w,h;

                if (sscanf(cp,"+%d+%dx%dx%d", &x, &y, &w, &h) == 4) {
                    gtk_window_set_default_size (GTK_WINDOW(window_root), w, h);
                    gtk_window_move(GTK_WINDOW(window_root), x, y);

                } else {
                    LOG(LOG_ERROR, "config.c::load_window_positions",
                        "Window size %s corrupt\n", cp);
                }
            } else if (strstr(buf,"paned_")) {
                /*
                 * The save names are a re-sizeable pane, but check to be sure
                 * it is a valid widget name if only to prevent sending a
                 * generic error to stderr if it does not exist in the current
                 * layout.
                 */
                widget = glade_xml_get_widget(xml_tree, buf);
                if (widget) {
                    gtk_paned_set_position(GTK_PANED(widget), atoi(cp));
                } else {
                    LOG(LOG_INFO, "config.c::load_window_positions", "%s in "
                        "%s not found in this UI layout.\n", buf, loadname);
                }
            } else {
                LOG(LOG_ERROR, "config.c::load_window_positions",
                    "Found unknown line %s in %s\n", buf, loadname);
            }
        }
    }
    fclose(load);
}
