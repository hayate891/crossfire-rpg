/*
 * static char *rcsid_newsocket_c =
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

/* Made this either client or server specific for 0.95.2 release - getting
 * too complicated to keep them the same, and the common code is pretty much
 * frozen now.
 */


#include <stdio.h>
#include <stdarg.h>
#include <errno.h>

#include <client.h>
#include <newclient.h>

/* The LOG function is normally part of the libcross.  If client compile,
 * we need to supply something since some of the common code calls this.
 */
void LOG (int logLevel, char *format, ...)
{
#ifdef DEBUG
    va_list ap;
    va_start(ap, format);
    vfprintf(stderr, format, ap);
    va_end(ap);
#endif
}


/* We don't care what these values are in the client, since
 * we toss them
 */
#define llevDebug 0
#define llevError 0


/*
 * This writes data to the socket.
 */
static int write_socket(int fd, unsigned char *buf, int len)
{
    int amt=0;
    unsigned char *pos=buf;

    /* If we manage to write more than we wanted, take it as a bonus */
    while (len>0) {
	do {
	    amt=write(fd, pos, len);
	} while ((amt<0) && (errno==EINTR));

	if (amt < 0) { /* We got an error */
	    LOG(llevError,"New socket (fd=%d) write failed.\n", fd);
	    return -1;
	}
	if (amt==0) {
	    LOG(llevError,"Write_To_Socket: No data written out.\n");
	}
	len -= amt;
	pos += amt;
    }
    return 0;
}



void SockList_Init(SockList *sl, char *buf)
{
    sl->len=0;
    sl->buf=buf + 2;	/* reserve two bytes for total length */
}

void SockList_AddChar(SockList *sl, char c)
{
    sl->buf[sl->len++]=c;
}

void SockList_AddShort(SockList *sl, uint16 data)
{
    sl->buf[sl->len++] = (data>>8)&0xff;
    sl->buf[sl->len++] = data & 0xff;
}


void SockList_AddInt(SockList *sl, uint32 data)
{
    sl->buf[sl->len++] = (data>>24)&0xff;
    sl->buf[sl->len++] = (data>>16)&0xff;
    sl->buf[sl->len++] = (data>>8)&0xff;
    sl->buf[sl->len++] = data & 0xff;
}

void SockList_AddString(SockList *sl, const char *str)
{
    int len = strlen(str);

    if (sl->len + len > MAX_BUF-2)
	len = MAX_BUF-2 - sl->len;
    memcpy(sl->buf + sl->len, str, len);
    sl->len += len;
}

int SockList_Send(SockList *sl, int fd)
{
    sl->buf[-2] = sl->len / 256;
    sl->buf[-1] = sl->len % 256;

    return write_socket(fd, sl->buf-2, sl->len+2);
}


char GetChar_String(unsigned char *data)
{
    return (data[0]);
}
/* Basically does the reverse of SockList_AddInt, but on
 * strings instead.  Same for the GetShort, but for 16 bits.
 */
int GetInt_String(unsigned char *data)
{
    return ((data[0]<<24) + (data[1]<<16) + (data[2]<<8) + data[3]);
}

/* 64 bit version of the above */
sint64 GetInt64_String(unsigned char *data)
{
    return (((uint64)data[0]<<56) + ((uint64)data[1]<<48) + 
	    ((uint64)data[2]<<40) + ((uint64)data[3]<<32) +
	    (data[4]<<24) + (data[5]<<16) + (data[6]<<8) + data[7]);
}

short GetShort_String(unsigned char *data) {
    return ((data[0]<<8)+data[1]);
}

/* this readsfrom fd and puts the data in sl.  We return true if we think
 * we have a full packet, 0 if we have a partial packet.  The only processing
 * we do is remove the intial size value.  len (As passed) is the size of the
 * buffer allocated in the socklist.  We make the assumption the buffer is
 * at least 2 bytes long.
 */
 
int SockList_ReadPacket(int fd, SockList *sl, int len)
{
    int stat,toread,readsome=0;
    extern int errno;

    /* We already have a partial packet */
    if (sl->len<2) {
	do {
	    stat=read(fd, sl->buf + sl->len, 2-sl->len);
	} while ((stat==-1) && (errno==EINTR));
	if (stat<0) {
	    /* In non blocking mode, EAGAIN is set when there is no
	     * data available.
	     */
	    if (errno!=EAGAIN && errno!=EWOULDBLOCK) {
		perror("ReadPacket got an error.");
		LOG(llevDebug,"ReadPacket got error %d, returning -1",errno);
		return -1;
	    }
	    return 0;	/*Error */
	}
	if (stat==0) return -1;
	sl->len += stat;
#ifdef CS_LOGSTATS
	cst_tot.ibytes += stat;
	cst_lst.ibytes += stat;
#endif
	if (stat<2) return 0;	/* Still don't have a full packet */
	readsome=1;
    }
    /* Figure out how much more data we need to read.  Add 2 from the
     * end of this - size header information is not included.
     */
    toread = 2+(sl->buf[0] << 8) + sl->buf[1] - sl->len;
    if ((toread + sl->len) > len) {
	LOG(llevError,"SockList_ReadPacket: Want to read more bytes than will fit in buffer.\n");
	/* return error so the socket is closed */
	return -1;
    }
    do {
	do {
	    stat = read(fd, sl->buf+ sl->len, toread);
	} while ((stat<0) && (errno==EINTR));
	if (stat<0) {
	    if (errno!=EAGAIN && errno!=EWOULDBLOCK) {
		perror("ReadPacket got an error.");
		LOG(llevDebug,"ReadPacket got error %d, returning 0",errno);
	    }
	    return 0;	/*Error */
	}
	if (stat==0) return -1;
	sl->len += stat;
#ifdef CS_LOGSTATS
	cst_tot.ibytes += stat;
	cst_lst.ibytes += stat;
#endif
	toread -= stat;
	if (toread==0) return 1;
	if (toread < 0) {
	    LOG(llevError,"SockList_ReadPacket: Read more bytes than desired.");
	    return 1;
	}
    } while (toread>0);
    return 0;
}

/*
 * Send a printf-formatted packet to the socket.
 */
int cs_print_string(int fd, char *str, ...)
{
    va_list args;
    SockList sl;
    char buf[MAX_BUF];

    SockList_Init(&sl, buf);
    va_start(args, str);
    sl.len += vsprintf(sl.buf + sl.len, str, args);
    va_end(args);

    return SockList_Send(&sl, fd);
}
