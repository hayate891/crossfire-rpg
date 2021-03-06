/* image.c */
int is_valid_faceset(int fsn);
void free_socket_images(void);
void read_client_images(void);
void set_face_mode_cmd(char *buf, int len, socket_struct *ns);
void send_face_cmd(char *buff, int len, socket_struct *ns);
void esrv_send_face(socket_struct *ns, short face_num, int nocache);
void send_image_info(socket_struct *ns, char *params);
void send_image_sums(socket_struct *ns, char *params);
/* info.c */
void flush_output_element(const object *pl, Output_Buf *outputs);
void new_draw_info(int flags, int pri, const object *pl, const char *buf);
void new_draw_info_format(int flags, int pri, const object *pl, const char *format, ...);
void draw_ext_info(int flags, int pri, const object *pl, uint8 type, uint8 subtype, const char *message, const char *oldmessage);
void draw_ext_info_format(int flags, int pri, const object *pl, uint8 type, uint8 subtype, const char *old_format, const char *new_format, ...);
void new_info_map_except(int color, const mapstruct *map, const object *op, const char *str);
void new_info_map_except2(int color, const mapstruct *map, const object *op1, const object *op2, const char *str);
void new_info_map(int color, const mapstruct *map, const char *str);
void clear_win_info(object *op);
void rangetostring(const object *pl, char *obuf);
void set_title(const object *pl, char *buf);
void magic_mapping_mark(object *pl, char *map_mark, int strength);
void draw_magic_map(object *pl);
void log_kill(const char *Who, const char *What, int WhatType, const char *With, int WithType);
/* init.c */
void init_connection(socket_struct *ns, const char *from_ip);
void init_ericserver(void);
void free_all_newserver(void);
void free_newsocket(socket_struct *ns);
void final_free_player(player *pl);
/* item.c */
void esrv_draw_look(object *pl);
void esrv_send_inventory(object *pl, object *op);
void esrv_update_item(int flags, object *pl, object *op);
void esrv_send_item(object *pl, object *op);
void esrv_del_item(player *pl, int tag);
void examine_cmd(char *buf, int len, player *pl);
void apply_cmd(char *buf, int len, player *pl);
void lock_item_cmd(uint8 *data, int len, player *pl);
void mark_item_cmd(uint8 *data, int len, player *pl);
void look_at(object *op, int dx, int dy);
void look_at_cmd(char *buf, int len, player *pl);
void esrv_move_object(object *pl, tag_t to, tag_t tag, long nrof);
/* loop.c */
void request_info_cmd(char *buf, int len, socket_struct *ns);
void handle_oldsocket(socket_struct *ns);
void handle_client(socket_struct *ns, player *pl);
void watchdog(void);
void doeric_server(void);
/* lowlevel.c */
void SockList_Init(SockList *sl);
void SockList_AddChar(SockList *sl, char c);
void SockList_AddShort(SockList *sl, uint16 data);
void SockList_AddInt(SockList *sl, uint32 data);
void SockList_AddInt64(SockList *sl, uint64 data);
int GetInt_String(const unsigned char *data);
short GetShort_String(const unsigned char *data);
int SockList_ReadPacket(int fd, SockList *sl, int len);
void write_socket_buffer(socket_struct *ns);
void cs_write_string(socket_struct *ns, const char *buf, int len);
void Send_With_Handling(socket_struct *ns, const SockList *msg);
void Write_String_To_Socket(socket_struct *ns, const char *buf, int len);
void write_cs_stats(void);
/* metaserver.c */
void metaserver_init(void);
void metaserver_update(void);
/* request.c */
void set_up_cmd(char *buf, int len, socket_struct *ns);
void add_me_cmd(char *buf, int len, socket_struct *ns);
void toggle_extended_infos_cmd(char *buf, int len, socket_struct *ns);
void toggle_extended_text_cmd(char *buf, int len, socket_struct *ns);
void ask_smooth_cmd(char *buf, int len, socket_struct *ns);
void player_cmd(char *buf, int len, player *pl);
void new_player_cmd(uint8 *buf, int len, player *pl);
void reply_cmd(char *buf, int len, player *pl);
void version_cmd(char *buf, int len, socket_struct *ns);
void set_sound_cmd(char *buf, int len, socket_struct *ns);
void map_redraw_cmd(char *buf, int len, player *pl);
void map_newmap_cmd(player *pl);
void move_cmd(char *buf, int len, player *pl);
void send_query(socket_struct *ns, uint8 flags, const char *text);
void esrv_update_stats(player *pl);
void esrv_new_player(player *pl, uint32 weight);
void esrv_send_animation(socket_struct *ns, short anim_num);
void draw_client_map1(object *pl);
void draw_client_map2(object *pl);
void draw_client_map(object *pl);
void esrv_map_scroll(socket_struct *ns, int dx, int dy);
void send_plugin_custom_message(object *pl, char *buf);
void send_skill_info(socket_struct *ns, char *params);
void send_spell_paths(socket_struct *ns, char *params);
void esrv_update_spells(player *pl);
void esrv_remove_spell(player *pl, object *spell);
void esrv_add_spells(player *pl, object *spell);
void send_tick(player *pl);
/* sounds.c */
void play_sound_player_only(player *pl, short soundnum, sint8 x, sint8 y);
void play_sound_map(const mapstruct *map, int x, int y, short sound_num);
