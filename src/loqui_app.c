/* -*- Mode: C; tab-width: 8; indent-tabs-mode: t; c-basic-offset: 8 -*- */
/*
 * Loqui -- IRC client for Gtk2 <http://loqui.good-day.net/>
 * Copyright (C) 2002-2003 Yoichi Imai <yoichi@silver-forest.com>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.
 */
#include "config.h"

#include "loqui_app.h"
#include "channel_tree.h"
#include "utils.h"
#include "nick_list.h"
#include "account_manager.h"
#include "prefs_general.h"
#include "loqui_channelbar.h"
#include "loqui_statusbar.h"
#include "gtkutils.h"
#include "remark_entry.h"
#include "command_dialog.h"
#include "loqui_profile_account.h"
#include "account.h"
#include "loqui_actions.h"
#include "buffer_menu.h"

#include "embedtxt/loqui_app_ui.h"

#include "intl.h"
#include "utils.h"

#include <string.h>

#include <egg-toggle-action.h>

struct _LoquiAppPrivate
{
	GtkWidget *common_textview;
	GtkWidget *handlebox_channelbar;

	GtkWidget *buffers_menu;

	guint channel_buffer_inserted_signal_id;
	guint common_buffer_inserted_signal_id;
};

static GtkWindowClass *parent_class = NULL;
#define PARENT_TYPE GTK_TYPE_WINDOW

static void loqui_app_class_init(LoquiAppClass *klass);
static void loqui_app_init(LoquiApp *app);
static void loqui_app_finalize(GObject *object);
static void loqui_app_destroy(GtkObject *object);

static gint loqui_app_delete_event_cb(GtkWidget *widget, GdkEventAny *event);

static void loqui_app_restore_size(LoquiApp *app);
static void loqui_app_save_size(LoquiApp *app);
static void loqui_app_entry_activate_cb(GtkWidget *widget, gpointer data);
static void loqui_app_entry_toggle_command_toggled_cb(GtkWidget *widget, gpointer data);

static void loqui_app_menu_merge_add_widget_cb(EggMenuMerge *merge, GtkWidget *widget, GtkBox *box);

static void loqui_app_channel_textview_inserted_cb(GtkTextBuffer *textbuf,
						   GtkTextIter *pos,
						   const gchar *text,
						   gint length,
						   gpointer data);
static void loqui_app_common_textview_inserted_cb(GtkTextBuffer *textbuf,
						  GtkTextIter *pos,
						  const gchar *text,
						  gint length,
						  gpointer data);

static void loqui_app_textview_scroll_value_changed_cb(GtkAdjustment *adj, gpointer data);
static void loqui_app_statusbar_nick_clicked_cb(GtkWidget *widget, gpointer data);
static void loqui_app_statusbar_nick_selected_cb(GtkWidget *widget, gchar *nick, gpointer data);
static void loqui_app_statusbar_away_selected_cb(GtkWidget *widget, AwayState away_state, gpointer data);

static void loqui_app_menu_account_activate_cb(GtkWidget *widget, gpointer data);
static void loqui_app_menu_channel_activate_cb(GtkWidget *widget, gpointer data);

/* utilities */
static void scroll_channel_buffer(GtkWidget *textview);
static void set_channel_buffer(LoquiApp *app, GtkWidget *textview, ChannelBuffer *buffer, guint *signal_id_ptr,
			       GCallback func);

GType
loqui_app_get_type(void)
{
	static GType type = 0;
	if (type == 0) {
		static const GTypeInfo our_info =
			{
				sizeof(LoquiAppClass),
				NULL,           /* base_init */
				NULL,           /* base_finalize */
				(GClassInitFunc) loqui_app_class_init,
				NULL,           /* class_finalize */
				NULL,           /* class_data */
				sizeof(LoquiApp),
				0,              /* n_preallocs */
				(GInstanceInitFunc) loqui_app_init
			};
		
		type = g_type_register_static(PARENT_TYPE,
					      "LoquiApp",
					      &our_info,
					      0);
	}
	
	return type;
}
static void
loqui_app_class_init (LoquiAppClass *klass)
{
        GObjectClass *object_class = G_OBJECT_CLASS(klass);
        GtkObjectClass *gtk_object_class = GTK_OBJECT_CLASS(klass);

        parent_class = g_type_class_peek_parent(klass);
	
        object_class->finalize = loqui_app_finalize;
	gtk_object_class->destroy = loqui_app_destroy;
}
static void 
loqui_app_init (LoquiApp *app)
{
	LoquiAppPrivate *priv;

	priv = g_new0(LoquiAppPrivate, 1);
	app->priv = priv;

	priv->channel_buffer_inserted_signal_id = 0;
	priv->common_buffer_inserted_signal_id = 0;
}
static void
loqui_app_destroy(GtkObject *object)
{
	LoquiApp *app;

        g_return_if_fail(object != NULL);
        g_return_if_fail(LOQUI_IS_APP(object));

	app = LOQUI_APP(object);
		
	if (GTK_OBJECT_CLASS(parent_class)->destroy)
                (* GTK_OBJECT_CLASS(parent_class)->destroy) (object);

}	
static void 
loqui_app_finalize(GObject *object)
{
	LoquiApp *app;

        g_return_if_fail (object != NULL);
        g_return_if_fail (LOQUI_IS_APP(object));

        app = LOQUI_APP(object);

	if (G_OBJECT_CLASS(parent_class)->finalize)
                (* G_OBJECT_CLASS(parent_class)->finalize) (object);

	g_free(app->priv);
}

static gint
loqui_app_delete_event_cb(GtkWidget *widget, GdkEventAny *event)
{
	account_manager_disconnect_all(account_manager_get());

        if (prefs_general.save_size) {
		loqui_app_save_size(LOQUI_APP(widget));
        }
	gtk_widget_destroy(widget);
	gtk_main_quit();

	return TRUE;
}
static void loqui_app_save_size(LoquiApp *app)
{
	LoquiAppPrivate *priv;

	gint height;
	gint width;

        g_return_if_fail(app != NULL);
        g_return_if_fail(LOQUI_IS_APP(app));

	priv = app->priv;

	gtk_window_get_size(GTK_WINDOW(app), &width, &height);
	prefs_general.window_width = width;
	prefs_general.window_height = height;
	
	prefs_general.common_buffer_height = priv->common_textview->allocation.height;

	prefs_general.channel_tree_height = GTK_WIDGET(app->channel_tree)->allocation.height;
	prefs_general.channel_tree_width = GTK_WIDGET(app->channel_tree)->allocation.width;
}
static void loqui_app_restore_size(LoquiApp *app)
{
	LoquiAppPrivate *priv;

        g_return_if_fail(app != NULL);
        g_return_if_fail(LOQUI_IS_APP(app));

	priv = app->priv;
        gtk_window_set_default_size(GTK_WINDOW(app),
				    prefs_general.window_width,
				    prefs_general.window_height);

	gtk_widget_set_usize(priv->common_textview, -1,
			     prefs_general.common_buffer_height);

	gtk_widget_set_usize(GTK_WIDGET(app->channel_tree),
			     prefs_general.channel_tree_width,
			     prefs_general.channel_tree_height);
}

static void
loqui_app_entry_activate_cb(GtkWidget *widget, gpointer data)
{
	Account *account;
	RemarkEntry *remark_entry;
	const gchar *str;

	remark_entry = REMARK_ENTRY(widget);
	
	str = remark_entry_get_text(remark_entry);
	if (str == NULL || strlen(str) == 0)
		return;
	
	account = account_manager_get_current_account(account_manager_get());
	if (account)
		account_speak(account, account_manager_get_current_channel(account_manager_get()), str,
			      remark_entry_get_command_mode(remark_entry));
	else
		gtkutils_msgbox_info(GTK_MESSAGE_ERROR, _("No accounts are selected!"));

	remark_entry_clear_text(remark_entry);
}
static void
loqui_app_entry_toggle_command_toggled_cb(GtkWidget *widget, gpointer data)
{
	LoquiApp *app;
	EggAction *action;

	g_return_if_fail(data != NULL);
	g_return_if_fail(LOQUI_IS_APP(data));

	app = LOQUI_APP(data);

	action = egg_action_group_get_action(app->action_group, "ToggleCommandMode");
	egg_toggle_action_set_active(EGG_TOGGLE_ACTION(action), remark_entry_get_command_mode(REMARK_ENTRY(app->remark_entry)));
}
static void
loqui_app_statusbar_nick_clicked_cb(GtkWidget *widget, gpointer data)
{
	Account *account;
	LoquiApp *app;

	g_return_if_fail(data != NULL);
	g_return_if_fail(LOQUI_IS_APP(data));

	app = LOQUI_APP(data);
	account = account_manager_get_current_account(account_manager_get());
	if (account)
		command_dialog_nick(GTK_WINDOW(app), account);
	else
		gtkutils_msgbox_info(GTK_MESSAGE_ERROR, _("Not selected an account!"));
}
static void
loqui_app_statusbar_nick_selected_cb(GtkWidget *widget, gchar *nick, gpointer data)
{
	Account *account;
	LoquiApp *app;

	g_return_if_fail(data != NULL);
	g_return_if_fail(LOQUI_IS_APP(data));

	app = LOQUI_APP(data);
	account = account_manager_get_current_account(account_manager_get());
	if(account)
		account_change_nick(account, nick);
	else
		gtkutils_msgbox_info(GTK_MESSAGE_ERROR, _("Not selected an account!"));	
}
static void
loqui_app_statusbar_away_selected_cb(GtkWidget *widget, AwayState away_state, gpointer data)
{
	Account *account;
	LoquiApp *app;

	g_return_if_fail(data != NULL);
	g_return_if_fail(LOQUI_IS_APP(data));

	app = LOQUI_APP(data);
	account = account_manager_get_current_account(account_manager_get());
	if(account) {
		if(account_is_connected(account)) {
			switch(away_state) {
			case AWAY_STATE_ONLINE:
				account_set_away(account, FALSE);
				break;
			case AWAY_STATE_AWAY:
				account_set_away(account, TRUE);
				break;
			default:
				break;
			}
		} else {
			gtkutils_msgbox_info(GTK_MESSAGE_ERROR, _("The account is not connected!"));
		}
	} else {
		gtkutils_msgbox_info(GTK_MESSAGE_ERROR, _("Not selected an account!"));	
	}
}

static void loqui_app_channel_textview_inserted_cb(GtkTextBuffer *textbuf,
						   GtkTextIter *pos,
						   const gchar *text,
						   gint length,
						   gpointer data)
{
	LoquiApp *app;

	app = LOQUI_APP(data);

	if(!account_manager_get_whether_scrolling(account_manager_get()))
		return;

	loqui_app_scroll_channel_buffer(app);
}
static void loqui_app_common_textview_inserted_cb(GtkTextBuffer *textbuf,
						  GtkTextIter *pos,
						  const gchar *text,
						  gint length,
						  gpointer data)
{
	LoquiApp *app;

	app = LOQUI_APP(data);

	loqui_app_scroll_common_buffer(app);
}
static void loqui_app_textview_scroll_value_changed_cb(GtkAdjustment *adj, gpointer data)
{
	gboolean reached_to_end;
	AccountManager *manager;
	Channel *channel;

	manager = account_manager_get();

	if(!prefs_general.auto_switch_scrolling)
		return;

	/* upper - page_size is max virtually. */
	reached_to_end = (ABS(adj->upper - adj->page_size - adj->value) < adj->step_increment);

	if(reached_to_end && !account_manager_get_whether_scrolling(manager)) {
		account_manager_set_whether_scrolling(manager, TRUE);
	} else if(!reached_to_end && account_manager_get_whether_scrolling(manager)) {
		account_manager_set_whether_scrolling(manager, FALSE);
	}

	channel = account_manager_get_current_channel(manager);
	if(channel && reached_to_end && channel_get_updated(channel))
		channel_set_updated(channel, FALSE);
}

void
loqui_app_update_info(LoquiApp *app, 
		      gboolean is_account_changed, Account *account,
		      gboolean is_channel_changed, Channel *channel)

{
	LoquiAppPrivate *priv;
	gchar *buf;
	
	const gchar *account_name = NULL, *channel_name = NULL, *topic = NULL;
	guint user_number, op_number;
	gchar *channel_mode = NULL, *user_number_str = NULL, *op_number_str = NULL;

        g_return_if_fail(app != NULL);
        g_return_if_fail(LOQUI_IS_APP(app));

	priv = app->priv;

	if(account) {
		account_name = loqui_profile_account_get_name(account_get_profile(account));
	}
	if(channel) {
		channel_name = channel_get_name(channel);
		topic = channel_get_topic(channel);
		
		if(!channel_is_private_talk(channel)) {
			channel_mode = channel_get_mode(channel);			
			channel_get_user_number(channel, &user_number, &op_number);
			user_number_str = g_strdup_printf("%d", user_number);
			op_number_str = g_strdup_printf("%d", op_number);
		}
	}
	
	if(is_account_changed) {
		loqui_statusbar_set_current_account(LOQUI_STATUSBAR(app->statusbar), account);
	}

	if(is_channel_changed) {
		loqui_statusbar_set_current_channel(LOQUI_STATUSBAR(app->statusbar), channel);
	}
	
	if(!channel) {
		loqui_channelbar_set_current_account(LOQUI_CHANNELBAR(app->channelbar), account);
	} else if (is_channel_changed) {
		loqui_channelbar_set_current_channel(LOQUI_CHANNELBAR(app->channelbar), channel);
	}

#define FORMAT_INFO(format) \
 utils_format(format, 'c', channel_name, 'a', account_name, \
	              't', topic, 'm', channel_mode, \
                      'u', user_number_str, 'o', op_number_str, 'v', VERSION, -1);

	buf = FORMAT_INFO("%c?c{ @ }%a?a{ - }Loqui version %v");

	gtk_window_set_title(GTK_WINDOW(app), buf);
	g_free(buf);
	
	G_FREE_UNLESS_NULL(channel_mode);
	G_FREE_UNLESS_NULL(user_number_str);
	G_FREE_UNLESS_NULL(op_number_str);
}
static void
loqui_app_menu_merge_add_widget_cb(EggMenuMerge *merge, GtkWidget *widget, GtkBox *box)
{
        gtk_box_pack_start(box, widget, FALSE, FALSE, 0);
        gtk_widget_show(widget);
}

GtkWidget*
loqui_app_new(void)
{
	LoquiApp *app;
	LoquiAppPrivate *priv;
	GError *error = NULL;

	GtkWidget *channel_tree;
	GtkWidget *nick_list;

	GtkWidget *vbox;
	GtkWidget *hpaned;
	GtkWidget *vpaned;
	GtkWidget *scrolled_win;
	
	GtkWidget *menu_box;

	app = g_object_new(loqui_app_get_type(), NULL);
	priv = app->priv;

	g_signal_connect(G_OBJECT(app), "delete_event",
			 G_CALLBACK(loqui_app_delete_event_cb), NULL);

	gtk_window_set_policy(GTK_WINDOW (app), TRUE, TRUE, TRUE);

	vbox = gtk_vbox_new(FALSE, 0);
	gtk_container_add(GTK_CONTAINER(app), vbox);

	menu_box = gtk_vbox_new(FALSE, 0);
	gtk_box_pack_start(GTK_BOX(vbox), menu_box, FALSE, FALSE, 0);

	app->accel_group = gtk_accel_group_new();
	app->action_group = loqui_actions_create_group(app, app->accel_group);

	app->menu_merge = egg_menu_merge_new();
	egg_menu_merge_set_accel_group(app->menu_merge, app->accel_group);
	egg_menu_merge_insert_action_group(app->menu_merge, app->action_group, 0);

	g_signal_connect(app->menu_merge, "add_widget",
			 G_CALLBACK(loqui_app_menu_merge_add_widget_cb), menu_box);
	if(!egg_menu_merge_add_ui_from_string(app->menu_merge, embedtxt_loqui_app_ui, -1, &error))
		g_error("Failed to load UI XML: %s", error->message);

	egg_menu_merge_ensure_update(app->menu_merge);
	gtk_window_add_accel_group(GTK_WINDOW(app), app->accel_group);
	
	priv->handlebox_channelbar = gtk_handle_box_new();
	gtk_box_pack_start(GTK_BOX(vbox), priv->handlebox_channelbar, FALSE, FALSE, 0);

	app->channelbar = loqui_channelbar_new();
	gtk_container_add(GTK_CONTAINER(priv->handlebox_channelbar), app->channelbar);

#define SET_SCROLLED_WINDOW(s, w, vpolicy, hpolicy) \
{ \
	s = gtk_scrolled_window_new(NULL, NULL); \
        gtk_scrolled_window_set_policy(GTK_SCROLLED_WINDOW(s), vpolicy, hpolicy); \
	gtk_container_add(GTK_CONTAINER(s), w); \
}

	hpaned = gtk_hpaned_new();
	gtk_box_pack_start_defaults(GTK_BOX(vbox), hpaned);

	app->statusbar = loqui_statusbar_new(app);
	gtk_box_pack_start(GTK_BOX(vbox), app->statusbar, FALSE, FALSE, 1);	
	
	/* left side */
	vpaned = gtk_vpaned_new();
	gtk_paned_pack1(GTK_PANED(hpaned), vpaned, TRUE, TRUE);
	
	vbox = gtk_vbox_new(FALSE, 0);
	gtk_paned_pack1(GTK_PANED(vpaned), vbox, TRUE, TRUE);

	app->channel_textview = gtk_text_view_new();
	gtk_text_view_set_editable(GTK_TEXT_VIEW(app->channel_textview), FALSE);
	gtk_text_view_set_wrap_mode(GTK_TEXT_VIEW(app->channel_textview), GTK_WRAP_CHAR);
	SET_SCROLLED_WINDOW(scrolled_win, app->channel_textview, 
			    GTK_POLICY_NEVER, GTK_POLICY_ALWAYS);
	gtk_box_pack_start_defaults(GTK_BOX(vbox), scrolled_win);
	g_signal_connect(G_OBJECT(GTK_TEXT_VIEW(app->channel_textview)->vadjustment), "value-changed",
			 G_CALLBACK(loqui_app_textview_scroll_value_changed_cb), app);

	app->remark_entry = remark_entry_new(app);
	gtk_box_pack_end(GTK_BOX(vbox), app->remark_entry, FALSE, FALSE, 0);
	g_signal_connect(G_OBJECT(app->remark_entry), "activate",
			 G_CALLBACK(loqui_app_entry_activate_cb), NULL);
	g_signal_connect(G_OBJECT(app->statusbar), "nick-change",
			 G_CALLBACK(loqui_app_statusbar_nick_clicked_cb), app);
	g_signal_connect(G_OBJECT(app->statusbar), "nick-selected",
			 G_CALLBACK(loqui_app_statusbar_nick_selected_cb), app);
	g_signal_connect(G_OBJECT(app->statusbar), "away-selected",
			 G_CALLBACK(loqui_app_statusbar_away_selected_cb), app);

	priv->common_textview = gtk_text_view_new();
	gtk_text_view_set_editable(GTK_TEXT_VIEW(priv->common_textview), FALSE);
	gtk_text_view_set_wrap_mode(GTK_TEXT_VIEW(priv->common_textview), GTK_WRAP_CHAR);
	SET_SCROLLED_WINDOW(scrolled_win, priv->common_textview, 
			    GTK_POLICY_NEVER, GTK_POLICY_ALWAYS);
	gtk_paned_pack2(GTK_PANED(vpaned), scrolled_win, FALSE, TRUE);

	/* right side */
	vpaned = gtk_vpaned_new();
	gtk_paned_pack2(GTK_PANED(hpaned), vpaned, FALSE, TRUE);
	
	nick_list = nick_list_new();
	SET_SCROLLED_WINDOW(scrolled_win, nick_list, GTK_POLICY_AUTOMATIC, GTK_POLICY_AUTOMATIC);
	gtk_paned_pack1(GTK_PANED(vpaned), scrolled_win, TRUE, TRUE);	

	channel_tree = channel_tree_new();
	SET_SCROLLED_WINDOW(scrolled_win, channel_tree, GTK_POLICY_AUTOMATIC, GTK_POLICY_AUTOMATIC); 
	gtk_paned_pack2(GTK_PANED(vpaned), scrolled_win, FALSE, TRUE);
	
	app->channel_tree = CHANNEL_TREE(channel_tree);
	app->nick_list = NICK_LIST(nick_list);

	g_signal_connect(G_OBJECT(app->remark_entry), "toggle_command_mode",
			 G_CALLBACK(loqui_app_entry_toggle_command_toggled_cb), app);
	
	loqui_app_update_info(app, TRUE, NULL, TRUE, NULL);
	loqui_app_restore_size(app);

#undef SET_SCROLLED_WINDOW

	gtk_widget_show_all(GTK_WIDGET(app));

	loqui_app_set_show_statusbar(app, prefs_general.show_statusbar);
	loqui_app_set_show_channelbar(app, prefs_general.show_channelbar);

	ACTION_GROUP_ACTION_SET_SENSITIVE(app->action_group, "ToggleStatusbar", prefs_general.show_statusbar);
	ACTION_GROUP_ACTION_SET_SENSITIVE(app->action_group, "ToggleChannelbar", prefs_general.show_channelbar);
	
	loqui_app_set_focus(app);

	return GTK_WIDGET(app);
}
void loqui_app_set_focus(LoquiApp *app)
{
        g_return_if_fail(app != NULL);
        g_return_if_fail(LOQUI_IS_APP(app));

	remark_entry_grab_focus(REMARK_ENTRY(app->remark_entry));
}
static void
scroll_channel_buffer(GtkWidget *textview)
{
	GtkTextBuffer *buffer = NULL;

	buffer = gtk_text_view_get_buffer(GTK_TEXT_VIEW(textview));
	if(buffer && IS_CHANNEL_BUFFER(buffer))
		gtk_text_view_scroll_mark_onscreen(GTK_TEXT_VIEW(textview),
						   gtk_text_buffer_get_mark(buffer, "end"));
}
void loqui_app_scroll_channel_buffer(LoquiApp *app)
{
        g_return_if_fail(app != NULL);
        g_return_if_fail(LOQUI_IS_APP(app));

	scroll_channel_buffer(app->channel_textview);
}
void loqui_app_scroll_common_buffer(LoquiApp *app)
{
        g_return_if_fail(app != NULL);
        g_return_if_fail(LOQUI_IS_APP(app));

	scroll_channel_buffer(app->priv->common_textview);
}
void
loqui_app_scroll_page_channel_buffer(LoquiApp *app, gint pages)
{
        g_return_if_fail(app != NULL);
        g_return_if_fail(LOQUI_IS_APP(app));

	g_signal_emit_by_name(app->channel_textview, "move_cursor", GTK_MOVEMENT_PAGES, pages, FALSE);
}
void
loqui_app_scroll_page_common_buffer(LoquiApp *app, gint pages)
{
        g_return_if_fail(app != NULL);
        g_return_if_fail(LOQUI_IS_APP(app));

	g_signal_emit_by_name(app->priv->common_textview, "move_cursor", GTK_MOVEMENT_PAGES, pages, FALSE);
}

static void
set_channel_buffer(LoquiApp *app, GtkWidget *textview, ChannelBuffer *buffer, guint *signal_id_ptr,
		   GCallback func)
{
	GtkTextBuffer *old_buf;
	GtkStyle *style;
	GdkColor *transparent_color;
	GtkTextTag *transparent_tag;

	if(*signal_id_ptr > 0) {
		old_buf = gtk_text_view_get_buffer(GTK_TEXT_VIEW(textview));
		g_signal_handler_disconnect(old_buf, *signal_id_ptr);
	}

	style = gtk_widget_get_style(textview);
	transparent_color = &style->base[GTK_STATE_NORMAL];
	transparent_tag = gtk_text_tag_table_lookup(gtk_text_buffer_get_tag_table(GTK_TEXT_BUFFER(buffer)),
						    "transparent");
	if(transparent_tag)
		g_object_set(G_OBJECT(transparent_tag), "foreground-gdk", transparent_color, NULL);
	gtk_text_view_set_buffer(GTK_TEXT_VIEW(textview), GTK_TEXT_BUFFER(buffer));
	*signal_id_ptr = g_signal_connect(G_OBJECT(buffer), "insert-text", func, app);
	scroll_channel_buffer(textview);
}
void loqui_app_set_channel_buffer(LoquiApp *app, ChannelBuffer *buffer)
{
        g_return_if_fail(app != NULL);
        g_return_if_fail(LOQUI_IS_APP(app));

	set_channel_buffer(app, app->channel_textview, buffer, &app->priv->channel_buffer_inserted_signal_id,
			   G_CALLBACK(loqui_app_channel_textview_inserted_cb));
}
ChannelBuffer *loqui_app_get_channel_buffer(LoquiApp *app)
{
	GtkTextBuffer *buffer;

        g_return_val_if_fail(app != NULL, NULL);
        g_return_val_if_fail(LOQUI_IS_APP(app), NULL);
	
	buffer = gtk_text_view_get_buffer(GTK_TEXT_VIEW(app->channel_textview));
	if(buffer == NULL || !IS_CHANNEL_BUFFER(buffer))
		return NULL;

	return CHANNEL_BUFFER(buffer);
}
void loqui_app_set_common_buffer(LoquiApp *app, ChannelBuffer *buffer)
{
        g_return_if_fail(app != NULL);
        g_return_if_fail(LOQUI_IS_APP(app));

	set_channel_buffer(app, app->priv->common_textview, buffer, &app->priv->common_buffer_inserted_signal_id,
			   G_CALLBACK(loqui_app_common_textview_inserted_cb));
}
void
loqui_app_set_show_statusbar(LoquiApp *app, gboolean show)
{
	LoquiAppPrivate *priv;

        g_return_if_fail(app != NULL);
        g_return_if_fail(LOQUI_IS_APP(app));

	priv = app->priv;

	if(show)
		gtk_widget_show(app->statusbar);
	else
		gtk_widget_hide(app->statusbar);

	prefs_general.show_statusbar = show;
}

void
loqui_app_set_show_channelbar(LoquiApp *app, gboolean show)
{
	LoquiAppPrivate *priv;

        g_return_if_fail(app != NULL);
        g_return_if_fail(LOQUI_IS_APP(app));

	priv = app->priv;

	if(show)
		gtk_widget_show(priv->handlebox_channelbar);
	else
		gtk_widget_hide(priv->handlebox_channelbar);

	prefs_general.show_channelbar = show;
}

void
loqui_app_get_current_widget_editing_status(LoquiApp *app, gboolean *cutable, gboolean *copiable, gboolean *pastable,
					    gboolean *clearable, gboolean *findable)
{
	LoquiAppPrivate *priv;
	GtkWidget *widget;
	GtkTextBuffer *buffer;
	gboolean editable, selected;

        g_return_if_fail(app != NULL);
        g_return_if_fail(LOQUI_IS_APP(app));
	g_return_if_fail(cutable != NULL);
	g_return_if_fail(copiable != NULL);
	g_return_if_fail(pastable != NULL);
	g_return_if_fail(findable != NULL);
	g_return_if_fail(clearable != NULL);

	priv = app->priv;

	widget = gtk_window_get_focus(GTK_WINDOW(app));

	if(GTK_IS_ENTRY(widget)) {
		editable = TRUE;		
		selected = gtk_editable_get_selection_bounds(GTK_EDITABLE(widget), NULL, NULL);

		*cutable = editable && selected;
		*copiable = selected;
		*pastable = editable;

		*findable = FALSE;
		*clearable = TRUE;
	} else if(GTK_IS_TEXT_VIEW(widget)) {
		editable = gtk_text_view_get_editable(GTK_TEXT_VIEW(widget));
		buffer = gtk_text_view_get_buffer(GTK_TEXT_VIEW(widget));
		selected = gtk_text_buffer_get_selection_bounds(buffer, NULL, NULL);

		*cutable = editable && selected;
		*copiable = selected;
		*pastable = editable;

		*findable = TRUE;
		*clearable = TRUE;
	} else {
		*cutable = FALSE;
		*copiable = FALSE;
		*pastable = FALSE;
		*findable = FALSE;
		*clearable = FALSE;
	}
}

void
loqui_app_current_widget_cut(LoquiApp *app)
{
	GtkWidget *widget;
	GtkTextBuffer *buffer;

        g_return_if_fail(app != NULL);
        g_return_if_fail(LOQUI_IS_APP(app));

	widget = gtk_window_get_focus(GTK_WINDOW(app));

	if(GTK_IS_ENTRY(widget)) {
		gtk_editable_cut_clipboard(GTK_EDITABLE(widget));
	} else if(GTK_IS_TEXT_VIEW(widget)) {
		buffer = gtk_text_view_get_buffer(GTK_TEXT_VIEW(widget));
		gtk_text_buffer_cut_clipboard(buffer,
					      gtk_clipboard_get(GDK_NONE),
					      FALSE);
	}
}

void
loqui_app_current_widget_copy(LoquiApp *app)
{
	GtkWidget *widget;
	GtkTextBuffer *buffer;

        g_return_if_fail(app != NULL);
        g_return_if_fail(LOQUI_IS_APP(app));

	widget = gtk_window_get_focus(GTK_WINDOW(app));

	if(GTK_IS_ENTRY(widget)) {
		gtk_editable_copy_clipboard(GTK_EDITABLE(widget));
	} else if(GTK_IS_TEXT_VIEW(widget)) {
		buffer = gtk_text_view_get_buffer(GTK_TEXT_VIEW(widget));
		gtk_text_buffer_copy_clipboard(buffer,
					       gtk_clipboard_get(GDK_NONE));
	}
}

void
loqui_app_current_widget_paste(LoquiApp *app)
{
	GtkWidget *widget;
	GtkTextBuffer *buffer;

        g_return_if_fail(app != NULL);
        g_return_if_fail(LOQUI_IS_APP(app));

	widget = gtk_window_get_focus(GTK_WINDOW(app));

	if(GTK_IS_ENTRY(widget)) {
		gtk_editable_paste_clipboard(GTK_EDITABLE(widget));
	} else if(GTK_IS_TEXT_VIEW(widget)) {
		buffer = gtk_text_view_get_buffer(GTK_TEXT_VIEW(widget));
		gtk_text_buffer_paste_clipboard(buffer,
						gtk_clipboard_get(GDK_NONE),
						NULL, FALSE);
	}
}

void
loqui_app_current_widget_clear(LoquiApp *app)
{
	GtkWidget *widget;
	GtkTextBuffer *buffer;
	GtkTextIter start, end;

        g_return_if_fail(app != NULL);
        g_return_if_fail(LOQUI_IS_APP(app));

	widget = gtk_window_get_focus(GTK_WINDOW(app));

	if(GTK_IS_ENTRY(widget)) {
		gtk_editable_delete_text(GTK_EDITABLE(widget), 0, -1);
	} else if(GTK_IS_TEXT_VIEW(widget)) {
		buffer = gtk_text_view_get_buffer(GTK_TEXT_VIEW(widget));
		gtk_text_buffer_get_start_iter(buffer, &start);
		gtk_text_buffer_get_end_iter(buffer, &end);
		gtk_text_buffer_delete(buffer, &start, &end);
	}
}
void loqui_app_current_widget_find(LoquiApp *app)
{

}
void loqui_app_current_widget_find_next(LoquiApp *app)
{

}

static GtkWidget *
loqui_app_menu_get_buffers_menu(LoquiApp *app)
{
	GtkWidget *widget;

        g_return_val_if_fail(app != NULL, NULL);
        g_return_val_if_fail(LOQUI_IS_APP(app), NULL);
	
        widget = egg_menu_merge_get_widget(app->menu_merge, "/menu/Buffers");

	if (GTK_IS_MENU_ITEM(widget))
		return gtk_menu_item_get_submenu(GTK_MENU_ITEM(widget));
	if (GTK_IS_MENU_SHELL(widget))
		return widget;

	g_error("Can't get buffers menu");
	return NULL;
}

static void
loqui_app_menu_account_activate_cb(GtkWidget *widget, gpointer data)
{
	Account *account;

	g_return_if_fail(widget != NULL);

	account = g_object_get_data(G_OBJECT(widget), "account");
	g_return_if_fail(account != NULL);

	account_manager_set_current_account(account_manager_get(), account);
}
static void
loqui_app_menu_channel_activate_cb(GtkWidget *widget, gpointer data)
{	
	Channel *channel;

	g_return_if_fail(widget != NULL);

	channel = g_object_get_data(G_OBJECT(widget), "channel");
	g_return_if_fail(channel != NULL);

	account_manager_set_current_channel(account_manager_get(), channel);
}
void
loqui_app_menu_buffers_add_account(LoquiApp *app, Account *account)
{
	GtkWidget *menuitem;

	menuitem = buffer_menu_add_account(GTK_MENU_SHELL(loqui_app_menu_get_buffers_menu(app)),
					   account);
	g_signal_connect(G_OBJECT(menuitem), "activate",
			 G_CALLBACK(loqui_app_menu_account_activate_cb), menuitem);
	buffer_menu_update_accel_keys(GTK_MENU_SHELL(loqui_app_menu_get_buffers_menu(app)));
}
void
loqui_app_menu_buffers_update_account(LoquiApp *app, Account *account)
{
	buffer_menu_update_account(GTK_MENU_SHELL(loqui_app_menu_get_buffers_menu(app)),
				   account);
}

void
loqui_app_menu_buffers_remove_account(LoquiApp *app, Account *account)
{
	buffer_menu_remove_account(GTK_MENU_SHELL(loqui_app_menu_get_buffers_menu(app)),
				   account);
	buffer_menu_update_accel_keys(GTK_MENU_SHELL(loqui_app_menu_get_buffers_menu(app)));
}
void
loqui_app_menu_buffers_add_channel(LoquiApp *app, Channel *channel)
{
	GtkWidget *menuitem;

	menuitem = buffer_menu_add_channel(GTK_MENU_SHELL(loqui_app_menu_get_buffers_menu(app)), channel);
	g_signal_connect(G_OBJECT(menuitem), "activate",
			 G_CALLBACK(loqui_app_menu_channel_activate_cb), menuitem);
	buffer_menu_update_accel_keys(GTK_MENU_SHELL(loqui_app_menu_get_buffers_menu(app)));
}
void
loqui_app_menu_buffers_remove_channel(LoquiApp *app, Channel *channel)
{
	buffer_menu_remove_channel(GTK_MENU_SHELL(loqui_app_menu_get_buffers_menu(app)),
				   channel);
	buffer_menu_update_accel_keys(GTK_MENU_SHELL(loqui_app_menu_get_buffers_menu(app)));
}
void
loqui_app_menu_buffers_update_channel(LoquiApp *app, Channel *channel)
{
	buffer_menu_update_channel(GTK_MENU_SHELL(loqui_app_menu_get_buffers_menu(app)),
				   channel);
}
