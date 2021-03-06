/* account.c */
void account_load_entries(void);
void accounts_save(void);
const char *account_exists(char *account_name);
int account_check_name_password(char *account_name, char *account_password);
int account_check_string(const char *str);
int account_add_account(char *account_name, char *account_password);
int account_add_player_to_account(char *account_name, char *player_name);
int account_remove_player_from_account(const char *account_name, const char *player_name);
char **account_get_players_for_account(char *account_name);
const char *account_get_account_for_char(const char *charname);
int account_is_logged_in(const char *name);
socket_struct *account_get_logged_in_init_socket(const char *name);
player *account_get_logged_in_player(const char *name);
/* account_char.c */
Account_Char *account_char_load(const char *account_name);
void account_char_save(const char *account, Account_Char *chars);
Account_Char *account_char_add(Account_Char *chars, player *pl);
Account_Char *account_char_remove(Account_Char *chars, const char *pl_name);
void account_char_free(Account_Char *chars);
/* alchemy.c */
int use_alchemy(object *op);
/* apply.c */
int transport_can_hold(const object *transport, const object *op, int nrof);
int should_director_abort(object *op, object *victim);
void apply_handle_yield(object *tmp);
int set_object_face_main(object *op);
int apply_container(object *op, object *sack);
void do_learn_spell(object *op, object *spell, int special_prayer);
void do_forget_spell(object *op, const char *spell);
int apply_manual(object *op, object *tmp, int aflag);
int apply_by_living(object *pl, object *op, int aflag, int quiet);
void apply_by_living_below(object *pl);
int apply_can_apply_object(const object *who, const object *op);
int apply_check_weapon_power(const object *who, int improves);
int apply_special(object *who, object *op, int aflags);
int apply_auto(object *op);
void apply_auto_fix(mapstruct *m);
void scroll_failure(object *op, int failure, int power);
void apply_changes_to_player(object *pl, object *change);
void legacy_apply_container(object *op, object *sack);
/* attack.c */
void save_throw_object(object *op, uint32 type, object *originator);
int hit_map(object *op, int dir, uint32 type, int full_hit);
int attack_ob(object *op, object *hitter);
object *hit_with_arrow(object *op, object *victim);
int friendly_fire(object *op, object *hitter);
int hit_player(object *op, int dam, object *hitter, uint32 type, int full_hit);
void confuse_living(object *op, object *hitter, int dam);
void blind_living(object *op, object *hitter, int dam);
void paralyze_living(object *op, object *hitter, int dam);
/* ban.c */
int checkbanned(const char *login, const char *host);
/* build_map.c */
void apply_builder_remove(object *pl, int dir);
void apply_map_builder(object *pl, int dir);
/* c_chat.c */
int command_say(object *op, char *params);
int command_me(object *op, char *params);
int command_cointoss(object *op, char *params);
int command_orcknuckle(object *op, char *params);
int command_shout(object *op, char *params);
int command_chat(object *op, char *params);
int command_tell(object *op, char *params);
int command_dmtell(object *op, char *params);
int command_reply(object *op, char *params);
int command_nod(object *op, char *params);
int command_dance(object *op, char *params);
int command_kiss(object *op, char *params);
int command_bounce(object *op, char *params);
int command_smile(object *op, char *params);
int command_cackle(object *op, char *params);
int command_laugh(object *op, char *params);
int command_giggle(object *op, char *params);
int command_shake(object *op, char *params);
int command_puke(object *op, char *params);
int command_growl(object *op, char *params);
int command_scream(object *op, char *params);
int command_sigh(object *op, char *params);
int command_sulk(object *op, char *params);
int command_hug(object *op, char *params);
int command_cry(object *op, char *params);
int command_poke(object *op, char *params);
int command_accuse(object *op, char *params);
int command_grin(object *op, char *params);
int command_bow(object *op, char *params);
int command_clap(object *op, char *params);
int command_blush(object *op, char *params);
int command_burp(object *op, char *params);
int command_chuckle(object *op, char *params);
int command_cough(object *op, char *params);
int command_flip(object *op, char *params);
int command_frown(object *op, char *params);
int command_gasp(object *op, char *params);
int command_glare(object *op, char *params);
int command_groan(object *op, char *params);
int command_hiccup(object *op, char *params);
int command_lick(object *op, char *params);
int command_pout(object *op, char *params);
int command_shiver(object *op, char *params);
int command_shrug(object *op, char *params);
int command_slap(object *op, char *params);
int command_smirk(object *op, char *params);
int command_snap(object *op, char *params);
int command_sneeze(object *op, char *params);
int command_snicker(object *op, char *params);
int command_sniff(object *op, char *params);
int command_snore(object *op, char *params);
int command_spit(object *op, char *params);
int command_strut(object *op, char *params);
int command_thank(object *op, char *params);
int command_twiddle(object *op, char *params);
int command_wave(object *op, char *params);
int command_whistle(object *op, char *params);
int command_wink(object *op, char *params);
int command_yawn(object *op, char *params);
int command_beg(object *op, char *params);
int command_bleed(object *op, char *params);
int command_cringe(object *op, char *params);
int command_think(object *op, char *params);
/* c_misc.c */
void map_info(object *op, char *search);
int command_language(object *op, char *params);
int command_body(object *op, char *params);
int command_motd(object *op, char *params);
int command_rules(object *op, char *params);
int command_news(object *op, char *params);
void malloc_info(object *op);
void current_region_info(object *op);
void current_map_info(object *op);
int command_whereabouts(object *op, char *params);
void list_players(object *op, region *reg, partylist *party);
int command_who(object *op, char *params);
void display_who_entry(object *op, player *pl, const char *format);
void get_who_escape_code_value(char *return_val, int size, const char letter, player *pl);
int command_afk(object *op, char *params);
int command_malloc(object *op, char *params);
int command_mapinfo(object *op, char *params);
int command_whereami(object *op, char *params);
int command_maps(object *op, char *params);
int command_strings(object *op, char *params);
int command_time(object *op, char *params);
int command_archs(object *op, char *params);
int command_hiscore(object *op, char *params);
int command_debug(object *op, char *params);
int command_dumpbelow(object *op, char *params);
int command_wizpass(object *op, char *params);
int command_wizcast(object *op, char *params);
int command_dumpallobjects(object *op, char *params);
int command_dumpfriendlyobjects(object *op, char *params);
int command_dumpallarchetypes(object *op, char *params);
int command_ssdumptable(object *op, char *params);
int command_dumpmap(object *op, char *params);
int command_dumpallmaps(object *op, char *params);
int command_printlos(object *op, char *params);
int command_version(object *op, char *params);
int command_listen(object *op, char *params);
int command_statistics(object *pl, char *params);
int command_fix_me(object *op, char *params);
int command_players(object *op, char *params);
int command_applymode(object *op, char *params);
int command_bowmode(object *op, char *params);
int command_unarmed_skill(object *op, char *params);
int command_petmode(object *op, char *params);
int command_showpets(object *op, char *params);
int command_usekeys(object *op, char *params);
int command_resistances(object *op, char *params);
int command_help(object *op, char *params);
int onoff_value(const char *line);
int command_quit(object *op, char *params);
int command_sound(object *op, char *params);
void receive_player_name(object *op);
void receive_player_password(object *op);
int command_title(object *op, char *params);
int command_save(object *op, char *params);
int command_peaceful(object *op, char *params);
int command_wimpy(object *op, char *params);
int command_brace(object *op, char *params);
int command_kill_pets(object *op, char *params);
int command_passwd(object *pl, char *params);
int do_harvest(object *pl, int dir, object *skill);
/* c_move.c */
int command_east(object *op, char *params);
int command_north(object *op, char *params);
int command_northeast(object *op, char *params);
int command_northwest(object *op, char *params);
int command_south(object *op, char *params);
int command_southeast(object *op, char *params);
int command_southwest(object *op, char *params);
int command_west(object *op, char *params);
int command_stay(object *op, char *params);
/* c_new.c */
int execute_newserver_command(object *pl, char *command);
int command_run(object *op, char *params);
int command_run_stop(object *op, char *params);
int command_fire(object *op, char *params);
int command_fire_stop(object *op, char *params);
/* c_object.c */
int command_uskill(object *pl, char *params);
int command_rskill(object *pl, char *params);
int command_search(object *op, char *params);
int command_disarm(object *op, char *params);
int command_throw(object *op, char *params);
int command_apply(object *op, char *params);
int sack_can_hold(const object *pl, const object *sack, const object *op, uint32 nrof);
void pick_up(object *op, object *alt);
int command_take(object *op, char *params);
void put_object_in_sack(object *op, object *sack, object *tmp, uint32 nrof);
object *drop_object(object *op, object *tmp, uint32 nrof);
void drop(object *op, object *tmp);
int command_dropall(object *op, char *params);
int command_drop(object *op, char *params);
int command_empty(object *op, char *params);
int command_examine(object *op, char *params);
object *find_marked_object(object *op);
int command_mark(object *op, char *params);
void examine_monster(object *op, object *tmp);
void examine(object *op, object *tmp);
void inventory(object *op, object *inv);
int command_pickup(object *op, char *params);
int command_search_items(object *op, char *params);
int command_rename_item(object *op, char *params);
int command_lock_item(object *op, char *params);
int command_use(object *op, char *params);
/* c_party.c */
int confirm_party_password(object *op);
void receive_party_password(object *op);
int command_gsay(object *op, char *params);
int command_party(object *op, char *params);
int command_party_rejoin(object *op, char *params);
/* c_range.c */
int command_invoke(object *op, char *params);
int command_cast(object *op, char *params);
int command_prepare(object *op, char *params);
int command_cast_spell(object *op, char *params, char command);
int legal_range(object *op, int r);
void change_spell(object *op, char k);
int command_rotateshoottype(object *op, char *params);
/* c_wiz.c */
int command_loadtest(object *op, char *params);
void do_wizard_hide(object *op, int silent_dm);
int command_hide(object *op, char *params);
int command_setgod(object *op, char *params);
int command_banish(object *op, char *params);
int command_kick(object *op, char *params);
int command_overlay_save(object *op, char *params);
int command_overlay_reset(object *op, char *params);
int command_toggle_shout(object *op, char *params);
int command_shutdown(object *op, char *params);
int command_goto(object *op, char *params);
int command_freeze(object *op, char *params);
int command_arrest(object *op, char *params);
int command_summon(object *op, char *params);
int command_teleport(object *op, char *params);
int command_create(object *op, char *params);
int command_inventory(object *op, char *params);
int command_skills(object *op, char *params);
int command_dump(object *op, char *params);
int command_mon_aggr(object *op, char *params);
int command_possess(object *op, char *params);
int command_patch(object *op, char *params);
int command_remove(object *op, char *params);
int command_free(object *op, char *params);
int command_addexp(object *op, char *params);
int command_speed(object *op, char *params);
int command_stats(object *op, char *params);
int command_abil(object *op, char *params);
int command_reset(object *op, char *params);
int command_nowiz(object *op, char *params);
int do_wizard_dm(object *op, char *params, int silent);
int command_dm(object *op, char *params);
int command_invisible(object *op, char *params);
int command_learn_spell(object *op, char *params);
int command_learn_special_prayer(object *op, char *params);
int command_forget_spell(object *op, char *params);
int command_listplugins(object *op, char *params);
int command_loadplugin(object *op, char *params);
int command_unloadplugin(object *op, char *params);
int command_dmhide(object *op, char *params);
void dm_stack_pop(player *pl);
object *dm_stack_peek(player *pl);
void dm_stack_push(player *pl, tag_t item);
object *get_dm_object(player *pl, char **params, int *from);
int command_stack_pop(object *op, char *params);
int command_stack_push(object *op, char *params);
int command_stack_list(object *op, char *params);
int command_stack_clear(object *op, char *params);
int command_diff(object *op, char *params);
int command_insert_into(object *op, char *params);
int command_style_map_info(object *op, char *params);
int command_follow(object *op, char *params);
/* commands.c */
void init_commands(void);
/* daemon.c */
void become_daemon(void);
/* disease.c */
int move_disease(object *disease);
int infect_object(object *victim, object *disease, int force);
void move_symptom(object *symptom);
void check_physically_infect(object *victim, object *hitter);
int cure_disease(object *sufferer, object *caster);
/* hiscore.c */
void hiscore_init(void);
void hiscore_check(object *op, int quiet);
void hiscore_display(object *op, int max, const char *match);
/* gods.c */
const object *find_god(const char *name);
const char *determine_god(object *op);
void pray_at_altar(object *pl, object *altar, object *skill);
int become_follower(object *op, const object *new_god);
archetype *determine_holy_arch(const object *god, const char *type);
int tailor_god_spell(object *spellop, object *caster);
/* init.c */
void init(int argc, char **argv);
void free_server(void);
racelink *find_racelink(const char *name);
/* knowledge.c */
void knowledge_read(player *pl, object *book);
int command_knowledge(object *pl, char *params);
void free_knowledge(void);
/* login.c */
void emergency_save(int flag);
void delete_character(const char *name);
int verify_player(const char *name, char *password);
int check_name(player *me, const char *name);
void destroy_object(object *op);
int save_player(object *op, int flag);
void check_login(object *op, int check_pass);
/* monster.c */
object *monster_check_enemy(object *npc, rv_vector *rv);
object *monster_find_nearest_living_creature(object *npc);
int monster_compute_path(object *source, object *target, int default_dir);
int monster_move(object *op);
void monster_check_apply_all(object *monster);
void monster_check_apply(object *mon, object *item);
void monster_npc_call_help(object *op);
void monster_check_earthwalls(object *op, mapstruct *m, int x, int y);
void monster_check_doors(object *op, mapstruct *m, int x, int y);
void monster_communicate(object *op, const char *txt);
void monster_npc_say(object *npc, const char *cp);
object *monster_find_throw_ob(object *op);
int monster_can_detect_enemy(object *op, object *enemy, rv_vector *rv);
int monster_stand_in_light(object *op);
int monster_can_see_enemy(object *op, object *enemy);
/* move.c */
int move_object(object *op, int dir);
int move_ob(object *op, int dir, object *originator);
int transfer_ob(object *op, int x, int y, int randomly, object *originator);
int teleport(object *teleporter, uint8 tele_type, object *user);
void recursive_roll(object *op, int dir, object *pusher);
int push_ob(object *who, int dir, object *pusher);
int move_to(object *op, int x, int y);
/* ob_methods.c */
void init_ob_methods(void);
/* ob_types.c */
void register_all_ob_types(void);
/* party.c */
partylist *party_form(object *op, const char *partyname);
void party_join(object *op, partylist *party);
void party_leave(object *op);
partylist *party_find(const char *partyname);
void party_remove(partylist *party);
partylist *party_get_first(void);
partylist *party_get_next(const partylist *party);
void party_obsolete_parties(void);
const char *party_get_password(const partylist *party);
void party_set_password(partylist *party, const char *password);
int party_confirm_password(const partylist *party, const char *password);
void party_send_message(object *op, const char *message);
const char *party_get_leader(const partylist *party);
/* pets.c */
object *pets_get_enemy(object *pet, rv_vector *rv);
void pets_terminate_all(object *owner);
void pets_remove_all(void);
void pets_follow_owner(object *ob, object *owner);
void pets_move(object *ob);
void pets_move_golem(object *op);
void pets_control_golem(object *op, int dir);
int pets_summon_golem(object *op, object *caster, int dir, object *spob);
int pets_summon_object(object *op, object *caster, object *spell_ob, int dir, const char *stringarg);
int pets_should_arena_attack(object *pet, object *owner, object *target);
/* player.c */
void set_first_map(object *op);
player *get_player(player *p);
player *find_player(const char *plname);
player *find_player_partial_name(const char *plname);
player* find_player_socket(const socket_struct *ns);
void display_motd(const object *op);
void send_rules(const object *op);
void send_news(const object *op);
int playername_ok(const char *cp);
player *add_player(socket_struct *ns, int new_char);
object *get_nearest_player(object *mon);
int path_to_player(object *mon, object *pl, unsigned mindiff);
void give_initial_items(object *pl, treasurelist *items);
void get_name(object *op);
void get_password(object *op);
void play_again(object *op);
void receive_play_again(object *op, char key);
void confirm_password(object *op);
int get_party_password(object *op, partylist *party);
int roll_stat(void);
void roll_stats(object *op);
void roll_again(object *op);
void key_roll_stat(object *op, char key);
void key_change_class(object *op, char key);
void key_confirm_quit(object *op, char key);
int check_pick(object *op);
int fire_bow(object *op, object *arrow, int dir, int wc_mod, sint16 sx, sint16 sy);
void fire(object *op, int dir);
object *find_key(object *pl, object *container, object *door);
void move_player_attack(object *op, int dir);
int move_player(object *op, int dir);
int handle_newcs_player(object *op);
void remove_unpaid_objects(object *op, object *env, int free_items);
void do_some_living(object *op);
void kill_player(object *op);
void fix_weight(void);
void fix_luck(void);
void cast_dust(object *op, object *throw_ob, int dir);
void make_visible(object *op);
int is_true_undead(object *op);
int hideability(object *ob);
void do_hidden_move(object *op);
int stand_near_hostile(object *who);
int player_can_view(object *pl, object *op);
int op_on_battleground(object *op, int *x, int *y, archetype **trophy);
void dragon_ability_gain(object *who, int atnr, int level);
void player_unready_range_ob(player *pl, object *ob);
/* plugins.c */
int user_event(object *op, object *activator, object *third, const char *message, int fix);
int execute_event(object *op, int eventcode, object *activator, object *third, const char *message, int fix);
int execute_global_event(int eventcode, ...);
int plugins_init_plugin(const char *libfile);
void *cfapi_get_hooks(int *type, ...);
int plugins_remove_plugin(const char *id);
crossfire_plugin *plugins_find_plugin(const char *id);
void plugins_display_list(object *op);
void *cfapi_system_find_animation(int *type, ...);
void *cfapi_system_find_face(int *type, ...);
void *cfapi_system_strdup_local(int *type, ...);
void *cfapi_system_register_global_event(int *type, ...);
void *cfapi_system_unregister_global_event(int *type, ...);
void *cfapi_system_add_string(int *type, ...);
void *cfapi_system_remove_string(int *type, ...);
void *cfapi_system_find_string(int *type, ...);
void *cfapi_system_check_path(int *type, ...);
void *cfapi_system_re_cmp(int *type, ...);
void *cfapi_system_directory(int *type, ...);
void *cfapi_get_time(int *type, ...);
void *cfapi_get_season_name(int *type, ...);
void *cfapi_get_weekday_name(int *type, ...);
void *cfapi_get_month_name(int *type, ...);
void *cfapi_get_periodofday_name(int *type, ...);
void *cfapi_timer_create(int *type, ...);
void *cfapi_timer_destroy(int *type, ...);
void *cfapi_log(int *type, ...);
void *cfapi_map_get_map(int *type, ...);
void *cfapi_map_has_been_loaded(int *type, ...);
void *cfapi_map_create_path(int *type, ...);
void *cfapi_map_get_map_property(int *type, ...);
void *cfapi_map_set_map_property(int *type, ...);
void *cfapi_map_out_of_map(int *type, ...);
void *cfapi_map_update_position(int *type, ...);
void *cfapi_map_delete_map(int *type, ...);
void *cfapi_map_message(int *type, ...);
void *cfapi_map_get_object_at(int *type, ...);
void *cfapi_map_find_by_archetype_name(int *type, ...);
void *cfapi_map_change_light(int *type, ...);
void *cfapi_object_move(int *type, ...);
void *cfapi_object_get_key(int *type, ...);
void *cfapi_object_set_key(int *type, ...);
void *cfapi_object_get_property(int *type, ...);
void *cfapi_object_set_property(int *type, ...);
void *cfapi_object_apply_below(int *type, ...);
void *cfapi_object_apply(int *type, ...);
void *cfapi_object_identify(int *type, ...);
void *cfapi_object_describe(int *type, ...);
void *cfapi_object_drain(int *type, ...);
void *cfapi_object_remove_depletion(int *type, ...);
void *cfapi_object_fix(int *type, ...);
void *cfapi_object_give_skill(int *type, ...);
void *cfapi_object_transmute(int *type, ...);
void *cfapi_object_remove(int *type, ...);
void *cfapi_object_delete(int *type, ...);
void *cfapi_object_clone(int *type, ...);
void *cfapi_object_find(int *type, ...);
void *cfapi_object_create(int *type, ...);
void *cfapi_object_insert(int *type, ...);
void *cfapi_object_split(int *type, ...);
void *cfapi_object_merge(int *type, ...);
void *cfapi_object_distance(int *type, ...);
void *cfapi_object_update(int *type, ...);
void *cfapi_object_clear(int *type, ...);
void *cfapi_object_reset(int *type, ...);
void *cfapi_object_check_inventory(int *type, ...);
void *cfapi_object_clean_object(int *type, ...);
void *cfapi_object_on_same_map(int *type, ...);
void *cfapi_object_spring_trap(int *type, ...);
void *cfapi_object_check_trigger(int *type, ...);
void *cfapi_map_trigger_connected(int *type, ...);
void *cfapi_object_query_cost(int *type, ...);
void *cfapi_object_query_cost_string(int *type, ...);
void *cfapi_object_query_money(int *type, ...);
void *cfapi_object_cast(int *type, ...);
void *cfapi_object_learn_spell(int *type, ...);
void *cfapi_object_forget_spell(int *type, ...);
void *cfapi_object_check_spell(int *type, ...);
void *cfapi_object_pay_amount(int *type, ...);
void *cfapi_object_pay_item(int *type, ...);
void *cfapi_object_transfer(int *type, ...);
void *cfapi_object_find_archetype_inside(int *type, ...);
void *cfapi_object_drop(int *type, ...);
void *cfapi_object_change_abil(int *type, ...);
void *cfapi_object_say(int *type, ...);
void *cfapi_player_find(int *type, ...);
void *cfapi_player_message(int *type, ...);
void *cfapi_object_change_exp(int *type, ...);
void *cfapi_player_can_pay(int *type, ...);
void *cfapi_object_teleport(int *type, ...);
void *cfapi_object_pickup(int *type, ...);
void *cfapi_object_find_by_arch_name(int *type, ...);
void *cfapi_object_find_by_name(int *type, ...);
void *cfapi_archetype_get_property(int *type, ...);
void *cfapi_party_get_property(int *type, ...);
void *cfapi_region_get_property(int *type, ...);
void *cfapi_friendlylist_get_next(int *type, ...);
void *cfapi_set_random_map_variable(int *type, ...);
void *cfapi_generate_random_map(int *type, ...);
void *cfapi_object_user_event(int *type, ...);
void *cfapi_cost_string_from_value(int *type, ...);
void *cfapi_player_quest(int *type, ...);
command_array_struct *find_plugin_command(char *cmd, object *op);
int initPlugins(void);
void cleanupPlugins(void);
/* quest.c */
int quest_get_player_state(player *pl, sstring quest_code);
void quest_start(player *pl, sstring quest_code, int state);
void quest_end(player *pl, sstring quest_code);
void quest_set_player_state(player *pl, sstring quest_code, int state);
int quest_was_completed(player *pl, sstring quest_code);
int command_quest(object *op, char *params);
void free_quest(void);
/* resurrection.c */
int cast_raise_dead_spell(object *op, object *caster, object *spell, int dir, const char *arg);
void dead_player(object *op);
/* rune.c */
int write_rune(object *op, object *caster, object *spell, int dir, const char *runename);
void spring_trap(object *trap, object *victim);
int dispel_rune(object *op, object *caster, object *spell, object *skill, int dir);
int trap_see(object *op, object *trap);
int trap_show(object *trap, object *where);
int trap_disarm(object *disarmer, object *trap, int risk, object *skill);
void trap_adjust(object *trap, int difficulty);
/* shop.c */
uint64 query_cost(const object *tmp, object *who, int flag);
StringBuffer *query_cost_string(const object *tmp, object *who, int flag, StringBuffer *buf);
uint64 query_money(const object *op);
int pay_for_amount(uint64 to_pay, object *pl);
int pay_for_item(object *op, object *pl);
int can_pay(object *pl);
int get_payment(object *pl, object *op);
void sell_item(object *op, object *pl);
double shopkeeper_approval(const mapstruct *map, const object *player);
int describe_shop(const object *op);
int is_in_shop(object *ob);
int coords_in_shop(mapstruct *map, int x, int y);
StringBuffer *cost_string_from_value(uint64 cost, StringBuffer *buf);
/* skills.c */
int steal(object *op, int dir, object *skill);
int pick_lock(object *pl, int dir, object *skill);
int hide(object *op, object *skill);
int jump(object *pl, int dir, object *skill);
int skill_ident(object *pl, object *skill);
int use_oratory(object *pl, int dir, object *skill);
int singing(object *pl, int dir, object *skill);
int find_traps(object *pl, object *skill);
int remove_trap(object *op, object *skill);
int pray(object *pl, object *skill);
void meditate(object *pl, object *skill);
int write_on_item(object *pl, const char *params, object *skill);
int skill_throw(object *op, object *part, int dir, const char *params, object *skill);
/* skill_util.c */
void init_skills(void);
void link_player_skills(object *op);
object *find_skill_by_name(object *who, const char *name);
object *find_skill_by_number(object *who, int skillno);
int change_skill(object *who, object *new_skill, int flag);
void clear_skill(object *who);
int do_skill(object *op, object *part, object *skill, int dir, const char *string);
sint64 calc_skill_exp(object *who, object *op, object *skill);
int learn_skill(object *pl, object *scroll);
void show_skills(object *op, const char *search);
int use_skill(object *op, const char *string);
int skill_attack(object *tmp, object *pl, int dir, const char *string, object *skill);
/* spell_attack.c */
int fire_bolt(object *op, object *caster, int dir, object *spob);
void explode_bullet(object *op);
void check_bullet(object *op);
void cone_drop(object *op);
int cast_cone(object *op, object *caster, int dir, object *spell);
int create_bomb(object *op, object *caster, int dir, object *spell);
int cast_smite_spell(object *op, object *caster, int dir, object *spell);
int cast_destruction(object *op, object *caster, object *spell_ob);
int cast_curse(object *op, object *caster, object *spell_ob, int dir);
int mood_change(object *op, object *caster, object *spell);
int fire_swarm(object *op, object *caster, object *spell, int dir);
int cast_light(object *op, object *caster, object *spell, int dir);
int cast_cause_disease(object *op, object *caster, object *spell, int dir);
/* spell_effect.c */
void cast_magic_storm(object *op, object *tmp, int lvl);
int recharge(object *op, object *caster, object *spell_ob);
void polymorph(object *op, object *who, int level);
int cast_polymorph(object *op, object *caster, object *spell_ob, int dir);
int cast_create_missile(object *op, object *caster, object *spell, int dir, const char *stringarg);
int cast_create_food(object *op, object *caster, object *spell_ob, int dir, const char *stringarg);
int probe(object *op, object *caster, object *spell_ob, int dir);
int makes_invisible_to(object *pl, object *mon);
int cast_invisible(object *op, object *caster, object *spell_ob);
int cast_earth_to_dust(object *op, object *caster, object *spell_ob);
int cast_word_of_recall(object *op, object *caster, object *spell_ob);
int cast_wonder(object *op, object *caster, int dir, object *spell_ob);
int perceive_self(object *op);
int cast_create_town_portal(object *op, object *caster, object *spell, int dir);
int magic_wall(object *op, object *caster, int dir, object *spell_ob);
int dimension_door(object *op, object *caster, object *spob, int dir);
int cast_heal(object *op, object *caster, object *spell, int dir);
int cast_change_ability(object *op, object *caster, object *spell_ob, int dir, int silent);
int cast_bless(object *op, object *caster, object *spell_ob, int dir);
int alchemy(object *op, object *caster, object *spell_ob);
int remove_curse(object *op, object *caster, object *spell);
int cast_item_curse_or_curse(object *op, object *caster, object *spell_ob);
int cast_identify(object *op, object *caster, object *spell);
int cast_detection(object *op, object *caster, object *spell);
int cast_transfer(object *op, object *caster, object *spell, int dir);
void counterspell(object *op, int dir);
int cast_consecrate(object *op, object *caster, object *spell);
int animate_weapon(object *op, object *caster, object *spell, int dir);
int cast_change_map_lightlevel(object *op, object *caster, object *spell);
int create_aura(object *op, object *caster, object *spell);
int write_mark(object *op, object *spell, const char *msg);
/* spell_util.c */
object *find_random_spell_in_ob(object *ob, const char *skill);
void set_spell_skill(object *op, object *caster, object *spob, object *dest);
void check_spells(void);
void dump_spells(void);
void spell_effect(object *spob, int x, int y, mapstruct *map, object *originator);
int min_casting_level(const object *caster, const object *spell);
int caster_level(const object *caster, const object *spell);
sint16 SP_level_spellpoint_cost(object *caster, object *spell, int flags);
int SP_level_dam_adjust(const object *caster, const object *spob);
int SP_level_duration_adjust(const object *caster, const object *spob);
int SP_level_range_adjust(const object *caster, const object *spob);
object *check_spell_known(object *op, const char *name);
object *lookup_spell_by_name(object *op, const char *spname);
int reflwall(mapstruct *m, int x, int y, object *sp_op);
int cast_create_obj(object *op, object *new_op, int dir);
int ok_to_put_more(mapstruct *m, sint16 x, sint16 y, object *op, uint32 immune_stop);
int fire_arch_from_position(object *op, object *caster, sint16 x, sint16 y, int dir, object *spell);
void regenerate_rod(object *rod);
void drain_rod_charge(object *rod);
void drain_wand_charge(object *wand);
object *find_target_for_friendly_spell(object *op, int dir);
int spell_find_dir(mapstruct *m, int x, int y, object *exclude);
int summon_hostile_monsters(object *op, int n, const char *monstername);
void shuffle_attack(object *op, int change_face);
void spell_failure(object *op, int failure, int power, object *skill);
int cast_spell(object *op, object *caster, int dir, object *spell_ob, char *stringarg);
void store_spell_expiry(object *spell);
void check_spell_expiry(object *spell);
void rod_adjust(object *rod);
/* swap.c */
void read_map_log(void);
int swap_map(mapstruct *map);
void check_active_maps(void);
void swap_below_max(const char *except_level);
int players_on_map(mapstruct *m, int show_all);
void flush_old_maps(void);
/* time.c */
void remove_door(object *op);
void remove_locked_door(object *op);
object *stop_item(object *op);
void fix_stopped_item(object *op, mapstruct *map, object *originator);
object *fix_stopped_arrow(object *op);
int free_no_drop(object *op);
void change_object(object *op);
void move_firewall(object *op);
void move_player_mover(object *op);
int process_object(object *op);
void legacy_remove_force(object *op);
void legacy_animate_trigger(object *op);
void legacy_move_hole(object *op);
/* timers.c */
void cftimer_process_timers(void);
int cftimer_create(int id, long delay, object *ob, int mode);
int cftimer_destroy(int id);
int cftimer_find_free_id(void);
void cftimer_init(void);
/* weather.c */
void set_darkness_map(mapstruct *m);
void tick_the_clock(void);
int similar_direction(int a, int b);
/* server.c */
void version(object *op);
void start_info(object *op);
char *crypt_string(char *str, char *salt);
int check_password(char *typed, char *crypted);
void enter_player_savebed(object *op);
void set_map_timeout(mapstruct *oldmap);
void enter_exit(object *op, object *exit_ob);
void process_events(void);
void clean_tmp_files(void);
void cleanup(void);
void leave(player *pl, int draw_exit);
int forbid_play(void);
int server_main(int argc, char **argv);
