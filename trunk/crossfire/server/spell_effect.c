/*
 * static char *rcsid_spell_effect_c =
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
#include <object.h>
#include <living.h>
#ifndef __CEXTRACT__
#include <sproto.h>
#endif
#include <spells.h>
#include <sounds.h>

extern object *objects;

/* 
 * spell_failure()  handles the various effects for differing degrees
 * of failure badness.
 */
#ifdef SPELL_FAILURE_EFFECTS

/* Might be a better way to do this, but this should work */
#define ISQRT(val) ((int)sqrt((double) val))

void spell_failure(object *op, int failure,int power)
{  
    if(failure<= -20&&failure > -40) /* wonder */
    {
	new_draw_info(NDI_UNIQUE, 0,op,"Your spell causes an unexpected effect.");
	cast_cone(op,op,0,10,SP_WOW,spellarch[SP_WOW],0);
    }

    else if (failure <= -40&&failure > -60) /* confusion */
    {
	new_draw_info(NDI_UNIQUE, 0,op,"Your magic recoils on you!");
	confuse_player(op,op,99);
    }
    else if (failure <= -60&&failure> -80) /* paralysis */
    {
	new_draw_info(NDI_UNIQUE, 0,op,"Your magic recoils on you!");
	paralyze_player(op,op,99);
    }
    else if (failure <= -80) /* blast the immediate area */
    { 
	object *tmp;
	/* Safety check to make sure we don't get any mana storms in scorn */
	if (blocks_magic(op->map, op->x, op->y)) {
	    new_draw_info(NDI_UNIQUE, 0, op, "The magic warps and you are turned inside out!");
	    hit_player(tmp,9999,op,AT_INTERNAL);

	} else {
	    new_draw_info(NDI_UNIQUE, 0,op,"You lose control of the mana!  The uncontrolled magic blasts you!");
	    tmp=get_archetype("loose_magic");
	    tmp->level=SK_level(op);
	    tmp->x=op->x;tmp->y=op->y;

	    /* increase the area of destruction a little for more powerful spells */
	    tmp->stats.hp+=ISQRT(power);

	    if (power>25) tmp->stats.dam = 25 + ISQRT(power);
	    else tmp->stats.dam=power; /* nasty recoils! */

	    tmp->stats.maxhp=tmp->count; /*??*/
	    insert_ob_in_map(tmp,op->map,NULL);
	}
    }
}
#endif

/* Oct 95 - hacked on this to bring in cosmetic differences for MULTIPLE_GOD hack -b.t. */

void prayer_failure(object *op, int failure,int power)
{  
#ifdef MULTIPLE_GODS 
  char *godname;

  if(!strcmp((godname=determine_god(op)),"none")) godname="Your spirit";
#endif 

  if(failure<= -20&&failure > -40) /* wonder */
    {
#ifdef MULTIPLE_GODS 
     new_draw_info_format(NDI_UNIQUE, 0,op,"%s gives a sign to renew your faith.",godname);
#else
     new_draw_info(NDI_UNIQUE, 0,op,"God gives a sign to renew your faith.");
#endif
     cast_cone(op,op,0,10,SP_WOW,spellarch[SP_WOW],0);
    }

  else if (failure <= -40&&failure > -60) /* confusion */
   {
    new_draw_info(NDI_UNIQUE, 0,op,"Your diety touches your mind!");
    confuse_player(op,op,99);
   }
  else if (failure <= -60&&failure> -150) /* paralysis */
  {
#ifdef MULTIPLE_GODS
    new_draw_info_format(NDI_UNIQUE, 0,op,"%s requires you to pray NOW.",godname);
#else
    new_draw_info(NDI_UNIQUE, 0,op,"Your god requires you to pray NOW.");
#endif
	 new_draw_info(NDI_UNIQUE, 0,op,"You comply, ignoring all else.");
    paralyze_player(op,op,99);
  }
  else if (failure <= -150) /* blast the immediate area */
  { 
#ifdef MULTIPLE_GODS
   new_draw_info_format(NDI_UNIQUE, 0,op,"%s smites you!",godname);
#else
   new_draw_info(NDI_UNIQUE, 0,op,"God smites you!");
#endif
   
   cast_magic_storm(op,get_archetype("god_power"), power);

  }
}

/* Should really just replace all calls to cast_mana_storm to call
 * cast_magic_storm directly.
 */

void cast_mana_storm(object *op, int lvl) {
    object *tmp=get_archetype("loose_magic"); 

    cast_magic_storm(op, tmp, lvl);
}

    
void cast_magic_storm(object *op, object *tmp, int lvl) {

    if (!tmp) return;	/* error */
    tmp->level=SK_level(op);
    tmp->x=op->x;tmp->y=op->y; 
    tmp->stats.hp+=lvl/5;  /* increase the area of destruction */
    tmp->stats.dam=lvl; /* nasty recoils! */ 
    tmp->stats.maxhp=tmp->count; /*??*/ 
    insert_ob_in_map(tmp,op->map,op); 

}

void aggravate_monsters(object *op) {
  int i,j;
  object *tmp;

  spell_effect(SP_AGGRAVATION, op->x, op->y, op->map, op);

  for (i = 0; i < op->map->mapx; i++)
    for (j = 0; j < op->map->mapy; j++) {
      if (out_of_map(op->map, op->x + i , op->y + j))
        continue;
      for (tmp = get_map_ob(op->map, op->x + i, op->y + j);
           tmp; tmp = tmp->above)
        if (QUERY_FLAG(tmp, FLAG_MONSTER)) {
	  CLEAR_FLAG(tmp, FLAG_SLEEP);
          if (!QUERY_FLAG(tmp, FLAG_FRIENDLY))
            tmp->enemy = op;
        }
    }
}


int recharge(object *op) {
  object *wand;

  for(wand = op->inv; wand != NULL; wand = wand->below)
    if(wand->type == WAND && QUERY_FLAG(wand, FLAG_APPLIED))
      break;
  if(wand == NULL)
    return 0;
  if(!(RANDOM()%4)) {
    new_draw_info_format(NDI_UNIQUE, 0, op,
	"The %s vibrates violently, then explodes!",query_name(wand));
    play_sound_map(op->map, op->x, op->y, SOUND_OB_EXPLODE);
    spell_effect(SP_DESTRUCTION, op->x, op->y, op->map, op);
    remove_ob(wand);
    free_object(wand);
    return 1;
  }
  new_draw_info_format(NDI_UNIQUE, 0, op,
	"The %s glows with power.",query_name(wand));

  wand->stats.food += RANDOM()%spells[wand->stats.sp].charges + 1;
  if(wand->arch&&QUERY_FLAG(&wand->arch->clone, FLAG_ANIMATE))
  {
    SET_FLAG(wand, FLAG_ANIMATE);
    wand->speed = wand->arch->clone.speed;
    update_ob_speed(wand);
  }
  return 1;
}


/******************************************************************************
 * Start of polymorph related functions.
 *
 * Changed around for 0.94.3 - it will now look through and use all the
 * possible choices for objects/monsters (before it was teh first 80 -
 * arbitrary hardcoded limit in this file.)  Doing this will be a bit
 * slower however - while before, it traversed the archetypes once and
 * stored them into an array, it will now potentially traverse it
 * an average of 1.5 times.  This is probably more costly on the polymorph
 * item function, since it is possible a couple lookups might be needed before
 * an item of proper value is generated.
 */

/* polymorph_living - takes a living object (op) and turns it into 
 * another monster of some sort.  Currently, we only deal with single
 * space monsters.
 */

void polymorph_living(object *op) {
    archetype *at;
    int nr = 0, x = op->x, y = op->y, numat=0, choice,friendly;
    mapstruct *map = op->map;
    object *tmp, *next, *owner;

    if(op->head != NULL || op->more != NULL)
	return;

    /* First, count up the number of legal matches */
    for(at = first_archetype ; at != NULL; at = at->next)
	if(QUERY_FLAG((&at->clone),FLAG_MONSTER) == QUERY_FLAG(op, FLAG_MONSTER) &&
	   QUERY_FLAG((&at->clone),FLAG_GENERATOR) == QUERY_FLAG(op,FLAG_GENERATOR) &&
	   at->more == NULL && EDITABLE((&at->clone)))
    {
	numat++;
    }
    if (!numat) return;	/* no valid matches? if so, return */

    /* Next make a choice, and loop through until we get to it */
    choice = RANDOM()%numat;
    for(at = first_archetype ; at != NULL; at = at->next)
	if(QUERY_FLAG((&at->clone),FLAG_MONSTER) == QUERY_FLAG(op, FLAG_MONSTER) &&
	   QUERY_FLAG((&at->clone),FLAG_GENERATOR) == QUERY_FLAG(op,FLAG_GENERATOR) &&
	   at->more == NULL && EDITABLE((&at->clone)))
    {
	if (!choice) break;
	else choice--;
    }

    /* Look through the monster.  Unapply anything they have applied, 
     * and remove any abilities.
     */
    for(tmp = op->inv; tmp != NULL; tmp = next) {
	next = tmp->below;
	if(QUERY_FLAG(tmp, FLAG_APPLIED))
	    manual_apply(op,tmp,0);
	if(tmp->type == ABILITY) {
	    remove_ob(tmp);
	    free_object(tmp);
	}
    }

    /* Remove the object, preserve some values for the new object */
    remove_ob(op);
    owner = get_owner(op);
    friendly = QUERY_FLAG(op, FLAG_FRIENDLY);
    if (friendly)
	remove_friendly_object(op);

    copy_object(&(at->clone),op);
    if (owner != NULL)
	set_owner(op,owner);
    if (friendly) {
	SET_FLAG(op, FLAG_FRIENDLY);
	op->move_type = PETMOVE;
	add_friendly_object(op);
    }

    /* Put the new creature on the map */
    op->x = x; op->y = y;
    if ((op = insert_ob_in_map (op, map, owner)) == NULL)
        return;

    if(op->randomitems != NULL)
	create_treasure(op->randomitems,op,GT_INVISIBLE,map->difficulty,0);

    /* Apply any objects.  This limits it to the first 20 items, which
     * I guess is reasonable.
     */
    for(tmp = op->inv, nr = 0; tmp != NULL && ++nr < 20; tmp = next) {
	next = tmp->below;
	(void) monster_check_apply(op,tmp);
    }
}


/* polymorph_melt Destroys item from polymorph failure 
 * who is the caster of the polymorph, op is the
 * object destroyed.  We should probably do something
 * more clever ala nethack - create an iron golem or
 * something.
 */
void polymorph_melt(object *who, object *op)
{

    new_draw_info_format(NDI_UNIQUE, 0, who,
	"%s%s glows red, melts and evaporates!",
            op->nrof?"":"The ",query_name(op));
    play_sound_map(op->map, op->x, op->y, SOUND_OB_EVAPORATE);
    remove_ob(op);
    free_object(op);
    return;
}

/* polymorph_item - changes an item to another item of similar type.
 * who is the caster of spell, op is the object to be changed.
 */

void polymorph_item(object *who, object *op) {
    archetype *at;
    int max_value, difficulty, tries=0,choice, charges=op->stats.food,numat=0;
    object *new_ob;


    /* We try and limit the maximum value of the changd object. */
    max_value = op->value * 2;
    if(max_value > 20000)
	max_value = 20000 + (max_value - 20000) / 2;

    /* Look through and try to find matching items.  Can't turn into something
     * invisible.  Also, if the value is too high now, it would almost
     * certainly be too high below.
     */
    for(at = first_archetype ; at != NULL; at = at->next) {
	if(at->clone.type == op->type && !at->clone.invisible && 
	   at->clone.value < max_value &&
	   !QUERY_FLAG(&at->clone, FLAG_NO_DROP))
		numat++;
    }

    if(!numat)
	return;

    difficulty = op->magic * 5;
    if (difficulty<0) difficulty=0;
    new_ob = get_object();
    do {
	choice = RANDOM()%numat;
	for(at = first_archetype ; at != NULL; at = at->next) {
	    if(at->clone.type == op->type && !at->clone.invisible && 
	       at->clone.value < max_value && 
	       !QUERY_FLAG(&at->clone, FLAG_NO_DROP)) {
		if (!choice) break;
		else choice--;
	    }
	}
	copy_object(&(at->clone),new_ob);
	fix_generated_item(new_ob,op,difficulty,FABS(op->magic));
	++tries;
    } while (new_ob->value > max_value && tries<10);
    if (new_ob->invisible) {
	LOG(llevError,"polymorph_item: fix_generated_object made %s invisible?!\n", new_ob->name);
    }

    /* Unable to generate an acceptable item?  Melt it */
    if (tries==10) {
	polymorph_melt(who, op);
	free_object(new_ob);
	return;
    }

    if(op->nrof && new_ob->nrof) {
	new_ob->nrof = op->nrof;
	/* decrease the number of items */
	if (new_ob->nrof>2) new_ob->nrof -= RANDOM() % (op->nrof/2);
    }

    /* We don't want rings to keep sustenance/hungry status. There are propably
       other cases too that should be checked. */
    if(charges && op->type != RING && op->type != FOOD)
	op->stats.food = charges;

    new_ob->x = op->x;
    new_ob->y = op->y;
    remove_ob(op);
    free_object(op);
    /* insert_ob_in_map_simple is caused so that objects don't get
     * merged/rearranged.
     */
    insert_ob_in_map_simple(new_ob,who->map);
}

/* polymorh - caster who has hit object op. */
void polymorph(object *op, object *who) {

    int tmp;

    /* Can't polymorph players right now */
    if(op->type == PLAYER)
	return;

    if(QUERY_FLAG(op, FLAG_MONSTER) || QUERY_FLAG(op,FLAG_GENERATOR)) {
	polymorph_living(op);
	return;
    }
    /* If it is a living object of some other type, don't handle
     * it now.
     */
    if(QUERY_FLAG(op, FLAG_ALIVE))
	return;

    /* Don't want to morph flying arrows, etc... */
    if(FABS(op->speed) > 0.001 && !QUERY_FLAG(op, FLAG_ANIMATE))
	return;

    /* Do some sanity checking here.  type=0 is unkown, objects
     * without archetypes are not good.  As are a few other
     * cases.
     */
    if(op->type == 0 || op->arch == NULL || 
       QUERY_FLAG(op,FLAG_NO_PICK) 
       || QUERY_FLAG(op, FLAG_NO_PASS) || op->type == TREASURE)
	return;

    tmp = RANDOM() % 8;
    if (tmp) polymorph_item(who, op);
    else polymorph_melt(who, op);
}


/* cast_polymorph - object *op has cast it
 *dir is the direction.
 * Returns 0 on illegal cast, otherwise 1.
 */

int cast_polymorph(object *op, int dir) {
    object *tmp, *next;
    int range;
    archetype *poly;

    if(dir == 0)
	return 0;

    poly = find_archetype("polymorph");
    /* Range is until it hits a wall */
    for(range = 1;;range++) {
	int x=op->x+freearr_x[dir]*range,y=op->y+freearr_y[dir]*range;
	object *image;

	if(wall(op->map,x,y) || blocks_magic(op->map,x,y))
	    break;

	/* Get the top most object */
	for(tmp = get_map_ob(op->map,x,y); tmp != NULL && tmp->above != NULL;
	    tmp = tmp->above);

	/* Now start polymorphing the objects, top down */
	while (tmp!=NULL) {
	    /* Once we find the floor, no need to go further */
	    if (QUERY_FLAG(tmp, FLAG_IS_FLOOR)) break;
	    next = tmp->below;
	    polymorph(tmp, op);
	    tmp = next;
	}
	image = arch_to_object(poly);
	image->x = x; image->y = y;
	/* I am not sure what this below does with the
	 * range - msw 7/4/1998
	 */
	image->stats.food += range;
	image->speed_left = 0.1;
	insert_ob_in_map(image,op->map,op);
    }
    return 1;
}


/*  allows the choice of what sort of food object to make.
    If stringarg is NULL, it will create food dependent on level  --PeterM*/
int cast_create_food(object *op,object *caster, int dir, char *stringarg)
{
  int food_value;
  archetype *at=NULL;
  object *new_op;

  food_value=SP_PARAMETERS[SP_CREATE_FOOD].bdam
              + 50 * SP_level_dam_adjust(op,caster,SP_CREATE_FOOD);

  if(stringarg) {
	at=find_archetype(stringarg);
	if (at==NULL || ((at->clone.type != FOOD && at->clone.type != DRINK)
	    || (at->clone.stats.food > food_value)))
		stringarg = NULL;
  }
  
  if(!stringarg) {
	archetype *at_tmp;
	/* We try to find the archetype with the maximum food value.
	 * This removes the dependancy of hard coded food values in this
	 * function, and addition of new food types is automatically added.
	 * We don't use flesh types because the weight values of those need
	 * to be altered from the donor.
	 */

	/* We assume the food items don't have multiple parts */
	for (at_tmp=first_archetype; at_tmp!=NULL; at_tmp=at_tmp->next) {
	    if (at_tmp->clone.type==FOOD || at_tmp->clone.type==DRINK) {
		/* Basically, if the food value is something that is creatable
		 * under the limits of the spell and it is higher than
		 * the item we have now, take it instead.
		 */
		if (at_tmp->clone.stats.food<=food_value && 
		    (!at || at_tmp->clone.stats.food>at->clone.stats.food))
			at=at_tmp;
	    }
	}
  }
  /* Pretty unlikely (there are some very low food items), but you never
   * know
   */
  if (!at) {
    new_draw_info(NDI_UNIQUE, 0, op, "You don't have enough experience to create any food.");
    return 0;
  }

  food_value/=at->clone.stats.food;
  new_op = get_object();
  copy_object(&at->clone, new_op);
  new_op->nrof = food_value;
  new_op->value = 0;
  SET_FLAG(new_op, FLAG_STARTEQUIP);
  if (new_op->nrof<1) new_op->nrof = 1;

  cast_create_obj(op, caster,new_op, dir);
  return 1;
}
  
  
int cast_speedball(object *op, int dir, int type) {
  object *spb;
  if(blocked(op->map,op->x+freearr_x[dir],op->y+freearr_y[dir]))
    return 0;
  spb=clone_arch(SPEEDBALL);
  spb->x=op->x+freearr_x[dir],spb->y=op->y+freearr_y[dir];
  spb->speed_left= -0.1;
  if(type==SP_LARGE_SPEEDBALL)
    spb->stats.dam=30;
  insert_ob_in_map(spb,op->map,op);
  return 1;
}

int probe(object *op, int dir) {
  int r;
  object *tmp;

  if(!dir) {
    examine_monster(op,op);
    return 1;
  }
  for(r=1;;r++) {
    int x=op->x+r*freearr_x[dir],y=op->y+r*freearr_y[dir];
    if(out_of_map(op->map,x,y))
      break;
    if(blocks_magic(op->map,x,y)) {
      new_draw_info(NDI_UNIQUE, 0,op,"Something blocks your magic.");
      return 0;
    }
    for(tmp=get_map_ob(op->map,x,y);tmp!=NULL;tmp=tmp->above)
      if(QUERY_FLAG(tmp, FLAG_ALIVE)&&(tmp->type==PLAYER||QUERY_FLAG(tmp, FLAG_MONSTER))) {
        new_draw_info(NDI_UNIQUE, 0,op,"You detect something.");
        if(tmp->head!=NULL)
          tmp=tmp->head;
        examine_monster(op,tmp);
        return 1;
      }
  }
  new_draw_info(NDI_UNIQUE, 0,op,"You detect nothing.");
  return 1;
}

int cast_invisible(object *op, object *caster, int spell_type) {
  object *tmp;

  if(op->invisible>1000) {
    new_draw_info(NDI_UNIQUE, 0,op,"You are already as invisible as you can get.");
    return 0;
  }
  switch(spell_type) {
  case SP_INVIS:
    CLEAR_FLAG(op, FLAG_UNDEAD);
    op->invisible+=SP_PARAMETERS[spell_type].bdur;  /* set the base */
    op->invisible+=SP_PARAMETERS[spell_type].ldam *
                  SP_level_strength_adjust(op,caster,spell_type);  /*  set the level bonus */
    if(op->type==PLAYER)
      op->contr->tmp_invis=1;
    break;
  case SP_INVIS_UNDEAD:
    SET_FLAG(op, FLAG_UNDEAD);
    op->invisible+=SP_PARAMETERS[spell_type].bdur;  /* set the base */
    op->invisible+=SP_PARAMETERS[spell_type].ldam *
                  SP_level_strength_adjust(op,caster,spell_type);  /*  set the level bonus */

    if(op->type==PLAYER)
      op->contr->tmp_invis=1;
    break;
  case SP_IMPROVED_INVIS:
    op->invisible+=SP_PARAMETERS[spell_type].bdur;  /* set the base */
    op->invisible+=SP_PARAMETERS[spell_type].ldam *
                  SP_level_strength_adjust(op,caster,spell_type);  /*  set the level bonus */
    break;
  }
  new_draw_info(NDI_UNIQUE, 0,op,"You can't see your hands!");
  update_object(op);
  for (tmp = objects; tmp != NULL; tmp = tmp->next)
    if (tmp->enemy == op)
      tmp->enemy = NULL;
  return 1;
}


int 
cast_earth2dust(object *op,object *caster) {
  object *tmp, *next;
  int strength,i,j;

  if(op->type!=PLAYER)
    return 0;
  strength=SP_PARAMETERS[SP_EARTH_DUST].bdur + SP_level_strength_adjust(op,caster,SP_EARTH_DUST);
  strength=(strength>15)?15:strength;
  for(i= -strength;i<strength;i++)
    for(j= -strength;j<strength;j++) {
      if(out_of_map(op->map,op->x+i,op->y+j))
        continue;
      for(tmp=get_map_ob(op->map,op->x+i,op->y+j);tmp!=NULL;tmp=next) {
        next=tmp->above;
        if(tmp&&QUERY_FLAG(tmp, FLAG_TEAR_DOWN))
          hit_player(tmp,9999,op,AT_PHYSICAL);
      }
    }
  return 1;
}

/* puts a 'WORD_OF_RECALL_' object in player */
/* modified to work faster for higher level casters -- DAMN		*/
int cast_wor(object *op, object *caster) {
  object *dummy;

  if(op->type!=PLAYER)
    return 0;
  if(blocks_magic(op->map,op->x,op->y)) {
    new_draw_info(NDI_UNIQUE, 0,op,"Something blocks your spell.");
    return 0;
  }
  dummy=get_archetype("force");
  if(dummy == NULL){
    new_draw_info(NDI_UNIQUE, 0,op,"Oops, program error!");
    LOG(llevError,"get_object failed!\n");
    return 0;
  }
  if(op->owner) op=op->owner; /* better insert the spell in the player */
  dummy->speed = 0.002 * ((float)(SP_PARAMETERS[SP_WOR].bdur
				 + SP_level_strength_adjust(op,caster,SP_WOR)));
  update_ob_speed(dummy);
  dummy->speed_left= -1;
  dummy->type=WORD_OF_RECALL;
  EXIT_PATH(dummy)=add_string(first_map_path);
  (void) insert_ob_in_ob(dummy,op);
  new_draw_info(NDI_UNIQUE, 0,op,"You feel a force starting to build up inside you.");
  LOG(llevDebug,"Word of Recall for %s in %f ticks.\n", op->name,
      ((-dummy->speed_left)/(dummy->speed==0?0.0001:dummy->speed)));
  LOG(llevDebug,"Word of Recall for player level %d, caster level %d: 0.002 * %d + %d\n",
      SK_level(op), SK_level(caster), SP_PARAMETERS[SP_WOR].bdur, SP_level_strength_adjust(op,caster,SP_WOR));
  return 1;
}

int cast_wow(object *op, int dir, int ability, SpellTypeFrom item) {
  int sp;
  if(!(RANDOM()%4))
    return cast_cone(op,op,0,10,SP_WOW,spellarch[SP_WOW],0);
  do
    sp=RANDOM()%NROFREALSPELLS;
  while (!spells[sp].books);
  return cast_spell(op,op,dir,sp,ability,item,NULL);
}

int perceive_self(object *op) {
    char *cp=describe_item(op), buf[MAX_BUF];
    archetype *at=find_archetype("depletion");
    object *tmp;
    int i;

    tmp=find_god(determine_god(op));
    if (tmp) 
	new_draw_info_format(NDI_UNIQUE, 0, op, "You worship %s", tmp->name);
    else
	new_draw_info(NDI_UNIQUE, 0,op,"You worship no god");

    tmp=present_arch_in_ob(at,op);

    if(*cp=='\0' && tmp==NULL)
	new_draw_info(NDI_UNIQUE, 0,op,"You feel very mundane");
    else {
	new_draw_info(NDI_UNIQUE, 0,op,"You have:");
	new_draw_info(NDI_UNIQUE, 0,op,cp);
	if (tmp!=NULL) {
	    for (i=0; i<7; i++) {
		if (get_attr_value(&tmp->stats, i)<0) {
		    sprintf(buf,"Your %s is depleted by %d", statname[i],
			    -(get_attr_value(&tmp->stats,i)));
		    new_draw_info(NDI_UNIQUE, 0,op, buf);
		}
	    }
	}
    }
    return 1;
}

int cast_destruction(object *op, object *caster, int dam, int attacktype) {
  int i,j;
  int r;    /*  peterm:  added to make area of effect level dep.  */
  object *tmp;
  if(op->type!=PLAYER)
    return 0;
  r=5 + SP_level_strength_adjust(op,caster,SP_DESTRUCTION);
  dam+=SP_level_dam_adjust(op,caster,SP_DESTRUCTION);
  for(i= -r;i<r;i++)
    for(j= -r;j<r;j++) {
      if(out_of_map(op->map,op->x+i,op->y+j))
        continue;
      tmp=get_map_ob(op->map,op->x+i,op->y+j);
      while(tmp!=NULL&&(!QUERY_FLAG(tmp, FLAG_ALIVE)||tmp->type==PLAYER))
        tmp=tmp->above;
      if(tmp==NULL)
        continue;
      hit_player(tmp,dam,op,attacktype);
    }
  return 1;
}

int magic_wall(object *op,object *caster,int dir,int spell_type) {
  object *tmp;
  if(!dir) {
    new_draw_info(NDI_UNIQUE, 0,op,"In what direction?");
    return 0;
  }
  if(blocked(op->map,op->x+freearr_x[dir],op->y+freearr_y[dir])) {
    new_draw_info(NDI_UNIQUE, 0,op,"Something is in the way.");
    return 0;
  }
  switch(spell_type) {
  case SP_EARTH_WALL:
    tmp=get_archetype("earthwall");
    tmp->immune=0,tmp->protected=0;
    tmp->stats.hp = SP_PARAMETERS[spell_type].bdur +
                  10* SP_level_strength_adjust(op,caster,spell_type);
	/* More solid, since they can be torn down */
    tmp->stats.maxhp = tmp->stats.hp;
    break;
  case SP_FIRE_WALL:
    tmp=get_archetype("firebreath");
    tmp->attacktype |= AT_MAGIC;
    tmp->stats.hp=SP_PARAMETERS[spell_type].bdur
              + 5*SP_level_strength_adjust(op,caster,spell_type);
    tmp->stats.dam=SP_PARAMETERS[spell_type].bdam
                  +SP_level_dam_adjust(op,caster,spell_type);
    tmp->stats.food=1;                        /* so it doesn't propagate */
#if 0
    SET_SLOW_MOVE(tmp);
    SET_SLOW_PENALTY(tmp,2);
#endif
    SET_FLAG(tmp, FLAG_WALK_ON);
    SET_FLAG(tmp, FLAG_FLY_ON);
    set_owner(tmp,op);
    break;
  case SP_FROST_WALL:
    tmp=get_archetype("icestorm");
    tmp->attacktype |= AT_MAGIC;
    tmp->stats.hp=SP_PARAMETERS[spell_type].bdur
              + 5*SP_level_strength_adjust(op,caster,spell_type);
    tmp->stats.dam=SP_PARAMETERS[spell_type].bdam
                  +SP_level_dam_adjust(op,caster,spell_type);

    tmp->stats.food=1;                        /* so it doesn't propagate */
#if 0
    SET_SLOW_MOVE(tmp);
    SET_SLOW_PENALTY(tmp,3);
#endif
    SET_FLAG(tmp, FLAG_WALK_ON);
    SET_FLAG(tmp, FLAG_FLY_ON);
    set_owner(tmp,op);
    break;
  case SP_WALL_OF_THORNS:
    tmp=get_archetype("thorns");
    tmp->stats.hp=SP_PARAMETERS[spell_type].bdur
              + 3*SP_level_strength_adjust(op,caster,spell_type);
    tmp->stats.dam=SP_PARAMETERS[spell_type].bdam
                  +SP_level_dam_adjust(op,caster,spell_type);
    SET_FLAG(tmp, FLAG_WALK_ON);
    /* SET_FLAG(tmp, FLAG_FLY_ON); */
    /* SET_FLAG(tmp, FLAG_BLOCKSVIEW); */ 
    set_owner(tmp,op);
    break;
  case SP_CHAOS_POOL:
    tmp=get_archetype("color_spray");
    tmp->attacktype|=AT_MAGIC;
    tmp->stats.hp=SP_PARAMETERS[spell_type].bdur
              + 5*SP_level_strength_adjust(op,caster,spell_type);
    tmp->stats.dam=SP_PARAMETERS[spell_type].bdam
                  +SP_level_dam_adjust(op,caster,spell_type);

    tmp->stats.food=1;  /* so the color spray object won't propagate */
    SET_FLAG(tmp, FLAG_WALK_ON);
    SET_FLAG(tmp, FLAG_FLY_ON);
    set_owner(tmp,op);
    break;
  case SP_DARKNESS:
    tmp=get_archetype("darkness");
    tmp->stats.food = SP_PARAMETERS[SP_DARKNESS].bdur
                      + SP_level_strength_adjust (op, caster, SP_DARKNESS);
    break; 
  case SP_COUNTERWALL:
    tmp=get_archetype("counterspell");
    tmp->attacktype|=AT_MAGIC;
    tmp->stats.hp=SP_PARAMETERS[spell_type].bdur
              + 5*SP_level_strength_adjust(op,caster,spell_type);
    tmp->stats.dam=SP_PARAMETERS[spell_type].bdam
                  +SP_level_dam_adjust(op,caster,spell_type);
    tmp->stats.food=1;

/*
    tmp->level=op->level;
*/
    tmp->level=SK_level(op);
    SET_FLAG(tmp, FLAG_WALK_ON);
    SET_FLAG(tmp, FLAG_FLY_ON);
    set_owner(tmp,op);
    break;
  default:
    LOG(llevError,"Unimplemented magic_wall spell: %d\n",spell_type);
    return 0;
  }
  tmp->x=op->x+freearr_x[dir],tmp->y=op->y+freearr_y[dir];
  if ((tmp = insert_ob_in_map (tmp, op->map, op)) == NULL) {
    new_draw_info(NDI_UNIQUE, 0,op,"Something destroys your wall");
    return 0;
  }

  /*  This code causes the wall to extend to a distance of 5 in
		each direction, or until an obstruction is encountered. 
		posblocked and negblocked help determine how far the
		created wall can extend, it won't go extend through
		blocked spaces.  */

  {int i,posblocked=0,negblocked=0; object *tmp2;  
  for(i=1;i<5;i++) {
	 int x,y,dir2;
	 
	 dir2 = (dir<4)?(dir+2):dir-2;
	 
	 x = tmp->x+i*freearr_x[dir2]; y = tmp->y+i*freearr_y[dir2];
	 if(!blocked(op->map,x,y)&&!posblocked) {
		tmp2 = get_object();
		copy_object(tmp,tmp2);
		tmp2->x = x; tmp2->y = y;
		insert_ob_in_map(tmp2,op->map,op);
	 } else posblocked=1;
	 x = tmp->x-i*freearr_x[dir2]; y = tmp->y-i*freearr_y[dir2];
	 if(!blocked(op->map,x,y)&&!negblocked) {
		tmp2 = get_object();
		copy_object(tmp,tmp2);
		tmp2->x = x; tmp2->y = y;
		insert_ob_in_map(tmp2,op->map,op);
	 } else negblocked=1;
  }
  }

  if(QUERY_FLAG(tmp, FLAG_BLOCKSVIEW))
    update_all_los(op->map);
  if(op->type==PLAYER)
    draw(op);
  else
    SET_FLAG(op, FLAG_SCARED); /* We don't want them to walk through the wall! */
  return 1;
}

/* cast_light() - I wanted this to be able to blind opponents who stand
 * adjacent to the caster, so I couldnt use magic_wall(). -b.t. 
 */

int cast_light(object *op,object *caster,int dir) {
  object *target=NULL,*tmp=NULL;
  int x,y,dam=SP_PARAMETERS[SP_LIGHT].bdam    
                  +SP_level_dam_adjust(op,caster,SP_LIGHT);;

  if(!dir) {
    new_draw_info(NDI_UNIQUE, 0,op,"In what direction?");
    return 0;
  }

  x=op->x+freearr_x[dir],y=op->y+freearr_y[dir];

  if (!out_of_map(op->map,x,y)) {
    for(target=get_map_ob(op->map,x,y);target;target=target->above)
     if(QUERY_FLAG(target,FLAG_MONSTER)) {
        /* coky doky. got a target monster. Lets make a blinding attack */
         if(target->head) target = target->head;
         (void) hit_player(target,dam,op,(AT_BLIND|AT_MAGIC));
         return 1; /* one success only! */
      }
  }

  /* no live target, perhaps a wall is in the way? */
  if(blocked(op->map,x,y)) {
      new_draw_info(NDI_UNIQUE, 0,op,"Something is in the way.");
      return 0;
  }
  /* ok, looks groovy to just insert a new light on the map */
  tmp=get_archetype("light");
  if(!tmp) { 
	LOG(llevError,"Error: spell arch for cast_light() missing.\n");
	return 0;
  }
  tmp->speed = 0.000001 * (SP_PARAMETERS[SP_LIGHT].bdur
              - (10*SP_level_strength_adjust(op,caster,SP_LIGHT)));
  if (tmp->speed < MIN_ACTIVE_SPEED) tmp->speed = MIN_ACTIVE_SPEED;
  tmp->glow_radius=dam;
  tmp->x=x,tmp->y=y;
  insert_ob_in_map(tmp,op->map,op);

  if(op->type==PLAYER) draw(op);
  return 1;
}

int dimension_door(object *op,int dir) {
    int dist;

    if(op->type!=PLAYER)
	return 0;

    if(!dir) {
	new_draw_info(NDI_UNIQUE, 0,op,"In what direction?");
	return 0;
    }

    if(op->contr->count) {
	for(dist=0;dist<op->contr->count; dist++)
	    if (blocks_magic(op->map,op->x+freearr_x[dir]*(dist+1),
                             op->y+freearr_y[dir]*(dist+1))) break;

	if(dist<op->contr->count) {
	    new_draw_info(NDI_UNIQUE, 0,op,"Something blocks your magic.\n");
	    op->contr->count=0;
	    return 0;
	}
	op->contr->count=0;

	/* If the player is trying to dimension door to solid rock, choose
	 * a random place on the map to put the player.
	 * Changed in 0.94.3 so that the player can not get put in
	 * a no magic spot.
	 */
	if(blocked(op->map,op->x+freearr_x[dir]*dist, op->y+freearr_y[dir]*dist)){
	    int x=RANDOM()%op->map->mapx,y=RANDOM()%op->map->mapy;

	    if(blocked(op->map,x,y) || blocks_magic(op->map,x,y)) {
		new_draw_info(NDI_UNIQUE, 0,op,"You cast your spell, but nothing happens.\n");
		return 1; /* Maybe the penalty should be more severe... */
	    }

	    remove_ob(op);
	    op->x=x,op->y=y;
	    if ((op = insert_ob_in_map(op,op->map,op)) != NULL)
	        draw(op);
	    return 1;
	}
    } else { /* Player didn't specify a distance, so lets see how far
	      * we can move the player.
	      */
	for(dist=0;!blocks_view (op->map,op->x+freearr_x[dir]*(dist+1),
                                     op->y+freearr_y[dir]*(dist+1))&&
               !blocks_magic(op->map,op->x+freearr_x[dir]*(dist+1),
                                     op->y+freearr_y[dir]*(dist+1));
	    dist++);

	/* If the destinate is blocked, keep backing up until we
	 * find a place for the player.
	 */
	for(;dist>0&&blocked(op->map,op->x+freearr_x[dir]*dist,
                                 op->y+freearr_y[dir]*dist);dist--);

	if(!dist) {
	    new_draw_info(NDI_UNIQUE, 0,op,"Your spell failed!\n");
	    return 0;
	}
    }

    /* Actually move the player now */
    remove_ob(op);
    op->x+=freearr_x[dir]*dist,op->y+=freearr_y[dir]*dist;
    if ((op = insert_ob_in_map(op,op->map,op)) == NULL)
        return 1;
    draw(op);
    op->speed_left= -FABS(op->speed)*5; /* Freeze them for a short while */
    return 1;
}



int
cast_heal(object *op,int dir,int spell_type) {
  object *tmp;
  archetype *at;
  object *poison;
  int heal=0;

  tmp = find_target_for_friendly_spell(op,dir);

  if(tmp==NULL) return 0;
  switch(spell_type) {
  case SP_CURE_DISEASE:
	 heal = 0;
	 cure_disease(tmp,op);
	 break;
  case SP_MINOR_HEAL:
    heal=(RANDOM()%7)+1;
    new_draw_info(NDI_UNIQUE, 0,tmp, "Your wounds start to close.");
    break;
  case SP_MED_HEAL:
    heal=(RANDOM()%6)+(RANDOM()%6)+(RANDOM()%6)+7;
    new_draw_info(NDI_UNIQUE, 0,tmp, "Your wounds start to fade.");
    break;
  case SP_MAJOR_HEAL:
    new_draw_info(NDI_UNIQUE, 0,tmp, "Your skin looks as good as new!");
    heal=(RANDOM()%8)+(RANDOM()%8)+(RANDOM()%8)+(RANDOM()%8)+12;
    break;
  case SP_HEAL:
    heal=tmp->stats.maxhp;  /* or should be this tmp->stats.maxhp? */
    new_draw_info(NDI_UNIQUE, 0,tmp, "You feel just fine!");
    break;
  case SP_CURE_POISON: 
    at = find_archetype("poisoning");
    poison=present_arch_in_ob(at,tmp);

    if (poison) {
      new_draw_info(NDI_UNIQUE, 0,tmp, "Your body feels cleansed");
      poison->stats.food = 1;
    }
    break;                            
  case SP_CURE_CONFUSION:
    at=find_archetype("confusion");
    poison=present_arch_in_ob(at,tmp);
    if (poison) {
      new_draw_info(NDI_UNIQUE, 0,tmp, "Your mind feels clearer");
      poison->stats.food = 1;
    }
    break;
  case SP_CURE_BLINDNESS:
    at=find_archetype("blindness");
    poison=present_arch_in_ob(at,tmp);
    if (poison) {
      new_draw_info(NDI_UNIQUE, 0,tmp,"Your vision begins to return.");
      poison->stats.food = 1;
    }
    break;
  case SP_RESTORATION:  /* does cure poison, cure madness, heal,and removes depletion, food=999. */
    cast_heal(op,dir,SP_CURE_POISON);
    cast_heal(op,dir,SP_CURE_CONFUSION);
	 cast_heal(op,dir,SP_CURE_DISEASE);
    tmp->stats.food=999;
#if 0
    /* Leave removing depletion to the restore potion. */
    at=find_archetype("depletion");
    poison=present_arch_in_ob(at,tmp);
    if (poison) {
      new_draw_info(NDI_UNIQUE, 0,tmp, "Your abilities seem to have recovered.");
      remove_ob(poison);
      free_object(poison);
      poison = present_arch_in_ob(at, tmp);
    }
#endif
    cast_heal(op,dir,SP_HEAL);  /*  put this one last because
				    it'll redraw the stats
				    as a side effect. */
    return 1;
  }
  reduce_symptoms(tmp,heal);
  if (tmp->stats.hp==tmp->stats.maxhp)
    return 0;
  tmp->stats.hp+=heal;
  if(tmp->stats.hp>tmp->stats.maxhp)
    tmp->stats.hp=tmp->stats.maxhp;

  op->speed_left= -FABS(op->speed)*3; /* Freeze them for a short while */
  return 1;
}

int cast_regenerate_spellpoints(object *op) {
  object *tmp;

  tmp = find_target_for_friendly_spell(op,0);

  if(tmp==NULL) return 0;

  tmp->stats.sp = tmp->stats.maxsp;
  new_draw_info(NDI_UNIQUE, 0,tmp, "Magical energies surge through your body!");
  return 1;
}

int
cast_change_attr(object *op,object *caster,int dir,int spell_type) {
  object *tmp = op;
  object *force;
  int i;

  /* if dir = 99 op defaults to tmp, eat_special_food() requires this. */
  if(dir!=99)
     tmp=find_target_for_friendly_spell(op,dir);

  if(tmp==NULL) return 0;

/*  if((force=present_in_ob(FORCE,tmp))!=NULL)
    remove_force(force);
    We should be able to have more than one thing in force! */
  force=get_archetype("force");
  switch(spell_type) {
  case SP_STRENGTH:
    if(tmp->type!=PLAYER)
      break;
    if(!(RANDOM()%(MAX(1,(10 - MAX_STAT + tmp->stats.Str))))) {
    for(i=20,force->stats.Str=1;i>tmp->stats.Str;i-=2)
      force->stats.Str++; }
        else { new_draw_info(NDI_UNIQUE, 0,op,"You grow no stronger."); force->stats.Str=0; }
    break;
  case SP_DEXTERITY:
    if(tmp->type!=PLAYER)
      break;
    if(!(RANDOM()%(MAX(1,(10 - MAX_STAT + tmp->stats.Dex))))) {
    for(i=20,force->stats.Dex=1;i>tmp->stats.Dex;i-=2)
      force->stats.Dex++; }
        else { new_draw_info(NDI_UNIQUE, 0,op,"You grow no more agile."); force->stats.Dex=0; }

    break;
  case SP_CONSTITUTION:
    if(tmp->type!=PLAYER)
      break;
    if(!(RANDOM()%(MAX(1,(10 - MAX_STAT + tmp->stats.Con))))) {
    for(i=20,force->stats.Con=1;i>tmp->stats.Con;i-=2)
      force->stats.Con++;}
        else { new_draw_info(NDI_UNIQUE, 0,op,"You don't feel any healthier."); force->stats.Con=0; }

    break;
  case SP_CHARISMA:
    if(tmp->type!=PLAYER)
      break;
    if(!(RANDOM()%(MAX(1,(10 - MAX_STAT + tmp->stats.Cha))))) {
    for(i=20,force->stats.Cha=1;i>tmp->stats.Cha;i-=2)
      force->stats.Cha++;}
        else { new_draw_info(NDI_UNIQUE, 0,op,"You are no easier to look at."); force->stats.Cha=0; }

    break;
  case SP_ARMOUR: {
   /* peterm, modified so that it uses my functions */
    force->stats.ac=2+SP_level_dam_adjust(op,caster,spell_type);
    if((tmp->stats.ac-force->stats.ac)<-20) 
	force->stats.ac=tmp->stats.ac+20;

/*    if(force->stats.ac>5)
      force->stats.ac=5;  */
    force->armour = 5+4*SP_level_dam_adjust(op,caster,spell_type);
    if (force->armour > 25)
      force->armour = 25;
    if(tmp->armour>70&& force->armour>(100-tmp->armour)/3)
	force->armour=3;  /* diminishing returns at high armor. */
    new_draw_info(NDI_UNIQUE, 0,tmp,"A force shimmers around you.");
    break; }
  case SP_CONFUSION:
    force->attacktype |= (AT_CONFUSION|AT_PHYSICAL);
    force->protected |= AT_CONFUSION;
    break;
  case SP_HEROISM:
    if (tmp->type != PLAYER)
      break;
/*    if (tmp->contr->orig_stats.Str > 19)
      force->stats.Str = 1;
    else
      force->stats.Str = 2;
    if (tmp->contr->orig_stats.Con > 19)
      force->stats.Con = 1;
    else if (tmp->contr->orig_stats.Con > 15)
      force->stats.Con = 2;
    else
      force->stats.Con = 3;  */
  /*  peterm: heroism is much more elegantly implemented by
	having it cast stat spells for con, str, dex.  Repeated
	castings will make you more heroic, but only up to a point */
    cast_change_attr(op,caster,dir,SP_STRENGTH);
    cast_change_attr(op,caster,dir,SP_DEXTERITY);
    cast_change_attr(op,caster,dir,SP_CONSTITUTION);
    break;
  case SP_HOLY_POSSESSION: {
#ifdef MULTIPLE_GODS
    object *god = find_god(determine_god(op));
    if(god) {
      force->attacktype|=god->attacktype;
      if(god->slaying) force->slaying = add_string(god->slaying);
      force->immune|=god->immune;
      force->protected|=god->protected;
      force->path_attuned|=god->path_attuned;
      new_draw_info_format(NDI_UNIQUE, 0,tmp,
	   "You are possessed by the essence of %s!",god->name);
    } else 
        new_draw_info(NDI_UNIQUE, 0,op,"Your blessing seems empty.");
#endif
    if(tmp!=op) new_draw_info_format(NDI_UNIQUE, 0,tmp,
	"You bless %s mightily!",tmp->name);
    force->stats.wc += SP_level_dam_adjust(op, caster,SP_HOLY_POSSESSION); 
    force->stats.ac += SP_level_dam_adjust(op, caster,SP_HOLY_POSSESSION); 
    break; } 
  case SP_REGENERATION:
    force->stats.hp = 1 + SP_level_dam_adjust(op, caster,SP_REGENERATION);
    break;
  case SP_CURSE: {  
#ifdef MULTIPLE_GODS
    object *god = find_god(determine_god(op));
    if(god) {
      force->path_repelled|=god->path_repelled;
      force->path_denied|=god->path_denied;
      new_draw_info_format(NDI_UNIQUE, 0,tmp,
	"You are a victim of %s's curse!",god->name);
    } else 
        new_draw_info(NDI_UNIQUE, 0,op,"Your curse seems empty.");
#endif
    if(tmp!=op) new_draw_info_format(NDI_UNIQUE, 0,tmp,"You curse %s!",tmp->name);
    force->stats.ac -= SP_level_dam_adjust(op, caster,SP_CURSE); 
    force->stats.wc -= SP_level_dam_adjust(op, caster,SP_CURSE);
    break; } 
  case SP_BLESS: { 
#ifdef MULTIPLE_GODS
    object *god = find_god(determine_god(op));
    if(god) {
       force->protected|=god->protected;
       force->path_attuned|=god->path_attuned;
          new_draw_info_format(NDI_UNIQUE, 0,tmp,
		"You receive the blessing of %s.",god->name);
    } else 
        new_draw_info(NDI_UNIQUE, 0,op,"Your blessing seems empty.");
#endif
    if(tmp!=op) new_draw_info_format(NDI_UNIQUE, 0,tmp,"You bless %s.",tmp->name);
    force->stats.wc += SP_level_dam_adjust(op, caster,SP_BLESS);
    force->stats.ac += SP_level_dam_adjust(op, caster,SP_BLESS);
    break; } 
  case SP_DARK_VISION:
    SET_FLAG(force,FLAG_SEE_IN_DARK);
    break;
  case SP_PROT_COLD:
    force->protected|=AT_COLD;
    break;
  case SP_PROT_FIRE:
    force->protected|=AT_FIRE;
    break;
  case SP_PROT_ELEC:
    force->protected|=AT_ELECTRICITY;
    break;
  case SP_PROT_POISON:
    force->protected|=AT_POISON;
    break;
  case SP_PROT_SLOW:
    force->protected|=AT_SLOW;
    break;
  case SP_PROT_PARALYZE:
    force->protected|=AT_PARALYZE;
    break;
  case SP_PROT_DRAIN:
    force->protected|=AT_DRAIN;
    break;
  case SP_PROT_ATTACK:
    force->protected|=AT_PHYSICAL;
    break;
  case SP_PROT_MAGIC:
    force->protected|=AT_MAGIC;
    break;
  case SP_PROT_CONFUSE:
    force->protected|=AT_CONFUSION;
    break;
  case SP_PROT_CANCEL:
    force->protected|=AT_CANCELLATION;
    break;
  case SP_PROT_DEPLETE:
    force->protected|=AT_DEPLETE;
    break;
  case SP_LEVITATE:
    SET_FLAG(force, FLAG_FLYING);
    break;
	/*mlee*/
  case SP_IMMUNE_COLD:
    force->immune|=AT_COLD;
    break;
  case SP_IMMUNE_FIRE:
    force->immune|=AT_FIRE;
    break;
  case SP_IMMUNE_ELEC:
    force->immune|=AT_ELECTRICITY;
    break;
  case SP_IMMUNE_POISON:
    force->immune|=AT_POISON;
    break;
  case SP_IMMUNE_SLOW:
    force->immune|=AT_SLOW;
    break;
  case SP_IMMUNE_PARALYZE:
    force->immune|=AT_PARALYZE;
    break;
  case SP_IMMUNE_DRAIN:
    force->immune|=AT_DRAIN;
    break;
  case SP_IMMUNE_ATTACK:
    force->immune|=AT_PHYSICAL;
    break;
  case SP_IMMUNE_MAGIC:
    force->immune|=AT_MAGIC;
    break;
  case SP_INVULNERABILITY:
    force->immune|=262143;
    break;
  case SP_PROTECTION:
    force->protected|=262143;
    break;
  case SP_HASTE:
    force->stats.exp=(3+SP_level_dam_adjust(op, caster,SP_HASTE));
    if(op->speed > 0.2 * SP_level_strength_adjust(op,caster,SP_HASTE)) 
	force->stats.exp=0;
    break;
  case SP_XRAY:
    SET_FLAG(force,FLAG_XRAYS);
    break;

  }
  force->speed_left= -1-SP_level_strength_adjust(op, caster,spell_type)*0.1;
  SET_FLAG(force, FLAG_APPLIED);
  force = insert_ob_in_ob(force,tmp);
  change_abil(tmp,force); /* Mostly to display any messages */
  fix_player(tmp);        /* This takes care of some stuff that change_abil() */
			  /* unfortunately is incapable off. */

  return 1;
}


#define MAX_PET_MONSTERS 5
char mage_pet_monsters [MAX_PET_MONSTERS][16] =
                {"bat","spider","stalker","beholder","dark_elf"};
int mage_num_called [MAX_PET_MONSTERS] = {2,1,1,2,3};
char priest_pet_monsters [MAX_PET_MONSTERS][16] =
                {"bee","killer_bee","devil","angel","panther"};
int priest_num_called [MAX_PET_MONSTERS] = {3,2,2,2,5};
char altern_pet_monsters [MAX_PET_MONSTERS][16] =
                {"bird","pixie","skeleton","skull","vampire"};
int altern_num_called [MAX_PET_MONSTERS] = {1,1,2,1,1};

/* this pet monster stuff is total crap!
** We should replace it with:
struct summoned_mon int foo {
        char * mon_arch;
        int  num_summoned;
        }
struct summoned_mon pets_summoned = {
       { "bird", 5 },
       { "vampire", 6},
       { NULL, 0 }     -* terminator *-
}
**
*/


int summon_pet(object *op, int dir, SpellTypeFrom item) {
  int level, number, i;
  char *monster;
  archetype *at;

/*
  level = ((op->head?op->head->level:op->level) / 4); 
*/
  level = ((op->head?op->head->level:SK_level(op)) / 4);
  if (level >= MAX_PET_MONSTERS)
    level = MAX_PET_MONSTERS - 1;
  switch(RANDOM()%3) {
  case 0:
    number = priest_num_called[level];
    monster = priest_pet_monsters[level];
    break;
  case 1:
    number = mage_num_called[level];
    monster = mage_pet_monsters[level];
    break;
  default:
    number = altern_num_called[level];
    monster = altern_pet_monsters[level];
    break;
  }
  at = find_archetype(monster);
  if(at == NULL) {
    LOG(llevError,"Unknown archetype in summon pet: %s\n",monster);
    return 0;
  }
  if (!dir)
    dir = find_free_spot(at, op->map, op->x, op->y, 1, SIZEOFFREE);
  if((dir==-1) || arch_blocked(at,op->map, op->x + freearr_x[dir], op->y+freearr_y[dir]))
  {
    new_draw_info(NDI_UNIQUE, 0,op, "There is something in the way.");
    if(op->type == PLAYER)
      op->contr->count_left = 0;
    return 0;
  }
  if (item != spellNormal)
    /* op->stats.sp -= 5 + 10*level + op->level; */ 
    op->stats.sp -= 5 + 10*level + SK_level(op);
  for (i = 1; i < number + 1; i++) {
    archetype *atmp;
    object *prev = NULL, *head = NULL; /* We want to summon dragons *grin* */

    for(atmp = at; atmp!=NULL; atmp = atmp->more) {
      object *tmp;
      tmp = arch_to_object(atmp);
      if (atmp == at) {
        set_owner(tmp, op);
        SET_FLAG(tmp, FLAG_MONSTER);
        if (op->type == PLAYER) {
          tmp->stats.exp = 0;
          add_friendly_object(tmp);
          SET_FLAG(tmp, FLAG_FRIENDLY);
          tmp->move_type = PETMOVE;
        } else if (QUERY_FLAG (op, FLAG_FRIENDLY)) {
      	    object *owner = get_owner(op);
            if (owner != NULL) {
               set_owner (tmp, owner);
               tmp->move_type = PETMOVE;
               add_friendly_object (tmp);
               SET_FLAG (tmp, FLAG_FRIENDLY);
	    }
	}
        tmp->speed_left = -1;
        tmp->enemy = op->enemy;
        tmp->type = 0;
      }
      if(head == NULL)
        head = tmp;
      tmp->x = op->x + freearr_x[dir] + tmp->arch->clone.x;
      tmp->y = op->y + freearr_y[dir] + tmp->arch->clone.y;
      tmp->map = op->map;
      if(head != tmp)
        tmp->head = head, prev->more = tmp;
      prev = tmp;
    }
    head->direction = dir;
    head = insert_ob_in_map (head, op->map, op);
    if (head != NULL && head->randomitems != NULL) {
      object *tmp;
      create_treasure(head->randomitems,head,GT_INVENTORY,6,0);
      for(tmp = head->inv; tmp != NULL; tmp = tmp->below)
        if(!tmp->nrof)
          SET_FLAG(tmp, FLAG_NO_DROP);
    }
    dir = absdir(dir + 1);
    if (arch_blocked(at,op->map, op->x + freearr_x[dir],
                     op->y + freearr_y[dir]))
    {
      if (i < number) {
        new_draw_info(NDI_UNIQUE, 0,op, "There is something in the way,");
        new_draw_info(NDI_UNIQUE, 0,op, "no more pets for this casting.");
        if (item != spellNormal) {
          /* op->stats.sp += (5 + 12 * level + op->level) / (number - i); */ 
          op->stats.sp += (5 + 12 * level + SK_level(op)) / (number - i);
          if (op->stats.sp < 0)
            op->stats.sp = 0;
        }
        return 1;
      }
    }
  }
  if (item != spellNormal && op->stats.sp < 0)
    op->stats.sp = 0;
  return 1;
}

int create_bomb(object *op,object *caster,int dir,int spell_type,char *name) {
  object *tmp;
  int dx=op->x+freearr_x[dir],dy=op->y+freearr_y[dir];
  if(wall(op->map,dx,dy)) {
    new_draw_info(NDI_UNIQUE, 0,op,"There is something in the way.");
    return 0;
  }
  tmp=get_archetype(name);

  /*  level dependencies for bomb  */
  tmp->stats.dam=SP_PARAMETERS[spell_type].bdam + SP_level_dam_adjust(op,caster,spell_type);
  tmp->stats.hp=SP_PARAMETERS[spell_type].bdur + SP_level_strength_adjust(op,caster,spell_type);
  tmp->level = casting_level (caster, spell_type);

  set_owner(tmp,op);
  tmp->x=dx,tmp->y=dy;
  insert_ob_in_map(tmp,op->map,op);
  return 1;
}

void animate_bomb(object *op) {
  int i;
  object *env;
  archetype *at;

  if(op->state!=NUM_ANIMATIONS(op)-1)
    return;
  at = find_archetype("splint");
  for(env=op;env->env!=NULL;env=env->env);
  if (op->env) {
        if (env->map == NULL)
            return;
	if (env->type==PLAYER) drop(env,op);
	else {
	    remove_ob(op);
	    if ((op = insert_ob_in_map (op, env->map, op)) == NULL)
                return;
	}
  }
  if (at)
    for(i=1;i<9;i++)
      fire_arch(op,op,i,at,0,0);
  explode_object(op);
}


int fire_cancellation(object *op,int dir,archetype *at, int magic) {
    object *tmp;

    if(at==NULL)
	return 0;

    tmp=arch_to_object(at);

    if(tmp==NULL)
	return 0;

    tmp->x=op->x,tmp->y=op->y;
    tmp->direction=dir;
    if(magic)
	tmp->attacktype|=AT_MAGIC;

    if(op->type==PLAYER)
	set_owner(tmp,op);

    if ((tmp = insert_ob_in_map(tmp,op->map,op)) != NULL)
	move_cancellation(tmp);
    return 1;
}

void move_cancellation(object *op) {
  remove_ob(op);
  op->x+=DIRX(op),op->y+=DIRY(op);
  if(!op->direction||wall(op->map,op->x,op->y)) {
    free_object(op);
    return;
  }
  if(reflwall(op->map,op->x,op->y)) {

    op->direction=absdir(op->direction+4);
    insert_ob_in_map(op,op->map,op);
    return;
  }
  if ((op = insert_ob_in_map (op, op->map, op)) != NULL)
    hit_map (op, 0, op->attacktype);
}

void cancellation(object *op)
{
  object *tmp;

  if (QUERY_FLAG (op, FLAG_ALIVE) || op->type == CONTAINER
      || op->type == THROWN_OBJ)
  {
    /* Recur through the inventory */
    for(tmp=op->inv;tmp!=NULL;tmp=tmp->below)
      if (!did_make_save_item(tmp, AT_CANCELLATION,op))
	cancellation(tmp);
  }
  else				/* Nullify this object. */
    if(FABS(op->magic)<=(RANDOM()%6)) {
      op->magic=0;
      CLEAR_FLAG(op, FLAG_DAMNED);
      CLEAR_FLAG(op, FLAG_CURSED);
      CLEAR_FLAG(op, FLAG_KNOWN_MAGICAL);
      CLEAR_FLAG(op, FLAG_KNOWN_CURSED);
      if (op->env && op->env->type == PLAYER) {
	esrv_send_item (op->env, op);
      }
    }
}


/* Create a missile (nonmagic - magic +4). Will either create bolts or arrows
 * based on whether a crossbow or bow is equiped. If neither, it defaults to
 * arrows.
 * Sets the plus based on the casters level. It is also settable with the
 * invoke command. If the caster attempts to create missiles with too
 * great a plus, the default is used.
 * The # of arrows created also goes up with level, so if a 30th level mage
 * wants LOTS of arrows, and doesn't care what the plus is he could
 * create nonnmagic arrows, or even -1, etc...
 *
 * Written by Ben Fennema (huma@netcom.com) - bugs fixed by Raphael Quinet
 */

int cast_create_missile(object *op, object *caster,int dir, char *stringarg)
{
  int missile_plus=0;
  char *missile_name;
  object *tmp, *missile;
  tag_t tag;

  missile_name = "arrow";

  for (tmp=op->inv; tmp != NULL; tmp=tmp->below)
    if (tmp->type == BOW && QUERY_FLAG(tmp, FLAG_APPLIED)) {
      if (strstr(tmp->race, "bolt")) /* crossbow bolts */
	missile_name="bolt";
      break;
    }

  if (stringarg)
    missile_plus = atoi(stringarg);
  if (!stringarg || ((1 + SP_level_strength_adjust(op, caster,SP_CREATE_MISSILE)) -
                     (3 * missile_plus)) < 0)
    missile_plus = SP_PARAMETERS[SP_CREATE_MISSILE].bdam +
                   SP_level_dam_adjust(op, caster,SP_CREATE_MISSILE);
  if (missile_plus > 4)
    missile_plus = 4;
  else if (missile_plus < -4)
    missile_plus = -4;

  if (find_archetype(missile_name)==NULL) {
	LOG(llevDebug, "Cast create_missile: could not find archtype %s\n", 
	    missile_name);
	return 0;
  }
  missile = get_archetype(missile_name);
  missile->nrof = SP_PARAMETERS[SP_CREATE_MISSILE].bdur
		* ((1 + SP_level_strength_adjust(op, caster,SP_CREATE_MISSILE))
		   - (3 * missile_plus));
  if (missile->nrof < 1)
    missile->nrof=1;

  missile->magic = missile_plus;
  missile->value=0; /* it would be too easy to get money by creating
			arrows +4 and selling them, even with value = 1 */

  SET_FLAG(missile, FLAG_IDENTIFIED);
  tag = missile->count;
  if ( ! cast_create_obj (op, caster, missile, dir) && op->type == PLAYER
      && ! was_destroyed (missile, tag))
  {
    tmp= get_owner(op);
    if (!tmp)
      pick_up(op, missile);
    else
      pick_up(tmp, missile);
  }
  return 1;
}


/* Alchemy code by Mark Wedel (master@rahul.net)
 *
 * This code adds a new spell, called alchemy.  Alchemy will turn
 * objects to gold nuggets, the value of the gold nuggets being
 * about 90% of that of the item itself.  It uses the value of the
 * object before charisma adjustments, because the nuggets themselves
 * will be will be adjusted by charisma when sold.
 *
 * Large nuggets are worth 25 gp each (base).  You will always get
 * the maximum number of large nuggets you could get.
 * Small nuggets are worth 1 gp each (base).  You will get from 0
 * to the max amount of small nuggets as you could get.
 *
 * For example, if an item is worth 110 gold, you will get
 * 4 large nuggets, and from 0-10 small nuggets.
 *
 * There is also a chance (1:30) that you will get nothing at all
 * for the object.  There is also a maximum weight that will be
 * alchemied.
 */
 
/* I didn't feel like passing these as arguements to the
 * two functions that need them.  Real values are put in them
 * when the spell is cast, and these are freed when the spell
 * is finished.
 */
static object *small, *large;

static void alchemy_object(object *obj, int *small_nuggets,
	 int *large_nuggets, int *weight)
{
	int	value=query_cost(obj, NULL, F_TRUE);

	/* Give half price when we alchemy money (This should hopefully
	 * make it so that it isn't worth it to alchemy money, sell
	 * the nuggets, alchemy the gold from that, etc.
	 * Otherwise, give 9 silver on the gold for other objects,
	 * so that it would still be more affordable to haul
	 * the stuff back to town.
	 */
    if (QUERY_FLAG(obj, FLAG_UNPAID))
	value=0;
    else if (obj->type==MONEY || obj->type==GEM)
	value /=3;
    else if (QUERY_FLAG(obj,FLAG_UNPAID)) value=0;
    else
	value *= 0.9;

    if ((obj->value>0) && RANDOM()%30) {
#ifdef LOSSY_ALCHEMY
	int tmp = (value % large->value) / small->value;

	*large_nuggets += value/ large->value;
	if (tmp)
	    *small_nuggets += RANDOM() % (tmp + 1);
#else
	static int value_store;
	int count;
	value_store += value;
	count = value_store / large->value;
	*large_nuggets += count;
	value_store -= count * large->value;
	count = value_store / small->value;
	*small_nuggets += count;
	value_store -= count * small->value;
	/* LOG(llevDebug, "alchemize value %d, remainder %d\n", value, value_store); */
#endif
    }

    /* Turn 25 small nuggets into 1 large nugget.  If the value
     * of large nuggets is not evenly divisable my the small nugget
     * value, take off an extra small_nugget (Assuming small_nuggets!=0)
     */
    if (*small_nuggets * small->value >= large->value) {
	(*large_nuggets)++;
	*small_nuggets -= large->value / small->value;
	if (*small_nuggets && large->value % small->value)
		(*small_nuggets)--;
    }
    weight += obj->weight;
    remove_ob(obj);
    free_object(obj);
}

static void update_map(object *op, int small_nuggets, int large_nuggets,
	int x, int y)
{
	object *tmp;

	if (small_nuggets) {
		tmp = get_object();
		copy_object(small, tmp);
		tmp-> nrof = small_nuggets;
		tmp->x = x;
		tmp->y = y;
		insert_ob_in_map(tmp, op->map, op);
	}
	if (large_nuggets) {
		tmp = get_object();
		copy_object(large, tmp);
		tmp-> nrof = large_nuggets;
		tmp->x = x;
		tmp->y = y;
		insert_ob_in_map(tmp, op->map, op);
	}
}

int alchemy(object *op)
{
	int x,y,weight=0,weight_max,large_nuggets,small_nuggets,did_alc=0;
	object *next,*tmp;

  if(op->type!=PLAYER)
    return 0;
  /* Put a maximum weight of items that can be alchemied.  Limits the power
   * some, and also prevents people from alcheming every table/chair/clock
   * in sight
   */
  /* weight_max = 100000 + 50000*op->level; */
  weight_max = 100000 + 50000*SK_level(op);
  small=get_archetype("smallnugget"),
  large=get_archetype("largenugget");

#ifdef ALCHEMY
  for(y= op->y-1;y<=op->y+1;y++) {
    for(x= op->x-1;x<=op->x+1;x++) {
      if(out_of_map(op->map,x,y) || wall(op->map,x,y) ||
	 blocks_view(op->map,x,y))
        continue;
	for(tmp=get_map_ob(op->map,x,y);tmp!=NULL;tmp=next) {
          next=tmp->above;
	  if(QUERY_FLAG(tmp,FLAG_IS_CAULDRON)) {
		attempt_do_alchemy(op, tmp);
		did_alc=1;
		continue;
	  }
	}
    }
  }
  if (did_alc) return 1;
#endif

  for(y= op->y-1;y<=op->y+1;y++) {
    for(x= op->x-1;x<=op->x+1;x++) {
      if(out_of_map(op->map,x,y) || wall(op->map,x,y) ||
	    blocks_view(op->map,x,y))
	    continue;

	small_nuggets=0;
	large_nuggets=0;

	for(tmp=get_map_ob(op->map,x,y);tmp!=NULL;tmp=next) {
          next=tmp->above;
	  if (tmp->weight>0 && !QUERY_FLAG(tmp, FLAG_NO_PICK)
	      && !QUERY_FLAG(tmp, FLAG_ALIVE)) {
	    if (tmp->inv) {
		object *next1,*tmp1;
		for (tmp1 = tmp->inv; tmp1!=NULL; tmp1=next1) {
		    next1 = tmp1->below;
		    if (tmp1->weight>0 && !QUERY_FLAG(tmp1, FLAG_NO_PICK) && !QUERY_FLAG(tmp1, FLAG_ALIVE))
		        alchemy_object(tmp1, &small_nuggets, &large_nuggets,
			   &weight);
		}
	    }
	    alchemy_object(tmp, &small_nuggets, &large_nuggets, &weight);
	    
	    if (weight>weight_max) {
		update_map(op, small_nuggets, large_nuggets, x, y);
		free_object(large);
		free_object(small);
		return 1;
	    }
	  } /* is alchemable object */
	} /* process all objects on this space */

/* Insert all the nuggets at one time.  This probably saves time, but
 * it also prevents us from alcheming nuggets that were just created
 * with this spell.
 */
	update_map(op, small_nuggets, large_nuggets, x, y);
    }
  }
  free_object(large);
  free_object(small);
  return 1;
}

int remove_curse(object *op, int type, SpellTypeFrom src) {
  object *tmp;
  int success = 0, was_one = 0;

  for (tmp = op->inv; tmp; tmp = tmp->below)
    if (QUERY_FLAG(tmp, FLAG_APPLIED) && (QUERY_FLAG(tmp, FLAG_CURSED) ||
    (type == SP_REMOVE_DAMNATION && QUERY_FLAG(tmp, FLAG_DAMNED))))
    {
      was_one++;
      /* if (tmp->level <= op->level) */ 
      if (tmp->level <= SK_level(op))
      {
        success++;
        if (type == SP_REMOVE_DAMNATION)
	  CLEAR_FLAG(tmp, FLAG_DAMNED);

        CLEAR_FLAG(tmp, FLAG_CURSED);
        CLEAR_FLAG(tmp, FLAG_KNOWN_CURSED);
        tmp->value = 0; /* Still can't sell it */
	if (op->type == PLAYER)
	    esrv_send_item(op, tmp);
      }
    }
  if (op->type==PLAYER) {
    if (success) {
      new_draw_info(NDI_UNIQUE, 0,op, "You feel like someone is helping you.");
    } else
      if (was_one)
        new_draw_info(NDI_UNIQUE, 0,op, "You failed to remove the curse.");
      else if (src == spellNormal)
        new_draw_info(NDI_UNIQUE, 0,op, "You are not using any cursed items.");
      else
        new_draw_info(NDI_UNIQUE, 0,op, "You hear manical laughter in the distance.");
  }
  return success;
}


int cast_identify(object *op) {
  object *tmp;
  int success = 0, random_val=0;
  int chance = 8 + op->stats.luck + op->stats.Wis;

  if (chance < 1)
    chance = 1;

  for (tmp = op->inv; tmp ; tmp = tmp->below)
    if (!QUERY_FLAG(tmp, FLAG_IDENTIFIED) && !tmp->invisible &&
	 need_identify(tmp))
    {
      identify(tmp);
      if (op->type==PLAYER) {

	new_draw_info_format(NDI_UNIQUE, 0, op,
		"You have %s.", long_desc(tmp));
	if (tmp->msg) {
	  new_draw_info(NDI_UNIQUE, 0,op, "The item has a story:");
	  new_draw_info(NDI_UNIQUE, 0,op, tmp->msg);
	}
      }

      if ((random_val=RANDOM()%chance) > (chance - ++success - 2))
        break;
    }
  /* If all the power of the spell has been used up, don't go and identify
   * stuff on the floor.  Only identify stuff on the floor if the spell
   * was not fully used.
   */
  if (random_val<=chance - success -2) {
    for(tmp = get_map_ob(op->map,op->x,op->y);tmp!=NULL;tmp=tmp->above)
    if (!QUERY_FLAG(tmp, FLAG_IDENTIFIED) && !tmp->invisible &&
	need_identify(tmp))
    {
      identify(tmp);
      if (op->type==PLAYER) {
	new_draw_info_format(NDI_UNIQUE, 0,op,
		"On the ground is %s.", long_desc(tmp));
	if (tmp->msg) {
	  new_draw_info(NDI_UNIQUE, 0,op, "The item has a story:");
	  new_draw_info(NDI_UNIQUE, 0,op, tmp->msg);
	}
	esrv_send_item(op, tmp);
      }
      if (RANDOM() %chance > (chance - ++success - 2))
	break;
    }
  }
  if (!success)
    new_draw_info(NDI_UNIQUE, 0,op, "You can't reach anything unidentified.");
  else {
    spell_effect(SP_IDENTIFY, op->x, op->y, op->map, op);
  }
  return success;
}

int cast_detection(object *op, int type) {
  object *tmp;
  int x,y,done_one;
  archetype *detect_arch;

  detect_arch = find_archetype("detect_magic");
  if (detect_arch == (archetype *) NULL)
  {
    LOG(llevError, "Couldn't find archetype detect_magic.\n");
    return 0;
  }

  for (x = op->x + WINLEFT; x <= op->x + WINRIGHT; x++)
    for (y = op->y + WINUPPER; y <= op->y + WINLOWER; y++) {
      if (out_of_map(op->map, x, y))
        continue;
      done_one = 0;
      for (tmp = get_map_ob(op->map, x, y); tmp &&
	    (!done_one || type==SP_DETECT_MAGIC || type==SP_DETECT_CURSE);
	      tmp = tmp->above)
      {
        switch(type) {
        case SP_DETECT_MAGIC:
          if (!QUERY_FLAG(tmp,FLAG_KNOWN_MAGICAL) && !QUERY_FLAG(tmp, FLAG_IDENTIFIED) &&
	    is_magical(tmp)) {
            SET_FLAG(tmp,FLAG_KNOWN_MAGICAL);
            if(tmp->type==RUNE) /*peterm:  make runes more visible*/
                if(tmp->attacktype&AT_MAGIC)  /* if they're magic! */
                       tmp->stats.Cha/=4;
            done_one = 1;
          }
          break;
        case SP_DETECT_MONSTER:
          if (op->type == PLAYER)
            done_one = QUERY_FLAG(tmp, FLAG_MONSTER);
          else
            done_one = (tmp->type == PLAYER);
          break;
        case SP_DETECT_EVIL: { 
#ifdef MULTIPLE_GODS
	  done_one = 0;
 	  if(QUERY_FLAG(tmp,FLAG_MONSTER)&&tmp->race) {
	    object *god=find_god(determine_god(op));
            if(god&&god->slaying&&strstr(god->slaying,tmp->race))
		done_one = 1;
	  } 
#else
          if (op->type == PLAYER)
            done_one = (QUERY_FLAG(tmp, FLAG_MONSTER)&&
		!QUERY_FLAG(tmp, FLAG_UNAGGRESSIVE)&&
		!QUERY_FLAG(tmp, FLAG_FRIENDLY));
          else
            done_one = (tmp->type == PLAYER);
#endif
          break; } 
        case SP_SHOW_INVIS:
	    /* Might there be other objects that we can make visibile? */
	    if (tmp->invisible && (QUERY_FLAG(tmp, FLAG_MONSTER) || 
		tmp->type==PLAYER || tmp->type==HANDLE || 
		tmp->type==TRAPDOOR || tmp->type==EXIT || tmp->type==HOLE ||
		tmp->type==BUTTON || tmp->type==TELEPORTER ||
		tmp->type==GATE || tmp->type==LOCKED_DOOR ||
		tmp->type==WEAPON || tmp->type==ALTAR || tmp->type==SIGN ||
		tmp->type==TRIGGER_PEDESTAL || tmp->type==SPECIAL_KEY ||
		tmp->type==TREASURE || tmp->type==BOOK ||
		tmp->type==HOLY_ALTAR)) {
		if(RANDOM()%(SK_level(op)) > tmp->level/4) {
		    tmp->invisible=0;
		    done_one = 1;
		}
	    }
          break;
        case SP_DETECT_CURSE:
          if (!QUERY_FLAG(tmp, FLAG_KNOWN_CURSED) &&
		 (QUERY_FLAG(tmp, FLAG_CURSED) ||
		 QUERY_FLAG(tmp, FLAG_DAMNED))) {
	    SET_FLAG(tmp, FLAG_KNOWN_CURSED);
            done_one = 1;
          }
          break;
        }
      } /* Done all the object on this square */
      if (done_one) {
          object *detect_ob = arch_to_object(detect_arch);
          detect_ob->x = x;
          detect_ob->y = y;
          insert_ob_in_map(detect_ob, op->map, op);
      }
    }
  if ((type == SP_DETECT_MAGIC || type == SP_DETECT_CURSE) &&
      op->type == PLAYER)
  {
    done_one = 0;
    for (tmp = op->inv; tmp; tmp = tmp->below)
      if (!QUERY_FLAG(tmp, FLAG_IDENTIFIED))
        switch(type) {
        case SP_DETECT_MAGIC:
          if (is_magical(tmp) && !QUERY_FLAG(tmp,FLAG_KNOWN_MAGICAL)) {
            SET_FLAG(tmp,FLAG_KNOWN_MAGICAL);
	    esrv_send_item (op, tmp);
            done_one = 1;
          }
          break;
        case SP_DETECT_CURSE:
          if (!QUERY_FLAG(tmp, FLAG_KNOWN_CURSED) &&
		 (QUERY_FLAG(tmp, FLAG_CURSED) ||
		 QUERY_FLAG(tmp, FLAG_DAMNED))) {
	    SET_FLAG(tmp, FLAG_KNOWN_CURSED);
	    esrv_send_item (op, tmp);
            done_one = 1;
          }
          break;
        }
  }
  return 1;
}

/* Shamelessly hacked from PeterM's cast_charm and destruction code 
 *  - b.t. thomas@nomad.astro.psu.edu		 
 */ 

/* Changes in the spell code to make it more powerfull - now it can 
 * pacify multi-square creatures, and has greater range - Aug 95 b.t. 
 */
/* New modification -- w/ Multigod hack, now if its a member of an aligned
 * race, we automatically pacify it. b.t.
 */ 

int cast_pacify(object *op, object *weap, archetype *arch,int spellnum ) {
  int i,r,j; 
  object *tmp,*effect;
#ifdef MULTIPLE_GODS
  object *god = find_god(determine_god(op));
#endif
  
  r= 1 + SP_level_strength_adjust(op,weap,SP_PACIFY);

  for(i= -r;i<r;i++)
    for(j= -r;j<r;j++) {
      if(out_of_map(op->map,op->x+i,op->y+j))
        continue;
      for(tmp=get_map_ob(op->map,op->x+i,op->y+j);
            tmp&&(!QUERY_FLAG(tmp,FLAG_MONSTER));tmp=tmp->above);
        if(!tmp) continue;
        if(tmp->type==PLAYER) continue;

#ifdef MULTIPLE_GODS /* we only go through checking if the monster is not aligned 
	member, we dont worship a god, or monster has no race */ 
        if(!tmp->race||!god||!god->race
          ||!strstr(god->race,tmp->race)) {
#endif
          if(tmp->immune&AT_MAGIC||tmp->immune&AT_GODPOWER) continue;
	/* multiple square monsters only when caster is => level of creature */
          if((tmp->more || tmp->head) && (SK_level(op) < tmp->level)) continue;  
	  if(weap->slaying) 		/* selective pacify */ 
		if(tmp->race != weap->slaying && tmp->name != weap->slaying) continue;
        /* if(op->level <( (RANDOM()%(2*tmp->level+1))-(op->stats.Cha-10)/2)) continue; */ 
          if(SK_level(op) <( (RANDOM()%(2*tmp->level+1))-(op->stats.Cha-10)/2)) continue;
#ifdef MULTIPLE_GODS
        }
#endif
        if((effect=get_archetype("detect_magic"))){
                effect->x = tmp->x;
                effect->y = tmp->y;
                insert_ob_in_map(effect,tmp->map,op);
        }
        SET_FLAG(tmp,FLAG_UNAGGRESSIVE);
  }      

  return 1;
}

/* summon fog code. I could'nt decide whether this
 * could just go into another routine (like create_
 * the_feature) or have it alone. For now, its separate
 * function. This code just creates a variable amount of
 * fog archetypes around the character.
 * Implementation by b.t. (thomas@nomad.astro.psu.edu)
 * (based on create bomb code)
 */

int summon_fog(object *op, object *caster,int dir,int spellnum) { 
  object *tmp;
  int i,dx=op->x+freearr_x[dir],dy=op->y+freearr_y[dir];

  if (!spellarch[spellnum])
    return 0;

  for(i=1;i<MIN(2+SP_level_strength_adjust(op,caster,spellnum),SIZEOFFREE);i++) {

     if(wall(op->map,dx,dy)) {
        new_draw_info(NDI_UNIQUE, 0,op,"There is something in the way.");
        return 0;
     }
     tmp=get_archetype(spellarch[spellnum]->name);
     tmp->x=dx,tmp->y=dy;	/* all fog starts in 1 place */ 
#ifdef WALL_CREDIT              /* does someone get exp for the kills? */ 
     set_owner(tmp,op);		/* note however, that after 'fog' moves */ 
				/* it is no longer owned. It is unlikely */
				/* that players will garner much exp with */
				/* this spell */
#endif
     insert_ob_in_map(tmp,op->map,op);
  }

  return 1;

}

/*  create_the_feature:  peterm  */
/*  implementation of the spells which build directors, lightning
    walls, bullet walls, and fireballwalls.  */

int create_the_feature(object *op, object *caster,int dir, int spell_effect)
{
    object *tmp=NULL;
    char buf1[20];
    int putflag=0;

    if(!dir) dir=op->facing;
    else putflag=1;

    if(blocked(op->map,op->x+freearr_x[dir],op->y+freearr_y[dir])) {
	new_draw_info(NDI_UNIQUE, 0,op,"Something is in the way.");
	return 0;
    }

    switch(spell_effect) {
	case SP_BUILD_DIRECTOR:
	    sprintf(buf1,"director_%d",dir);
	    tmp=get_archetype(buf1);
	    SET_FLAG(tmp, FLAG_IS_USED_UP);
	    tmp->stats.food=SP_PARAMETERS[spell_effect].bdur+10*SP_level_strength_adjust(op,caster,spell_effect);
	    tmp->stats.hp=SP_PARAMETERS[spell_effect].bdam+5*SP_level_dam_adjust(op,caster,spell_effect);
	    tmp->stats.maxhp=tmp->stats.hp;
	    break;

	case SP_BUILD_LWALL:
	    sprintf(buf1,"lightningwall_%d",dir);
	    tmp=get_archetype(buf1);
	    SET_FLAG(tmp, FLAG_IS_USED_UP);
	    SET_FLAG(tmp, FLAG_TEAR_DOWN);
	    SET_FLAG(tmp, FLAG_ALIVE);
	    tmp->stats.food=SP_PARAMETERS[spell_effect].bdur+10*SP_level_strength_adjust(op,caster,spell_effect);
	    tmp->stats.hp=SP_PARAMETERS[spell_effect].bdam+5*SP_level_dam_adjust(op,caster,spell_effect);
	    tmp->stats.maxhp=tmp->stats.hp;
	    break;

	case SP_BUILD_BWALL:
	    sprintf(buf1,"lbulletwall_%d",dir);
	    tmp=get_archetype(buf1);
	    SET_FLAG(tmp, FLAG_IS_USED_UP);
	    SET_FLAG(tmp, FLAG_TEAR_DOWN);
	    SET_FLAG(tmp, FLAG_ALIVE);
	    tmp->stats.food=SP_PARAMETERS[spell_effect].bdur+10*SP_level_strength_adjust(op,caster,spell_effect);
	    tmp->stats.hp=SP_PARAMETERS[spell_effect].bdam+5*SP_level_dam_adjust(op,caster,spell_effect);
	    tmp->stats.maxhp=tmp->stats.hp;
	    break;

	case SP_BUILD_FWALL:
	    sprintf(buf1,"firewall_%d",dir);
	    tmp=get_archetype(buf1);
	    SET_FLAG(tmp, FLAG_IS_USED_UP);
	    SET_FLAG(tmp, FLAG_TEAR_DOWN);
	    SET_FLAG(tmp, FLAG_ALIVE);
	    tmp->stats.food=SP_PARAMETERS[spell_effect].bdur+10*SP_level_strength_adjust(op,caster,spell_effect);
	    tmp->stats.hp=SP_PARAMETERS[spell_effect].bdam+5*SP_level_dam_adjust(op,caster,spell_effect);
	    tmp->stats.maxhp=tmp->stats.hp;
	    break;
    }

#ifdef WALL_CREDIT
    set_owner(tmp,op);
#endif  /* determines whether someone gets credit for kills. */

    tmp->level=SK_level(op)/2;  /*  so that the spell that the wall casts
				 inherit part of the effectiveness of
				 of the wall builder */
    tmp->x=op->x;tmp->y=op->y;
    if(putflag) { 
	tmp->x+=freearr_x[dir];
	tmp->y+=freearr_y[dir];
    }
    if (out_of_map(op->map, tmp->x, tmp->y)) {
	free_object(tmp);
	return 0;
    }
    if ((tmp = insert_ob_in_map (tmp, op->map, op)) == NULL)
        return 1;
    if(QUERY_FLAG(tmp, FLAG_BLOCKSVIEW))
	update_all_los(op->map);
    if(op->type==PLAYER)
	draw(op);
    else
	SET_FLAG(op, FLAG_SCARED); /* We don't want them to walk through the wall! */
    return 1;
}


/*  cast_transfer:  peterm  */
/*  following spell transfers mana from one person to another.
    right now, it's no respecter of maximum sp limits.  WOn't
    fix that, regard it as a feature.  be nice to make someone's
    head explode if they supercharge too much, though.  */

int cast_transfer(object *op,int dir) {
    object *plyr=NULL;
/*  see if we can find someone to give sp to.  */
    if (!out_of_map(op->map, op->x+freearr_x[dir], op->y+freearr_y[dir])) {
	for(plyr=get_map_ob(op->map,op->x+freearr_x[dir],op->y+freearr_y[dir]);
	    plyr!=NULL; plyr=plyr->above)
		if(QUERY_FLAG(plyr, FLAG_ALIVE))
		    break;
    }
     /*  If we did not find a player in the specified direction, transfer
	to anyone on top of us. */
    if(plyr==NULL)
        for(plyr=get_map_ob(op->map,op->x,op->y); plyr!=NULL; plyr=plyr->above)
	    if(QUERY_FLAG(plyr,FLAG_ALIVE))
		break;

    if(plyr) {
      /* DAMN: added spell strength adjust; higher level casters transfer mana faster */
	int maxsp=plyr->stats.maxsp;
	int sp=(plyr->stats.sp += SP_PARAMETERS[SP_TRANSFER].bdam
		+ SP_level_dam_adjust(op,op,SP_TRANSFER));

	new_draw_info(NDI_UNIQUE, 0,plyr,"You feel energy course through you.");
	if(sp>=maxsp*2) {
            new_draw_info(NDI_UNIQUE, 0,plyr,"Your head explodes!");
            fire_arch (op, plyr,0, spellarch[SP_L_FIREBALL], SP_L_FIREBALL, 0);
                /* Explodes a large fireball centered at player */
/*            hit_player(plyr, 9999, op, AT_PHYSICAL);*/
	    plyr->stats.sp = 2*maxsp;

        }
        else if(sp>=maxsp*1.88)
            new_draw_info(NDI_UNIQUE, NDI_ORANGE,plyr,"You feel like your head is going to explode.");
        else if(sp>=maxsp*1.66)
            new_draw_info(NDI_UNIQUE, 0,plyr, "You get a splitting headache!");
        else if(sp>=maxsp*1.5) {
            new_draw_info(NDI_UNIQUE, 0,plyr,"CHaOs fills your world.");
	    confuse_player(op,op,99);
	}
        else if(sp>=maxsp*1.25)
            new_draw_info(NDI_UNIQUE, 0,plyr,"You start hearing voices.");
        return 1;
    }
    else return 0;
}

/*  drain_magic:  peterm  */
/*  drains all the magic out of the victim.  */
int drain_magic(object *op,int dir) {
  object *tmp=NULL;
  double mana, rate;

  if (!out_of_map(op->map,op->x+freearr_x[dir],op->y+freearr_y[dir])) {
    for(tmp=get_map_ob(op->map,op->x+freearr_x[dir],op->y+freearr_y[dir]);
	tmp!=NULL;
	tmp=tmp->above)
      if(QUERY_FLAG(tmp, FLAG_ALIVE))
	break;
     /*  If we did not find a player in the specified direction, transfer
	to anyone on top of us. */
  }

  if(tmp==NULL)
    for(tmp=get_map_ob(op->map,op->x,op->y); tmp!=NULL; tmp=tmp->above)
      if(QUERY_FLAG(tmp, FLAG_ALIVE))
        break;

  /* DAMN: Percent spell point loss determined by caster level
     Caster gains percent of drained mana, also determined by caster level */
  if(tmp&&op!=tmp) {
    rate = (double)(SP_PARAMETERS[SP_MAGIC_DRAIN].bdam
      + 5* SP_level_dam_adjust(op,op,SP_MAGIC_DRAIN)) / 100.0;
    if(rate > 0.95) rate = 0.95;
    mana = tmp->stats.sp * rate;
    tmp->stats.sp -= mana;
    if(QUERY_FLAG(op, FLAG_ALIVE)) {
      rate = (double)(SP_PARAMETERS[SP_MAGIC_DRAIN].bdam
	+ 5* SP_level_strength_adjust(op,op,SP_MAGIC_DRAIN)) / 100.0;
      if(rate > 0.95) rate = 0.95;
      mana = mana * rate;
      op->stats.sp += mana;
    }
    return 1;
  } else {
    return 0;
  }
}
     
/*  counterspell:  peterm  */
/*  an object of type counterspell will nullify cone objects,
    explosion objects, and anything else that |=magic.  */

void counterspell(object *op,int dir)
{
    object *tmp;
    int nflag=0;
    
    for(tmp=get_map_ob(op->map,op->x+freearr_x[dir],op->y+freearr_y[dir]); tmp!=NULL; tmp=tmp->above,nflag=0)
    {	  
	/* Basially, if the object is magical and not counterspell, 
	 * we will more or less remove the object.  Changed in 0.94.3
	 * so that we don't kill monsters with this (some monsters attacktype
	 * has magic in it).
         */
	if(tmp->material==0 && tmp->attacktype&AT_MAGIC &&
	   !(tmp->attacktype&AT_COUNTERSPELL) && 
	   !QUERY_FLAG(tmp,FLAG_MONSTER))
		nflag=1;

	else switch(tmp->type) {
	    case CONE: case FBALL: case LIGHTNING:
	    case FBULLET: case MMISSILE: case SPEEDBALL:
	    case BOMB: case POISONCLOUD: case CANCELLATION:
	    case SWARM_SPELL:
	    case BALL_LIGHTNING:
		nflag=1;
		break;

	    case RUNE:
		nflag=2;
		break;

	    default:
		nflag=0;
	}
	switch(nflag) {
	    case 1: {
	    /* { if(op->level > tmp->level) { */ 
	        if(SK_level(op) > tmp->level) {
		    remove_ob(tmp);
		    free_object(tmp);
		}
		break;
	    }
	    case 2: {
		if(RANDOM()%150 == 0) {
		    tmp->stats.hp--;  /* weaken the rune */
		    if(!tmp->stats.hp) {
			remove_ob(tmp);
			free_object(tmp);
		    }
		}
		break;
	    }
	}
    }
}



/*  peterm:  function which summons hostile monsters and
  places them in nearby squares.  */
    
int summon_hostile_monsters(object *op,int n,char *monstername){
  int i;
  for(i=0;i<n;i++)
    put_a_monster(op,monstername);
  return n;
}


/*  charm spell by peterm@soda.berkeley.edu 
    searches nearby squares for monsters to charm.  Each of them
    is subject to being charmed, that is, to becoming the pet
    monster of the caster.  Monsters larger than 1 square
    are uncharmeable right now.  */

/* Aug 95 - hack on the code to make it charm only undead for 
 * priests, and not charm undead for magicians -b.t
 */

int cast_charm(object *op, object *caster,archetype *arch,int spellnum) {
  int i;
  object *tmp,*effect;
  
  if (op->type != PLAYER)
    return 0;
  for(i=1;i<MIN(9+SP_level_strength_adjust(op,caster,spellnum),SIZEOFFREE);i++) {
	if (out_of_map(op->map,op->x+freearr_x[i],op->y+freearr_y[i]))
	    continue;
	for(tmp=get_map_ob(op->map,op->x+freearr_x[i],op->y+freearr_y[i]);
	    tmp&&(!QUERY_FLAG(tmp,FLAG_MONSTER));tmp=tmp->above);
	if(!tmp) continue;
	if(tmp->type==PLAYER) continue;
	if(tmp->immune & AT_MAGIC) continue;
	if(QUERY_FLAG(tmp,FLAG_UNDEAD)) continue;
	if(tmp->more || tmp->head) continue;  /* multiple square monsters NOT */
	/* if(op->level <( (RANDOM()%(2*tmp->level+1))-(op->stats.Cha-10)/2)) continue; */ 
	if(SK_level(op) <( (RANDOM()%(2*tmp->level+1))-(op->stats.Cha-10)/2)) continue;

	if((effect=get_archetype("detect_magic"))){
		effect->x = tmp->x;
		effect->y = tmp->y;
		insert_ob_in_map(effect,tmp->map,op);
	}
	set_owner(tmp,op);
	SET_FLAG(tmp,FLAG_MONSTER);
	SET_FLAG(tmp,FLAG_FRIENDLY);
        add_friendly_object (tmp);
	tmp->stats.exp = 0;
	tmp->move_type = PETMOVE;
  }
  return 1;
}

int cast_charm_undead(object *op, object *caster,archetype *arch,int spellnum) {
  int i,bonus;
  object *tmp,*effect;

  if (op->type != PLAYER)
    return 0; 
  if (QUERY_FLAG(caster,FLAG_UNDEAD) || strstr(find_god(determine_god(op))->race,undead_name)!=NULL) {
    bonus = 5;
  } else if (strstr(find_god(determine_god(op))->slaying,undead_name)!=NULL) {
    bonus = -5;
  } else {
    bonus = -1;
  }
  for(i=1;i<MIN(9+SP_level_strength_adjust(op,caster,spellnum),SIZEOFFREE);i++) {
	if (out_of_map(op->map,op->x+freearr_x[i],op->y+freearr_y[i]))
	    continue;
        for(tmp=get_map_ob(op->map,op->x+freearr_x[i],op->y+freearr_y[i]);
            tmp&&(!QUERY_FLAG(tmp,FLAG_MONSTER));tmp=tmp->above);
        if(!tmp) continue;
        if(tmp->type==PLAYER) continue;
        if(tmp->immune & AT_MAGIC) continue;
        if(!QUERY_FLAG(tmp,FLAG_UNDEAD)) continue;
        if(tmp->more || tmp->head) continue;  /* multiple square monsters NOT */
        if(SK_level(op)+bonus < ( (RANDOM()%(2*tmp->level+1))-(op->stats.Wis-10)/2)) continue;

        if((effect=get_archetype("detect_magic"))){
                effect->x = tmp->x;
                effect->y = tmp->y;
                insert_ob_in_map(effect,tmp->map,op);
        }
        set_owner(tmp,op);
        SET_FLAG(tmp,FLAG_MONSTER);
        SET_FLAG(tmp,FLAG_FRIENDLY);
        add_friendly_object (tmp);
	tmp->stats.exp = 0;
        tmp->move_type = PETMOVE;
  }
  return 1;
}

/* Returns a monster (chosen at random) that this particular player (and his
 * god) find acceptable.  This checks level, races allowed by god, etc
 * to determine what is acceptable.
 * This returns NULL if no match was found.
 */

object *choose_cult_monster(object *pl, object *god, int summon_level) {
    char buf[MAX_BUF],*race;
    int racenr, mon_nr,i;
    racelink *list;
    objectlink *tobl;
    object *otmp;

    /* Determine the number of races available */
    racenr=0;
    strcpy(buf,god->race);
    race = strtok(buf,","); 
    while(race) { 
	racenr++; 
	race = strtok(NULL,",");  
     }
 
    /* next, randomly select a race from the aligned_races string */
    if(racenr>1) { 
	racenr = RANDOM()%racenr;
	strcpy(buf,god->race);
        race = strtok(buf,",");
        for(i=0;i<racenr;i++) 
	     race = strtok(NULL,",");
    } else 
        race = god->race;


    /* see if a we can match a race list of monsters.  This should not
     * happen, so graceful recovery isn't really needed, but this sanity
     * checking is good for cases where the god archetypes mismatch the
     * race file
     */
    if((list=find_racelink(race))==NULL) { 
	new_draw_info_format(NDI_UNIQUE, 0,pl,
	    "The spell fails! %s's creatures are beyond",god->name);
	new_draw_info(NDI_UNIQUE, 0,pl,
            "the range of your summons.");
	LOG(llevDebug,"choose_cult_monster() requested non-existant aligned race!\n");
	return 0; 
    }

    /* search for an apprplritate monster on this race list */ 
    mon_nr=0;
    for(tobl=list->member;tobl;tobl=tobl->next) {
	otmp=tobl->ob;
	if(!otmp||!QUERY_FLAG(otmp,FLAG_MONSTER)) continue;
	if(otmp->level<=summon_level) mon_nr++;
    }
    /* If this god has multiple race entries, we should really choose another.
     * But then we either need to track which ones we have tried, or just
     * make so many calls to this function, and if we get so many without
     * a valid entry, assuming nothing is available and quit.
     */
    if (!mon_nr) return NULL;
    mon_nr = RANDOM() % mon_nr;
    for(tobl=list->member;tobl;tobl=tobl->next) {
	otmp=tobl->ob;
	if(!otmp||!QUERY_FLAG(otmp,FLAG_MONSTER)) continue;
	if(otmp->level<=summon_level && !mon_nr--) return otmp;
    }
    /* This should not happen */
    LOG(llevDebug,"choose_cult_monster() mon_nr was set, but did not find a monster\n");
    return NULL;
}

    

/* summons a monster - the monster chosen is determined by the god
 * that is worshiiped.  return 0 on failure, 1 on success
 */

int summon_cult_monsters(object *op, int old_dir) {
    object *mon,*otmp,*god = find_god(determine_god(op));
    int tries=0,i,summon_level,number,dir;
    char buf[MAX_BUF];

    /* find deity */
    if (!god) {
	new_draw_info(NDI_UNIQUE, 0,op, "You worship no living deity!");
	return 0;
    } else if(!god->race) {
	new_draw_info_format(NDI_UNIQUE, 0,op,
		"%s has no creatures that you may summon!",god->name);
	return 0;
    }
    /* the summon level */
    i=SK_level(op)+op->stats.Wis/10;
    if (i==0) i=1;
    summon_level = RANDOM()%i;
    if(op->path_attuned&PATH_SUMMON) summon_level += 5;
    if(op->path_repelled&PATH_SUMMON) summon_level -= 5;

    do {
	/* Need to set dir each time, as it may get clobbered */
	dir=old_dir;
	mon=choose_cult_monster(op, god,summon_level);
	tries++;

	/* As per note in choose_cult_monster, if we have multiple race
	 * entries, we should really try again.
	 */
	if(!mon) {
	    new_draw_info_format(NDI_UNIQUE, 0,op, 
		 "%s fails to send anything.",god->name);
	    return 0;
	}
	/* Now lets see if we can find a place for this monster. */
	if (!dir) 
	    dir = find_free_spot(mon->arch, op->map, op->x, op->y, 1, SIZEOFFREE);

	/* This only checks for the head of the monster.  We still need
	 * to check for the body.  But if there is no space for the
	 * head, trying a different monster won't help, so might as well
	 * return now.
	 */
	if((dir==-1) || arch_blocked(mon->arch,op->map, op->x + freearr_x[dir], op->y+freearr_y[dir])) {
	    dir= -1;
	    if (tries == 5) {
		new_draw_info(NDI_UNIQUE, 0,op, "There is something in the way.");
		return 0;
	    }
	    else continue;  /* Try a different monster */
	}
    } while (dir==-1);

    /* Aha - we have found a monster - lets customize it an put it on the
     * map.
     */

    if(mon->level>(summon_level/2))
	number = RANDOM()%2 + 1;
    else
	number = RANDOM()%2 + RANDOM()%2 + 2;

    for (i = 1; i < number + 1; i++) {
	object *head;

	/* this allows multisq monsters to be done right */
	if(!(head = fix_summon_pet( mon->arch, op, dir, 0))) continue;

	/* now, a little bit of tailoring. If the monster is much lower in 
	 * level than the summon_level, we get a better monster */

	if((head->level+5)<summon_level) { 
	    int ii;

	    for(ii=summon_level-(head->level)-5;ii>0;ii--) {
		switch(RANDOM()%3+1) {
		    case 1:
			head->stats.wc--;
			break;
		    case 2:
			head->stats.ac--;
			break;
		    case 3:
			head->stats.dam+=3;
			break;
		    default:
			break;
		}
		head->stats.hp+=3;
	    }

	    head->stats.maxhp=head->stats.hp;
	    for(otmp=head;otmp;otmp=otmp->more) {
		if(otmp->name) {
		    if(summon_level>30+head->level) 
		    sprintf(buf,"Arch %s of %s",head->name,god->name);
		    else
			sprintf(buf,"%s of %s",head->name,god->name);

		    free_string(otmp->name);
		    otmp->name=add_string(buf);
		}
	    }
	} /* if monster level is much less than character level */

	head = insert_ob_in_map (head, op->map, op);
	if (head != NULL && head->randomitems != NULL) {
	    object *tmp;
	    create_treasure(head->randomitems,head,GT_INVENTORY,6,0);
	    for(tmp = head->inv; tmp != NULL; tmp = tmp->below)
		if(!tmp->nrof)
		    SET_FLAG(tmp, FLAG_NO_DROP);
	}
	dir = absdir(dir + 1);
	if (arch_blocked(head->arch,op->map, op->x + freearr_x[dir],
		op->y + freearr_y[dir])) {

	    if (i < number) {
		new_draw_info(NDI_UNIQUE, 0,op, "There is something in the way,");
		new_draw_info(NDI_UNIQUE, 0,op, "no more monsters for this casting.");
		return 1;
	    }
	}
    } /* Loop to insert all the monster */
    return 1;
}

/* summon_avatar() - taken from the code which summons golems. We 
 * cant use because we need to throw in a few extra's here. b.t.
 */
 
int summon_avatar(object *op,object *caster,int dir, archetype *at, int spellnum) {
  object *tmp;
#ifdef MULTIPLE_GODS
  char buf[MAX_BUF];
  object *god = find_god(determine_god(caster)); 

  if(god)
    at=(spellnum==SP_HOLY_SERVANT)?god->other_arch:god->arch;
  else {
      new_draw_info(NDI_UNIQUE, 0,op,"You must worship a god first.");
      return 0; 
  }
  if(!at) 
      new_draw_info_format(NDI_UNIQUE, 0,op,"%s has no %s for you to call."
	  ,god->name,spellnum==SP_SUMMON_AVATAR?"avatar":"servant");
#endif

  /* safety checks... */
  if(op->type==PLAYER)
    if(op->contr->golem!=NULL&&!QUERY_FLAG(op->contr->golem,FLAG_FREED)) {
      control_golem(op->contr->golem,dir);
      return 0;
    }
  if(!dir)
    dir = find_free_spot(at, op->map, op->x, op->y, 1, SIZEOFFREE);

  if((dir==-1) || arch_blocked(at,op->map, op->x + freearr_x[dir], op->y+freearr_y[dir])) { 
    new_draw_info(NDI_UNIQUE, 0,op,"There is something in the way.");
    if(op->type==PLAYER)
      op->contr->count_left=0;
    return 0;
  }

  if(!(tmp=fix_summon_pet(at,op,dir,GOLEM))) { 
    new_draw_info(NDI_UNIQUE, 0,op,"Your spell fails.");
    return 0;
  }

  if(op->type==PLAYER) {
    op->contr->golem=tmp;
    /* give the player control of the golem */
    op->contr->shoottype=range_scroll;
  } 

  /*  This sets the level dependencies on dam, wc and hp */
  tmp->stats.wc -= SP_level_strength_adjust(op,caster,spellnum);
  if(tmp->stats.wc<-127) tmp->stats.wc = -127;
  tmp->stats.hp += SP_PARAMETERS[spellnum].bdur +
                         12 * SP_level_strength_adjust(op,caster,spellnum);
  tmp->stats.dam += SP_PARAMETERS[spellnum].bdam +(2*SP_level_dam_adjust(op,caster,spellnum));
  if(tmp->stats.dam<0) tmp->stats.dam=127;  /*seen this go negative!*/
  if(tmp->other_arch) tmp->other_arch=NULL;

#ifdef MULTIPLE_GODS /* tailor it to the gods nature */
  if(tmp) { 
    object *tmp2;
    for(tmp2=tmp;tmp2;tmp2=tmp2->more) { 
      sprintf(buf,"%s of %s",spellnum==SP_SUMMON_AVATAR?"Avatar":"Servant",god->name);
      if(tmp2->name) free_string(tmp2->name);
      tmp2->name = add_string(buf);
    }
  }
  tmp->attacktype|=god->attacktype;
  tmp->vulnerable|=god->vulnerable;
  tmp->immune|=god->immune;
  tmp->protected|=god->protected;
  if (tmp->race) {
    free_string (tmp->race);
    tmp->race = NULL;
  }
  if (tmp->slaying) {
    free_string (tmp->slaying);
    tmp->slaying = NULL;
  }
  if(god->race) tmp->race = add_string(god->race);
  if(god->slaying) tmp->slaying = add_string(god->slaying);
  /* safety, we must allow a god's servants some reasonable attack */
  if(!(tmp->attacktype&AT_PHYSICAL)) tmp->attacktype|=AT_PHYSICAL;
#else
  tmp->attacktype|=AT_WEAPONMAGIC;
#endif

  /*  make experience increase in proportion to the strength of 
   *  the summoned creature. */
  tmp->stats.exp *= SP_level_spellpoint_cost(op,caster,spellnum)/spells[spellnum].sp;
  tmp->speed_left= -1;
  tmp->x=op->x+freearr_x[dir],tmp->y=op->y+freearr_y[dir];
  tmp->direction=dir;
  insert_ob_in_map(tmp,op->map,op);
  return 1;
}
 
/* fix_summon_pet() - this makes multisquare/single square monsters
 * properly for map insertion. 
 */
object *fix_summon_pet(archetype *at, object *op, int dir, int type ) {
    archetype *atmp;
    object *tmp=NULL, *prev=NULL, *head=NULL; 
 
    for(atmp = at; atmp!=NULL; atmp = atmp->more) {
      tmp = arch_to_object(atmp);
      if (atmp == at) {
        if(type!=GOLEM) 
	  SET_FLAG(tmp, FLAG_MONSTER);
        set_owner(tmp, op);
        if (op->type == PLAYER) {
          tmp->stats.exp = 0;
          add_friendly_object(tmp);
          SET_FLAG(tmp, FLAG_FRIENDLY);
          if(type==GOLEM) CLEAR_FLAG(tmp, FLAG_MONSTER);
        } else { 
          if(QUERY_FLAG(op, FLAG_FRIENDLY)) {
      	    object *owner = get_owner(op);
            if(owner != NULL) {/* For now, we transfer ownership */
               set_owner(tmp,owner);
               tmp->move_type = PETMOVE;
               add_friendly_object(tmp);
               SET_FLAG(tmp, FLAG_FRIENDLY);
	    }
          }
        }
        if(op->type!=PLAYER||type!=GOLEM) { 
	  tmp->move_type = PETMOVE;
          tmp->speed_left = -1;
          tmp->type = 0;
          tmp->enemy = op->enemy; 
        } else
	  tmp->type = GOLEM;
       
      }  
      if(head == NULL)
        head = tmp;
      tmp->x = op->x + freearr_x[dir] + tmp->arch->clone.x;
      tmp->y = op->y + freearr_y[dir] + tmp->arch->clone.y;
      tmp->map = op->map;
      if(tmp->invisible) tmp->invisible=0;
      if(head != tmp)
        tmp->head = head, prev->more = tmp;
      prev = tmp;
    }
    head->direction = dir;

    /* need to change some monster attr to prevent problems/crashing */
    if(head->last_heal) head->last_heal=0;
    if(head->last_eat) head->last_eat=0;
    if(head->last_grace) head->last_grace=0;
    if(head->last_sp) head->last_sp=0;
    if(head->attacktype&AT_GHOSTHIT) head->attacktype=(AT_PHYSICAL|AT_DRAIN);
    if(head->other_arch) head->other_arch=NULL;
    if(QUERY_FLAG(head,FLAG_CHANGING)) CLEAR_FLAG(head,FLAG_CHANGING);
    if(QUERY_FLAG(head,FLAG_STAND_STILL)) CLEAR_FLAG(head,FLAG_STAND_STILL);
    if(QUERY_FLAG(head,FLAG_GENERATOR)) CLEAR_FLAG(head,FLAG_GENERATOR);
    if(QUERY_FLAG(head,FLAG_SPLITTING)) CLEAR_FLAG(head,FLAG_SPLITTING);

    return head;
}

/* cast_consecrate() - a spell to make an altar your god's */
 
int cast_consecrate(object *op) {
    char buf[MAX_BUF];
    int success=0;

#ifdef MULTIPLE_GODS
    object *tmp, *god=find_god(determine_god(op));

    if(!god) {
	new_draw_info(NDI_UNIQUE, 0,op,
		      "You can't consecrate anything if you don't worship a god!");
        return 0;
    }
 
    for(tmp=op->below;tmp;tmp=tmp->below) {
        if(tmp->type==HOLY_ALTAR) {
	    if(QUERY_FLAG(tmp,FLAG_IS_FLOOR)) break;

	    /* We use SK_level here instead of path_level mod because I think
	     * all the gods should give equal chance of re-consecrating altars
	     */
	    if(tmp->level > SK_level(op)) {
		new_draw_info_format(NDI_UNIQUE, 0,op,
		    "You are not poweful enough to reconsecrate the altar of %s", tmp->title);
	    } else {
	    /* If we got here, we are consecrating an altar */
		if(tmp->name)	free_string(tmp->name);
		sprintf(buf,"%s of %s",tmp->arch->clone.name,god->name);
		tmp->name = add_string(buf);
		tmp->level = SK_level(op);
		tmp->other_arch = god->arch;
		if(op->type==PLAYER) esrv_update_item(UPD_NAME, op, tmp);
		new_draw_info_format(NDI_UNIQUE,0, op,
				     "You consecrated the altar to %s!",god->name);
		return 1;
	    }
	}
    }
    new_draw_info(NDI_UNIQUE, 0,op,"You are not standing over an altar!");
    return 0;
#else
    new_draw_info(NDI_UNIQUE, 0,op,"Nothing happens.");
    return 0;
#endif
}


/* finger_of_death() - boss high-level cleric spell. */

int finger_of_death(object *op, object *caster, int dir) {
  object *hitter,*target=get_pointed_target(op,dir);
  int success = 1;

  if(!target || QUERY_FLAG(target,FLAG_REFL_SPELL)) {
        new_draw_info(NDI_UNIQUE,0,op,"Nothing happens.");
	return 0;
  }
 
  /* we create a hitter object -- the spell */
  hitter=get_archetype("face_of_death");
  hitter->level = path_level_mod (caster, SP_PARAMETERS[SP_FINGER_DEATH].bdam + 
        3*SP_level_dam_adjust(op,caster,SP_FINGER_DEATH), SP_FINGER_DEATH);
  set_owner(hitter,op);
  hitter->x=target->x;
  hitter->y=target->y;
  hitter->stats.maxhp=hitter->count; /*??*/

  /* there are 'grave' consequences for using this spell on the unliving! */
  if(QUERY_FLAG(target,FLAG_UNDEAD)) {
      success = 0;
      if(RANDOM()%3) { 
        new_draw_info(NDI_UNIQUE,0,op,"Idiot! Your spell boomerangs!"); 
	hitter->x=op->x;
	hitter->y=op->y;	
      } else { 
        new_draw_info_format(NDI_UNIQUE,0,op,"The %s looks stronger!",
	  query_name(target)); 	
	target->stats.hp = target->stats.maxhp*2; 
        free_object(hitter);
	return 0;	
      }
  }  

  insert_ob_in_map(hitter,op->map,op); 
 
  return success;
}

/* animate_weapon - 
 * Generalization of staff_to_snake.  Makes a golem out of the caster's weapon.
 * The golem is based on the archetype specified, modified by the caster's level
 * and the attributes of the weapon.  The weapon is inserted in the golem's 
 * inventory so that it falls to the ground when the golem dies.	-- DAMN	*/

int animate_weapon(object *op,object *caster,int dir, archetype *at, int spellnum) {
  object *weapon, *tmp;
  char buf[MAX_BUF];
  int a, i, j;
  int magic;
 
  if(!at){
    new_draw_info(NDI_UNIQUE, 0,op,"Oops, program error!");
    LOG(llevError,"animate_weapon failed: missing archetype!\n");
    return 0;
  }
  /* if player already has a golem, abort */
  if(op->type==PLAYER)
    if(op->contr->golem!=NULL&&!QUERY_FLAG(op->contr->golem,FLAG_FREED)) {
      control_golem(op->contr->golem,dir);
      return 0;
    }
  /* if no direction specified, pick one */
  if(!dir) 
    dir=find_free_spot(NULL,op->map,op->x,op->y,1,9);
  /* if there's no place to put the golem, abort */
  if((dir==-1) || blocked(op->map,op->x+freearr_x[dir],op->y+freearr_y[dir])) {
    new_draw_info(NDI_UNIQUE, 0,op,"There is something in the way.");
    if(op->type==PLAYER)
      op->contr->count_left=0;
    return 0;
  }

  if(spellnum == SP_DANCING_SWORD) {
    archetype *weapon_at = find_archetype("sword");
    if(weapon_at) {
      weapon = &(weapon_at->clone);
    } else {
      new_draw_info(NDI_UNIQUE, 0,op,"Oops, program error!");
      LOG(llevError,"animate_weapon failed: missing archetype!\n");
      return 0;
    }
  } else {
    /* get the weapon to transform */
    for(weapon = op->inv; weapon; weapon = weapon->below)
      if(weapon->type==WEAPON&&QUERY_FLAG(weapon,FLAG_APPLIED)) break;
    if(!weapon) {
      if(op->type==PLAYER) { 
	new_draw_info(NDI_UNIQUE, 0,op,"You need to wield a weapon to animate it.");
	op->contr->count_left=0;   
      } 	
      return 0;
    } else if (spellnum == SP_STAFF_TO_SNAKE && strcmp(weapon->name,"quarterstaff")) {
      if(op->type==PLAYER) { 
	new_draw_info(NDI_UNIQUE, 0,op,"The spell fails to transform your weapon.");
	op->contr->count_left=0;   
      }
      return 0;
    } else if(op->type==PLAYER && 
	      (QUERY_FLAG(weapon,FLAG_CURSED) || QUERY_FLAG(weapon,FLAG_DAMNED))) {
      new_draw_info(NDI_UNIQUE, 0,op,"You can't animate it.  It won't let go of your hand.");
      op->contr->count_left=0;   
      return 0;
    }
  }
  magic = weapon->magic>0 ? weapon->magic : -1*weapon->magic;

  /* create the golem object */
  tmp=arch_to_object(at);

  /* if animated by a player, give the player control of the golem */
  if(op->type==PLAYER) {
    CLEAR_FLAG(tmp, FLAG_MONSTER);
    SET_FLAG(tmp, FLAG_FRIENDLY);
    tmp->stats.exp=0;
    add_friendly_object(tmp);
    tmp->type=GOLEM;
    set_owner(tmp,op);
    op->contr->golem=tmp;
    op->contr->shoottype=range_scroll;
  } else {
  /* If spell is cast by a pet, and the weapon is not cursed, make the animated
   * weapon a pet. */
    if(QUERY_FLAG(op, FLAG_FRIENDLY)
       && !QUERY_FLAG(weapon,FLAG_CURSED)
       && !QUERY_FLAG(weapon,FLAG_DAMNED)){
      object *owner = get_owner(op);
      if(owner != NULL) {
        set_owner(tmp,owner);
        tmp->move_type = PETMOVE;
        add_friendly_object(tmp);
        SET_FLAG(tmp, FLAG_FRIENDLY);
      }
    }
    /* otherwise, make the golem an enemy */
    SET_FLAG(tmp, FLAG_MONSTER);
  }

  /* ok, tailor the golem's characteristics based on the weapon */
  if (spellnum == SP_STAFF_TO_SNAKE || spellnum == SP_ANIMATE_WEAPON) {
    if (apply_special (op, weapon,
                       AP_UNAPPLY | AP_IGNORE_CURSE | AP_NO_MERGE))
    {
      LOG (llevError, "BUG: animate_weapon(): can't unapply weapon\n");
      return 0;
    }
    remove_ob (weapon);
    insert_ob_in_ob (weapon, tmp);
    if (op->type == PLAYER)
      esrv_send_item(op, weapon);
    SET_FLAG (tmp, FLAG_USE_WEAPON);
    if (apply_special (tmp, weapon, AP_APPLY))
      LOG (llevError, "BUG: animate_weapon(): golem can't apply weapon\n");
  }

  /* modify weapon's animated wc */
  tmp->stats.wc = tmp->stats.wc
    - SP_level_dam_adjust(op,caster,spellnum)
    - 5 * weapon->stats.Dex
    - 2 * weapon->stats.Str
    - magic;
  /* Modify hit points for weapon */
  tmp->stats.maxhp = tmp->stats.maxhp
    + SP_PARAMETERS[spellnum].bdur
    + 4 * SP_level_strength_adjust(op,caster,spellnum)
    + 8 * magic
    + 12 * weapon->stats.Con;
  /* Modify weapon's damage */
  tmp->stats.dam = SP_PARAMETERS[spellnum].bdam
    + weapon->stats.dam
    + magic
    + 2 * SP_level_dam_adjust(op,caster,spellnum)
    + 5 * weapon->stats.Str;
  /* sanity checks */
  if(tmp->stats.wc<-127) tmp->stats.wc = -127;
  if(tmp->stats.maxhp<0) tmp->stats.maxhp=10;
  tmp->stats.hp = tmp->stats.maxhp;
  if(tmp->stats.dam<0) tmp->stats.dam=127;
  LOG(llevDebug,"animate_weapon: wc:%d  hp:%d  dam:%d.\n", tmp->stats.wc, tmp->stats.hp, tmp->stats.dam);
  /* attacktype */
  if ( ! tmp->attacktype)
    tmp->attacktype = AT_PHYSICAL;
  for(i=0; i<NROFMATERIALS; i++)
    for(j=0; j<NROFATTACKS; j++)
      if(weapon->material & (1<<i)) {
	if(material[i].save[j] < 3) 
	  tmp->protected |= (1<<j);
	else if(material[i].save[j] > 14) 
	  tmp->vulnerable |= (1<<j);
      }
  tmp->protected &= ~weapon->vulnerable;
  tmp->vulnerable &= ~(weapon->immune | weapon->protected);
  /* Set weapon's immunity */
  tmp->immune = weapon->immune | AT_CONFUSION | AT_POISON | AT_SLOW
    | AT_PARALYZE | AT_TURN_UNDEAD | AT_FEAR | AT_DEPLETE | AT_DEATH
    | AT_BLIND;
  /* Improve weapon's armour value according to best save vs. physical of its material */
  for(a=0,i=0; i<NROFMATERIALS; i++) {
    if(weapon->material & (1<<i) && material[i].save[0] > a) {
      a = material[i].save[0];
    }
  }
  tmp->armour = 100 - (int)((100.0-(float)tmp->armour)/(30.0-2.0*(a>14?14.0:(float)a)));
  LOG (llevDebug, "animate_weapon: slaying %s\n",
       tmp->slaying ? tmp->slaying : "nothing");

  /* Determine golem's speed */
  tmp->speed = ((weapon->last_sp>0.1)?weapon->last_sp:0.1) -
    (((weapon->last_sp>0.1)?weapon->last_sp:0.1)/
     ( (1.0 * (float)SP_level_dam_adjust(op, caster, spellnum) +
	1.5 * (float)weapon->stats.Dex +
	2.0 * (float)weapon->stats.Str +
	3.0 * (float)magic +
	5.0 * (float)weapon->stats.exp) )
     * ((float)((tmp->weight>1000)? tmp->weight : 1000)
	/ ((float)(weapon->weight>1000)? weapon->weight : 1000)));
  if(tmp->speed > 3.33) tmp->speed = 3.33;
  LOG(llevDebug,"animate_weapon: armour:%d  speed:%f  exp:%d.\n",
      tmp->armour, tmp->speed, tmp->stats.exp);

  /* spell-dependent finishing touches and descriptive text */
  switch(spellnum) {

  case SP_STAFF_TO_SNAKE:
    new_draw_info(NDI_UNIQUE, 0,op,"Your staff becomes a serpent and leaps to the ground!");
    break;

  case SP_ANIMATE_WEAPON:
    new_draw_info_format(NDI_UNIQUE, 0,op,"Your %s flies from your hand and hovers in mid-air!", weapon->name);
    sprintf(buf, "animated %s", weapon->name);
    if(tmp->name) free_string(tmp->name);
    tmp->name = add_string(buf);

    tmp->face = weapon->face;
    tmp->animation_id = weapon->animation_id;
    tmp->anim_speed = weapon->anim_speed;
    tmp->last_anim = weapon->last_anim;
    tmp->state = weapon->state;
    if(QUERY_FLAG(weapon, FLAG_ANIMATE)) {
      SET_FLAG(tmp,FLAG_ANIMATE); 
    } else {
      CLEAR_FLAG(tmp,FLAG_ANIMATE); 
    }
    update_ob_speed(tmp);
    break;

  case SP_DANCING_SWORD:
    new_draw_info(NDI_UNIQUE, 0,op,"A magical sword appears in mid air, eager to slay your foes for you!");
    break;
  default:
    break;
  }

  /*  make experience increase in proportion to the strength of the summoned creature. */
  tmp->stats.exp *= SP_level_spellpoint_cost(op,caster,spellnum)/spells[spellnum].sp;
  tmp->speed_left= -1;
  tmp->x=op->x+freearr_x[dir],tmp->y=op->y+freearr_y[dir];
  tmp->direction=dir;
  insert_ob_in_map(tmp,op->map,op);
  return 1;
}

/* cast_daylight() - changes the map darkness level *lower* */

int cast_daylight ( object *op ) {
  int success = 0;
  mapstruct *m=op->map;

  if(!m) return 0;   /* shouldnt happen */ 

  if(!(success=change_map_light(m,-1)))
        new_draw_info(NDI_UNIQUE,0,op,"It can be no brighter here."); 

  return success;
}

/* cast_nightfall() - changes the map darkness level *higher* */

int cast_nightfall ( object *op ) {
  int success=0;
  mapstruct *m=op->map;

  if(!m) return 0;   /* shouldnt happen */

  if(!(success=change_map_light(m,1)))
        new_draw_info(NDI_UNIQUE,0,op,"It can be no darker here."); 

  return success;
}

/* cast_faery_fire() - this spell primary purpose is to light
 * up all single-space monsters on a map. Magic immune and 
 * multi-space monsters are currently not supposed to light 
 * up. If USE_LIGHTING is not defined, this spell is only
 * capable of doing minor fire damage. I hacked this out of 
 * the destruction code. -b.t.
 */
 
int cast_faery_fire(object *op,object *caster) {
  int r,dam,i,j,success=0,factor;
  object *tmp;
  if(op->type!=PLAYER)
    return 0;
  /* the smaller this is, the longer it glows */
  factor=SP_PARAMETERS[SP_FAERY_FIRE].bdur 
	+ SP_level_strength_adjust(op,caster,SP_FAERY_FIRE);
  r=SP_PARAMETERS[SP_FAERY_FIRE].bdam
	+SP_level_dam_adjust(op,caster,SP_FAERY_FIRE);
  r = 5;
  factor = 10;
  dam=(SK_level(op)/10)+1;
  for(i= -r;i<r;i++)
    for(j= -r;j<r;j++) {
      if(out_of_map(op->map,op->x+i,op->y+j))
        continue;
      tmp=get_map_ob(op->map,op->x+i,op->y+j);
      while(tmp!=NULL&&(!QUERY_FLAG(tmp, FLAG_ALIVE)
		||tmp->type==PLAYER||tmp->more||tmp->head
		||tmp->immune==AT_MAGIC))
        tmp=tmp->above;
      if(tmp==NULL)
        continue;
#ifndef USE_LIGHTING
      success += hit_player(tmp,dam,op,AT_FIRE|AT_MAGIC);
#else
      if(make_object_glow(tmp,1,factor)) { 
	object *effect=get_archetype("detect_magic");
	success++;
        if(effect){
                effect->x = tmp->x;
                effect->y = tmp->y;
                insert_ob_in_map(effect,op->map,op);
        }
      }
#endif
    }
  return success;
}

#if 0
I set this out for now because the object code wont allow 
non-living objects to glow viz force in their inventory. This
is because (Ibelive) the code doesnt currently allow non-living
objects (exception: containers) to have an inventory. I may be
wrong, but have not time to check it out now.. 
 
/* cast_glow() */

int cast_glow( object *op, object *caster,int dir) {
  int x,y,time,radius;
  object *tmp;

  if(!dir) return 0;
  x=op->x+freearr_x[dir]; 
  y=op->y+freearr_y[dir];

  if(out_of_map(op->map,x,y)) return 0;

  for(tmp=get_map_ob(op->map,x,y);tmp;tmp=tmp->above) {
    if(QUERY_FLAG(tmp, FLAG_ALIVE)||tmp->immune&AT_MAGIC)
	continue;
    if(QUERY_FLAG(tmp,FLAG_IS_FLOOR)) break;
  }

  if(!tmp||QUERY_FLAG(tmp,FLAG_IS_FLOOR)) { /* argh. nothing on the floor, 
					     * lets look at the inventory */
	if(op->inv) tmp = op->inv; /* take first item */ 
	else return 0;
  }

  time = (SP_PARAMETERS[SP_GLOW].bdur 
              - (10*SP_level_strength_adjust(op,caster,SP_GLOW))); 
  radius=SP_PARAMETERS[SP_GLOW].bdam
              + SP_level_dam_adjust(op,caster,SP_GLOW);

  LOG(llevDebug,"cast_glow got item: %s to attempt glow\n");
  /* ok, lets try to make tmp glow */
  return make_object_glow(tmp,radius,time);
}

#endif

/* make_object_glow() - currently only makes living objects glow.
 * we do this by creating a "force" and inserting it in the 
 * object. if time is 0, the object glows permanently. To truely
 * make this work for non-living objects, we would have to 
 * give them the capability to have an inventory. b.t.
 */

int make_object_glow(object *op, int radius, int time) {
  object *tmp;

   /* some things are unaffected... */
   if(op->path_denied&PATH_LIGHT) 
	return 0;
  
   tmp=get_archetype("force");
   tmp->speed = 0.000001 * time;
   tmp->glow_radius=radius;
   tmp->x=op->x,tmp->y=op->y;
   if(tmp->speed<MIN_ACTIVE_SPEED) tmp->speed = MIN_ACTIVE_SPEED; /* safety */
   tmp=insert_ob_in_ob(tmp,op);

   if(!tmp->env||op!=tmp->env) { 
	LOG(llevError,"make_object_glow() failed to insert glowing force in %s\n", 
		op->name); 
	return 0; 
   }
   return 1;
}


/*  cast_cause_disease:  this spell looks along <dir> from the
player and infects someone. */

int cast_cause_disease(object *op, object *caster, int dir, archetype *disease_arch, int type) {
  int x,y,i;
  object *walk;

  x = op->x;  y = op->y;


  /* search in a line for a victim */
  for(i=0;i<5;i++) {
	 x += freearr_x[dir]; y+= freearr_y[dir];
	 if(out_of_map(op->map,x,y)) continue;
	 /* search this square for a victim */
	 for(walk=get_map_ob(op->map,x,y);walk;walk=walk->above)
		if(QUERY_FLAG(walk,FLAG_MONSTER) 
			|| !(walk->type==PLAYER)) {  /* found a victim */

		  object *disease = arch_to_object(disease_arch);
		  set_owner(disease,op);
		  disease->stats.exp = 0;
		  disease->level = op->level;
		  
		  /* Try to get the experience into the correct category */
		  if(op->chosen_skill && op->chosen_skill->exp_obj)
			 disease->exp_obj = op->chosen_skill->exp_obj;

		  /*do level adjustments */
		  if(disease->stats.wc)
			 disease->stats.wc +=  SP_level_strength_adjust(op,caster,type)/2;

		  if(disease->magic> 0)
			 disease->magic += SP_level_strength_adjust(op,caster,type)/4;

		  if(disease->stats.maxhp>0)
			 disease->stats.maxhp += SP_level_strength_adjust(op,caster,type);

		  if(disease->stats.maxgrace>0)
			 disease->stats.maxgrace += SP_level_strength_adjust(op,caster,type);

		  if(disease->stats.dam) {
			 if(disease->stats.dam > 0)
				disease->stats.dam += SP_level_dam_adjust(op,caster,type);
			 else disease->stats.dam -= SP_level_dam_adjust(op,caster,type);
		  }

		  if(disease->last_sp) {
			 disease->last_sp -= 2*SP_level_dam_adjust(op,caster,type);
			 if(disease->last_sp <1) disease->last_sp = 1;
		  }

		  if(disease->stats.maxsp) {
			 if(disease->stats.maxsp > 0)
				disease->stats.maxsp += SP_level_dam_adjust(op,caster,type);
			 else disease->stats.maxsp -= SP_level_dam_adjust(op,caster,type);
		  }
		  
		  if(disease->stats.ac) 
			 disease->stats.ac += SP_level_dam_adjust(op,caster,type);

		  if(disease->last_eat)
			 disease->last_eat -= SP_level_dam_adjust(op,caster,type);

		  if(disease->stats.hp)
			 disease->stats.hp -= SP_level_dam_adjust(op,caster,type);

		  if(disease->stats.sp)
			 disease->stats.sp -= SP_level_dam_adjust(op,caster,type);
		  

		  if(infect_object(walk,disease,1)) {
			 char buf[128];
			 object *flash;  /* visual effect for inflicting disease */
			 sprintf(buf,"You inflict %s on %s!",disease->name,walk->name);
			 free_object(disease); /* don't need this one anymore */
			 new_draw_info(NDI_UNIQUE,0,op,buf);
			 flash=get_archetype("detect_magic");
			 flash->x = x;
			 flash->y = y;
			 flash->map = walk->map;
			 insert_ob_in_map(flash,walk->map,op);
			 return 1;
		  }
		  free_object(disease);
		}
	 /* no more infecting through walls. */
	 if(blocked(op->map,x,y)) return 0;
  }
  new_draw_info(NDI_UNIQUE,0,op,"No one caught anything!");
  return 0;
}



/* move aura function.  An aura is a part of someone's inventory,
which he carries with him, but which acts on the map immediately
around him.
Aura parameters:
food:  duration counter.   
attacktype:  aura's attacktype 
other_arch:  archetype to drop where we attack

 */

void move_aura(object *aura) {
  int i;
  object *env;

  /* no matter what we've gotta remove the aura...
     we'll put it back if its time isn't up.  */
  remove_ob(aura);

  /* exit if we're out of gas */
  if(aura->stats.food--< 0) {
    free_object(aura);
    return;
  }
  /* auras only exist in inventories */
  if(aura->env == NULL || aura->env->map==NULL) {
    free_object(aura);
    return;
  }
  env = aura->env;
  aura->x = env->x;
  aura->y = env->y;

    /* we need to jump out of the inventory for a bit
       in order to hit the map conveniently. */
  insert_ob_in_map(aura,env->map,aura);
  for(i=1;i<9;i++) { 
    hit_map(aura,i,aura->attacktype);
    if(aura->other_arch) {
      object *new_ob;
      int nx, ny;
      nx = aura->x + freearr_x[i];
      ny = aura->y + freearr_y[i];
      /* we're done if the "i" square next to us is full */
      if(out_of_map(aura->map,nx,ny) ||
	 wall(aura->map,nx,ny)) continue;
      new_ob = arch_to_object(aura->other_arch);
      new_ob->x = nx;
      new_ob->y = ny;
      insert_ob_in_map(new_ob,env->map,aura);
    }
  }
}
      

  
