
#ifndef __GTK24_EXT_H__
#define __GTK24_EXT_H__

#ifdef USE_GTK_2_2

#define GTK_NO_SHOW_ALL 4194304 /* 1 << 22 */

gboolean gtk_button_get_focus_on_click (GtkButton *button);
void gtk_button_set_focus_on_click(GtkButton *button, gboolean focus_on_click);

void gtk_widget_set_no_show_all(GtkWidget *widget, gboolean   no_show_all);
gboolean gtk_widget_get_no_show_all(GtkWidget *widget);

void gtk_widget_queue_resize_no_redraw(GtkWidget *widget);

void gtk_check_menu_item_set_draw_as_radio (GtkCheckMenuItem *check_menu_item,gboolean draw_as_radio);
gboolean gtk_check_menu_item_get_draw_as_radio (GtkCheckMenuItem *check_menu_item);

#endif /* USE_GTK_2_2 */

#endif /* __GTK24_EXT_H__ */
