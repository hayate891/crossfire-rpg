/**
 * Those are dummy functions defined to resolve all symboles.
 * Added as part of glue cleaning.
 * Ryo 2005-07-15
 **/

#include <global.h>

void apply_auto_fix(mapstruct *m) {
}

void draw_ext_info(int flags, int pri, const object *pl, uint8 type, uint8 subtype, const char *message) {
    fprintf(logfile, "%s\n", message);
}

void move_firewall(object *ob) {
}

void emergency_save(int x) {
}

void clean_tmp_files(void) {
}

void esrv_send_item(object *ob, object *obx) {
}

void dragon_ability_gain(object *ob, int x, int y) {
}

void set_darkness_map(mapstruct *m) {
}

object *find_skill_by_number(object *who, int skillno) {
    return NULL;
}

void esrv_del_item(player *pl, int tag) {
}

void esrv_update_spells(player *pl) {
}

void monster_check_apply(object *ob, object *obt) {
}

void trap_adjust(object *ob, int x) {
}

void rod_adjust(object *rod) {
}
