char *def_sounds[] = {
"#\n",
"# This is a reasonably heavily modified version of the sounds file from\n",
"# the server.\n",
"#\n",
"# Since the client has to load the sounds on its own, full path names\n",
"# are needed.\n",
"#\n",
"# There are two sections - one for normal sounds, and one for spell\n",
"# sounds.  They are differentiated by the header 'Standard Sounds:' and\n",
"# 'Spell Sounds:'\n",
"#\n",
"# Empty lines and lines starting with # are ignored.\n",
"#\n",
"# Format of the file:  pathname to sound, default volume (what volume gets\n",
"# played if it is centered on the player), symbolic name (not currently\n",
"# used for anything, but may be in the future for the server to communicated\n",
"# to the client), and sound number..  Symbolic name and sound number may\n",
"# be ommitted.  If sound number is missing, the sound will be assigned one\n",
"# higher than the last sound read in.\n",
"#\n",
"# the symbolic name can be DEFAULT, in which case all sounds for that section\n",
"# which are otherwise not set will be set to that value.  This can be\n",
"# especially useful for that magic section, since new spells get added\n",
"# fairly often.\n",
"#\n",
"# The volume of 100 means max volume.  Anything higher may break the sound\n",
"# handling.\n",
"#\n",
"# The server currently communicates what sound to play via the sound\n",
"# number - you should not change those.  However, you can change most\n",
"# of the other sounds without problem.\n",
"#\n",
"# If a copy of this file is in ~/.crossfire/, it will be loaded\n",
"#\n",
"Standard Sounds:\n",
"\n",
"/usr/local/lib/sounds/su-fanf.au	 30	SOUND_NEW_PLAYER	0\n",
"/usr/local/lib/sounds/Teeswing.au 	 80	SOUND_FIRE_ARROW	1\n",
"/usr/local/lib/sounds/chord.au	100	SOUND_LEARN_SPELL	2\n",
"/usr/local/lib/sounds/Missed.au	 70	SOUND_FUMBLE_SPELL	3\n",
"/usr/local/lib/sounds/Missed.au	 70	SOUND_WAND_POOF		4\n",
"/usr/local/lib/sounds/Creaky-1.au	 90	SOUND_OPEN_DOOR		5\n",
"/usr/local/lib/sounds/blip.au 	 80	SOUND_PUSH_PLAYER	6\n",
"/usr/local/lib/sounds/click1.au	 60	SOUND_PLAYER_HITS1	7\n",
"/usr/local/lib/sounds/click2.au	 70	SOUND_PLAYER_HITS2	8\n",
"/usr/local/lib/sounds/click1.au	 80	SOUND_PLAYER_HITS3	9\n",
"/usr/local/lib/sounds/click2.au	 90	SOUND_PLAYER_HITS4	10\n",
"/usr/local/lib/sounds/FloorTom.au	 70	SOUND_PLAYER_IS_HIT1	11\n",
"/usr/local/lib/sounds/ouch1.au	 80	SOUND_PLAYER_IS_HIT2	12\n",
"/usr/local/lib/sounds/thru.au 	 90	SOUND_PLAYER_IS_HIT3	13\n",
"/usr/local/lib/sounds/drip.au 	 80	SOUND_PLAYER_KILLS	14\n",
"/usr/local/lib/sounds/squish.au	 80	SOUND_PET_IS_KILLED	15\n",
"/usr/local/lib/sounds/gong.au 	150	SOUND_PLAYER_DIES	16\n",
"/usr/local/lib/sounds/Whoosh.au	100	SOUND_OB_EVAPORATE	17\n",
"/usr/local/lib/sounds/Explosion.au	100	SOUND_OB_EXPLODE	18\n",
"/usr/local/lib/sounds/TowerClock.au	130	SOUND_CLOCK		19\n",
"/usr/local/lib/sounds/boink2.au 	 80	SOUND_TURN_HANDLE	20\n",
"/usr/local/lib/sounds/MetalCrash.au 	200	SOUND_FALL_HOLE		21\n",
"/usr/local/lib/sounds/Puke.au 	130	SOUND_DRINK_POISON     	22\n",
"\n",
"# Spells :		SOUND_CAST_SPELL_0      23, 24, 25 ...\n",
"Spell Sounds:\n",
"\n",
"/usr/local/lib/sounds/magic.au	100	DEFAULT\n",
"/usr/local/lib/sounds/swish.au	 70	\"magic bullet\" 			/* 0 */\n",
"/usr/local/lib/sounds/swish.au	 60	\"small fireball\"\n",
"/usr/local/lib/sounds/swish.au	 80	\"medium fireball\"\n",
"/usr/local/lib/sounds/swish.au	100	\"large fireball\"\n",
"/usr/local/lib/sounds/magic.au	100	\"burning hands\"\n",
"/usr/local/lib/sounds/lightning1.au	 70	\"small lightning\"\n",
"/usr/local/lib/sounds/lightning1.au	 90	\"large lightning\"\n",
"/usr/local/lib/sounds/swish.au	100	\"magic missile\"\n",
"/usr/local/lib/sounds/magic.au	100	\"create bomb\"\n",
"/usr/local/lib/sounds/magic.au	100	\"summon golem\"\n",
"/usr/local/lib/sounds/magic.au	100	\"summon fire elemental\"		/* 10 */\n",
"/usr/local/lib/sounds/magic.au	100	\"summon earth elemental\"\n",
"/usr/local/lib/sounds/magic.au	100	\"summon water elemental\"\n",
"/usr/local/lib/sounds/magic.au	100	\"summon air elemental\"\n",
"/usr/local/lib/sounds/first_try.au	100	\"dimension door\"\n",
"/usr/local/lib/sounds/magic.au	100	\"create earth wall\"\n",
"/usr/local/lib/sounds/Tear.au 	100	\"paralyze\"\n",
"/usr/local/lib/sounds/Missle1.au	 80	\"icestorm\"\n",
"/usr/local/lib/sounds/magic.au	100	\"magic mapping\"\n",
"/usr/local/lib/sounds/Tear.au 	 90	\"turn undead\"\n",
"/usr/local/lib/sounds/Tear.au 	 90	\"fear\"				/* 20 */\n",
"/usr/local/lib/sounds/Missle1.au	100	\"poison cloud\"\n",
"/usr/local/lib/sounds/magic.au	100	\"wonder\"\n",
"/usr/local/lib/sounds/Explosion.au	 80	\"destruction\"\n",
"/usr/local/lib/sounds/magic.au	100	\"perceive self\"\n",
"/usr/local/lib/sounds/sci_fi_gun.au	100	\"word of recall\"\n",
"/usr/local/lib/sounds/magic.au	100	\"invisible\"\n",
"/usr/local/lib/sounds/magic.au	100	\"invisible to undead\"\n",
"/usr/local/lib/sounds/magic.au	100	\"probe\"\n",
"/usr/local/lib/sounds/swish.au	100	\"large bullet\"\n",
"/usr/local/lib/sounds/magic.au	100	\"improved invisibility\"		/* 30 */\n",
"/usr/local/lib/sounds/Tear.au 	100	\"holy word\"\n",
"/usr/local/lib/sounds/magic.au	100	\"minor healing\"\n",
"/usr/local/lib/sounds/magic.au	100	\"medium healing\"\n",
"/usr/local/lib/sounds/magic.au	100	\"major healing\"\n",
"/usr/local/lib/sounds/magic.au	100	\"heal\"\n",
"/usr/local/lib/sounds/magic.au	100	\"create food\"\n",
"/usr/local/lib/sounds/Explosion.au	 60	\"earth to dust\"\n",
"/usr/local/lib/sounds/magic.au	100	\"armour\"\n",
"/usr/local/lib/sounds/magic.au	100	\"strength\"\n",
"/usr/local/lib/sounds/magic.au	100	\"dexterity\"			/* 40 */\n",
"/usr/local/lib/sounds/magic.au	100	\"constitution\"\n",
"/usr/local/lib/sounds/magic.au	100	\"charisma\"\n",
"/usr/local/lib/sounds/magic.au	100	\"create fire wall\"\n",
"/usr/local/lib/sounds/magic.au	100	\"create frost wall\"\n",
"/usr/local/lib/sounds/magic.au	100	\"protection from cold\"\n",
"/usr/local/lib/sounds/magic.au	100	\"protection from electricity\"\n",
"/usr/local/lib/sounds/magic.au	100	\"protection from fire\"\n",
"/usr/local/lib/sounds/magic.au	100	\"protection from poison\"\n",
"/usr/local/lib/sounds/magic.au	100	\"protection from slow\"\n",
"/usr/local/lib/sounds/magic.au	100	\"protection from paralysis\"	/* 50 */\n",
"/usr/local/lib/sounds/magic.au	100	\"protection from draining\"\n",
"/usr/local/lib/sounds/magic.au	100	\"protection from magic\"\n",
"/usr/local/lib/sounds/magic.au	100	\"protection from attack\"\n",
"/usr/local/lib/sounds/magic.au	100	\"levitate\"\n",
"/usr/local/lib/sounds/Gun-5.au	100	\"small speedball\"\n",
"/usr/local/lib/sounds/Gun-5.au	130	\"large speedball\"\n",
"/usr/local/lib/sounds/Missle1.au	150	\"hellfire\"\n",
"/usr/local/lib/sounds/Missle1.au	170	\"dragonbreath\"\n",
"/usr/local/lib/sounds/Missle1.au	170	\"large icestorm\"\n",
"/usr/local/lib/sounds/magic.au	100	\"charging\"			/* 60 */\n",
"/usr/local/lib/sounds/magic.au	100	\"polymorph\"\n",
"/usr/local/lib/sounds/swish.au	200	\"cancellation\"\n",
"/usr/local/lib/sounds/Tear.au 	100	\"confusion\"\n",
"/usr/local/lib/sounds/Tear.au 	150	\"mass confusion\"\n",
"/usr/local/lib/sounds/magic.au	100	\"summon pet monster\"\n",
"/usr/local/lib/sounds/magic.au 	100	\"slow\"\n",
"/usr/local/lib/sounds/magic.au	100	\"regenerate spellpoints\"\n",
"/usr/local/lib/sounds/magic.au	100	\"cure poison\"\n",
"/usr/local/lib/sounds/magic.au	100	\"protection from confusion\"\n",
"/usr/local/lib/sounds/magic.au	100	\"protection from cancellation\"	/* 70 */\n",
"/usr/local/lib/sounds/magic.au	100	\"protection from depletion\"\n",
"/usr/local/lib/sounds/magic.au	100	\"alchemy\"\n",
"/usr/local/lib/sounds/Evil_Laugh.au	100	\"remove curse\"\n",
"/usr/local/lib/sounds/Evil_Laugh.au	120	\"remove damnation\"\n",
"/usr/local/lib/sounds/magic.au	100	\"identify\"\n",
"/usr/local/lib/sounds/magic.au	100	\"detect magic\"\n",
"/usr/local/lib/sounds/magic.au	100	\"detect monster\"\n",
"/usr/local/lib/sounds/magic.au	100	\"detect evil\"\n",
"/usr/local/lib/sounds/magic.au	100	\"detect curse\"\n",
"/usr/local/lib/sounds/Explosion.au	100	\"heroism\"			/* 80 */\n",
"/usr/local/lib/sounds/bugle_charge.au	100	\"aggravation\"\n",
"/usr/local/lib/sounds/magic.au	100	\"firebolt\"\n",
"/usr/local/lib/sounds/magic.au	100	\"frostbolt\"\n",
"/usr/local/lib/sounds/Explosion.au	100	\"shockwave\"\n",
"/usr/local/lib/sounds/magic.au	100	\"color spray\"\n",
"/usr/local/lib/sounds/magic.au	100	\"haste\"\n",
"/usr/local/lib/sounds/magic.au	100	\"face of death\"\n",
"/usr/local/lib/sounds/lightning1.au	110	\"ball lightning\"\n",
"/usr/local/lib/sounds/swish.au	100	\"meteor swarm\"\n",
"/usr/local/lib/sounds/swish.au	100	\"comet\"				/* 90 */\n",
"/usr/local/lib/sounds/magic.au	100	\"mystic fist\"\n",
"/usr/local/lib/sounds/magic.au	100	\"raise dead\"\n",
"/usr/local/lib/sounds/magic.au	100	\"resurrection\"\n",
"/usr/local/lib/sounds/magic.au	100	\"reincarnation\"\n",
"/usr/local/lib/sounds/magic.au	100	\"immune cold\"\n",
"/usr/local/lib/sounds/magic.au	100	\"immune electricity\"\n",
"/usr/local/lib/sounds/magic.au	100	\"immune fire\"\n",
"/usr/local/lib/sounds/magic.au	100	\"immune poison\"\n",
"/usr/local/lib/sounds/magic.au	100	\"immune slow\"\n",
"/usr/local/lib/sounds/magic.au	100	\"immune paralysis\"		/* 100 */\n",
"/usr/local/lib/sounds/magic.au	100	\"immune drain\"\n",
"/usr/local/lib/sounds/magic.au	100	\"immune magic\"\n",
"/usr/local/lib/sounds/magic.au	100	\"immune attack\"\n",
"/usr/local/lib/sounds/magic.au	100	\"invulnerability\"\n",
"/usr/local/lib/sounds/magic.au	100	\"defense\n",
"/usr/local/lib/sounds/magic.au	100	\"rune fire\"\n",
"/usr/local/lib/sounds/magic.au	100	\"rune frost\"\n",
"/usr/local/lib/sounds/magic.au	100	\"rune shocking\"\n",
"/usr/local/lib/sounds/magic.au	100	\"rune blasting\"\n",
"/usr/local/lib/sounds/magic.au	100	\"rune death\"			/* 110 */\n",
"/usr/local/lib/sounds/magic.au	100	\"marking rune\"\n",
"/usr/local/lib/sounds/magic.au	100	\"build director\"\n",
"/usr/local/lib/sounds/magic.au	100	\"pool of chaos\"\n",
"/usr/local/lib/sounds/magic.au	100	\"build bullet wall\"\n",
"/usr/local/lib/sounds/magic.au	100	\"build ligtning wall\"\n",
"/usr/local/lib/sounds/magic.au	100	\"build fireball wall\"\n",
"/usr/local/lib/sounds/magic.au	100	\"magic \"\n",
"/usr/local/lib/sounds/magic.au	100	\"rune of magic drain\"\n",
"/usr/local/lib/sounds/magic.au	100	\"antimagic rune\"\n",
"/usr/local/lib/sounds/magic.au	100	\"rune transferrence\"		/* 120 */\n",
"/usr/local/lib/sounds/magic.au	100	\"transferrence\"\n",
"/usr/local/lib/sounds/magic.au	100	\"magic drain\"\n",
"/usr/local/lib/sounds/magic.au	100	\"counterspell\"\n",
"/usr/local/lib/sounds/magic.au	100	\"disarm\"\n",
"/usr/local/lib/sounds/magic.au	100	\"cure confusion\"\n",
"/usr/local/lib/sounds/magic.au	100	\"restoration\"\n",
"/usr/local/lib/sounds/magic.au	100	\"summon evil monster\"\n",
"/usr/local/lib/sounds/magic.au	100	\"counterwall\"\n",
"/usr/local/lib/sounds/magic.au	100	\"cause light wounds\"\n",
"/usr/local/lib/sounds/magic.au	100	\"cause medium wounds\"		/* 130 */\n",
"/usr/local/lib/sounds/magic.au	100	\"cause serious wounds\"\n",
"/usr/local/lib/sounds/magic.au	100	\"charm monsters\"\n",
"/usr/local/lib/sounds/magic.au	100	\"banishment\"\n",
"/usr/local/lib/sounds/magic.au	100	\"create missile\"\n",
};
