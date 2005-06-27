/* config.c */
extern void load_defaults(void);
extern void save_defaults(void);
/* image.c */
extern int create_and_rescale_image_from_data(Cache_Entry *ce, int pixmap_num, uint8 *rgba_data, int width, int height);
extern void addsmooth(uint16 face, uint16 smooth_face);
extern int associate_cache_entry(Cache_Entry *ce, int pixnum);
extern void reset_image_data(void);
extern void image_update_download_status(int start, int end, int total);
extern void get_map_image_size(int face, uint8 *w, uint8 *h);
extern void init_cache_data(void);
/* info.c */
extern void info_init(GtkWidget *window_root);
extern void draw_info(const char *str, int color);
extern void draw_color_info(int colr, const char *buf);
extern void menu_clear(void);
extern void set_scroll(char *s);
extern void set_autorepeat(char *s);
extern int get_info_width(void);
/* inventory.c */
extern gboolean list_selection_func(GtkTreeSelection *selection, GtkTreeModel *model, GtkTreePath *path, gboolean path_currently_selected, gpointer userdata);
extern void list_row_collapse(GtkTreeView *treeview, GtkTreeIter *iter, GtkTreePath *path, gpointer user_data);
extern void inventory_init(GtkWidget *window_root);
extern void set_show_icon(char *s);
extern void set_show_weight(char *s);
extern void close_container(item *op);
extern void open_container(item *op);
extern void command_show(char *params);
extern void set_weight_limit(uint32 wlim);
extern void get_row_color(item *it, int *fg, int *bg);
extern void item_event_item_deleting(item *it);
extern void item_event_container_clearing(item *container);
extern void item_event_item_changed(item *it);
extern void draw_look_list(void);
extern void draw_inv_list(int tab);
extern gboolean drawingarea_inventory_table_button_press_event(GtkWidget *widget, GdkEventButton *event, gpointer user_data);
extern gboolean drawingarea_inventory_table_expose_event(GtkWidget *widget, GdkEventExpose *event, gpointer user_data);
extern void draw_inv_table(int animate);
extern void draw_inv(int tab);
extern void draw_lists(void);
extern void on_notebook_switch_page(GtkNotebook *notebook, GtkNotebookPage *page, guint page_num, gpointer user_data);
extern gboolean on_inv_table_expose_event(GtkWidget *widget, GdkEventExpose *event, gpointer user_data);
extern void animate_inventory(void);
extern void inventory_tick(void);
/* keys.c */
extern void keys_init(GtkWidget *window_root);
extern void bind_key(char *params);
extern void unbind_key(char *params);
extern void keyrelfunc(GtkWidget *widget, GdkEventKey *event, GtkWidget *window);
extern void keyfunc(GtkWidget *widget, GdkEventKey *event, GtkWidget *window);
extern void draw_keybindings(GtkWidget *keylist);
extern void x_set_echo(void);
extern void draw_prompt(const char *str);
extern void gtk_command_history(int direction);
extern void gtk_complete_command(void);
extern void on_entry_commands_activate(GtkEntry *entry, gpointer user_data);
/* main.c */
extern char *get_metaserver(void);
extern int do_timeout(void);
extern void do_network(void);
extern void event_loop(void);
extern int parse_args(int argc, char **argv);
extern int main(int argc, char *argv[]);
extern void save_winpos(void);
extern void get_window_coord(GtkWidget *win, int *x, int *y, int *wx, int *wy, int *w, int *h);
/* map.c */
extern void map_init(GtkWidget *window_root);
extern void allocate_map(struct Map *new_map, int ax, int ay);
extern void reset_map(void);
extern void print_darkness(void);
extern void print_map(void);
extern void set_map_darkness(int x, int y, uint8 darkness);
extern void display_mapscroll(int dx, int dy);
extern void reset_map_data(void);
extern void drawsmooth(int mx, int my, int layer, int picx, int picy);
extern void gtk_draw_map(int redraw);
extern void display_map_newmap(void);
extern void resize_map_window(int x, int y);
extern void draw_splash(void);
extern void draw_map(int redraw);
extern gboolean on_drawingarea_map_expose_event(GtkWidget *widget, GdkEventExpose *event, gpointer user_data);
extern gboolean on_drawingarea_map_button_press_event(GtkWidget *widget, GdkEventButton *event, gpointer user_data);
extern void display_map_startupdate(void);
extern void display_map_doneupdate(int redraw);
/* magicmap.c */
extern void draw_magic_map(void);
extern void magic_map_flash_pos(void);
extern gboolean on_drawingarea_magic_map_expose_event(GtkWidget *widget, GdkEventExpose *event, gpointer user_data);
/* menubar.c */
extern void menu_quit_program(GtkMenuItem *menuitem, gpointer user_data);
extern void menu_quit_character(GtkMenuItem *menuitem, gpointer user_data);
extern void menu_about(GtkMenuItem *menuitem, gpointer user_data);
/* opengl.c */
extern void init_opengl(GtkWidget *drawingarea);
extern void opengl_gen_map(int redraw);
extern void create_opengl_map_image(uint8 *data, PixmapInfo *pi);
extern void opengl_free_pixmap(PixmapInfo *pi);
extern void create_opengl_question_mark(void);
/* pickup.c */
extern void on_menu_dont_pickup_activate(GtkMenuItem *menuitem, gpointer user_data);
extern void on_menu_stop_before_pickup_activate(GtkMenuItem *menuitem, gpointer user_data);
extern void on_menu_body_armor_activate(GtkMenuItem *menuitem, gpointer user_data);
extern void on_menu_boots_activate(GtkMenuItem *menuitem, gpointer user_data);
extern void on_menu_cloaks_activate(GtkMenuItem *menuitem, gpointer user_data);
extern void on_menu_gloves_activate(GtkMenuItem *menuitem, gpointer user_data);
extern void on_menu_helmets_activate(GtkMenuItem *menuitem, gpointer user_data);
extern void on_menu_shields_activate(GtkMenuItem *menuitem, gpointer user_data);
extern void on_menu_skillscrolls_activate(GtkMenuItem *menuitem, gpointer user_data);
extern void on_menu_normal_book_scrolls_activate(GtkMenuItem *menuitem, gpointer user_data);
extern void on_menu_spellbooks_activate(GtkMenuItem *menuitem, gpointer user_data);
extern void on_menu_drinks_activate(GtkMenuItem *menuitem, gpointer user_data);
extern void on_menu_food_activate(GtkMenuItem *menuitem, gpointer user_data);
extern void on_menu_keys_activate(GtkMenuItem *menuitem, gpointer user_data);
extern void on_menu_magical_items_activate(GtkMenuItem *menuitem, gpointer user_data);
extern void on_menu_potions_activate(GtkMenuItem *menuitem, gpointer user_data);
extern void on_menu_valuables_activate(GtkMenuItem *menuitem, gpointer user_data);
extern void on_menu_wands_rods_horns_activate(GtkMenuItem *menuitem, gpointer user_data);
extern void on_menu_all_weapons_activate(GtkMenuItem *menuitem, gpointer user_data);
extern void on_menu_missile_weapons_activate(GtkMenuItem *menuitem, gpointer user_data);
extern void on_menu_bows_activate(GtkMenuItem *menuitem, gpointer user_data);
extern void on_menu_arrows_activate(GtkMenuItem *menuitem, gpointer user_data);
extern void on_menu_ratio_pickup_off_activate(GtkMenuItem *menuitem, gpointer user_data);
extern void on_menu_ratio_5_activate(GtkMenuItem *menuitem, gpointer user_data);
extern void on_menu_ratio_10_activate(GtkMenuItem *menuitem, gpointer user_data);
extern void on_menu_ratio_15_activate(GtkMenuItem *menuitem, gpointer user_data);
extern void on_menu_ratio_20_activate(GtkMenuItem *menuitem, gpointer user_data);
extern void on_menu_ratio_25_activate(GtkMenuItem *menuitem, gpointer user_data);
extern void on_menu_ratio_30_activate(GtkMenuItem *menuitem, gpointer user_data);
extern void on_menu_ratio_35_activate(GtkMenuItem *menuitem, gpointer user_data);
extern void on_menu_ratio_40_activate(GtkMenuItem *menuitem, gpointer user_data);
extern void on_menu_ratio_45_activate(GtkMenuItem *menuitem, gpointer user_data);
extern void on_menu_ratio_50_activate(GtkMenuItem *menuitem, gpointer user_data);
/* png.c */
extern uint8 *png_to_data(uint8 *data, int len, uint32 *width, uint32 *height);
extern uint8 *rescale_rgba_data(uint8 *data, int *width, int *height, int scale);
extern int rgba_to_gdkpixmap(GdkWindow *window, uint8 *data, int width, int height, GdkPixmap **pix, GdkBitmap **mask, GdkColormap *colormap);
extern int rgba_to_gdkpixbuf(uint8 *data, int width, int height, GdkPixbuf **pix);
extern int png_to_gdkpixmap(GdkWindow *window, uint8 *data, int len, GdkPixmap **pix, GdkBitmap **mask, GdkColormap *colormap);
/* sdl.c */
extern void init_SDL(GtkWidget *sdl_window, int just_lightmap);
extern void drawquarterlightmap_sdl(int tl, int tr, int bl, int br, int width, int height, int startx, int starty, int endx, int endy, int destx, int desty);
extern void do_sdl_per_pixel_lighting(int x, int y, int mx, int my);
extern int sdl_square_need_redraw(int mx, int my);
extern void sdl_gen_map(int redraw);
extern void sdl_mapscroll(int dx, int dy);
/* sound.c */
extern void signal_pipe(int i);
extern int init_sounds(void);
extern void SoundCmd(unsigned char *data, int len);
/* stats.c */
extern void stats_init(GtkWidget *window_root);
extern void update_stat(int stat_no, int max_stat, int current_stat, const char *name);
extern void draw_message_window(int redraw);
extern void draw_stats(int redraw);
extern void clear_stat_mapping(void);
