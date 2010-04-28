const char * const rcsid_common_client_c =
    "$Id$";
/*
    Crossfire client, a client program for the crossfire program.

    Copyright (C) 2001,2010 Mark Wedel & Crossfire Development Team

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

/**
 * @file common/client.c
 * Client interface main routine.  Sets up a few global variables, connects to
 * the server, tells it what kind of pictures it wants, adds the client and
 * enters the main dispatch loop.
 *
 * The main event loop (event_loop()) checks the TCP socket for input and then
 * polls for x events.  This should be fixed since you can just block on both
 * filedescriptors.
 *
 * The DoClient function receives a message (an ArgList), unpacks it, and in a
 * slow for loop dispatches the command to the right function through the
 * commands table.   ArgLists are essentially like RPC things, only they don't
 * require going through RPCgen, and it's easy to get variable length lists.
 * They are just lists of longs, strings, characters, and byte arrays that can
 * be converted to a machine independent format
 */

#include <client.h>
#include <external.h>
#include <errno.h>
#include <script.h>
#include <ctype.h>

#include "mapdata.h"

/* actually declare the globals */

#ifdef SERVER
char *server=SERVER;
#else
char *server=NULL;
#endif

char VERSION_INFO[MAX_BUF];

char *client_libdir=NULL,*meta_server=META_SERVER;
char *sound_server="cfsndserv";
char *skill_names[MAX_SKILL];

int last_used_skills[MAX_SKILL+1];

int meta_port=META_PORT, want_skill_exp=0,
    replyinfo_status=0, requestinfo_sent=0, replyinfo_last_face=0,
    maxfd,metaserver_on=METASERVER, metaserver2_on=METASERVER2,
    wantloginmethod=0, serverloginmethod=0;
uint32	tick=0;

uint16	exp_table_max=0;
uint64	*exp_table=NULL;

NameMapping skill_mapping[MAX_SKILL], resist_mapping[NUM_RESISTS];

Client_Player cpl;
ClientSocket csocket;

const char *const resists_name[NUM_RESISTS] = {
"armor", "magic", "fire", "elec",
"cold", "conf", "acid", "drain",
"ghit", "pois", "slow", "para",
"t undead", "fear", "depl","death",
"hword", "blind"};

typedef void (*CmdProc)(unsigned char *, int len);

struct CmdMapping {
  const char *cmdname;
  void (*cmdproc)(unsigned char *, int );
  enum CmdFormat cmdformat;
};


struct CmdMapping commands[] = {
    /* Order of this table doesn't make a difference.  I tried to sort
     * of cluster the related stuff together.
     */
    { "map2", Map2Cmd, SHORT_ARRAY },
    { "map_scroll", (CmdProc)map_scrollCmd, ASCII },
    { "magicmap", MagicMapCmd, MIXED /* ASCII, then binary */},
    { "newmap", NewmapCmd, NODATA },
    { "mapextended", MapExtendedCmd, MIXED /* chars, then SHORT_ARRAY */ },

    { "item2", Item2Cmd, MIXED },
    { "upditem", UpdateItemCmd, MIXED },
    { "delitem", DeleteItem, INT_ARRAY },
    { "delinv",	DeleteInventory, ASCII },

    { "addspell", AddspellCmd, MIXED },
    { "updspell", UpdspellCmd, MIXED },
    { "delspell", DeleteSpell, INT_ARRAY },

    { "drawinfo", (CmdProc)DrawInfoCmd, ASCII },
    { "drawextinfo", (CmdProc)DrawExtInfoCmd, ASCII},
    { "stats", StatsCmd, STATS /* array of: int8, (int?s for that stat) */ },

    { "image2", Image2Cmd, MIXED /* int, int8, int, PNG */ },
    { "face2", Face2Cmd, MIXED /* int16, int8, int32, string */},
    { "tick", TickCmd, INT_ARRAY /* uint32 */},


    { "sound2", Sound2Cmd, MIXED /* int8, int8, int8, int8, int8, int8, chars, int8, chars */},
    { "music", (CmdProc)MusicCmd, ASCII },
    { "anim", AnimCmd, SHORT_ARRAY},
    { "smooth", SmoothCmd, SHORT_ARRAY},

    { "player", PlayerCmd, MIXED /* 3 ints, int8, str */ },
    { "comc", CompleteCmd, SHORT_INT },

    { "addme_failed", (CmdProc)AddMeFail, NODATA },
    { "addme_success", (CmdProc)AddMeSuccess, NODATA },
    { "version", (CmdProc)VersionCmd, ASCII },
    { "goodbye", (CmdProc)GoodbyeCmd, NODATA },
    { "setup", (CmdProc)SetupCmd, ASCII},
    { "failure", (CmdProc)FailureCmd, ASCII},
    { "accountplayers", (CmdProc)AccountPlayersCmd, ASCII},

    { "query", (CmdProc)handle_query, ASCII},
    { "replyinfo", ReplyInfoCmd, ASCII},
    { "ExtendedTextSet", (CmdProc)SinkCmd, NODATA},
    { "ExtendedInfoSet", (CmdProc)SinkCmd, NODATA},

    { "pickup", PickupCmd, INT_ARRAY /* uint32 */},
};

#define NCOMMANDS ((int)(sizeof(commands)/sizeof(struct CmdMapping)))

/**
 * Basic little function that closes the connection to the server.
 * it seems better to have it one palce here than the same
 * logic sprinkled about in half a dozen locations in the code.
 * also useful in that if this logic does change, just one place
 * to update it.
 */
void close_server_connection()
{
#ifdef WIN32
    closesocket(csocket.fd);
#else
    close(csocket.fd);
#endif
    csocket.fd=-1;
}

void DoClient(ClientSocket *csocket)
{
    int i,len;
    unsigned char *data;

    while (1) {
        i=SockList_ReadPacket(csocket->fd, &csocket->inbuf, MAXSOCKBUF-1);
        if (i==-1) {
            /* Need to add some better logic here */
            close_server_connection();
            return;
        }
        if (i==0) return;   /* Don't have a full packet */

        /* Terminate the buffer */
        csocket->inbuf.buf[csocket->inbuf.len]='\0';
        data = csocket->inbuf.buf+2;
        while ((*data != ' ') && (*data != '\0')) ++data;
        if (*data == ' ') {
            *data='\0';
            data++;
            len = csocket->inbuf.len - (data - csocket->inbuf.buf);
        }
        else len = 0;
        for(i=0;i < NCOMMANDS;i++) {
            if (strcmp((char*)csocket->inbuf.buf+2,commands[i].cmdname)==0) {
                script_watch((char*)csocket->inbuf.buf+2,data,len,commands[i].cmdformat);
                commands[i].cmdproc(data,len);
                break;
            }
        }
        csocket->inbuf.len=0;
        if (i == NCOMMANDS) {
            printf("Bad command from server (%s)\n",csocket->inbuf.buf+2);
        }
    }
}

#ifdef WIN32
#define socklen_t int
#else
#include <netdb.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netinet/tcp.h>
#include <ctype.h>
#include <arpa/inet.h>
#endif

/**
 * Return the file descripter of a connected socket, -1 on failure.  In the
 * case where HAVE_GETADDRINFO is true, a timeout connect() is implemented to
 * avoid very long (3 minute) client freeze-up when a host is not reachable.
 * It is easy for this to happen if a cached server entry is used instead of
 * metaserver results.
 */
int init_connection(char *host, int port)
{
    int fd = -1, oldbufsize, newbufsize=65535;
    socklen_t buflen=sizeof(int);
#if !HAVE_GETADDRINFO || WIN32
    struct sockaddr_in insock;
    struct protoent *protox;

    /* In my case, an empty host will be saved as (null) in
     * the defaults file.  However, upon loading, that doesn't
     * show up as a NULL, but rather this string.  For whatever
     * reasons, at least on my system, the lookup of this takes
     * a long time, and it isn't a valid host name in any case,
     * so just abort quickly
     */
    if (!strcmp(host,"(null)")) return -1;

    protox = getprotobyname("tcp");
    if (protox == (struct protoent  *) NULL)
    {
	LOG (LOG_ERROR,"common::init_connection", "Error getting protobyname (tcp)");
	return -1;
    }
    fd = socket(PF_INET, SOCK_STREAM, protox->p_proto);
    if (fd==-1) {
	    perror("init_connection:  Error on socket command.\n");
	    LOG (LOG_ERROR,"common::init_connection", "Error on socket command");
	return -1;
    }

    insock.sin_family = AF_INET;
    insock.sin_port = htons((unsigned short)port);
    if (isdigit(*host))
	insock.sin_addr.s_addr = inet_addr(host);
    else {
	struct hostent *hostbn = gethostbyname(host);
	if (hostbn == (struct hostent *) NULL)
	{
	    LOG (LOG_ERROR,"common::init_connection","Unknown host: %s",host);
	    return -1;
	}
	memcpy(&insock.sin_addr, hostbn->h_addr, hostbn->h_length);
    }

    if (connect(fd,(struct sockaddr *)&insock,sizeof(insock)) == (-1))
    {
        LOG (LOG_ERROR,"common::init_connection","Can't connect to server");
	    perror("Can't connect to server");
	    return -1;
    }

#else
    struct addrinfo hints;
    struct addrinfo *res = NULL, *ai;
    char port_str[6];
    int fd_status, fd_flags, fd_select, fd_sockopt;
    struct timeval tv;
    fd_set fdset;

    /* See note in section above about null hosts names */
    if (!strcmp(host,"(null)")) return -1;

    snprintf(port_str, sizeof(port_str), "%d", port);

    memset(&hints, 0, sizeof(hints));
    hints.ai_family = AF_UNSPEC;
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_protocol = IPPROTO_TCP;

    if (getaddrinfo(host, port_str, &hints, &res) != 0)
	return -1;

    /*
     * Assume that an error will not occur and that the socket is left open.
     */
    fd_status = 0;
    for (ai = res; ai != NULL; ai = ai->ai_next) {
        /*
         * Try to create a socket.
         */
	fd = socket(ai->ai_family, ai->ai_socktype, ai->ai_protocol);
	if (fd == -1) {
            LOG (LOG_ERROR,
                "common::init_connection","Error creating socket (%d %s)\n",
                    errno, strerror(errno));
            continue;
        }
        /*
         * Set the socket to non-blocking mode.
         */
        fd_flags = fcntl(fd, F_GETFL, NULL);
        if (fd_flags == -1) {
            LOG (LOG_ERROR,
                "common::init_connection","Error fcntl(fd, F_GETFL) (%s)\n",
                     strerror(errno));
            fd_status = -1;
            break;
        }
        fd_flags |= O_NONBLOCK;
        if (fcntl(fd, F_SETFL, fd_flags) == -1) {
            LOG (LOG_ERROR,
                "common::init_connection","Error fcntl(fd, F_SETFL) (%s)\n",
                     strerror(errno));
            fd_status = -1;
            break;
        }
        /*
         * Try to connect in non-blocking mode to avoid an extended client
         * lockup in the event that the connection fails for some reason.
         */
        if (connect(fd, ai->ai_addr, ai->ai_addrlen) == -1) {
            /*
             * Assume the connection will fail...
             */
            fd_status = -1;
            if (errno == EINPROGRESS) {
                do {
                    /*
                     * Do not block more than the amount of time specified
                     * here.  Prior to implementation of this timeout, the
                     * client was observed to freeze up for three minutes or
                     * so when the connection failed.
                     */
                    tv.tv_sec = 30;
                    tv.tv_usec = 0;
                    FD_ZERO(&fdset);
                    FD_SET(fd, &fdset);
                    fd_select = select(fd+1, NULL, &fdset, NULL, &tv);
                    if (fd_select == -1 && errno != EINTR) {
                        LOG (LOG_ERROR, "common::init_connection",
                            "Error connecting %d - %s\n",
                                errno, strerror(errno));
                        break;
                    } else if (fd_select > 0) {
                        /*
                         * Socket selected for write.
                         */
                        if (getsockopt(fd, SOL_SOCKET, SO_ERROR,
                                (void*)(&fd_sockopt), &buflen)) {
                            LOG (LOG_ERROR, "common::init_connection",
                                "Error in getsockopt %d - %s\n",
                                    errno, strerror(errno));
                            break;
                        }
                        if (fd_sockopt) {
                            LOG (LOG_ERROR, "common::init_connection",
                                "Error in delayed connection %d - %s\n",
                                    fd_sockopt, strerror(fd_sockopt));
                            break;
                        }
                        /*
                         * The assumption was wrong: the connection succeeded.
                         */
                        fd_status = 0;
                        break;
                    } else {
                        LOG (LOG_ERROR,"common::init_connection","Timeout\n");
                        break;
                    }
                } while (1);
            } else {
                LOG (LOG_ERROR, "common::init_connection",
                    "Error connecting %d - %s\n", errno, strerror(errno));
                break;
            }
        }
        if (! fd_status) {
            /*
             * Set the socket to blocking mode.
             */
            fd_flags = fcntl(fd, F_GETFL, NULL);
            if (fd_flags == -1) {
                LOG (LOG_ERROR, "common::init_connection",
                    "Error fcntl(..., F_GETFL) (%s)\n", strerror(errno));
                fd_status = -1;
            } else {
                fd_flags &= (~O_NONBLOCK);
                if (fcntl(fd, F_SETFL, fd_flags) == -1) {
                    LOG (LOG_ERROR, "common::init_connection",
                        "Error fcntl(..., F_SETFL) (%s)\n", strerror(errno));
                    fd_status = -1;
                }
            }
        }
        break;
    }

    if (fd_status) {
        close(fd);
        fd = -1;
    }

    freeaddrinfo(res);

    if (fd == -1)
	return -1;
#endif

    free(csocket.servername);
    csocket.servername = malloc(sizeof(char)*(strlen(host)+1));
    strcpy(csocket.servername, host);

#ifndef WIN32
    if (fcntl(fd, F_SETFL, O_NDELAY)==-1) {
	LOG (LOG_ERROR,"common::init_connection","Error on fcntl.");
    }
#else
    {
		unsigned long tmp = 1;
		if (ioctlsocket(fd, FIONBIO, &tmp)<0) {
	    LOG (LOG_ERROR,"common::init_connection","Error on ioctlsocket.");
        }
	}
#endif

#ifdef TCP_NODELAY
    /* turn off nagle algorithm */
    if (use_config[CONFIG_FASTTCP]) {
	int i=1;

#ifdef WIN32
	if (setsockopt(fd, SOL_TCP, TCP_NODELAY, ( const char* )&i, sizeof(i)) == -1)
	    perror("TCP_NODELAY");
#else
	if (setsockopt(fd, SOL_TCP, TCP_NODELAY, &i, sizeof(i)) == -1)
	    perror("TCP_NODELAY");
#endif
    }
#endif

    if (getsockopt(fd,SOL_SOCKET,SO_RCVBUF, (char*)&oldbufsize, &buflen)==-1)
        oldbufsize=0;

    if (oldbufsize<newbufsize) {
	if(setsockopt(fd,SOL_SOCKET,SO_RCVBUF, (char*)&newbufsize, sizeof(&newbufsize))) {
            LOG(LOG_WARNING,"common::init_connection: setsockopt"," unable to set output buf size to %d", newbufsize);
	    setsockopt(fd,SOL_SOCKET,SO_RCVBUF, (char*)&oldbufsize, sizeof(&oldbufsize));
	}
    }
    return fd;
}

/* This function negotiates/establishes the connection with the server.
 */
void negotiate_connection(int sound)
{
    int tries;

    SendVersion(csocket);

    /* We need to get the version command fairly early on because we need to
     * know if the server will support a request to use png images.  This
     * isn't done the best, because if the server never sends the version
     * command, we can loop here forever.  However, if it doesn't send the
     * version command, we have no idea what we are dealing with.
     */
    tries=0;
    while (csocket.cs_version==0) {
        DoClient(&csocket);
        if (csocket.fd == -1) return;

        usleep(10*1000);    /* 10 milliseconds */
        tries++;
        /* If we have't got a response in 10 seconds, bail out */
        if (tries > 1000) {
            close_server_connection();
            return;
        }
    }

    if (csocket.sc_version<1023) {
        LOG (LOG_WARNING,"common::negotiate_connection","Server does not support PNG images, yet that is all this client");
        LOG (LOG_WARNING,"common::negotiate_connection","supports.  Either the server needs to be upgraded, or you need to");
        LOG (LOG_WARNING,"common::negotiate_connection","downgrade your client.");
        exit(1);
    }

    /* If the user has specified a numeric face id, use it. If it is a string
     * like base, then that resolves to 0, so no real harm in that.
     */
    if (face_info.want_faceset) face_info.faceset = atoi(face_info.want_faceset);

    /* For spellmon, try each acceptable level, but make sure the one the
     * client prefers is last.
     */
    cs_print_string(csocket.fd,
                    "setup map2cmd 1 tick 1 sound2 %d darkness %d spellmon 1 spellmon 2 "
                    "faceset %d facecache %d want_pickup 1 loginmethod %d newmapcmd 1",
                    (sound>=0) ? 3 : 0, want_config[CONFIG_LIGHTING]?1:0,
                    face_info.faceset, want_config[CONFIG_CACHE], wantloginmethod);

    /* We can do this right now also - isn't any reason to wait */
    cs_print_string(csocket.fd, "requestinfo skill_info");
    cs_print_string(csocket.fd,"requestinfo exp_table");

    /* While these are only used for new login method, that
     * should hopefully become standard fairly soon, and
     * all of these are pretty small in any case, so don't
     * add much to the cost.  In this way, we are more likely
     * to have the information ready when we bring up the window.
     */
    cs_print_string(csocket.fd,"requestinfo motd");
    cs_print_string(csocket.fd,"requestinfo news");
    cs_print_string(csocket.fd,"requestinfo rules");

    use_config[CONFIG_MAPHEIGHT]=want_config[CONFIG_MAPHEIGHT];
    use_config[CONFIG_MAPWIDTH]=want_config[CONFIG_MAPWIDTH];
    mapdata_set_size(use_config[CONFIG_MAPWIDTH], use_config[CONFIG_MAPHEIGHT]);
    if (use_config[CONFIG_MAPHEIGHT]!=11 || use_config[CONFIG_MAPWIDTH]!=11)
        cs_print_string(csocket.fd,"setup mapsize %dx%d",use_config[CONFIG_MAPWIDTH], use_config[CONFIG_MAPHEIGHT]);

    use_config[CONFIG_SMOOTH]=want_config[CONFIG_SMOOTH];

    /* If the server will answer the requestinfo for image_info and image_data,
     * send it and wait for the response.
     */
    if (csocket.sc_version >= 1027) {
        /* last_start is -99.  This means the first face requested will be 1
         * (not 0) - this is OK because 0 is defined as the blank face.
         */
        int last_end=0, last_start=-99;

        cs_print_string(csocket.fd,"requestinfo image_info");
        requestinfo_sent = RI_IMAGE_INFO;
        replyinfo_status = 0;
        replyinfo_last_face = 0;

        do {
            DoClient(&csocket);

            /* it's rare, the connection can die while getting this info.
             */
            if (csocket.fd == -1) return;

            if (use_config[CONFIG_DOWNLOAD]) {
                /* we need to know how many faces to be able to make the
                 * request intelligently.  So only do the following block if
                 * we have that info.  By setting the sent flag, we will never
                 * exit this loop until that happens.
                 */
                requestinfo_sent |= RI_IMAGE_SUMS;
                if (face_info.num_images != 0) {
                    /* Sort of fake things out - if we have sent the request
                     * for image sums but have not got them all answered yet,
                     * we then clear the bit from the status so we continue to
                     * loop.
                     */
                    if (last_end == face_info.num_images) {
                        /* Mark that we're all done */
                        if (replyinfo_last_face == last_end) {
                            replyinfo_status |= RI_IMAGE_SUMS;
                            image_update_download_status(face_info.num_images, face_info.num_images, face_info.num_images);
                        }
                    } else {
                        /* If we are all caught up, request another 100 sums.
                         */
                        if (last_end <= (replyinfo_last_face+100)) {
                            last_start += 100;
                            last_end += 100;
                            if (last_end > face_info.num_images) last_end = face_info.num_images;
                            cs_print_string(csocket.fd,"requestinfo image_sums %d %d", last_start, last_end);
                            image_update_download_status(last_start, last_end, face_info.num_images);
                        }
                    }
                } /* Still have image_sums request to send */
            } /* endif download all faces */

            usleep(10*1000);    /* 10 milliseconds */
            /* Don't put in an upper time limit with tries like we did above -
             * if the player is downloading all the images, the time this
             * takes could be considerable.
             */

        } while (replyinfo_status != requestinfo_sent);
    }
    if (use_config[CONFIG_DOWNLOAD]) {
        char buf[MAX_BUF];

        snprintf(buf, sizeof(buf), "Download of images complete.  Found %d locally, downloaded %d from server\n",
                 face_info.cache_hits, face_info.cache_misses);
        draw_ext_info(NDI_GOLD, MSG_TYPE_CLIENT, MSG_TYPE_CLIENT_CONFIG, buf);
    }

    /* This needs to get changed around - we really don't want to send the
     * SendAddMe until we do all of our negotiation, which may include things
     * like downloading all the images and whatnot - this is more an issue if
     * the user is not using the default face set, as in that case, we might
     * end up building images from the wrong set.
     * Only run this if not using new login method
     */
    if (!serverloginmethod)
        SendAddMe(csocket);
}
