/* gnome-sound.c */
extern int init_sounds(void);
extern void SoundCmd(unsigned char *data, int len);
/* gnome.c */
extern void disconnect(GtkWidget *widget);
extern void gnome_client_quit(void);
extern void resize_map_window(int x, int y);
extern void finish_face_cmd(int pnum, uint32 checksum, int has_sum, char *face);
extern void bind_key(char *params);
extern void unbind_key(char *params);
extern void load_defaults(void);
extern void save_defaults(void);
extern int find_face_in_private_cache(char *face, int checksum);
extern char **xpmbuffertodata(char *buffer);
extern void freexpmdata(char **strings);
extern void do_network(void);
extern void event_loop(void);
extern void end_windows(void);
extern void freeanimview(gpointer data, gpointer user_data);
extern void animate_list(void);
extern void button_map_event(GtkWidget *widget, GdkEventButton *event);
extern void keyrelfunc(GtkWidget *widget, GdkEventKey *event, GtkWidget *window);
extern void keyfunc(GtkWidget *widget, GdkEventKey *event, GtkWidget *window);
extern void draw_prompt(const char *str);
extern void cast_diawin_click_cb(GtkWidget *widget, gint button_number);
extern void cast_diawin_select_cb(GtkWidget *widget, gint row, gint column);
extern void cast_diawin_unselect_cb(GtkWidget *widget, gint row, gint column);
extern void draw_info(const char *str, int color);
extern void draw_color_info(int colr, const char *buf);
extern void draw_stats(int redraw);
extern void create_stat_bar(GtkWidget *mtable, gint row, gchar *label, gint bar, GtkWidget **plabel);
extern void draw_message_window(int redraw);
extern void open_container(item *op);
extern void close_container(item *op);
extern void count_callback(GtkWidget *widget, GtkWidget *entry);
extern void create_notebook_page(GtkWidget *notebook, GtkWidget **list, GtkWidget **lists, gchar **label);
extern void draw_lists(void);
extern void set_show_icon(char *s);
extern void set_show_weight(char *s);
extern void aboutdialog(GtkWidget *widget);
extern void applyconfig(void);
extern void saveconfig(void);
extern void ckeyclear(void);
extern void cclist_button_event(GtkWidget *gtklist, gint row, gint column, GdkEventButton *event);
extern void draw_keybindings(GtkWidget *keylist);
extern void bind_callback(GtkWidget *gtklist, GdkEventButton *event);
extern void ckeyunbind(GtkWidget *gtklist, GdkEventButton *event);
extern void tbccb(GtkWidget *widget);
extern void configdialog(GtkWidget *widget);
extern void chelpdialog(GtkWidget *widget);
extern void shelpdialog(GtkWidget *widget);
extern void menu_pickup0(void);
extern void menu_pickup1(void);
extern void menu_pickup2(void);
extern void menu_pickup3(void);
extern void menu_pickup4(void);
extern void menu_pickup5(void);
extern void menu_pickup6(void);
extern void menu_pickup7(void);
extern void menu_pickup10(void);
extern void menu_who(void);
extern void menu_apply(void);
extern void menu_cast(void);
extern void menu_search(void);
extern void menu_disarm(void);
extern void menu_spells(void);
extern void menu_clear(void);
extern void sexit(void);
extern void create_splash(void);
extern void destroy_splash(void);
extern void create_windows(void);
extern void set_weight_limit(uint32 wlim);
extern void set_scroll(char *s);
extern void draw_all_info(void);
extern void resize_win_info(void);
extern int get_info_width(void);
extern void do_clearlock(void);
extern void x_set_echo(void);
extern int do_timeout(void);
extern void display_newbitmap(long face, long fg, long bg, char *buf);
extern void draw_magic_map(void);
extern void magic_map_flash_pos(void);
extern void command_show(char *params);
extern void display_mapcell_pixmap(int ax, int ay);
extern int display_usebitmaps(void);
extern int display_usexpm(void);
extern int display_usepng(void);
extern int display_willcache(void);
extern void display_map_newmap(void);
extern void display_map_doneupdate(int redraw);
extern void display_newpix(long face, char *buf, long buflen);
extern void display_newpng(long face, char *buf, long buflen);
extern void display_newpixmap(long face, char *buf, long buflen);
extern void display_map_startupdate(void);
extern char *get_metaserver(void);
extern void reset_image_data(void);
extern void save_winpos(void);
extern int main(int argc, char **argv);
/* map.c */
extern void allocate_map(struct Map *new_map, int ax, int ay);
extern void reset_map(void);
extern void display_map_clearcell(long x, long y);
extern void print_darkness(void);
extern void print_map(void);
extern void set_map_darkness(int x, int y, uint8 darkness);
extern void set_map_face(int x, int y, int layer, int face);
extern void display_map_addbelow(long x, long y, long face);
extern void display_mapscroll(int dx, int dy);
extern void reset_map_data(void);
extern void gtk_draw_map(void);
