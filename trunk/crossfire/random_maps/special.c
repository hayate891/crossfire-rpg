/*
 * static char *rcsid_special_c =
 *   "$Id$";
 */

/*
    CrossFire, A Multiplayer game for X-windows

    Copyright (C) 1994 Mark Wedel
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

    The author can be reached via e-mail to mark@pyramid.com
*/

/*  Specials in this file:
	 included maps */

#include <global.h>
#include <random_map.h>

#define NUM_OF_SPECIAL_TYPES 3
#define NO_SPECIAL 0
#define SPECIAL_SUBMAP 1
#define SPECIAL_FOUNTAIN 2

/* clear map completly of all objects:  a rectangular area of xsize, ysize
is cleared with the top left corner at xstart, ystart */

void nuke_map_region(mapstruct *map,int xstart,int ystart, int xsize, int ysize) {
  int i,j;
  object *tmp;
  for(i=xstart;i<xstart + xsize;i++) 
	 for(j=ystart;j<ystart +ysize;j++) {
		for(tmp=get_map_ob(map,i,j);tmp!=NULL;tmp=tmp->above) {
		  if(!QUERY_FLAG(tmp,FLAG_IS_FLOOR)) {
			 remove_ob(tmp);
			 free_object(tmp);
			 tmp=get_map_ob(map,i,j);
		  }
		}
	 }
}



/* copy in_map into dest_map at point x,y */ 


void include_map_in_map(mapstruct *dest_map, mapstruct *in_map,int x, int y) {
  int i,j;
  object *tmp;
  object *new_ob;
  
  /* First, splatter everything in the dest map at the location */
  nuke_map_region(dest_map,x,y,in_map->mapx,in_map->mapy);
		
  for(i=0;i<in_map->mapx;i++) 
	 for(j=0;j<in_map->mapy;j++) {
		for(tmp=get_map_ob(in_map,i,j);tmp!=NULL;tmp=tmp->above) {
		  /* don't copy things with multiple squares:  must be dealt with
			  specially. */
		  if(tmp->head!=NULL) continue;
		  new_ob = arch_to_object(tmp->arch);
		  copy_object(tmp,new_ob);
		  new_ob->x = i + x;
		  new_ob->y = j + y;
		  insert_multisquare_ob_in_map(new_ob,dest_map);
		}
	 }
}

int find_spot_for_submap(mapstruct *map,char **layout,int *ix, int *iy,int xsize, int ysize) {
  int tries;
  int i,j;
  int l,m;
  int is_occupied;
  /* don't even try to place a submap into a map if the big map isn't
	  sufficiently large. */
  if(3*xsize > map->mapx || 3*ysize > map->mapy) return 0;
  
  /* search a bit for a completely free spot. */
  for(tries=0;tries<10;tries++) {
	 /* pick a random location in the layout */
	 i = RANDOM() % (map->mapx - xsize-2)+1;
	 j = RANDOM() % (map->mapy - ysize-2)+1;
	 is_occupied=0;
	 for(l=i;l<i + xsize;l++)
		for(m=j;m<j + ysize;m++)
		  is_occupied|=layout[l][m];
	 if(!is_occupied) break;
  }

	 
  /* if we failed, relax the restrictions */
  
  if(is_occupied) { /* failure, try a relaxed placer. */
	 /* pick a random location in the layout */
	 for(tries=0;tries<10;tries++) {
		i = RANDOM() % (map->mapx - xsize-2)+1;
		j = RANDOM() % (map->mapy - ysize-2)+1;
		is_occupied=0;
		for(l=i;l<i + xsize;l++)
		  for(m=j;m<j + ysize;m++)
			 if(layout[l][m]=='C' || layout[l][m]=='>' || layout[l][m]=='<')
				is_occupied=1;
	 }
  }
  if(is_occupied) return 0;
  *ix=i;
  *iy=j;
  return 1;
}  


void place_fountain_with_specials(mapstruct *map) {
  int ix,iy,i=-1;
  mapstruct *fountain_style=find_style("/styles/misc","fountains",-1);
  object *fountain=get_archetype("fountain");
  object *potion=get_object();
  copy_object(pick_random_object(fountain_style),potion);
  while(i<0) {
	 ix = RANDOM() % (map->mapx -2) +1;
	 iy = RANDOM() % (map->mapx -2) +1;
	 i = find_first_free_spot(fountain->arch,map,ix,iy);
  };
  ix += freearr_x[i];
  iy += freearr_y[i];
  potion->face=fountain->face;
  SET_FLAG(potion,FLAG_NO_PICK);
  potion->name=add_string("fountain");
  potion->x = ix;
  potion->y = iy;
  potion->material=M_ADAMANT;
  fountain->x = ix;
  fountain->y = iy;
  insert_ob_in_map(fountain,map);
  insert_ob_in_map(potion,map);

}

		  
void place_specials_in_map(mapstruct *map, char **layout) {
  /* for now, simply include one of the special submaps */
  mapstruct *special_map;
  int ix,iy;  /* map insertion locatons */
  int special_type; /* type of special to make */

  
  special_type = RANDOM() % NUM_OF_SPECIAL_TYPES;
  switch(special_type) {

	 /* includes a special map into the random map being made. */
  case SPECIAL_SUBMAP: {
	 special_map = find_style("/styles/specialmaps",0,difficulty); 
	 if(special_map==NULL) return;
	 
	 if(find_spot_for_submap(map,layout,&ix,&iy,special_map->mapx,special_map->mapy)) 
		include_map_in_map(map,special_map,ix,iy);
	 break;
  }

  /* Make a special fountain:  an unpickable potion disguised as
	  a fountain, or rather, colocated with a fountain. */
  case SPECIAL_FOUNTAIN: {
	 place_fountain_with_specials(map);
	 break;
  }
  }
  
}
