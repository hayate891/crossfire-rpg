/* cfanim.c */
int get_dir_from_name(char *name);
long int initmovement(char *name, char *parameters, struct CFmovement_struct *move_entity);
int runmovement(struct CFanimation_struct *animation, long int id, void *parameters);
long int initfire(char *name, char *parameters, struct CFmovement_struct *move_entity);
int runfire(struct CFanimation_struct *animation, long int id, void *parameters);
long int initturn(char *name, char *parameters, struct CFmovement_struct *move_entity);
int runturn(struct CFanimation_struct *animation, long int id, void *parameters);
long int initcamera(char *name, char *parameters, struct CFmovement_struct *move_entity);
int runcamera(struct CFanimation_struct *animation, long int id, void *parameters);
long int initvisible(char *name, char *parameters, struct CFmovement_struct *move_entity);
int runvisible(struct CFanimation_struct *animation, long int id, void *parameters);
long int initwizard(char *name, char *parameters, struct CFmovement_struct *move_entity);
int runwizard(struct CFanimation_struct *animation, long int id, void *parameters);
long int initsay(char *name, char *parameters, struct CFmovement_struct *move_entity);
int runsay(struct CFanimation_struct *animation, long int id, void *parameters);
long int initapply(char *name, char *parameters, struct CFmovement_struct *move_entity);
int runapply(struct CFanimation_struct *animation, long int id, void *parameters);
long int initapplyobject(char *name, char *parameters, struct CFmovement_struct *move_entity);
int runapplyobject(struct CFanimation_struct *animation, long int id, void *parameters);
long int initdropobject(char *name, char *parameters, struct CFmovement_struct *move_entity);
int rundropobject(struct CFanimation_struct *animation, long int id, void *parameters);
long int initpickup(char *name, char *parameters, struct CFmovement_struct *move_entity);
int runpickup(struct CFanimation_struct *animation, long int id, void *parameters);
long int initpickupobject(char *name, char *parameters, struct CFmovement_struct *move_entity);
int runpickupobject(struct CFanimation_struct *animation, long int id, void *parameters);
long int initghosted(char *name, char *parameters, struct CFmovement_struct *move_entity);
int runghosted(struct CFanimation_struct *animation, long int id, void *parameters);
long int initteleport(char *name, char *parameters, struct CFmovement_struct *move_entity);
int runteleport(struct CFanimation_struct *animation, long int id, void *parameters);
long int initnotice(char *name, char *parameters, struct CFmovement_struct *move_entity);
int runnotice(struct CFanimation_struct *animation, long int id, void *parameters);
void prepare_commands(void);
int equality_split(char *buffer, char **variable, char **value);
int get_boolean(char *string, int *bool);
int is_animated_player(object *pl);
void free_events(object *who);
int start_animation(object *who, object *activator, char *file, char *options);
void animate(void);
void initContextStack(void);
void pushContext(CFPContext *context);
CFPContext *popContext(void);
CF_PLUGIN int initPlugin(const char *iversion, f_plug_api gethooksptr);
CF_PLUGIN void *getPluginProperty(int *type, ...);
CF_PLUGIN int runPluginCommand(object *op, char *params);
CF_PLUGIN int postInitPlugin(void);
CF_PLUGIN void *globalEventListener(int *type, ...);
CF_PLUGIN void *eventListener(int *type, ...);
CF_PLUGIN int closePlugin(void);
/* ../common/plugin_common.c */
int cf_init_plugin(f_plug_api getHooks);
void *cf_map_get_property(mapstruct *map, int propcode);
void *cf_map_set_int_property(mapstruct *map, int propcode, int value);
void *cf_object_get_property(object *op, int propcode);
int cf_object_get_resistance(object *op, int rtype);
void *cf_object_set_int_property(object *op, int propcode, int value);
void *cf_object_set_long_property(object *op, int propcode, long value);
void *cf_object_set_string_property(object *op, int propcode, char *value);
void cf_player_move(player *pl, int dir);
void cf_object_move(object *op, object *originator, int dir);
object *cf_player_send_inventory(object *op);
void cf_object_apply(object *op, object *author, int flags);
void cf_object_apply_below(object *op);
void cf_object_remove(object *op);
void cf_object_free(object *op);
object *cf_object_present_archname_inside(object *op, char *whatstr);
int cf_object_transfer(object *op, int x, int y, int r, object *orig);
int cf_object_change_map(object *op, int x, int y, mapstruct *map);
object *cf_map_get_object_at(mapstruct *m, int x, int y);
void cf_map_message(mapstruct *m, char *msg, int color);
object *cf_object_clone(object *op, int clonetype);
int cf_object_pay_item(object *op, object *buyer);
int cf_object_pay_amount(object *op, uint64 amount);
int cf_object_cast_spell(object *caster, object *ctoo, int dir, object *sp, char *flags);
int cf_object_cast_ability(object *caster, object *ctoo, int dir, object *sp, char *flags);
void cf_object_learn_spell(object *op, object *sp);
void cf_object_forget_spell(object *op, object *sp);
object *cf_object_check_for_spell(object *op, char *spellname);
void cf_player_message(object *op, char *txt, int flags);
player *cf_player_find(char *txt);
char *cf_player_get_ip(object *op);
mapstruct *cf_map_get_map(char *name);
int cf_object_query_money(object *op);
int cf_object_query_cost(object *op, object *who, int flags);
void cf_object_activate_rune(object *op, object *victim);
int cf_object_check_trigger(object *op, object *cause);
int cf_object_out_of_map(object *op, int x, int y);
void cf_object_drop(object *op, object *author);
void cf_object_take(object *op, object *author);
void cf_object_say(object *op, char *msg);
void cf_object_speak(object *op, char *msg);
object *cf_object_insert_object(object *op, object *container);
char *cf_get_maps_directory(char *str);
object *cf_create_object(void);
object *cf_create_object_by_name(const char *name);
void cf_free_object(object *ob);
void cf_system_register_global_event(int event, const char *name, f_plug_api hook);
void cf_fix_object(object *pl);
char *cf_add_string(char *str);
void cf_free_string(char *str);
char *cf_query_name(object *ob);
char *cf_query_base_name(object *ob, int plural);
object *cf_insert_ob_in_ob(object *ob, object *where);
const char *cf_object_get_msg(object *ob);
void cf_object_set_weight(object *ob, int weight);
void cf_object_set_weight_limit(object *ob, int weight);
int cf_object_get_weight(object *ob);
int cf_object_get_weight_limit(object *ob);
int cf_object_set_nrof(object *ob, int nrof);
int cf_object_get_nrof(object *ob);
int cf_object_get_flag(object *ob, int flag);
void cf_object_set_flag(object *ob, int flag, int value);
object *cf_object_insert_in_ob(object *ob, object *where);
object *cf_map_insert_object_there(mapstruct *where, object *op, object *originator, int flags);
object *cf_map_insert_object(mapstruct *where, object *op, int x, int y);
int cf_object_teleport(object *op, mapstruct *map, int x, int y);
object *cf_map_present_arch_by_name(const char *what, mapstruct *map, int nx, int ny);
void cf_object_update(object *op, int flags);
void cf_object_pickup(object *op, object *what);
char *cf_strdup_local(char *txt);
int cf_map_get_flags(mapstruct *map, mapstruct **nmap, sint16 x, sint16 y, sint16 *nx, sint16 *ny);
int cf_find_animation(char *txt);
char *cf_object_get_key(object *op, char *keyname);
void cf_object_set_key(object *op, char *keyname, char *value);
