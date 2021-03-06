/*
 * Crossfire -- cooperative multi-player graphical RPG and adventure game
 *
 * Copyright (c) 1999-2013 Mark Wedel and the Crossfire Development Team
 * Copyright (c) 1992 Frank Tore Johansen
 *
 * Crossfire is free software and comes with ABSOLUTELY NO WARRANTY. You are
 * welcome to redistribute it under certain conditions. For details, please
 * see COPYING and LICENSE.
 *
 * The authors can be reached via e-mail at <crossfire@metalforge.org>.
 */

/**
 * @file
 * Client startup and main loop.
 */

#include "client.h"

#include <errno.h>
#include <gtk/gtk.h>
#include <stdbool.h>

#ifdef WIN32
#include <windows.h>
#else
#include <signal.h>
#endif

#include "image.h"
#include "main.h"
#include "mapdata.h"
#include "metaserver.h"
#include "script.h"
#include "gtk2proto.h"

/* Sets up the basic colors. */
static const char *const colorname[NUM_COLORS] = {
    "Black",                /* 0  */
    "White",                /* 1  */
    "Navy",                 /* 2  */
    "Red",                  /* 3  */
    "Orange",               /* 4  */
    "DodgerBlue",           /* 5  */
    "DarkOrange2",          /* 6  */
    "SeaGreen",             /* 7  */
    "DarkSeaGreen",         /* 8  *//* Used for window background color */
    "Grey50",               /* 9  */
    "Sienna",               /* 10 */
    "Gold",                 /* 11 */
    "Khaki"                 /* 12 */
};

static gboolean updatekeycodes = FALSE;

/* TODO: Move these declarations to actual header files. */
extern int time_map_redraw;
extern int MINLOG;

/** Command line options, descriptions, and parameters. */
static GOptionEntry options[] = {
    { "cache", 0, 0, G_OPTION_ARG_NONE, &want_config[CONFIG_CACHE],
        "Cache images", NULL },
    { "prefetch", 0, 0, G_OPTION_ARG_NONE, &want_config[CONFIG_DOWNLOAD],
        "Download images before playing", NULL },
    { "faceset", 0, 0, G_OPTION_ARG_STRING, &face_info.want_faceset,
        "Use the given faceset (if available)", "FACESET" },

    { "sound_server", 0, 0, G_OPTION_ARG_FILENAME, &sound_server,
        "Path to the sound server", "PATH" },
    { "updatekeycodes", 0, 0, G_OPTION_ARG_NONE, &updatekeycodes,
        "Update the saved bindings for this keyboard", NULL },

    { "time-redraw", 't', 0, G_OPTION_ARG_NONE, &time_map_redraw,
        "Print map redraw times to stdout", NULL },
    { "verbose", 'v', 0, G_OPTION_ARG_INT, &MINLOG,
        "Set verbosity (0 is the most verbose)", "LEVEL" },
    { NULL }
};

char window_xml_file[MAX_BUF];

GdkColor root_color[NUM_COLORS];

GtkBuilder *dialog_xml, *window_xml;
GtkWidget *window_root, *magic_map;

/** Track whether the client has received a trick since the last redraw. */
bool next_tick = false;

#ifdef WIN32 /* Win32 scripting support */
#define PACKAGE_DATA_DIR "."

static int do_scriptout() {
    script_process(NULL);
    return (TRUE);
}
#endif /* WIN32 */

/**
 * Map, spell, and inventory maintenance.
 * @return TRUE
 */
static gboolean do_timeout(gpointer data) {
    if (cpl.showmagic) {
        if (gtk_notebook_get_current_page(GTK_NOTEBOOK(map_notebook)) !=
            MAGIC_MAP_PAGE) {
            // Stop flashing when the user switches back to the map window.
            cpl.showmagic = 0;
        } else {
            magic_map_flash_pos();
            cpl.showmagic ^= 2;
        }
    }
    if (cpl.spells_updated) {
        update_spell_information();
    }
    if (!tick) {
        inventory_tick();
        mapdata_animation();
    }

    return TRUE;
}

/**
 * Redraw the map. Do a full redraw if there are new images to show.
 */
static gboolean redraw(gpointer data) {
    // Do not redraw if no tick has arrived since the last redraw.
    if (next_tick) {
        next_tick = false;
    } else {
        // Sleep for 10 ms to prevent busy wait.
        g_usleep(10 * 1e3);
        return TRUE;
    }

    if (have_new_image) {
        if (cpl.container) {
            cpl.container->inv_updated = 1;
        }
        cpl.ob->inv_updated = 1;

        have_new_image = 0;
        draw_map(1);
        draw_lists();
    } else {
        draw_map(0);
    }
    return TRUE;
}

/**
 * X11 client doesn't care about this
 */
void client_tick(guint32 tick) {
    info_buffer_tick();
    inventory_tick();
    mapdata_animation();
    next_tick = true;
}
/**
 * Handles client shutdown.
 */
void on_window_destroy_event(GtkObject *object, gpointer user_data) {
#ifdef WIN32
    script_killall();
#endif

    LOG(LOG_DEBUG, "main.c::client_exit", "Exiting with return value 0.");
    exit(0);
}

/**
 * Callback from the event loop triggered when server input is available.
 */
static gboolean do_network(GObject *stream, gpointer data) {
    struct timeval timeout = {0, 0};
    fd_set tmp_read;
    int pollret;

    if (!client_is_connected()) {
        gtk_main_quit();
        return FALSE;
    }

    client_run();

    FD_ZERO(&tmp_read);
    script_fdset(&maxfd, &tmp_read);
    pollret = select(maxfd, &tmp_read, NULL, NULL, &timeout);
    if (pollret > 0) {
        script_process(&tmp_read);
    }

    draw_lists();
    return TRUE;
}

/**
 * Set up, enter, and exit event loop. Blocks until event loop returns.
 */
static void event_loop() {
    maxfd = csocket.fd + 1;
    guint source_redraw = g_idle_add(redraw, NULL);
    guint source_timeout = g_timeout_add(100, do_timeout, NULL);

#ifdef WIN32
    g_timeout_add(250, (GtkFunction) do_scriptout, NULL);
#endif

    GSource *net_source = client_get_source();
    g_source_set_callback(net_source, (GSourceFunc)do_network, NULL, NULL);
    g_source_attach(net_source, NULL);
    gtk_main();

    g_source_remove(source_redraw);
    g_source_remove(source_timeout);
    LOG(LOG_DEBUG, "event_loop", "Disconnected");
}

/**
 * parse_args: Parses command line options, and does variable initialization.
 * @param argc
 * @param argv
 */

/**
 * Parse command-line arguments and store settings in want_config.
 *
 * This function should be called after config_load().
 */
static void parse_args(int argc, char *argv[]) {
    GOptionContext *context = g_option_context_new("- Crossfire GTKv2 Client");
    GError *error = NULL;

    g_option_context_add_main_entries(context, options, NULL);
    g_option_context_add_group(context, gtk_get_option_group(TRUE));

    if (!g_option_context_parse(context, &argc, &argv, &error)) {
        g_print("%s\n", error->message);
        g_error_free(error);
        exit(EXIT_FAILURE);
    }

    g_option_context_free(context);

    /*
     * Move this after the parsing of command line options, since that can
     * change the default log level.
     */
    LOG(LOG_DEBUG, "Client Version", VERSION_INFO);
}

/**
 * Display an error message dialog. The dialog contains a multi-line, bolded
 * heading that includes the client version information, an error description,
 * and information relevant to the error condition.
 */
void error_dialog(char *error, char *message) {
    GtkWidget *dialog = gtk_message_dialog_new(
            NULL, GTK_DIALOG_DESTROY_WITH_PARENT,
            GTK_MESSAGE_ERROR, GTK_BUTTONS_CLOSE, "%s", error);
    gtk_window_set_title(GTK_WINDOW(dialog), "Crossfire Client");
    gtk_message_dialog_format_secondary_markup(
            GTK_MESSAGE_DIALOG(dialog), "%s", message);
    gtk_dialog_run(GTK_DIALOG(dialog));
    gtk_widget_destroy(dialog);
}

/**
 * This goes with the g_log_set_handler below in main().  I leave it here
 * since it may be useful - basically, it can prove handy to try and track
 * down error messages like:
 *
 * file gtklabel.c: line 1845: assertion `GTK_IS_LABEL (label)' failed
 *
 * In the debugger, you can set a breakpoint in this function, and then see
 * the stacktrace on what is trying to access a widget that isn't set or
 * otherwise having issues.
 */

void my_log_handler(const gchar *log_domain, GLogLevelFlags log_level,
                    const gchar *message, gpointer user_data) {
    g_usleep(1 * 1e6);
}

static void init_sockets() {
    /* Use the 'new' login method. */
    wantloginmethod = 2;

    csocket.inbuf.buf = g_malloc(MAXSOCKBUF);

#ifdef WIN32
    maxfd = 0; /* This is ignored on win32 platforms */

    /* This is required for sockets to be used under win32 */
    WORD Version = 0x0202;
    WSADATA wsaData;

    if (WSAStartup(Version, &wsaData) != 0) {
        LOG(LOG_CRITICAL, "main.c::main", "Could not load winsock!");
        exit(1);
    }
#else /* def WIN32 */
    signal(SIGPIPE, SIG_IGN);
#ifdef HAVE_SYSCONF
    maxfd = sysconf(_SC_OPEN_MAX);
#else
    maxfd = getdtablesize();
#endif
#endif /* def WIN32 */
}

/**
 * Load the UI from the given path. On success, store path in window_xml_file.
 */
static char *init_ui_layout(const char *name) {
    guint retval = gtk_builder_add_from_file(window_xml, name, NULL);
    if (retval > 0 && strlen(name) > 0) {
        strncpy(window_xml_file, name, sizeof(window_xml_file));
        return window_xml_file;
    } else {
        return NULL;
    }
}

static void init_ui() {
    GError *error = NULL;
    GdkGeometry geometry;
    int i;

    /* Load dialog windows using GtkBuilder. */
    dialog_xml = gtk_builder_new();
    if (!gtk_builder_add_from_file(dialog_xml, DIALOG_FILENAME, &error)) {
        error_dialog("Couldn't load UI dialogs.", error->message);
        g_warning("Couldn't load UI dialogs: %s", error->message);
        g_error_free(error);
        exit(EXIT_FAILURE);
    }

    /* Load main window using GtkBuilder. */
    window_xml = gtk_builder_new();
    if (init_ui_layout(window_xml_file) == NULL) {
        LOG(LOG_DEBUG, "init_ui_layout", "Using default layout");
        if (init_ui_layout(DEFAULT_UI) == NULL) {
            g_error("Could not load default layout!");
        }
    }

    /* Begin connecting signals for the root window. */
    window_root = GTK_WIDGET(gtk_builder_get_object(window_xml, "window_root"));
    if (window_root == NULL) {
        error_dialog("Could not load main window",
                "Check that your layout files are not corrupt.");
        exit(EXIT_FAILURE);
    }

    /* Request the window to receive focus in and out events */
    gtk_widget_add_events((gpointer) window_root, GDK_FOCUS_CHANGE_MASK);
    g_signal_connect((gpointer) window_root, "focus-out-event",
                     G_CALLBACK(focusoutfunc), NULL);

    g_signal_connect_swapped((gpointer) window_root, "key_press_event",
                             G_CALLBACK(keyfunc), GTK_OBJECT(window_root));
    g_signal_connect_swapped((gpointer) window_root, "key_release_event",
                             G_CALLBACK(keyrelfunc), GTK_OBJECT(window_root));
    g_signal_connect((gpointer) window_root, "destroy",
                     G_CALLBACK(on_window_destroy_event), NULL);

    /* Purely arbitrary min window size */
    geometry.min_width=640;
    geometry.min_height=480;

    gtk_window_set_geometry_hints(GTK_WINDOW(window_root), window_root,
                                  &geometry, GDK_HINT_MIN_SIZE);

    magic_map = GTK_WIDGET(gtk_builder_get_object(window_xml,
                "drawingarea_magic_map"));

    g_signal_connect((gpointer) magic_map, "expose_event",
                     G_CALLBACK(on_drawingarea_magic_map_expose_event), NULL);

    /* Set up colors before doing the other initialization functions */
    for (i = 0; i < NUM_COLORS; i++) {
        if (!gdk_color_parse(colorname[i], &root_color[i])) {
            fprintf(stderr, "gdk_color_parse failed (%s)\n", colorname[i]);
        }
        if (!gdk_colormap_alloc_color(gtk_widget_get_colormap(window_root),
                                      &root_color[i], FALSE, FALSE)) {
            fprintf(stderr, "gdk_color_alloc failed\n");
        }
    }

    inventory_init(window_root);
    info_init(window_root);
    keys_init(window_root);
    stats_init(window_root);
    config_init(window_root);
    pickup_init(window_root);
    msgctrl_init(window_root);
    init_create_character_window();
    metaserver_ui_init();

    load_window_positions(window_root);

    init_theme();
    load_theme(TRUE);
    init_menu_items();
}

/**
 * Main client entry point.
 */
int main(int argc, char *argv[]) {
#ifdef ENABLE_NLS
    bind_textdomain_codeset(GETTEXT_PACKAGE, "UTF-8");
    bindtextdomain(GETTEXT_PACKAGE, PACKAGE_LOCALE_DIR);
    textdomain(GETTEXT_PACKAGE);
#endif

    // Initialize GTK and client library.
    gtk_init(&argc, &argv);
    client_init();

    // Set defaults, load configuration, and parse arguments.
    snprintf(VERSION_INFO, MAX_BUF, "GTKv2 Client " FULL_VERSION);
    config_load();
    parse_args(argc, argv);
    config_check();

    // Initialize UI, sockets, and sound server.
    init_ui();
    init_sockets();

    if (init_sounds() == -1) {
        use_config[CONFIG_SOUND] = FALSE;
    } else {
        use_config[CONFIG_SOUND] = TRUE;
    }

    /* Load cached pixmaps. */
    init_image_cache_data();

    /* Show main client window as late as possible. */
    gtk_widget_show(window_root);
    map_init(window_root);

    while (true) {
        clear_stat_mapping();

        metaserver_show_prompt();
        gtk_main();

        client_negotiate(use_config[CONFIG_SOUND]);

        /* The event_loop will block until connection to the server is lost. */
        event_loop();

        remove_item_inventory(cpl.ob);
        /*
         * We know the following is the private map structure in item.c.  But
         * we don't have direct access to it, so we still use locate.
         */
        remove_item_inventory(locate_item(0));
        draw_look_list();

        /*
         * Need to reset the images so they match up properly and prevent
         * memory leaks.
         */
        reset_image_data();
        client_reset();
    }
}

/**
 * Gets the coordinates of a specified window.
 *
 * @param win Pass in a GtkWidget pointer to get its coordinates.
 * @param x Parent-relative window x coordinate
 * @param y Parent-relative window y coordinate
 * @param wx ?
 * @param wy ?
 * @param w Window width
 * @param h Window height
 */
void get_window_coord(GtkWidget *win, int *x, int *y, int *wx, int *wy,
        int *w, int *h) {
    /* Position of a window relative to its parent window. */
    gdk_window_get_geometry(win->window, x, y, w, h, NULL);
    /* Position of the window in root window coordinates. */
    gdk_window_get_origin(win->window, wx, wy);
    *wx -= *x;
    *wy -= *y;
}
