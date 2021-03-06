/*
 * static char *rcsid_shop_c =
 *   "$Id$";
 */

/*
    CrossFire, A Multiplayer game for X-windows

    Copyright (C) 2002 Mark Wedel & Crossfire Development Team
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

    The authors can be reached via e-mail at crossfire-devel@real-time.com
*/

#include <assert.h>
#include <global.h>
#include <spells.h>
#include <skills.h>
#include <living.h>
#include <newclient.h>
#ifndef __CEXTRACT__
#include <sproto.h>
#endif
#include <math.h>

/* this is a measure of how effective store specialisation is. A general store
 * will offer this proportion of the 'maximum' price, a specialised store will
 * offer a range of prices around it such that the maximum price is always one
 * therefore making this number higher, makes specialisation less effective. 
 * setting this value above 1 or to a negative value would have interesting,
 * (though not useful) effects.
 */
#define SPECIALISATION_EFFECT 0.5

/* price a shopkeeper will give to someone they disapprove of.*/
#define DISAPPROVAL_RATIO 0.2 

/* price a shopkeeper will give someone they neither like nor dislike */
#define NEUTRAL_RATIO 0.8

static uint64 pay_from_container(object *pl, object *pouch, uint64 to_pay);
static uint64 value_limit(uint64 val, int quantity, const object *who, int isshop);
static double shop_specialisation_ratio(const object *item, const mapstruct *map);
static double shop_greed(const mapstruct *map);

#define NUM_COINS 5	/* number of coin types */
#define LARGEST_COIN_GIVEN 2 /* never give amber or jade, but accept them */
static const char* const coins[] = {"ambercoin", "jadecoin","platinacoin", "goldcoin", "silvercoin", NULL};

/* Added F_TRUE flag to define.h to mean that the price should not
 * be adjusted by players charisma. With F_TRUE, it returns the amount
 * that the item is worth, if it was sold, but unadjusted by charisma.
 * This is needed for alchemy, to to determine what value of gold nuggets
 * should be given (the gold nuggets, when sold, will have the adjustment
 * by charisma done at that time).  NULL could have been passed as the
 * who parameter, but then the adjustment for expensive items (>10000)
 * would not be done.
 *
 * Added F_APPROX flag, which means that the price returned should be wrong by
 * an amount related to the player's bargaining skill.
 *
 * Added F_SHOP flag to mean that the specialisation of the shop on the player's 
 * current map should be taken into account when determining the price. Shops that
 * specialise in what is being traded will give better prices than those that do not. 
 *
 * CF 0.91.4 - This function got changed around a bit.  Now the
 * number of object is multiplied by the value early on.  This fixes problems
 * with items worth very little.  What happened before is that various
 * divisions took place, the value got rounded to 0 (Being an int), and
 * thus remained 0.
 *
 * Mark Wedel (mwedel@pyramid.com)
 */
uint64 query_cost(const object *tmp, object *who, int flag) {
    uint64 val;
    int number;	/* used to better calculate value */
    int no_bargain;
    int identified;
    int not_cursed;
    int approximate;
    int shop;
    float diff;
    float ratio;

    no_bargain = flag & F_NO_BARGAIN;
    identified = flag & F_IDENTIFIED;
    not_cursed = flag & F_NOT_CURSED;
    approximate = flag & F_APPROX;
    shop = flag & F_SHOP;
    flag &= ~(F_NO_BARGAIN|F_IDENTIFIED|F_NOT_CURSED|F_APPROX|F_SHOP);
 
    if (tmp->type==MONEY) return (tmp->nrof * tmp->value);
    if (tmp->type==GEM) {
	if (flag==F_TRUE) return (tmp->nrof * tmp->value);
	if (flag==F_BUY) return (1.03 * tmp->nrof * tmp->value);
	if (flag==F_SELL) return (0.97 * tmp->nrof * tmp->value);
	LOG(llevError,"Query_cost: Gem type with unknown flag : %d\n", flag);
	return 0;
    }
    number = tmp->nrof;
    if (number==0) number=1;
    if (QUERY_FLAG(tmp, FLAG_IDENTIFIED) || !need_identify(tmp) || identified) {
	if (!not_cursed && (QUERY_FLAG(tmp, FLAG_CURSED) || QUERY_FLAG(tmp, FLAG_DAMNED)))
	    return 0;
	else
	    val=tmp->value * number;
    }
    /* This area deals with objects that are not identified, but can be */
    else {
	if (tmp->arch != NULL) {
	    if (flag == F_BUY) {
		LOG(llevError, "Asking for buy-value of unidentified object.\n");
		val = tmp->arch->clone.value * 50 * number;
	    }
	    else {	/* Trying to sell something, or get true value */
		if (tmp->type == POTION)
		    val = number * 1000; /* Don't want to give anything away */
		else {
		    /* Get 2/3'rd value for applied objects, 1/3'rd for totally
		     * unknown objects
		     */
		    if (QUERY_FLAG(tmp, FLAG_BEEN_APPLIED))
			val = number * tmp->arch->clone.value *2 / 3;
		    else
			val = number * tmp->arch->clone.value / 3;
		}
	    }
	} else { /* No archetype with this object */
	    LOG(llevDebug,"In sell item: Have object with no archetype: %s\n", tmp->name);
	    if (flag == F_BUY) {
		LOG(llevError, "Asking for buy-value of unidentified object without arch.\n");
		val = number * tmp->value * 10;
	    }
	    else
		val = number * tmp->value / 5;
	}
    }

    /* If the item has been applied or identifed or does not need to be
     * identified, AND the object is magical and the archetype is non
     * magical, then change values accordingly.  The tmp->arch==NULL is
     * really just a check to prevent core dumps for when it checks
     * tmp->arch->clone.magic for any magic.  The check for archetype
     * magic is to not give extra money for archetypes that are by
     * default magical.  This is because the archetype value should have
     * already figured in that value.
     */
    if((QUERY_FLAG(tmp, FLAG_IDENTIFIED)||!need_identify(tmp)||identified||
	QUERY_FLAG(tmp, FLAG_BEEN_APPLIED)) &&
        tmp->magic&&(tmp->arch==NULL||!tmp->arch->clone.magic)) {
	    if(tmp->magic>0)
		val*=(3*tmp->magic*tmp->magic*tmp->magic);
	    else
		/* Note that tmp->magic is negative, so that this
		 * will actually be something like val /=2, /=3, etc.
		 */
	    val/=(1-tmp->magic);
    }

    if (tmp->type==WAND) {
	/* Value of the wand is multiplied by the number of
	 * charges.  the treasure code already sets up the value
	 * 50 charges is used as the baseline.
	 */
	if (QUERY_FLAG(tmp, FLAG_IDENTIFIED) || !need_identify(tmp) || identified)
	    val=(val*tmp->stats.food) / 50;
	else /* if not identified, presume one charge */
	    val/=50;
    }

    /* Limit amount of money you can get for really great items. */
    if (flag==F_TRUE || flag==F_SELL)
	val=value_limit(val, number, who, shop);

    /* we need to multiply these by 4.0 to keep buy costs roughly the same
     * (otherwise, you could buy a potion of charisma for around 400 pp.
     * Arguable, the costs in the archetypes should be updated to better
     * reflect values (potion charisma list for 1250 gold)
     */
    val *= 4;

    /* This modification is for bargaining skill.
     * Now only players with max level in bargaining
     * AND Cha = 30 will get optimal price.
     * Thus charisma will never get useless.
     * -b.e. edler@heydernet.de
     */

    if (who!=NULL && who->type==PLAYER) {
	int lev_bargain = 0;
	int lev_identify = 0;
	int idskill1=0;
	int idskill2=0;
	const typedata *tmptype;

	/* ratio determines how much of the price modification
	 * will come from the basic stat charisma
	 * the rest will come from the level in bargaining skill
	 */
	ratio = 0.5;
	tmptype=get_typedata(tmp->type);

	if (find_skill_by_number(who,SK_BARGAINING)) {
	    lev_bargain = find_skill_by_number(who,SK_BARGAINING)->level;
	}
	if (tmptype) {
	    idskill1=tmptype->identifyskill;
	    if (idskill1) {
		idskill2=tmptype->identifyskill2;
		if (find_skill_by_number(who,idskill1)) {
		    lev_identify = find_skill_by_number(who,idskill1)->level;
	    }
	    if (idskill2 && find_skill_by_number(who,idskill2)) {
	    	lev_identify += find_skill_by_number(who,idskill2)->level;
	   	}
	    }
	}
	else LOG(llevError, "Query_cost: item %s hasn't got a valid type\n", tmp->name);
	if ( !no_bargain && (lev_bargain>0) )
	    diff = (0.8 - 0.6*((lev_bargain+settings.max_level*0.05)
                         /(settings.max_level*1.05)));
	else
	    diff = 0.8;
	    
	diff *= 1-ratio;
	
	/* Diff is now a float between 0.2 and 0.8 */
	diff+=(cha_bonus[who->stats.Cha]-1)/(1+cha_bonus[who->stats.Cha])*ratio;
	
	if(flag==F_BUY)
          val=(val*(long)(1000*(1+diff)))/1000;
	else if (flag==F_SELL)
          val=(val*(long)(1000*(1-diff)))/1000;
	
	 /* If we are approximating, then the value returned should be allowed to be wrong
	  * however merely using a random number each time will not be sufficiant, as then
	  * multiple examinations would give different answers, so we'll use the count
	  * instead. By taking the sine of the count, a value between -1 and 1 is 
	  * generated, we then divide by the square root of the bargaining skill and the 
	  * appropriate identification skills, so that higher level players get better estimates.
	  * (we need a +1 there in case we would otherwise be dividing by zero.
	  */
	if (approximate) 
	    val = (sint64)val + (sint64)((sint64)val*(sin(tmp->count)/sqrt(lev_bargain+lev_identify*2+1.0)));
    }
   
    /* I don't think this should really happen - if it does, it indicates and
     * overflow of diff above.  That shoudl only happen if
     * we are selling objects - in that case, the person just
     * gets no money.
     */
    if((sint64)val<0)
	val=0;

    /* Unidentified stuff won't sell for more than 60gp */
    if(flag==F_SELL && !QUERY_FLAG(tmp, FLAG_IDENTIFIED) && need_identify(tmp) && !identified) {
	 val = (val > 600)? 600:val;
    }
    
    /* if we are in a shop, check how the type of shop should affect the price */
    if (shop && who) {
	if (flag==F_SELL) 
	    val=(sint64)val*shop_specialisation_ratio(tmp, who->map)*shopkeeper_approval(who->map, who)
		/shop_greed(who->map);
	else if (flag==F_BUY) {
	/* 
	 * when buying, if the item was sold by another player, it is ok to
	 * let the item be sold cheaper, according to the specialisation of
	 * the shop. If a player sold an item here, then his sale price was
	 * multiplied by the specialisation ratio, to do the same to the buy 
	 * price will not generate extra money. However, the 
	 * same is not true of generated items, these have to /divide/ by the 
	 * specialisation, so that the price is never less than what they could
	 * be sold for (otherwise players could camp map resets to make money).
	 * In game terms, a non-specialist shop, might not recognise the true 
	 * value of the items they sell (much like how people sometimes find 
	 * antiques in a junk shop in real life).
	 */
	    if (QUERY_FLAG(tmp, FLAG_PLAYER_SOLD))
		val=(sint64)val*shop_greed(who->map)*shop_specialisation_ratio(tmp, who->map)
		    /shopkeeper_approval(who->map, who);
	    else
		val=(sint64)val*shop_greed(who->map)
		    /(shop_specialisation_ratio(tmp, who->map)*shopkeeper_approval(who->map, who));
	}
	/* we will also have an extra 0-5% variation between shops of the same type 
	 * for valuable items (below a value of 50 this effect wouldn't be very 
	 * pointful, and could give fun with rounding.
	 */
	if(who->map->path!=NULL && val > 50)
	    val=(sint64)val+0.05*(sint64)val*cos(tmp->count+strlen(who->map->path));
    }
    return val;
}

/* Find the coin type that is worth more the 'c'.  Starts at the
 * cointype placement.
 */

static archetype *find_next_coin(uint64 c, int *cointype) {
  archetype *coin;

  do {
    if (coins[*cointype]==NULL) return NULL;
    coin = find_archetype(coins[*cointype]);
    if (coin == NULL)
      return NULL;
    *cointype += 1;
  } while (coin->clone.value > c);

  return coin;
}

/* This returns a string of how much something is worth based on
 * an integer being passed.
 * cost is the cost we need to represent.
 * While cost is 64 bit, the number of any coin is still really
 * limited to 32 bit (size of nrof field).  If it turns out players
 * have so much money that they have more than 2 billion platinum
 * coins, there are certainly issues - the easiest fix at that
 * time is to add a higher denomination (mithril piece with
 * 10,000 silver or something)
 */
static const char *cost_string_from_value(uint64 cost)
{
    static char buf[MAX_BUF];
    archetype *coin, *next_coin;
    char *endbuf;
    uint32 num;
    int cointype = LARGEST_COIN_GIVEN;

    coin = find_next_coin(cost, &cointype);
    if (coin == NULL)
	return "nothing";

    num = cost / coin->clone.value;
    /* so long as nrof is 32 bit, this is true.
     * If it takes more coins than a person can possibly carry, this
     * is basically true.
     */
    if ( (cost / coin->clone.value) > UINT32_MAX) {
	strcpy(buf,"an unimaginable sum of money.");
	return buf;
    }

    cost -= (uint64)num * (uint64)coin->clone.value;
    if (num == 1)
	sprintf(buf, "1 %s", coin->clone.name);
    else
        sprintf(buf, "%u %ss", num, coin->clone.name);

    next_coin = find_next_coin(cost, &cointype);
    if (next_coin == NULL)
	return buf;

    do {
	endbuf = buf + strlen(buf);

	coin = next_coin;
	num = cost / coin->clone.value;
	cost -= (uint64)num * (uint64)coin->clone.value;

	if (cost == 0)
	    next_coin = NULL;
	else
	    next_coin = find_next_coin(cost, &cointype);

        if (next_coin) {
            /* There will be at least one more string to add to the list,
             * use a comma.
             */
            strcat(endbuf, ", "); endbuf += 2;
        } else {
            strcat(endbuf, " and "); endbuf += 5;
        }
        if (num == 1)
            sprintf(endbuf, "1 %s", coin->clone.name);
        else
            sprintf(endbuf, "%u %ss", num, coin->clone.name);
    } while (next_coin);

    return buf;
}

/**
 * Returns a string representing the money's value, in plain coins.
 *
 * @param coin
 * coin. Must be of type MONEY.
 * @return
 * value in string, static buffer.
 * @todo remove static buffer.
 */
const char* real_money_value(const object* coin) {
    static char buf[MAX_BUF];

    assert(coin->type == MONEY);

    snprintf(buf, sizeof(buf), "%ld %s", coin->nrof, coin->nrof == 1 ? coin->name : coin->name_pl);
    return buf;
}

/**
 * Finds the price of an item.
 *
 * Price will be either an approximate value or the real value.
 * @param tmp
 * object to get the price of.
 * @param who
 * who is getting the price.
 * @param flag
 * combination of @ref F_xxx "F_xxx" values.
 * @return
 * buffer containing the price.
 * @todo remove evil static buffer thingy.
 */
const char *query_cost_string(const object *tmp,object *who,int flag) {
    uint64 real_value = query_cost(tmp,who,flag);
    int idskill1=0;
    int idskill2=0;
    const typedata *tmptype;

    /* money it's pretty hard to not give the exact price, so skip all logic and just return the real value. */
    if (tmp->type == MONEY) {
        return real_money_value(tmp);
    }

    tmptype=get_typedata(tmp->type);
    if (tmptype) {
	idskill1=tmptype->identifyskill;
	idskill2=tmptype->identifyskill2;
    }

    /* we show an approximate price if
     * 1) we are approximating
     * 2) there either is no id skill(s) for the item, or we don't have them
     * 3) we don't have bargaining skill either
     */
    if (flag & F_APPROX) {
	if (!idskill1 || !find_skill_by_number(who, idskill1)) {
	    if (!idskill2 || !find_skill_by_number(who, idskill2)) {
		if (!find_skill_by_number(who,SK_BARGAINING)) {
		    static char buf[MAX_BUF];
		    int num;
		    int cointype = LARGEST_COIN_GIVEN;
		    archetype *coin = find_next_coin(real_value, &cointype);
	
		    if (coin == NULL) return "nothing";
	
		    num = real_value / coin->clone.value;
		    if (num == 1)
	    		sprintf(buf, "about one %s", coin->clone.name);
		    else if (num < 5)
	    		sprintf(buf, "a few %s", coin->clone.name_pl);
		    else if (num < 10)
	    		sprintf(buf, "several %s", coin->clone.name_pl);
		    else if (num < 25)
	    		sprintf(buf, "a moderate amount of %s", coin->clone.name_pl);
		    else if (num < 100)
	    		sprintf(buf, "lots of %s", coin->clone.name_pl);
		    else if (num < 1000)
	    		sprintf(buf, "a great many %s", coin->clone.name_pl);
		    else 
	    		sprintf(buf, "a vast quantity of %s", coin->clone.name_pl);
		    return buf;
	        }
	    }
	}
    }
    return cost_string_from_value(real_value);
}

/* This function finds out how much money the player is carrying,
 * including what is in containers.
 */
uint64 query_money(const object *op) {
    object *tmp;
    uint64 total=0;

    if (op->type!=PLAYER && op->type!=CONTAINER) {
	LOG(llevError, "Query money called with non player/container\n");
	return 0;
    }
    for (tmp = op->inv; tmp; tmp= tmp->below) {
	if (tmp->type==MONEY) {
	    total += (uint64)tmp->nrof * (uint64)tmp->value;
	} else if (tmp->type==CONTAINER &&
		   QUERY_FLAG(tmp,FLAG_APPLIED) &&
		   (tmp->race==NULL || strstr(tmp->race,"gold"))) {
	    total += query_money(tmp);
	}
    }
    return total;
}
/* TCHIZE: This function takes the amount of money from the
 * the player inventory and from it's various pouches using the
 * pay_from_container function.
 * returns 0 if not possible. 1 if success
 */
int pay_for_amount(uint64 to_pay,object *pl) {
    object *pouch;

    if (to_pay==0) return 1;
    if (to_pay > query_money(pl)) return 0;

    to_pay = pay_from_container(pl, pl, to_pay);

    for (pouch=pl->inv; (pouch!=NULL) && (to_pay>0); pouch=pouch->below) {
        if (pouch->type == CONTAINER
          && QUERY_FLAG(pouch, FLAG_APPLIED)
          && (pouch->race == NULL || strstr(pouch->race, "gold"))) {
            to_pay = pay_from_container(pl, pouch, to_pay);
        }
    }
    if (to_pay > 0) {
        LOG(llevError, "pay_for_amount: Cannot remove enough money -- %"FMT64U" remains\n", to_pay);
    }

    fix_player(pl);
    return 1;
}

/* DAMN: This is now a wrapper for pay_from_container, which is
 * called for the player, then for each active container that can hold
 * money until op is paid for.  Change will be left wherever the last
 * of the price was paid from.
 */
int pay_for_item(object *op,object *pl) {
    uint64 to_pay = query_cost(op,pl,F_BUY | F_SHOP);
    object *pouch;
    uint64 saved_money;

    if (to_pay==0) return 1;
    if (to_pay>query_money(pl)) return 0;

    /* We compare the paid price with the one for a player
     * without bargaining skill.
     * This determins the amount of exp (if any) gained for bargaining.
     */
    saved_money = query_cost(op,pl,F_BUY | F_NO_BARGAIN | F_SHOP) - to_pay;

    if (saved_money > 0)
      change_exp(pl,saved_money,"bargaining",SK_EXP_NONE);

    to_pay = pay_from_container(pl, pl, to_pay);

    for (pouch=pl->inv; (pouch!=NULL) && (to_pay>0); pouch=pouch->below) {
        if (pouch->type == CONTAINER
                && QUERY_FLAG(pouch, FLAG_APPLIED)
                && (pouch->race == NULL || strstr(pouch->race, "gold"))) {
            to_pay = pay_from_container(pl, pouch, to_pay);
        }
    }
    if (to_pay > 0) {
        LOG(llevError, "pay_for_amount: Cannot remove enough money -- %"FMT64U" remains\n", to_pay);
    }
    if (settings.real_wiz == FALSE && QUERY_FLAG(pl, FLAG_WAS_WIZ))
        SET_FLAG(op, FLAG_WAS_WIZ);
    fix_player(pl);
    return 1;
}

/**
 * This function removes a given amount from a list of coins.
 *
 * @param coin_objs the list coins to remove from; the list must be ordered
 * from least to most valuable coin
 *
 * @param remain the value (in silver coins) to remove
 *
 * @return the value remaining
 */
static sint64 remove_value(object *coin_objs[], sint64 remain) {
    int i;

    for (i = 0; i < NUM_COINS; i++) {
        int count;
        int num_coins;

        if (coin_objs[i]->nrof*coin_objs[i]->value > remain) {
            num_coins = remain/coin_objs[i]->value;
            if ((uint64)num_coins*(uint64)coin_objs[i]->value < remain) {
                num_coins++;
            }
        } else {
            num_coins = coin_objs[i]->nrof;
        }
        remain -= (sint64)num_coins*(sint64)coin_objs[i]->value;
        coin_objs[i]->nrof -= num_coins;
        /* Now start making change.  Start at the coin value
         * below the one we just did, and work down to
         * the lowest value.
         */
        count = i-1;
        while (remain < 0 && count >= 0) {
            num_coins = -remain/coin_objs[count]->value;
            coin_objs[count]->nrof += num_coins;
            remain += num_coins*coin_objs[count]->value;
            count--;
        }
    }

    return remain;
}

/**
 * This function adds a given amount to a list of coins.
 *
 * @param coin_objs the list coins to add to; the list must be ordered
 * from least to most valuable coin
 *
 * @param amount the value (in silver coins) to add
 */
static void add_value(object *coin_objs[], sint64 value) {
    int i;

    for (i = NUM_COINS-LARGEST_COIN_GIVEN-1; i >= 0; i--) {
        uint32 nrof;

        nrof = (uint32)(value/coin_objs[i]->value);
        value -= nrof*coin_objs[i]->value;
        coin_objs[i]->nrof += nrof;
    }
}

/**
 * Insert a list of objects into a player object.
 *
 * @param pl the player to add to
 *
 * @param container the container (inside the player object) to add to
 *
 * @param objects the list of objects to add; the objects will be either
 * inserted into the player object or freed
 *
 * @param objects_len the length of objects
 */
static void insert_objects(object *pl, object *container, object *objects[], int objects_len) {
    int i, one = 0;

    for (i = 0; i < objects_len; i++) {
        if (objects[i]->nrof > 0) {
            object *tmp = insert_ob_in_ob(objects[i], container);
            one = 1;
        } else {
            free_object(objects[i]);
        }
    }
    if (one)
        esrv_update_item(UPD_WEIGHT, pl, container);
}

/* This pays for the item, and takes the proper amount of money off
 * the player.
 * CF 0.91.4 - this function is mostly redone in order to fix a bug
 * with weight not be subtracted properly.  We now remove and
 * insert the coin objects -  this should update the weight
 * appropriately
 *
 * DAMN: This function is used for the player, then for any active
 * containers that can hold money.
 *
 * pouch is the container (pouch or player) to remove the coins from.
 * to_pay is the required amount.
 * returns the amount still missing after using "pouch".
 */
static uint64 pay_from_container(object *pl, object *pouch, uint64 to_pay) {
    int i;
    sint64 remain;
    object *tmp, *coin_objs[NUM_COINS], *next;
    object *other_money[16]; /* collects MONEY objects not matching coins[] */
    size_t other_money_len; /* number of allocated entries in other_money[] */
    archetype *at;

    if (pouch->type != PLAYER && pouch->type != CONTAINER) return to_pay;

    remain = to_pay;
    for (i=0; i<NUM_COINS; i++) coin_objs[i] = NULL;

    /* This hunk should remove all the money objects from the player/container */
    other_money_len = 0;
    for (tmp=pouch->inv; tmp; tmp=next) {
        next = tmp->below;
        if (tmp->type == MONEY) {
            for (i=0; i<NUM_COINS; i++) {
                if (!strcmp(coins[NUM_COINS-1-i], tmp->arch->name) &&
                        (tmp->value == tmp->arch->clone.value) ) {

                    /* This should not happen, but if it does, just		*
                     * merge the two.						
                     */
                    if (coin_objs[i]!=NULL) {
                        LOG(llevError,"%s has two money entries of (%s)\n",
                            pouch->name, coins[NUM_COINS-1-i]);
                        remove_ob(tmp);
                        coin_objs[i]->nrof += tmp->nrof;
                        esrv_del_item(pl->contr, tmp->count);
                        free_object(tmp);
                    } else {
                        remove_ob(tmp);
                        if(pouch->type==PLAYER) esrv_del_item(pl->contr, tmp->count);
                        coin_objs[i] = tmp;
                    }
                    break;
                }
            }
            if (i==NUM_COINS) {
                if (other_money_len >= sizeof(other_money)/sizeof(*other_money)) {
                    LOG(llevError, "pay_for_item: Cannot store non-standard money object %s\n", tmp->arch->name);
                } else {
                    remove_ob(tmp);
                    if(pouch->type == PLAYER) {
                        esrv_del_item(pl->contr, tmp->count);
                    }
                    other_money[other_money_len++] = tmp;
                }
            }
        }
    }

    /* Fill in any gaps in the coin_objs array - needed to make change.      */
    /* Note that the coin_objs array goes from least value to greatest value */
    for (i=0; i<NUM_COINS; i++)
        if (coin_objs[i]==NULL) {
            at = find_archetype(coins[NUM_COINS-1-i]);
            if (at==NULL) LOG(llevError, "Could not find %s archetype\n", coins[NUM_COINS-1-i]);
            coin_objs[i] = get_object();
            copy_object(&at->clone, coin_objs[i]);
            coin_objs[i]->nrof = 0;
        }

    /* Try to pay from standard coins first. */
    remain = remove_value(coin_objs, remain);

    /* Now pay from non-standard coins until all is paid. */
    for (i = 0; i < other_money_len && remain > 0; i++) {
        uint32 nrof;
        object *coin;

        coin = other_money[i];

        /* Find the minimal number of coins to use. This prevents converting
	 * excess non-standard coins to standard money.
	 */
        nrof = (remain+coin->value-1)/coin->value;
        if (nrof > coin->nrof) {
            nrof = coin->nrof;
        }
        coin->nrof -= nrof;
        add_value(coin_objs, nrof*coin->value);

        remain = remove_value(coin_objs, remain);
    }

    /* re-insert remaining coins into player */
    insert_objects(pl, pouch, coin_objs, NUM_COINS);
    insert_objects(pl, pouch, other_money, other_money_len);

    return(remain);
}

/**
 * Helper function for can_pay. Checks all items from item and item->below, and recurse if inventory found.
 * coincount is supposed to be of size NUM_COINS. Parameters can't be NULL.
 */
static void count_unpaid(object* pl, object* item, int* unpaid_count, uint64* unpaid_price, uint32* coincount)
{
    int i;
    for (;item;item = item->below) {
        if QUERY_FLAG(item, FLAG_UNPAID) {
            (*unpaid_count)++;
            (*unpaid_price) += query_cost(item, pl, F_BUY | F_SHOP);
        }
        /* merely converting the player's monetary wealth won't do, if we did that, 
        * we could print the wrong numbers for the coins, so we count the money instead 
        */
        for (i=0; i< NUM_COINS; i++)
            if (!strcmp(coins[i], item->arch->name)) {
                coincount[i] += item->nrof;
                break;
            }
        if (item->inv) 
            count_unpaid(pl, item->inv, unpaid_count, unpaid_price, coincount);
    }
}

/**
 * Checks all unpaid items in op's inventory, adds up all the money they
 * have, and checks that they can actually afford what they want to buy.
 * Returns 1 if they can, and 0 if they can't. also prints an appropriate message 
 * to the player
 */

int can_pay(object *pl) {
    int unpaid_count = 0, i;
    uint64 unpaid_price = 0;
    uint64 player_wealth = query_money(pl);
    uint32 coincount[NUM_COINS];

    if (!pl || pl->type != PLAYER) {
        LOG(llevError, "can_pay(): called against something that isn't a player\n");
        return 0;
    }

    for (i=0; i< NUM_COINS; i++)
        coincount[i] = 0;

    count_unpaid(pl, pl->inv, &unpaid_count, &unpaid_price, coincount);

    if (unpaid_price > player_wealth) {
        char buf[MAX_BUF], coinbuf[MAX_BUF];
        int denominations = 0;
        snprintf(buf, sizeof(buf), "You have %d unpaid items that would cost you %s, ", 
                unpaid_count, cost_string_from_value(unpaid_price));
        for (i=0; i< NUM_COINS; i++) {
            if (coincount[i] > 0 && coins[i]) {
                if (denominations == 0)
                    snprintf(buf + strlen(buf), sizeof(buf) - strlen(buf), "but you only have");
                denominations++;
                snprintf(coinbuf, sizeof(coinbuf), " %d %s,", coincount[i],
                        find_archetype(coins[i])->clone.name_pl);
                snprintf(buf + strlen(buf), sizeof(buf) - strlen(buf), coinbuf);
            }
        }
        if (denominations == 0)
            snprintf(buf + strlen(buf), sizeof(buf) - strlen(buf), "but you don't have any money.");
        else if (denominations > 1)
            make_list_like(buf);
        draw_ext_info(NDI_UNIQUE, 0, pl, MSG_TYPE_SHOP, MSG_TYPE_SHOP_PAYMENT, buf, NULL);
        return 0;
    }
    else return 1;
}


/* Better get_payment, descends containers looking for
 * unpaid items, and pays for them.
 * returns 0 if the player still has unpaid items.
 * returns 1 if the player has paid for everything.
 * pl is the player buying the stuff.
 * op is the object we are examining.  If op has
 * and inventory, we examine that.  IF there are objects
 * below op, we descend down.
 */
int get_payment(object *pl, object *op) {
    char buf[MAX_BUF];
    int ret=1;

    if (op!=NULL&&op->inv)
        ret = get_payment(pl, op->inv);

    if (!ret)
        return 0;

    if (op!=NULL&&op->below)
        ret = get_payment (pl, op->below);

    if (!ret) 
        return 0;

    if(op!=NULL&&QUERY_FLAG(op,FLAG_UNPAID)) {
        strncpy(buf,query_cost_string(op,pl,F_BUY | F_SHOP),MAX_BUF);
        buf[MAX_BUF-1] = '\0';
        if(!pay_for_item(op,pl)) {
            uint64 i=query_cost(op,pl,F_BUY | F_SHOP) - query_money(pl);
            CLEAR_FLAG(op, FLAG_UNPAID);
	    draw_ext_info_format(NDI_UNIQUE, 0, pl, MSG_TYPE_SHOP, MSG_TYPE_SHOP_PAYMENT, 
                    "You lack %s to buy %s.", 
                    "You lack %s to buy %s.", 
		    cost_string_from_value(i), query_name(op));
            SET_FLAG(op, FLAG_UNPAID);
            return 0;
        } else {
            object *tmp;
            tag_t c = op->count;

            CLEAR_FLAG(op, FLAG_UNPAID);
            CLEAR_FLAG(op, FLAG_PLAYER_SOLD);
	    draw_ext_info_format(NDI_UNIQUE, 0, pl, MSG_TYPE_SHOP, MSG_TYPE_SHOP_PAYMENT, 
                    "You paid %s for %s.",
                    "You paid %s for %s.",
		    buf,query_name(op));
            tmp=merge_ob(op,NULL);
            if (pl->type == PLAYER && !tmp) {
                /* If item wasn't merged we update it. If merged, merge_ob handled everything for us. */
                esrv_update_item(UPD_FLAGS | UPD_NAME, pl, op);
            }
        }
    }
    return 1;
}


/* Modified function to give out platinum coins.  This function uses
 * the coins[] array to know what coins are available, just like
 * buy item.
 *
 * Modified to fill available race: gold containers before dumping
 * remaining coins in character's inventory.
 */
void sell_item(object *op, object *pl) {
    uint64 i=query_cost(op,pl,F_SELL | F_SHOP), extra_gain;
    int count;
    object *tmp, *pouch;
    archetype *at;

    if(pl==NULL||pl->type!=PLAYER) {
	LOG(llevDebug,"Object other than player tried to sell something.\n");
	return;
    }

    if(op->custom_name) FREE_AND_CLEAR_STR(op->custom_name);

    if(!i) {
	draw_ext_info_format(NDI_UNIQUE, 0, pl, MSG_TYPE_SHOP, MSG_TYPE_SHOP_SELL, 
	     "We're not interested in %s.",
	     "We're not interested in %s.",
	     query_name(op));

	/* Even if the character doesn't get anything for it, it may still be
	 * worth something.  If so, make it unpaid
	 */
	if (op->value) {
	    SET_FLAG(op, FLAG_UNPAID);
	    SET_FLAG(op, FLAG_PLAYER_SOLD);
	}
	identify(op);
	return;
    }

    /* We compare the price with the one for a player
     * without bargaining skill.
     * This determins the amount of exp (if any) gained for bargaining.
     * exp/10 -> 1 for each gold coin
     */
    extra_gain = i - query_cost(op,pl,F_SELL | F_NO_BARGAIN | F_SHOP);

    if (extra_gain > 0)
	change_exp(pl,extra_gain/10,"bargaining",SK_EXP_NONE);
  
    for (count=LARGEST_COIN_GIVEN; coins[count]!=NULL; count++) {
	at = find_archetype(coins[count]);
	if (at==NULL) LOG(llevError, "Could not find %s archetype\n", coins[count]);
	else if ((i/at->clone.value) > 0) {
	    for ( pouch=pl->inv ; pouch ; pouch=pouch->below ) {
		if ( pouch->type==CONTAINER && QUERY_FLAG(pouch, FLAG_APPLIED) && pouch->race && strstr(pouch->race, "gold") ) {
		    int w = at->clone.weight * (100-pouch->stats.Str)/100;
		    int n = i/at->clone.value;

		    if (w==0) w=1;    /* Prevent divide by zero */
		    if ( n>0 && (!pouch->weight_limit || pouch->carrying+w<=pouch->weight_limit)) {
			if (pouch->weight_limit && (pouch->weight_limit-pouch->carrying)/w<n)
			    n = (pouch->weight_limit-pouch->carrying)/w;

                        tmp = get_object();
                        copy_object(&at->clone, tmp);
                        tmp->nrof = n;
                        i -= (uint64)tmp->nrof * (uint64)tmp->value;
                        tmp = insert_ob_in_ob(tmp, pouch);
                        esrv_update_item(UPD_WEIGHT, pl, pl);
                    }
                }
            }
            if (i/at->clone.value > 0) {
                tmp = get_object();
                copy_object(&at->clone, tmp);
                tmp->nrof = i/tmp->value;
                i -= (uint64)tmp->nrof * (uint64)tmp->value;
                tmp = insert_ob_in_ob(tmp, pl);
                esrv_update_item (UPD_WEIGHT, pl, pl);
            }
        }
    }

    if (i!=0) 
#ifndef WIN32
	LOG(llevError,"Warning - payment not zero: %llu\n", i);
#else
	LOG(llevError,"Warning - payment not zero: %I64u\n", i);
#endif

    draw_ext_info_format(NDI_UNIQUE, 0, pl, MSG_TYPE_SHOP, MSG_TYPE_SHOP_SELL, 
	"You receive %s for %s.",
	"You receive %s for %s.",
	query_cost_string(op,pl,F_SELL | F_SHOP), query_name(op));

    SET_FLAG(op, FLAG_UNPAID);
    identify(op);
}

/* returns a double that is the ratio of the price that a shop will offer for
 * item based on the shops specialisation. Does not take account of greed, 
 * returned value is between (2*SPECIALISATION_EFFECT-1) and 1 and in any 
 * event is never less than 0.1 (calling functions divide by it)
 */
static double shop_specialisation_ratio(const object *item, const mapstruct *map) {
    shopitems *items=map->shopitems;
    double ratio = SPECIALISATION_EFFECT, likedness=0.001;
    int i;

    if (item==NULL) { 
	LOG(llevError, "shop_specialisation_ratio: passed a NULL item for map %s\n", map->path);
	return 0;
    }
    if (!item->type) {
	LOG(llevError, "shop_specialisation_ratio: passed an item with an invalid type\n");
	/* 
	 * I'm not really sure what the /right/ thing to do here is, these types of 
	 * item shouldn't exist anyway, but returning the ratio is probably the best bet.." 
	 */
	return ratio;
    }
    if (map->shopitems) {
	for (i=0; i<items[0].index; i++)
	    if (items[i].typenum==item->type || (!items[i].typenum && likedness == 0.001)) 
		likedness = items[i].strength/100.0;
    }
    if (likedness > 1.0) { /* someone has been rather silly with the map headers. */
	LOG(llevDebug, "shop_specialisation ratio: item type %d on map %s is above 100%%\n", 
	    item->type, map->path);
	likedness = 1.0; 
    }
    if (likedness < -1.0) {
	LOG(llevDebug, "shop_specialisation ratio: item type %d on map %s is below -100%%\n", 
	    item->type, map->path);
	likedness = -1.0; 
    }
    ratio = ratio + (1.0-ratio) * likedness;
    if (ratio <= 0.1) ratio=0.1; /* if the ratio were much lower than this, we would get silly prices */
    return ratio;
}

/*returns the greed of the shop on map, or 1 if it isn't specified. */
static double shop_greed(const mapstruct *map) {
    double greed=1.0;
    if (map->shopgreed)
	return map->shopgreed;
    return greed;
}

/* Returns a double based on how much the shopkeeper approves of the player.
 * this is based on the race of the shopkeeper and that of the player.
 */
double shopkeeper_approval(const mapstruct *map, const object *player) {
    double approval=1.0;

    if (map->shoprace) {
	approval=NEUTRAL_RATIO;
        if (player->race && !strcmp(player->race, map->shoprace)) approval = 1.0;
    }
    return approval;
}

/* limit the value of items based on the wealth of the shop. If the item is close
 * to the maximum value a shop will offer, we start to reduce it, if the item is 
 * below the minimum value the shop is prepared to trade in, then we don't 
 * want it and offer nothing. If it isn't a shop, check whether we should do generic
 * value reduction.
 * 
 */
static uint64 value_limit(uint64 val, int quantity, const object *who, int isshop) {
    uint64 newval, unit_price;
    mapstruct *map;
    unit_price=val/quantity;
    if (!isshop || !who) {
	if (unit_price > 10000)
	    newval=8000+isqrt(unit_price)*20;
	else
	    newval=unit_price;
    } else {
	if (!who->map) {
	    LOG(llevError, "value_limit: asked shop price for ob %s on NULL map\n", who->name);
	    return val;
	}
	map=who->map;
	if (map->shopmin && unit_price < map->shopmin) return 0;
	else if (map->shopmax && unit_price > map->shopmax/2)
	    newval=MIN((map->shopmax/2)+isqrt(unit_price-map->shopmax/2), map->shopmax);
	else if (unit_price>10000)
    	    newval=8000+isqrt(unit_price)*20;
	else
	    newval=unit_price;
    }
    newval *= quantity;
    return newval;
}

/* gives a desciption of the shop on their current map to the player op. */
int describe_shop(const object *op) {
    mapstruct *map = op->map;
    /*shopitems *items=map->shopitems;*/
    int pos=0, i;
    double opinion=0;
    char tmp[MAX_BUF]="\0";

    if (op->type != PLAYER) return 0;

    /*check if there is a shop specified for this map */
    if (map->shopitems || map->shopgreed || map->shoprace || map->shopmin || map->shopmax) {
	draw_ext_info(NDI_UNIQUE, 0, op, MSG_TYPE_SHOP, MSG_TYPE_SHOP_LISTING,
		      "From looking at the nearby shop you determine that it trades in:", 
		      NULL);

	if (map->shopitems) {
	    for (i=0; i < map->shopitems[0].index; i++) {
		if (map->shopitems[i].name && map->shopitems[i].strength > 10) {
		    snprintf(tmp+pos, sizeof(tmp)-pos, "%s, ", map->shopitems[i].name_pl);
		    pos += strlen(tmp+pos);
		}
	    }
	}
	if (!pos) strcpy(tmp, "a little of everything.");

	/* format the string into a list */
	make_list_like(tmp);
	draw_ext_info(NDI_UNIQUE, 0, op, MSG_TYPE_SHOP, MSG_TYPE_SHOP_LISTING, tmp, NULL);
	
	if (map->shopmax)
	    draw_ext_info_format(NDI_UNIQUE, 0, op, MSG_TYPE_SHOP, MSG_TYPE_SHOP_MISC, 
		"It won't trade for items above %s.",
		"It won't trade for items above %s.",
		cost_string_from_value(map->shopmax));

	if (map->shopmin)
	    draw_ext_info_format(NDI_UNIQUE, 0, op, MSG_TYPE_SHOP, MSG_TYPE_SHOP_MISC, 
		"It won't trade in items worth less than %s.",
		"It won't trade in items worth less than %s.",
		cost_string_from_value(map->shopmin));

	if (map->shopgreed) {
	    if (map->shopgreed >2.0)
		draw_ext_info(NDI_UNIQUE, 0, op, MSG_TYPE_SHOP, MSG_TYPE_SHOP_MISC, 
			      "It tends to overcharge massively.", NULL);
	    else if (map->shopgreed >1.5)
		draw_ext_info(NDI_UNIQUE, 0, op, MSG_TYPE_SHOP, MSG_TYPE_SHOP_MISC, 
			      "It tends to overcharge substantially.", NULL);
	    else if (map->shopgreed >1.1)
		draw_ext_info(NDI_UNIQUE, 0, op, MSG_TYPE_SHOP, MSG_TYPE_SHOP_MISC, 
			      "It tends to overcharge slightly.", NULL);
	    else if (map->shopgreed <0.9)
		draw_ext_info(NDI_UNIQUE, 0, op, MSG_TYPE_SHOP, MSG_TYPE_SHOP_MISC, 
			      "It tends to undercharge.", NULL);
	}
	if (map->shoprace) {
	    opinion=shopkeeper_approval(map, op);
	    if (opinion > 0.8) 
		draw_ext_info(NDI_UNIQUE, 0, op, MSG_TYPE_SHOP, MSG_TYPE_SHOP_MISC, 
			      "You think the shopkeeper likes you.", NULL);
	    else if (opinion > 0.5)
		draw_ext_info(NDI_UNIQUE, 0, op, MSG_TYPE_SHOP, MSG_TYPE_SHOP_MISC, 
			      "The shopkeeper seems unconcerned by you.", NULL);
	    else
		draw_ext_info(NDI_UNIQUE, 0, op, MSG_TYPE_SHOP, MSG_TYPE_SHOP_MISC, 
			      "The shopkeeper seems to have taken a dislike to you.", NULL);
	}
    }
    else draw_ext_info(NDI_UNIQUE, 0, op, MSG_TYPE_SHOP, MSG_TYPE_SHOP_MISC, 
		       "There is no shop nearby.", NULL);

    return 1;
}
typedef struct shopinv {
    char	*item_sort;
    char	*item_real;
    uint16	type;
    uint32	nrof;
} shopinv;

/* There are a lot fo extra casts in here just to suppress warnings - it
 * makes it look uglier than it really it.
 * The format of the strings we get is type:name.  So we first want to
 * sort by type (numerical) - if the same type, then sort by name.
 */
static int shop_sort(const void *a1, const void *a2)
{
    const shopinv *s1 = (const shopinv*)a1, *s2= (const shopinv*)a2;

    if (s1->type<s2->type) return -1;
    if (s1->type>s2->type) return 1;
    /* the type is the same (what atoi gets), so do a strcasecmp to sort
     * via alphabetical order
     */
    return strcasecmp(s1->item_sort, s2->item_sort);
}

static void add_shop_item(object *tmp, shopinv *items, int *numitems, int *numallocated)
{
#if 0
    char buf[MAX_BUF];
#endif
    /* clear unpaid flag so that doesn't come up in query
     * string.  We clear nrof so that we can better sort
     * the object names.
     */

    CLEAR_FLAG(tmp, FLAG_UNPAID);
    items[*numitems].nrof=tmp->nrof;
    /* Non mergable items have nrof of 0, but count them as one
     * so the display is properly.
     */
    if (tmp->nrof == 0) items[*numitems].nrof++;
    items[*numitems].type=tmp->type;

    switch (tmp->type) {
#if 0
	case BOOTS:
	case GLOVES:
	case RING:
	case AMULET:
	case BRACERS:
	case GIRDLE:
	    sprintf(buf,"%s %s",query_base_name(tmp,0),describe_item(tmp, NULL));
	    items[*numitems].item_sort = strdup_local(buf);
	    sprintf(buf,"%s %s",query_name(tmp),describe_item(tmp, NULL));
	    items[*numitems].item_real = strdup_local(buf);
	    (*numitems)++;
	    break;
#endif

	default:
	    items[*numitems].item_sort = strdup_local(query_base_name(tmp, 0));
	    items[*numitems].item_real = strdup_local(query_base_name(tmp, 1));
	    (*numitems)++;
	    break;
    }
    SET_FLAG(tmp, FLAG_UNPAID);
}

void shop_listing(object *op)
{
    int i,j,numitems=0,numallocated=0, nx, ny;
    char *map_mark = (char *) calloc(MAGIC_MAP_SIZE * MAGIC_MAP_SIZE,1);
    object	*stack;
    shopinv	*items;

    /* Should never happen, but just in case a monster does apply a sign */
    if (op->type!=PLAYER) return;

    draw_ext_info(NDI_UNIQUE, 0, op, MSG_TYPE_SHOP, MSG_TYPE_SHOP_LISTING, 
		  "\nThe shop contains:", NULL);

    magic_mapping_mark(op, map_mark, 3);
    items=malloc(40*sizeof(shopinv));
    numallocated=40;

    /* Find all the appropriate items */
    for (i=0; i<MAP_WIDTH(op->map); i++) {
	for (j=0; j<MAP_HEIGHT(op->map); j++) {
	    /* magic map code now centers the map on the object at MAGIC_MAP_HALF.
	     *
	     */
	    nx = i - op->x + MAGIC_MAP_HALF;
	    ny = j - op->y + MAGIC_MAP_HALF;
	    /* unlikely, but really big shops could run into this issue */
	    if (nx < 0 || ny < 0 || nx > MAGIC_MAP_SIZE || ny > MAGIC_MAP_SIZE) continue;

	    if (map_mark[nx + MAGIC_MAP_SIZE * ny] & FACE_FLOOR) {
		stack  =get_map_ob(op->map,i,j);

		while (stack) {
		    if (QUERY_FLAG(stack, FLAG_UNPAID)) {
			if (numitems==numallocated) {
			    items=realloc(items, sizeof(shopinv)*(numallocated+10));
			    numallocated+=10;
			}
			add_shop_item(stack, items, &numitems, &numallocated);
		    }
		    stack = stack->above;
		}
	    }
	}
    }
    free(map_mark);
    if (numitems == 0) {
	draw_ext_info(NDI_UNIQUE, 0, op, MSG_TYPE_SHOP, MSG_TYPE_SHOP_LISTING, 
		      "The shop is currently empty.\n", NULL);
	free(items);
	return;
    }
    qsort(items, numitems, sizeof(shopinv), (int (*)(const void*, const void*))shop_sort);

    for (i=0; i<numitems; i++) {
	/* Collapse items of the same name together */
	if ((i+1)<numitems && !strcmp(items[i].item_real, items[i+1].item_real)) {
	    items[i+1].nrof += items[i].nrof;
	    free(items[i].item_sort);
	    free(items[i].item_real);
	} else {
	    draw_ext_info_format(NDI_UNIQUE, 0, op, MSG_TYPE_SHOP, MSG_TYPE_SHOP_LISTING, 
		"%d %s", "%d %s", 
		 items[i].nrof? items[i].nrof:1,
		 items[i].nrof==1?items[i].item_sort: items[i].item_real);
	    free(items[i].item_sort);
	    free(items[i].item_real);
	}
    }
    free(items);
}

/* Check if an object is in a shop. Returns 1 if so, 0 otherwise. */
int is_in_shop(object *ob) {
    if (!ob->map)
        return 0;
    return coords_in_shop(ob->map, ob->x, ob->y);
}

/* Check if given map coords are in a shop. Returns 1 if so, 0 otherwise. */
int coords_in_shop(mapstruct *map, int x, int y) {
    object *floor;
    for (floor = get_map_ob (map, x, y); floor; floor = floor->above)
        if (floor->type == SHOP_FLOOR)
            return 1;
    return 0;
}
