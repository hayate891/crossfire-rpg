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
 * @file common/p_cmd.c
 * Contains a lot about the commands typed into the client.
 */

#ifndef CPROTO
/* use declarations from p_cmd.h instead of doing make proto on this file */

#include <client.h>
#include <external.h>
#include <script.h>
#include <p_cmd.h>

/**
 * @defgroup PCmdHelpCommands Common client player commands.
 * @{
 */

/* TODO This should really be under /help commands or something... */

/* This dynamically generates a list from the ConsoleCommand list. */
#undef CLIENTHELP_LONG_LIST

/*
long-list:
category
name - description
name - description
...

not long list:
category
name name name ...
*/

#undef HELP_USE_COLOR
#ifdef HELP_USE_COLOR
#error Oops, need to put them back.
#else
#define H1(a) draw_ext_info(NDI_BLACK, MSG_TYPE_CLIENT, MSG_TYPE_CLIENT_NOTICE, a)
#define H2(a) draw_ext_info(NDI_BLACK, MSG_TYPE_CLIENT, MSG_TYPE_CLIENT_NOTICE, a)
#define LINE(a) draw_ext_info(NDI_BLACK, MSG_TYPE_CLIENT, MSG_TYPE_CLIENT_NOTICE, a)
#endif

#define assumed_wrap get_info_width()

/* TODO Help topics other than commands? Refer to other documents? */

/**
 *
 */
static void do_clienthelp_list(void)
{
    ConsoleCommand ** commands_array;
    ConsoleCommand * commands_copy;
    int i;
    CommCat current_cat = COMM_CAT_MISC;
#ifndef CLIENTHELP_LONG_LIST
    char line_buf[MAX_BUF];
    size_t line_len = 0;

    line_buf[0] = '\0';
#endif

    commands_array = get_cat_sorted_commands();

    /* Now we have a nice sorted list. */

    H1(" === Client Side Commands === ");

    for (i = 0; i < get_num_commands(); i++) {
        commands_copy = commands_array[i];

        /* Should be LOG_SPAM but I'm too lazy to tweak it. */
        /* LOG(LOG_INFO, "p_cmd::do_clienthelp_list", "%s Command %s", get_category_name(commands_copy->cat), commands_copy->name); */

        if (commands_copy->cat != current_cat) {
            char buf[MAX_BUF];

#ifndef CLIENTHELP_LONG_LIST
            if (line_len > 0) {
                LINE(line_buf);
                line_buf[0] = '\0';
                line_len = 0;
            }
#endif

#ifdef HELP_USE_COLOR
            snprintf(buf, MAX_BUF - 1, "%s Commands:", get_category_name(commands_copy->cat));
#else
            snprintf(buf, MAX_BUF - 1, " --- %s Commands --- ", get_category_name(commands_copy->cat));
#endif

            H2(buf);
            current_cat = commands_copy->cat;
        }

#ifdef CLIENTHELP_LONG_LIST
        if (commands_copy->desc != NULL) {
            char buf[MAX_BUF];
            snprintf(buf, MAX_BUF - 1, "%s - %s", commands_copy->name, commands_copy->desc);
            LINE(buf);
        } else {
            LINE(commands_copy->name);
        }
#else
        {
            size_t name_len;

            name_len = strlen(commands_copy->name);

            if (strlen(commands_copy->name) > MAX_BUF) {
                LINE(commands_copy->name);
            } else if (name_len > assumed_wrap) {
                LINE(line_buf);
                LINE(commands_copy->name);
                line_len = 0;
            } else if (line_len + name_len > assumed_wrap) {
                LINE(line_buf);
                strncpy(line_buf, commands_copy->name, name_len + 1);
                line_len = name_len;
            } else {
                if (line_len > 0) {
                    strncat(line_buf, " ", 2);
                    line_len += 1;
                }
                strncat(line_buf, commands_copy->name, name_len + 1);
                line_len += name_len;
            }
        }
#endif
    }

#ifndef CLIENTHELP_LONG_LIST
    if (line_len) {
        LINE(line_buf);
    }
#endif
}

/**
 *
 * @param cc
 */
static void show_help(const ConsoleCommand *cc) {
    char buf[MAX_BUF];

    if (cc->desc != NULL) {
        snprintf(buf, MAX_BUF - 1, "%s - %s", cc->name, cc->desc);
        H2(buf);
    } else {
        snprintf(buf, MAX_BUF - 1, "Help for '%s':", cc->name);
        H2(buf);
    }

    if (cc->helpfunc != NULL) {
        const char *long_help = NULL;
        long_help = cc->helpfunc();

        if (long_help != NULL) {
            LINE(long_help);
        } else {
            LINE("Extended help for this command is broken.");
        }
    } else {
        LINE("No extended help is available for this command.");
    }
}

/**
 *
 * @param arg
 */
static void do_clienthelp(const char * arg)
{
    const ConsoleCommand * cc;

    if (!arg || !strlen(arg)) {
        do_clienthelp_list();
        return;
    }

    cc = find_command(arg);

    if (cc == NULL) {
        char buf[MAX_BUF];
        snprintf(buf, MAX_BUF - 1, "clienthelp: Unknown command %s.", arg);
        draw_ext_info(NDI_BLACK, MSG_TYPE_CLIENT, MSG_TYPE_CLIENT_NOTICE, buf);
        return;
    }

    show_help(cc);

}

/**
 *
 */
static const char * help_clienthelp(void)
{
    return
        "Syntax:\n"
        "\n"
        "    clienthelp\n"
        "    clienthelp <command>\n"
        "\n"
        "Without any arguments, displays a list of client-side "
        "commands.\n"
        "\n"
        "With arguments, displays the help for the client-side "
        "command <command>.\n"
        "\n"
        "See also: serverhelp, help.";
}

/**
 *
 * @param arg
 */
static void do_serverhelp(const char * arg)
{

    if (arg) {
        char buf[MAX_BUF];
        snprintf(buf, sizeof(buf), "help %s", arg);
        /* maybe not a must send, but we probably don't want to drop it */
        send_command(buf, -1, 1);
    } else {
        send_command("help", -1, 1); /* TODO make install in server branch doesn't install def_help. */
    }
}

/**
 *
 */
static const char * help_serverhelp(void)
{
    return
        "Syntax:\n"
        "\n"
        "    serverhelp\n"
        "    serverhelp <command>\n"
        "\n"
        "Fetches help from the server.\n"
        "\n"
        "Note that currently nothing can be done (without a recompile) if a "
        "client command masks a server command.\n"
        "\n"
        "See also: clienthelp, help.";
}

/**
 *
 * @param cpnext
 */
static void command_help(const char *cpnext)
{
    if (cpnext) {
        const ConsoleCommand * cc;
        char buf[MAX_BUF];

        cc = find_command(cpnext);
        if (cc != NULL) {
            show_help(cc);
        } else  {
            snprintf(buf, sizeof(buf), "help %s", cpnext);
            /* maybe not a must send, but we probably don't want to drop it */
            send_command(buf, -1, 1);
        }
    } else {
        do_clienthelp_list();
        /* Now fetch (in theory) command list from the server.
        TODO Protocol command - feed it to the tab completer.

        Nope! It effectivey fetches '/help commands for commands'.
        */
        send_command("help", -1, 1); /* TODO make install in server branch doesn't install def_help. */
    }
}

/**
 *
 */
static const char * help_help(void)
{
    return
        "Syntax:\n"
        "\n"
        "    help\n"
        "    help <topic>\n"
        "\n"
        "Without any arguments, displays a list of client-side "
        "commands, and fetches the without-arguments help from "
        "the server.\n"
        "\n"
        "With arguments, first checks if there's a client command "
        "named <topic>. If there is, display it's help. If there "
        "isn't, send the topic to the server.\n"
        "\n"
        "See also: clienthelp, serverhelp.";
}

/**
 * @} */ /* EndOf PCmdHelpCommands
 */

/*
 * Other commands.
 */

/**
 *
 * @param cpnext
 */
static void set_command_window(const char *cpnext)
{
    if (!cpnext) {
        draw_ext_info(NDI_BLACK, MSG_TYPE_CLIENT, MSG_TYPE_CLIENT_NOTICE,
                      "cwindow command requires a number parameter");
    } else {
        want_config[CONFIG_CWINDOW] = atoi(cpnext);
        if (want_config[CONFIG_CWINDOW]<1 || want_config[CONFIG_CWINDOW]>127) {
            want_config[CONFIG_CWINDOW]=COMMAND_WINDOW;
        } else {
            use_config[CONFIG_CWINDOW] = want_config[CONFIG_CWINDOW];
        }
    }
}

/**
 *
 * @param cpnext
 */
static void command_foodbeep(const char *cpnext)
{
    (void)cpnext; /* __UNUSED__ */
    if (want_config[CONFIG_FOODBEEP]) {
        want_config[CONFIG_FOODBEEP]=0;
        draw_ext_info(NDI_BLACK, MSG_TYPE_CLIENT, MSG_TYPE_CLIENT_NOTICE,
                      "Warning bell when low on food disabled");
    } else {
        want_config[CONFIG_FOODBEEP]=1;
        draw_ext_info(NDI_BLACK, MSG_TYPE_CLIENT, MSG_TYPE_CLIENT_NOTICE,
                      "Warning bell when low on food enabled");
    }
    use_config[CONFIG_FOODBEEP] = want_config[CONFIG_FOODBEEP];
}

/**
 *
 * @param cat
 */
const char * get_category_name(CommCat cat)
{
    const char * cat_name;

    /* HACK Need to keep this in sync. with player.h */
    switch(cat) {
    case COMM_CAT_MISC:
        cat_name = "Miscellaneous";
        break;
    case COMM_CAT_HELP:
        cat_name = "Help";
        break;
    case COMM_CAT_INFO:
        cat_name = "Informational";
        break;
    case COMM_CAT_SETUP:
        cat_name = "Configuration";
        break;
    case COMM_CAT_SCRIPT:
        cat_name = "Scripting";
        break;
    case COMM_CAT_DEBUG:
        cat_name = "Debugging";
        break;
    default:
        cat_name = "PROGRAMMER ERROR";
        break;
    }

    return cat_name;
}

/*
 * Command table.
 *
 * Implementation basically stolen verbatim from the server.
 */

/* "Typecasters" (and some forwards) */

/**
 *
 * @param ignored
 */
static void do_script_list(const char * ignored)
{
    script_list();
}

/**
 *
 * @param ignored
 */
static void do_clearinfo(const char * ignored)
{
    menu_clear();
}

/**
 *
 * @param ignored
 */
static void do_disconnect(const char * ignored)
{
    close_server_connection();

    /* the gtk clients need to do some cleanup logic - otherwise,
     * they start hogging CPU.
     */
    cleanup_connection();
    return;
}

#ifdef HAVE_DMALLOC_H
#ifndef DMALLOC_VERIFY_NOERROR
#define DMALLOC_VERIFY_NOERROR  1
#endif
/**
 *
 * @param ignored
 */
static void do_dmalloc(const char * ignored)
{
    if (dmalloc_verify(NULL)==DMALLOC_VERIFY_NOERROR)
        draw_ext_info(NDI_BLACK, MSG_TYPE_CLIENT, MSG_TYPE_CLIENT_NOTICE,
                      "Heap checks out OK");
    else
        draw_ext_info(NDI_RED, MSG_TYPE_CLIENT, MSG_TYPE_CLIENT_ERROR,
                      "Heap corruption detected");
}
#endif

/**
 *
 * @param ignored
 */
static void do_inv(const char * ignored)
{
    print_inventory (cpl.ob);
}

static void do_magicmap(const char * ignored)
{
    cpl.showmagic=1;
    draw_magic_map();
}

/**
 *
 * @param ignored
 */
static void do_savedefaults(const char * ignored)
{
    save_defaults();
}

/**
 *
 * @param ignored
 */
static void do_savewinpos(const char * ignored)
{
    save_winpos();
}

/**
 *
 * @param used
 */
static void do_take(const char * used)
{
    command_take("take", used); /* I dunno why they want it. */
}

/**
 *
 * @param ignored
 */
static void do_num_free_items(const char * ignored)
{
    LOG(LOG_INFO,"common::extended_command","num_free_items=%d", num_free_items());
}

static void do_clienthelp(const char * arg); /* Forward. */

/* Help "typecasters". */
#include "chelp.h"

/**
 *
 */
static const char * help_bind(void)
{
    return HELP_BIND_LONG;
}

/**
 *
 */
static const char * help_unbind(void)
{
    return HELP_UNBIND_LONG;
}

/**
 *
 */
static const char * help_magicmap(void)
{
    return HELP_MAGICMAP_LONG;
}

/**
 *
 */
static const char * help_inv(void)
{
    return HELP_INV_LONG;
}

/**
 *
 */
static const char * help_cwindow(void)
{
    return
        "Syntax:\n"
        "\n"
        "    cwindow <val>\n"
        "\n"
        "set size of command"
        "window (if val is exceeded"
        "client won't send new"
        "commands to server\n\n"
        "(What does this mean, 'put a lid on it'?) TODO";
}

/**
 *
 */
static const char * help_script(void) {
    return
        "Syntax: script <path>\n\n"
        "Start an executable client script located at <path>. For details on "
        "client-side scripting, please see the Crossfire Wiki.";
}

/**
 *
 */
static const char * help_scripttell(void)
{
    return
        "Syntax:\n"
        "\n"
        "    scripttell <yourname> <data>\n"
        "\n"
        "?";
}

/* Toolkit-dependent. */

/**
 *
 */
static const char * help_savewinpos(void)
{
    return
        "Syntax:\n"
        "\n"
        "    savewinpos\n"
        "\n"
        "save window positions - split windows mode only.";
}

static const char * help_scriptkill(void)
{
    return
        "Syntax:\n"
        "\n"
        "    scriptkill <name>\n"
        "\n"
        "Stop scripts named <name>.\n"
        "(Not guaranteed to work?)";
}

/**
 *
 */
static const char * help_showweight(void)
{
    return
        "Syntax:\n"
        "\n"
        "    showweight\n"
        "    showweight inventory\n"
        "    showweight look\n"
        "\n"
        "(Or any prefix of the arguments.)"
        "Toggles if you see the weight of"
        "items in your inventory (also if"
        "no argument given) or your"
        "look-window.";
}

/*
*	draw_ext_info(NDI_NAVY, MSG_TYPE_CLIENT, MSG_TYPE_CLIENT_NOTICE,
            "Information Commands");*
	draw_ext_info(NDI_BLACK, MSG_TYPE_CLIENT, MSG_TYPE_CLIENT_NOTICE,
            " inv         - *recursively* print your");
	draw_ext_info(NDI_BLACK, MSG_TYPE_CLIENT, MSG_TYPE_CLIENT_NOTICE,
            "               inventory - includes containers.");
	draw_ext_info(NDI_BLACK, MSG_TYPE_CLIENT, MSG_TYPE_CLIENT_NOTICE,
            " showinfo, take");
	draw_ext_info(NDI_BLACK, MSG_TYPE_CLIENT, MSG_TYPE_CLIENT_NOTICE,
            " help        - show this message");
	draw_ext_info(NDI_BLACK, MSG_TYPE_CLIENT, MSG_TYPE_CLIENT_NOTICE,
            " help <command> - get more information on a");
	draw_ext_info(NDI_BLACK, MSG_TYPE_CLIENT, MSG_TYPE_CLIENT_NOTICE,
            "                command (Server command only?)");
	draw_ext_info(NDI_BLACK, MSG_TYPE_CLIENT, MSG_TYPE_CLIENT_NOTICE,
            " showicon    - draw status icons in");
	draw_ext_info(NDI_BLACK, MSG_TYPE_CLIENT, MSG_TYPE_CLIENT_NOTICE,
            "               inventory window");
	draw_ext_info(NDI_BLACK, MSG_TYPE_CLIENT, MSG_TYPE_CLIENT_NOTICE,
            " showweight  - show weight in inventory");
	draw_ext_info(NDI_BLACK, MSG_TYPE_CLIENT, MSG_TYPE_CLIENT_NOTICE,
            "               look windows");
	draw_ext_info(NDI_NAVY, MSG_TYPE_CLIENT, MSG_TYPE_CLIENT_NOTICE,
            "Scripting Commands");
	draw_ext_info(NDI_NAVY, MSG_TYPE_CLIENT, MSG_TYPE_CLIENT_NOTICE,
            "Client Side Debugging Commands");
#ifdef HAVE_DMALLOC_H
	draw_ext_info(NDI_BLACK, MSG_TYPE_CLIENT, MSG_TYPE_CLIENT_NOTICE,
            " dmalloc     - Check heap?");
#endif
*/

/* TODO Wrap these? Um. */
static ConsoleCommand CommonCommands[] = {
    /* From player.h:
        name, cat,
        func, helpfunc,
        long_desc
    */

    {
        "autorepeat", COMM_CAT_MISC,
        set_autorepeat, NULL,
        "toggle autorepeat" /* XXX Eh? */
    },

    {
        "bind", COMM_CAT_SETUP,
        bind_key, help_bind,
        HELP_BIND_SHORT
    },

    {
        "script", COMM_CAT_SCRIPT,
        script_init, help_script,
        NULL
    },
#ifdef HAVE_LUA
    {
        "lua_load", COMM_CAT_SCRIPT,
        script_lua_load, NULL, NULL
    },

    {
        "lua_list", COMM_CAT_SCRIPT,
        script_lua_list, NULL, NULL
    },

    {
        "lua_kill", COMM_CAT_SCRIPT,
        script_lua_kill, NULL, NULL
    },
#endif
    {
        "scripts", COMM_CAT_SCRIPT,
        do_script_list, NULL,
        "List the running scripts(?)"
    },

    {
        "scriptkill", COMM_CAT_SCRIPT,
        script_kill, help_scriptkill,
        NULL
    },

    {
        "scripttell", COMM_CAT_SCRIPT,
        script_tell, help_scripttell,
        NULL
    },

    {
        "clearinfo", COMM_CAT_MISC,
        do_clearinfo, NULL,
        "clear the info window"
    },

    {
        "cwindow", COMM_CAT_SETUP,
        set_command_window, help_cwindow,
        NULL
    },

    {
        "disconnect", COMM_CAT_MISC,
        do_disconnect, NULL,
        "close connection to server"
    },


#ifdef HAVE_DMALLOC_H
    {
        "dmalloc", COMM_CAT_DEBUG,
        do_dmalloc, NULL,
        NULL
    },
#endif

    {
        "foodbeep", COMM_CAT_SETUP,
        command_foodbeep, NULL,
        "toggle audible low on food warning"

    },

    {
        "help", COMM_CAT_HELP,
        command_help, help_help,
        NULL
    },

    {
        "clienthelp", COMM_CAT_HELP,
        do_clienthelp, help_clienthelp,
        "Client-side command information"
    },

    {
        "serverhelp", COMM_CAT_HELP,
        do_serverhelp, help_serverhelp,
        "Server-side command information"
    },

    {
        "inv", COMM_CAT_DEBUG,
        do_inv, help_inv,
        HELP_INV_SHORT
    },

    {
        "magicmap", COMM_CAT_MISC,
        do_magicmap, help_magicmap,
        HELP_MAGICMAP_SHORT
    },

    {
        "savedefaults", COMM_CAT_SETUP,
        do_savedefaults, NULL,
        HELP_SAVEDEFAULTS_SHORT /* How do we make sure showicons stays on? */
    },

    {
        "savewinpos", COMM_CAT_SETUP,
        do_savewinpos, help_savewinpos,
        "Saves the position and sizes of windows." /* Panes? */
    },

    {
        "scroll", COMM_CAT_SETUP,
        set_scroll, NULL,
        "toggle scroll/wrap mode in info window"
    },

    {
        "showicon", COMM_CAT_SETUP,
        set_show_icon, NULL,
        "Toggles if you see the worn, locked, cursed etc state in the inventory pane."
    },

    {
        "showweight", COMM_CAT_SETUP,
        set_show_weight, help_showweight,
        "Toggles if you see item weights in inventory look windows."
    },

    {
        "take", COMM_CAT_MISC,
        do_take, NULL,
        NULL
    },

    {
        "unbind", COMM_CAT_SETUP,
        unbind_key, help_unbind,
        NULL
    },

    {
        "num_free_items", COMM_CAT_DEBUG,
        do_num_free_items, NULL,
        "log the number of free items?"
    },
    {
        "show", COMM_CAT_SETUP,
        command_show, NULL,
        "Change what items to show in inventory"
    },

};

const int CommonCommandsSize = sizeof(CommonCommands) / sizeof(ConsoleCommand);

#ifdef TOOLKIT_COMMANDS
extern ConsoleCommand ToolkitCommands[];

extern const int ToolkitCommandsSize;
#endif

/* ------------------------------------------------------------------ */

int num_commands;

/**
 *
 */
int get_num_commands(void)
{
    return num_commands;
}

static ConsoleCommand ** name_sorted_commands;

/**
 *
 * @param a_
 * @param b_
 */
static int sort_by_name(const void * a_, const void * b_)
{
    ConsoleCommand * a = *((ConsoleCommand **)a_);
    ConsoleCommand * b = *((ConsoleCommand **)b_);

    return strcmp(a->name, b->name);
}

static ConsoleCommand ** cat_sorted_commands;

/* Sort by category, then by name. */

/**
 *
 * @param a_
 * @param b_
 */
static int sort_by_category(const void *a_, const void *b_)
{
    /* Typecasts, so it goes. */
    ConsoleCommand * a = *((ConsoleCommand **)a_);
    ConsoleCommand * b = *((ConsoleCommand **)b_);

    if (a->cat == b->cat) {
        return strcmp(a->name, b->name);
    }

    return a->cat - b->cat;
}

/**
 *
 */
void init_commands(void)
{
    int i;

#ifdef TOOLKIT_COMMANDS
    init_toolkit_commands();

    /* TODO I dunno ... go through the list and print commands without helps? */
    num_commands = CommonCommandsSize + ToolkitCommandsSize;
#else
    num_commands = CommonCommandsSize;
#endif

    /* Make a list of (pointers to statically allocated!) all the commands.
       We have a list; the toolkit has a
       ToolkitCommands and ToolkitCommandsSize, initialized before calling this.
    */

    /* XXX Leak! */
    name_sorted_commands = g_malloc(sizeof(ConsoleCommand *) * num_commands);

    for (i = 0; i < CommonCommandsSize; i++) {
        name_sorted_commands[i] = &CommonCommands[i];
    }

#ifdef TOOLKIT_COMMANDS
    for(i = 0; i < ToolkitCommandsSize; i++) {
        name_sorted_commands[CommonCommandsSize + i] = &ToolkitCommands[i];
    }
#endif

    /* Sort them. */
    qsort(name_sorted_commands, num_commands, sizeof(ConsoleCommand *), sort_by_name);

    /* Copy the list, then sort it by category. */
    cat_sorted_commands = g_malloc(sizeof(ConsoleCommand *) * num_commands);

    memcpy(cat_sorted_commands, name_sorted_commands, sizeof(ConsoleCommand *) * num_commands);

    qsort(cat_sorted_commands, num_commands, sizeof(ConsoleCommand *), sort_by_category);

    /* TODO Add to the list of tab-completion items. */
}

#ifndef tolower
#define tolower(C)      (((C) >= 'A' && (C) <= 'Z')? (C) - 'A' + 'a': (C))
#endif

/**
 *
 * @param cmd
 */
const ConsoleCommand * find_command(const char * cmd)
{
    ConsoleCommand ** asp_p = NULL, dummy;
    ConsoleCommand * dummy_p;
    ConsoleCommand * asp;
    char *cp, *cmd_cpy;
    cmd_cpy = g_strdup(cmd);

    for (cp=cmd_cpy; *cp; cp++) {
        *cp =tolower(*cp);
    }

    dummy.name = cmd_cpy;
    dummy_p = &dummy;
    asp_p = bsearch(
                (void *)&dummy_p,
                (void *)name_sorted_commands,
                num_commands,
                sizeof(ConsoleCommand *),
                sort_by_name);

    if (asp_p == NULL) {
        free(cmd_cpy);
        return NULL;
    }

    asp = *asp_p;

    /* TODO The server's find_command() searches first the commands,
    then the emotes. We might have to do something similar someday, too. */
    /* if (asp == NULL) search something else? */

    free(cmd_cpy);

    return asp;
}

/**
 * Returns a pointer to the head of an array of ConsoleCommands sorted by
 * category, then by name.  It's num_commands long.
 */
ConsoleCommand ** get_cat_sorted_commands(void)
{
    return cat_sorted_commands;
}

/**
 * Tries to handle command cp (with optional params in cpnext, which may be
 * null) as a local command. If this was a local command, returns true to
 * indicate command was handled.  This code was moved from extended_command so
 * scripts ca issue local commands to handle keybindings or anything else.
 *
 * @param cp
 * @param cpnext
 */
int handle_local_command(const char* cp, const char * cpnext)
{
    const ConsoleCommand * cc = NULL;

    cc = find_command(cp);

    if (cc == NULL) {
        return FALSE;
    }

    if (cc->dofunc == NULL) {
        char buf[MAX_BUF];

        snprintf(buf, MAX_BUF - 1, "Client command %s has no implementation!", cc->name);
        draw_ext_info(NDI_RED, MSG_TYPE_CLIENT, MSG_TYPE_CLIENT_NOTICE, buf);

        return FALSE;
    }

    cc->dofunc(cpnext);

    return TRUE;
}

/**
 * This is an extended command (ie, 'who, 'whatever, etc).  In general, we
 * just send the command to the server, but there are a few that we care about
 * (bind, unbind)
 *
 * The command passed to us can not be modified - if it is a keybinding, we
 * get passed the string that is that binding - modifying it effectively
 * changes the binding.
 *
 * @param ocommand
 */
void extended_command(const char *ocommand)
{
    const char *cp = ocommand;
    char *cpnext, command[MAX_BUF];

    if ((cpnext = strchr(cp, ' '))!=NULL) {
        int len = cpnext - ocommand;
        if (len > (MAX_BUF -1 )) {
            len = MAX_BUF-1;
        }

        strncpy(command, ocommand, len);
        command[len] = '\0';
        cp = command;
        while (*cpnext == ' ') {
            cpnext++;
        }
        if (*cpnext == 0) {
            cpnext = NULL;
        }
    }
    /*
     * Try to prevent potential client hang by trying to delete a
     * character when there is no character to delete.
     * Thus, only send quit command if there is a player to delete.
     */
    if (cpl.title[0] == '\0' && strcmp(cp, "quit") == 0){
        // Bail here, there isn't anything this should be doing.
        return;
    }

    /* cp now contains the command (everything before first space),
     * and cpnext contains everything after that first space.  cpnext
     * could be NULL.
     */
#ifdef HAVE_LUA
    if ( script_lua_command(cp, cpnext) ) {
        return;
    }
#endif

    /* If this isn't a client-side command, send it to the server. */
    if (!handle_local_command(cp, cpnext)) {
        /* just send the command(s)  (if `ocommand' is a compound command */
        /* then split it and send each part seperately */
        /* TODO Remove this from the server; end of commands.c. */
        strncpy(command, ocommand, MAX_BUF-1);
        command[MAX_BUF-1]=0;
        cp = strtok(command, ";");
        while ( cp ) {
            while( *cp == ' ' ) {
                cp++;
            } /* throw out leading spaces; server
				       does not like them */
            send_command(cp, cpl.count, 0);
            cp = strtok(NULL, ";");
        }
    }
}

/* ------------------------------------------------------------------ */

/* This list is used for the 'tab' completion, and nothing else.
 * Therefore, if it is out of date, it isn't that terrible, but
 * ideally it should stay somewhat up to date with regards to
 * the commands the server supports.
 */

/* TODO Dynamically generate. */

static const char *const commands[] = {
    "accuse", "afk", "apply", "applymode", "archs", "beg", "bleed", "blush",
    "body", "bounce", "bow", "bowmode", "brace", "build", "burp", "cackle", "cast",
    "chat", "chuckle", "clap", "cointoss", "cough", "cringe", "cry", "dance",
    "disarm", "dm", "dmhide", "drop", "dropall", "east", "examine", "explore",
    "fire", "fire_stop", "fix_me", "flip", "frown", "gasp", "get", "giggle",
    "glare", "grin", "groan", "growl", "gsay", "help", "hiccup", "hiscore", "hug",
    "inventory", "invoke", "killpets", "kiss", "laugh", "lick", "listen", "logs",
    "mapinfo", "maps", "mark", "me", "motd", "nod", "north", "northeast",
    "northwest", "orcknuckle", "output-count", "output-sync", "party", "peaceful",
    "petmode", "pickup", "players", "poke", "pout", "prepare", "printlos", "puke",
    "quests", "quit", "ready_skill", "rename", "reply", "resistances",
    "rotateshoottype", "run", "run_stop", "save", "say", "scream", "search",
    "search-items", "shake", "shiver", "shout", "showpets", "shrug", "shutdown",
    "sigh", "skills", "slap", "smile", "smirk", "snap", "sneeze", "snicker",
    "sniff", "snore", "sound", "south", "southeast", "southwest", "spit",
    "statistics", "stay", "strings", "strut", "sulk", "take", "tell", "thank",
    "think", "throw", "time", "title", "twiddle", "use_skill", "usekeys",
    "version", "wave", "weather", "west", "whereabouts", "whereami", "whistle",
    "who", "wimpy", "wink", "yawn",
};
#define NUM_COMMANDS ((int)(sizeof(commands) / sizeof(char*)))

/**
 * Player has entered 'command' and hit tab to complete it.  See if we can
 * find a completion.  Returns matching command. Returns NULL if no command
 * matches.
 *
 * @param command
 */
const char * complete_command(const char *command)
{
    int i, len, display;
    const char *match;
    static char result[64];
    char list[500];

    len = strlen(command);

    if (len == 0) {
        return NULL;
    }

    display = 0;
    strcpy(list, "Matching commands:");

    /* TODO Partial match, e.g.:
         If the completion list was:
           wear
           wet #?

         If we type 'w' then hit tab, put in the e.

       Basically part of bash (readline?)'s behaviour.
    */

    match = NULL;

    /* check server side commands */
    for (i=0; i<NUM_COMMANDS; i++) {
        if (!strncmp(command, commands[i], len)) {
            if (display) {
                snprintf(list + strlen(list), 499 - strlen(list), " %s", commands[i]);
            } else if (match != NULL) {
                display = 1;
                snprintf(list + strlen(list), 499 - strlen(list), " %s %s", match, commands[i]);
                match = NULL;
            } else {
                match = commands[i];
            }
        }
    }

    /* check client side commands */
    for (i=0; i<CommonCommandsSize; i++) {
        if (!strncmp(command, CommonCommands[i].name, len)) {
            if (display) {
                snprintf(list + strlen(list), 499 - strlen(list), " %s", CommonCommands[i].name);
            } else if (match != NULL) {
                display = 1;
                snprintf(list + strlen(list), 499 - strlen(list), " %s %s", match, CommonCommands[i].name);
                match = NULL;
            } else {
                match = CommonCommands[i].name;
            }
        }
    }

    if (match == NULL) {
        if (display) {
            strncat(list, "\n", 499 - strlen(list));
            draw_ext_info(
                NDI_BLACK, MSG_TYPE_CLIENT, MSG_TYPE_CLIENT_NOTICE, list);
        } else
            draw_ext_info(NDI_BLACK, MSG_TYPE_CLIENT, MSG_TYPE_CLIENT_NOTICE,
                          "No matching command.\n");
        /* No match. */
        return NULL;
    }

    /*
     * Append a space to allow typing arguments. For commands without arguments
     * the excess space should be stripped off automatically.
     */
    snprintf(result, sizeof(result), "%s ", match);

    return result;
}

#endif /* CPROTO */
