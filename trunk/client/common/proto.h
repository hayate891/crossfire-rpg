/* client.c */
extern void DoClient(ClientSocket *csocket);
extern int init_connection(char *host, int port);
extern void negotiate_connection(int sound);
/* commands.c */
extern void ReplyInfoCmd(char *buf, int len);
extern void SetupCmd(char *buf, int len);
extern void AddMeFail(char *data, int len);
extern void AddMeSuccess(char *data, int len);
extern void GoodbyeCmd(char *data, int len);
extern void AnimCmd(unsigned char *data, int len);
extern void DrawInfoCmd(char *data, int len);
extern void StatsCmd(unsigned char *data, int len);
extern void handle_query(char *data, int len);
extern void send_reply(char *text);
extern void PlayerCmd(unsigned char *data, int len);
extern void item_actions(item *op);
extern void ItemCmd(unsigned char *data, int len);
extern void Item1Cmd(unsigned char *data, int len);
extern void UpdateItemCmd(unsigned char *data, int len);
extern void DeleteItem(unsigned char *data, int len);
extern void DeleteInventory(unsigned char *data, int len);
extern void NewmapCmd(unsigned char *data, int len);
extern void Map_unpacklayer(unsigned char *cur, unsigned char *end);
extern void MapCmd(unsigned char *data, int len);
extern void Map1Cmd(unsigned char *data, int len);
extern void map_scrollCmd(char *data, int len);
extern void MagicMapCmd(unsigned char *data, int len);
/* image.c */
extern void init_common_cache_data(void);
extern void requestface(int pnum, char *facename);
extern void finish_face_cmd(int pnum, uint32 checksum, int has_sum, char *face, int faceset);
extern void reset_image_cache_data(void);
extern void FaceCmd(unsigned char *data, int len);
extern void Face1Cmd(unsigned char *data, int len);
extern void Face2Cmd(uint8 *data, int len);
extern void ImageCmd(uint8 *data, int len);
extern void Image2Cmd(uint8 *data, int len);
extern void display_newpng(long face, uint8 *buf, long buflen, int setnum);
extern void get_image_info(char *data, int len);
extern void get_image_sums(uint8 *data, int len);
/* init.c */
extern void VersionCmd(char *data, int len);
extern void SendVersion(ClientSocket csock);
extern void SendAddMe(ClientSocket csock);
extern void SendSetFaceMode(ClientSocket csock, int mode);
extern void init_client_vars(void);
extern void reset_client_vars(void);
/* item.c */
extern void init_item_types(void);
extern uint8 get_type_from_name(const char *name);
extern void update_item_sort(item *it);
extern char *get_number(int i);
extern void free_all_items(item *op);
extern item *locate_item(sint32 tag);
extern void remove_item(item *op);
extern void remove_item_inventory(item *op);
extern item *create_new_item(item *env, sint32 tag);
extern void set_item_values(item *op, char *name, sint32 weight, uint16 face, uint16 flags, uint16 anim, uint16 animspeed, sint32 nrof);
extern void toggle_locked(item *op);
extern void send_mark_obj(item *op);
extern item *player_item(void);
extern item *map_item(void);
extern void update_item(int tag, int loc, char *name, int weight, int face, int flags, int anim, int animspeed, int nrof);
extern void print_inventory(item *op);
extern void animate_objects(void);
/* metaserver.c */
extern int metaserver_get_info(char *metaserver, int meta_port);
extern void metaserver_show(int show_selection);
extern int metaserver_select(char *sel);
/* misc.c */
extern int make_path_to_dir(char *directory);
extern int make_path_to_file(char *filename);
extern char *strdup_local(char *str);
/* newsocket.c */
extern void LOG(int logLevel, char *format, ...);
extern void SockList_Init(SockList *sl, char *buf);
extern void SockList_AddChar(SockList *sl, char c);
extern void SockList_AddShort(SockList *sl, uint16 data);
extern void SockList_AddInt(SockList *sl, uint32 data);
extern void SockList_AddString(SockList *sl, const char *str);
extern int SockList_Send(SockList *sl, int fd);
extern int GetInt_String(unsigned char *data);
extern short GetShort_String(unsigned char *data);
extern int SockList_ReadPacket(int fd, SockList *sl, int len);
extern int cs_print_string(int fd, char *str, ...);
/* player.c */
extern void new_player(long tag, char *name, long weight, long face);
extern void look_at(int x, int y);
extern void client_send_apply(int tag);
extern void client_send_examine(int tag);
extern void client_send_move(int loc, int tag, int nrof);
extern void move_player(int dir);
extern void stop_fire(void);
extern void clear_fire_run(void);
extern void clear_fire(void);
extern void clear_run(void);
extern void fire_dir(int dir);
extern void stop_run(void);
extern void run_dir(int dir);
extern int send_command(const char *command, int repeat, int must_send);
extern void CompleteCmd(unsigned char *data, int len);
extern void show_help(void);
extern void extended_command(const char *ocommand);
extern char *complete_command(char *command);
