/*
 * static char *rcsid_arch_c =
 *   "$Id$";
 */

/*
    CrossFire, A Multiplayer game for X-windows

    Copyright (C) 2000 Mark Wedel
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

#include <global.h>
#include <arch.h>
#include <funcpoint.h>
#include <loader.h>

/* IF set, does a little timing on the archetype load. */
#define TIME_ARCH_LOAD 0

static archetype *arch_table[ARCHTABLE];
int arch_cmp=0;		/* How many strcmp's */
int arch_search=0;	/* How many searches */
int arch_init;		/* True if doing arch initialization */
/*
 * Initialises the internal linked list of archetypes (read from file).
 * Then the global "empty_archetype" pointer is initialised.
 * Then the blocksview[] array is initialised.
 */

void init_archetypes() { /* called from add_player() and edit() */
  if(first_archetype!=NULL) /* Only do this once */
    return;
  arch_init = 1;
  load_archetypes();
  arch_init = 0;
  empty_archetype=find_archetype("empty_archetype");
/*  init_blocksview();*/
}

/*
 * Stores debug-information about how efficient the hashtable
 * used for archetypes has been in the static errmsg array.
 */

void arch_info(object *op) {
  sprintf(errmsg,"%d searches and %d strcmp()'s",arch_search,arch_cmp);
  (*draw_info_func)(NDI_BLACK, 0, op,errmsg);
}

/*
 * Initialise the hashtable used by the archetypes.
 */

void clear_archetable() {
  memset((void *) arch_table,0,ARCHTABLE*sizeof(archetype *));
}

/*
 * An alternative way to init the hashtable which is slower, but _works_...
 */

void init_archetable() {
  archetype *at;
  LOG(llevDebug," Setting up archetable...");
  for(at=first_archetype;at!=NULL;at=(at->more==NULL)?at->next:at->more)
    add_arch(at);
  LOG(llevDebug,"done\n");
}

/*
 * Dumps an archetype to debug-level output.
 */

void dump_arch(archetype *at) {
  dump_object(&at->clone);
}

/*
 * Dumps _all_ archetypes to debug-level output.
 * If you run crossfire with debug, and enter DM-mode, you can trigger
 * this with the O key.
 */

void dump_all_archetypes() {
  archetype *at;
  for(at=first_archetype;at!=NULL;at=(at->more==NULL)?at->next:at->more)
    dump_arch(at);
}

void free_all_archs()
{
    archetype *at, *next;
    int i=0,f=0;

    for (at=first_archetype; at!=NULL; at=next) {
#if 0	/* Right now, we could probably free the entries in the animation
	 * indexes.  However, we should really call a seperate function
	 * to do that.
	 */
	if (at->faces) {
		free(at->faces);
		f++;
	}
#endif
	if (at->more) next=at->more;
	else next=at->next;
	free(at);
	i++;
    }
    LOG(llevDebug,"Freed %d archetypes, %d faces\n", i, f);
}

/*
 * Allocates, initialises and returns the pointer to an archetype structure.
 */

archetype *get_archetype_struct() {
  archetype *new;

  new=(archetype *)CALLOC(1,sizeof(archetype));
  if(new==NULL)
    fatal(OUT_OF_MEMORY);
  new->next=NULL;
  new->name=NULL;
  new->clone.other_arch=NULL;
  new->clone.name=NULL;
  new->clone.title=NULL;
  new->clone.race=NULL;
  new->clone.slaying=NULL;
  new->clone.msg=NULL;
  clear_object(&new->clone);  /* to initial state other also */
  CLEAR_FLAG((&new->clone),FLAG_FREED); /* This shouldn't matter, since copy_object() */
  SET_FLAG((&new->clone), FLAG_REMOVED); /* doesn't copy these flags... */
  new->head=NULL;
  new->more=NULL;
  return new;
}

/*
 * Reads/parses the archetype-file, and copies into a linked list
 * of archetype-structures.
 */
void first_arch_pass(FILE *fp) {
  object *op;
  archetype *at,*prev=NULL,*last_more=NULL;
  int i,first=2;

  op=get_object();
  op->arch=first_archetype=at=get_archetype_struct();
  while((i=load_object(fp,op,first,0))) {
    first=0;
    copy_object(op,&at->clone);
    at->clone.speed_left= -0.1;
    switch(i) {
    case LL_NORMAL: /* A new archetype, just link it with the previous */
      if(last_more!=NULL)
        last_more->next=at;
      if(prev!=NULL)
        prev->next=at;
      prev=last_more=at;
      break;
#if 0
    case 2:
      LOG(llevError,"Error: Archetype with inventory is illegal!\n");
      exit(-1);
#endif
    case LL_MORE: /* Another part of the previous archetype, link it correctly */
      at->head=prev;
      at->clone.head = &prev->clone;
      if(last_more!=NULL) {
        last_more->more=at;
        last_more->clone.more = &at->clone;
      }
      last_more=at;
      break;
    }
    at=get_archetype_struct();
    clear_object(op);
    op->arch=at;
  }
  free(at);
  free_object(op);
}

/*
 * Reads the archetype file once more, and links all pointers between
 * archetypes.
 */

void second_arch_pass(FILE *fp) {
  char buf[MAX_BUF],*variable=buf,*argument,*cp;
  archetype *at=NULL,*other;

  while(fgets(buf,MAX_BUF,fp)!=NULL) {
    if(*buf=='#')
      continue;
    if((argument=strchr(buf,' '))!=NULL) {
	*argument='\0',argument++;
	cp = argument + strlen(argument)-1;
	while (isspace(*cp)) {
	    *cp='\0';
	    cp--;
	}
    }
    if(!strcmp("Object",variable)) {
      if((at=find_archetype(argument))==NULL)
        LOG(llevError,"Warning: failed to find arch %s\n",argument);
    } else if(!strcmp("other_arch",variable)) {
      if(at!=NULL&&at->clone.other_arch==NULL) {
        if((other=find_archetype(argument))==NULL)
          LOG(llevError,"Warning: failed to find other_arch %s\n",argument);
        else if(at!=NULL)
          at->clone.other_arch=other;
      }
    } else if(!strcmp("randomitems",variable)) {
      if(at!=NULL) {
        treasurelist *tl=find_treasurelist(argument);
        if(tl==NULL)
          LOG(llevError,"Failed to link treasure to arch: %s\n",variable);
        else
          at->clone.randomitems=tl;
      }
    }
  }
}

#ifdef DEBUG
void check_generators() {
  archetype *at;
  for(at=first_archetype;at!=NULL;at=at->next)
    if(QUERY_FLAG(&at->clone,FLAG_GENERATOR)&&at->clone.other_arch==NULL)
      LOG(llevError,"Warning: %s is generator but lacks other_arch.\n",
              at->name);
}
#endif

/*
 * First initialises the archtype hash-table (init_archetable()).
 * Reads and parses the archetype file (with the first and second-pass
 * functions).
 * Then initialises treasures by calling load_treasures().
 */

void load_archetypes() {
    FILE *fp;
    char filename[MAX_BUF];
    int comp;
#if TIME_ARCH_LOAD
    struct timeval tv1,tv2;
#endif

    sprintf(filename,"%s/%s",settings.datadir,settings.archetypes);
    LOG(llevDebug,"Reading archetypes from %s...\n",filename);
    if((fp=open_and_uncompress(filename,0,&comp))==NULL) {
	LOG(llevError," Can't open archetype file.\n");
	return;
    }
    clear_archetable();
    LOG(llevDebug," arch-pass 1...");
#if TIME_ARCH_LOAD
    gettimeofday(&tv1, NULL);
#endif
    first_arch_pass(fp);
#if TIME_ARCH_LOAD
    { int sec, usec;
	gettimeofday(&tv2, NULL);
	sec = tv2.tv_sec - tv1.tv_sec;
	usec = tv2.tv_usec - tv1.tv_usec;
	if (usec<0) { usec +=1000000; sec--;}
	LOG(llevDebug,"Load took %d.%06d seconds\n", sec, usec);
     }
#endif

  LOG(llevDebug," done\n");
  init_archetable();
  warn_archetypes=1;

  /* do a close and reopen instead of a rewind - necessary in case the
   * file has been compressed.
   */
  close_and_delete(fp, comp);
  fp=open_and_uncompress(filename,0,&comp);

  LOG(llevDebug," loading treasure...");
  load_treasures();
  LOG(llevDebug,"done\n arch-pass 2...");
  second_arch_pass(fp);
  LOG(llevDebug,"done\n");
#ifdef DEBUG
  check_generators();
#endif
  close_and_delete(fp, comp);
  LOG(llevDebug,"done\n");
}

/*
 * Creates and returns a new object which is a copy of the given archetype.
 * This function returns NULL on failure.
 */

object *arch_to_object(archetype *at) {
  object *op;
  if(at==NULL) {
    if(warn_archetypes)
      LOG(llevError,"Couldn't find archetype.\n");
    return NULL;
  }
  op=get_object();
  copy_object(&at->clone,op);
  op->arch=at;
  return op;
}

/*
 * Creates an object.  This function is called by get_archetype()
 * if it fails to find the appropriate archetype.
 * Thus get_archetype() will be guaranteed to always return
 * an object, and never NULL.
 */

object *create_singularity(char *name) {
  object *op;
  char buf[MAX_BUF];
  sprintf(buf,"singluarity (%s)",name);
  op = get_object();
  op->name = add_string(buf);
  SET_FLAG(op,FLAG_NO_PICK);
  return op;
}

/*
 * Finds which archetype matches the given name, and returns a new
 * object containing a copy of the archetype.
 */

object *get_archetype(char *name) {
  archetype *at;
  at = find_archetype(name);
  if (at == NULL)
    return create_singularity(name);
  return arch_to_object(at);
}

/*
 * Hash-function used by the arch-hashtable.
 */

unsigned long
hasharch(char *str, int tablesize) {
    unsigned long hash = 0;
    int i = 0, rot = 0;
    char *p;

    for (p = str; i < MAXSTRING && *p; p++, i++) {
        hash ^= (unsigned long) *p << rot;
        rot += 2;
        if (rot >= (sizeof(long) - sizeof(char)) * 8)
            rot = 0;
    }
    return (hash % tablesize);
}

/*
 * Finds, using the hashtable, which archetype matches the given name.
 * returns a pointer to the found archetype, otherwise NULL.
 */

archetype *find_archetype(char *name) {
  archetype *at;
  unsigned long index;

  if (name == NULL)
    return (archetype *) NULL;

  index=hasharch(name, ARCHTABLE);
  arch_search++;
  for(;;) {
    at = arch_table[index];
    if (at==NULL) {
      if(warn_archetypes)
        LOG(llevError,"Couldn't find archetype %s\n",name);
      return NULL;
    }
    arch_cmp++;
    if (!strcmp(at->name,name))
      return at;
    if(++index>=ARCHTABLE)
      index=0;
  }
}

/*
 * Adds an archetype to the hashtable.
 */

void add_arch(archetype *at) {
  int index=hasharch(at->name, ARCHTABLE),org_index=index;
  for(;;) {
    if(arch_table[index]==NULL) {
      arch_table[index]=at;
      return;
    }
    if(++index==ARCHTABLE)
      index=0;
    if(index==org_index)
      fatal(ARCHTABLE_TOO_SMALL);
  }
}

/*
 * Returns the first archetype using the given type.
 * Used in treasure-generation.
 */

archetype *type_to_archetype(int type) {
  archetype *at;

  for(at=first_archetype;at!=NULL;at=(at->more==NULL)?at->next:at->more)
    if(at->clone.type==type)
      return at;
  return NULL;
}

/*
 * Returns a new object copied from the first archetype matching
 * the given type.
 * Used in treasure-generation.
 */

object *clone_arch(int type) {
  archetype *at;
  object *op=get_object();

  if((at=type_to_archetype(type))==NULL) {
    LOG(llevError,"Can't clone archetype %d\n",type);
    free_object(op);
    return NULL;
  }
  copy_object(&at->clone,op);
  return op;
}

/*
 * member: make instance from class
 */

object *ObjectCreateArch (archetype * at)
{
    object *op, *prev = NULL, *head = NULL;

    while (at) {
        op = arch_to_object (at);
        op->x = at->clone.x;
        op->y = at->clone.y;
        if (head)
            op->head = head, prev->more = op;
        if (!head)
            head = op;
        prev = op;
        at = at->more;
    }
    return (head);
}

/*** end of arch.c ***/
