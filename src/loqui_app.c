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
#include "loqui_app_actions.h"
#include "main.h"
#include "loqui_channel_entry.h"
#include "loqui_channel_entry_action.h"
#include "loqui_channel_entry_store.h"
#include "loqui_channel_entry_ui.h"
#include "loqui_channel_text_view.h"
#include "loqui_gtk.h"
#include "loqui_account_manager_store.h"
#include "loqui_member_sort_funcs.h"
#include "loqui_account_manager_iter.h"

#include "embedtxt/loqui_app_ui.h"
#include "icons/pixbufs.h"

#include "intl.h"
#include "utils.h"

#include <string.h>
#include <time.h>

#include <gtk24backports.h>
#include <gdk/gdkkeysyms.h>

#define CHANNEL_ENTRY_STORE_KEY "channel-entry-store"
#define CHANNEL_TEXT_VIEW_KEY "channel-text-view"

struct _LoquiAppPrivate
{
	GtkWidget *handlebox_channelbar;

	LoquiChannelEntry *current_chent;

	Account *current_account; /* read only */
	LoquiChannel *current_channel; /* read only */

	MessageText *last_msgtext;
	ChannelBuffer *common_buffer;

	gboolean is_pending_update_account_info;
	gboolean is_pending_update_channel_info;
	
	gboolean is_pending_set_current_channel;
	LoquiChannel *wait_current_channel;

	guint updated_channel_number;
	guint updated_private_talk_number;

	GCompareFunc sort_func;
};

static GtkWindowClass *parent_class = NULL;
#define PARENT_TYPE GTK_TYPE_WINDOW

static void loqui_app_class_init(LoquiAppClass *klass);
static void loqui_app_init(LoquiApp *app);
static void loqui_app_finalize(GObject *object);
static void loqui_app_destroy(GtkObject *object);
static gint loqui_app_delete_event(GtkWidget *widget, GdkEventAny *event);

static void loqui_app_restore_size(LoquiApp *app);
static void loqui_app_save_size(LoquiApp *app);

static void loqui_app_add_account_after_cb(AccountManager *manager, Account *account, LoquiApp *app);
static void loqui_app_remove_account_cb(AccountManager *manager, Account *account, LoquiApp *app);
static void loqui_app_remove_account_after_cb(AccountManager *manager, Account *account, LoquiApp *app);

static void loqui_app_add_channel_after_cb(Account *account, LoquiChannel *channel, LoquiApp *app);
static void loqui_app_remove_channel_cb(Account *account, LoquiChannel *channel, LoquiApp *app);
static void loqui_app_remove_channel_after_cb(Account *account, LoquiChannel *channel, LoquiApp *app);

static gboolean loqui_app_update_account_info(LoquiApp *app);
static gboolean loqui_app_update_channel_info(LoquiApp *app);

static void loqui_app_channel_text_view_scrolled_to_end_cb(LoquiChannelTextView *chview, LoquiChannelEntry *chent);
static void loqui_app_channel_text_view_notify_is_scroll_cb(LoquiChannelTextView *chview, GParamSpec *pspec, LoquiApp *app);
static void loqui_app_channel_text_view_notify_is_scroll_common_buffer_cb(LoquiChannelTextView *chview, GParamSpec *pspec, LoquiApp *app);

static void loqui_app_account_changed_cb(GObject *object, gpointer data);
static void loqui_app_channel_changed_cb(GObject *object, gpointer data);
static void loqui_app_channel_entry_notify_cb(LoquiChannelEntry *chent, GParamSpec *pspec, gpointer data);
static void loqui_app_channel_entry_notify_is_updated_cb(LoquiChannelEntry *chent, GParamSpec *pspec, LoquiApp *app);
static void loqui_app_channel_entry_notify_number_cb(LoquiChannelEntry *chent, GParamSpec *pspec, LoquiApp *app);

static void loqui_app_channel_buffer_append_cb(ChannelBuffer *buffer, MessageText *msgtext, LoquiApp *app);
static void loqui_app_append_log(LoquiApp *app, MessageText *msgtext);

static gboolean loqui_app_set_current_channel_for_idle(LoquiApp *app);
static void loqui_app_set_current_channel_lazy(LoquiApp *app, LoquiChannel *channel);
static void loqui_app_update_channel_entry_accel_key(LoquiApp *app);

/* utilities */
static void loqui_app_set_channel_entry_accel_key(LoquiApp *app, LoquiChannelEntry *chent);

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
	GtkWidgetClass *gtk_widget_class = GTK_WIDGET_CLASS(klass);

        parent_class = g_type_class_peek_parent(klass);
	
        object_class->finalize = loqui_app_finalize;
	gtk_object_class->destroy = loqui_app_destroy;
	gtk_widget_class->delete_event = loqui_app_delete_event;
}
static void 
loqui_app_init (LoquiApp *app)
{
	LoquiAppPrivate *priv;

	priv = g_new0(LoquiAppPrivate, 1);
	app->priv = priv;
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
	LoquiAppPrivate *priv;

        g_return_if_fail (object != NULL);
        g_return_if_fail (LOQUI_IS_APP(object));

        app = LOQUI_APP(object);
	priv = app->priv;

	if(priv->common_buffer) {
		g_object_unref(priv->common_buffer);
		priv->common_buffer = NULL;
	}

	g_signal_handlers_disconnect_by_func(G_OBJECT(app->account_manager), loqui_app_add_account_after_cb, app);
	g_signal_handlers_disconnect_by_func(G_OBJECT(app->account_manager), loqui_app_remove_account_cb, app);
	g_signal_handlers_disconnect_by_func(G_OBJECT(app->account_manager), loqui_app_remove_account_after_cb, app);

	G_OBJECT_UNREF_UNLESS_NULL(app->account_manager);

	if (G_OBJECT_CLASS(parent_class)->finalize)
                (* G_OBJECT_CLASS(parent_class)->finalize) (object);

	g_free(app->priv);
}

static gint
loqui_app_delete_event(GtkWidget *widget, GdkEventAny *event)
{
	LoquiApp *app;

        g_return_val_if_fail(widget != NULL, FALSE);
        g_return_val_if_fail(LOQUI_IS_APP(widget), FALSE);

	app = LOQUI_APP(widget);

	account_manager_disconnect_all(loqui_app_get_account_manager(app));

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
	
	prefs_general.common_buffer_height = app->common_textview->allocation.height;

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

	gtk_widget_set_usize(app->common_textview, -1,
			     prefs_general.common_buffer_height);

	gtk_widget_set_usize(GTK_WIDGET(app->channel_tree),
			     prefs_general.channel_tree_width,
			     prefs_general.channel_tree_height);
}

static void
loqui_app_channel_text_view_scrolled_to_end_cb(LoquiChannelTextView *chview, LoquiChannelEntry *chent)
{
        g_return_if_fail(chview != NULL);
        g_return_if_fail(LOQUI_IS_CHANNEL_TEXT_VIEW(chview));
        g_return_if_fail(chent != NULL);
        g_return_if_fail(LOQUI_IS_CHANNEL_ENTRY(chent));
	
	loqui_channel_entry_set_as_read(chent);
}
static void
loqui_app_channel_text_view_notify_is_scroll_cb(LoquiChannelTextView *chview, GParamSpec *pspec, LoquiApp *app)
{
	loqui_app_actions_toggle_action_set_active(app, LOQUI_ACTION_TOGGLE_SCROLL,
						   loqui_channel_text_view_get_is_scroll(chview));
}
static void
loqui_app_channel_text_view_notify_is_scroll_common_buffer_cb(LoquiChannelTextView *chview, GParamSpec *pspec, LoquiApp *app)
{
	loqui_app_actions_toggle_action_set_active(app, LOQUI_ACTION_TOGGLE_SCROLL_COMMON_BUFFER,
						   loqui_channel_text_view_get_is_scroll(chview));
}
void
loqui_app_grab_focus_if_key_unused(LoquiApp *app, const gchar *class_name, guint modifiers, guint keyval)
{
	gboolean found = FALSE;

	switch (keyval) {
	case GDK_Shift_L:
	case GDK_Shift_R:
	case GDK_Control_L:
	case GDK_Control_R:
	case GDK_Caps_Lock:
	case GDK_Shift_Lock:
	case GDK_Meta_L:
	case GDK_Meta_R:
	case GDK_Alt_L:
	case GDK_Alt_R:
	case GDK_Super_L:
	case GDK_Super_R:
	case GDK_Hyper_L:
	case GDK_Hyper_R: /* FIXME: modifiers, enough? */
	case GDK_ISO_Left_Tab: /* FIXME: if this doesn't exist, shift + tab does not work... */
	case GDK_Tab:
		found = TRUE;
		break;
	default:
		found = gtkutils_bindings_has_matched_entry(class_name, modifiers, keyval);
	}
	if (!found)
		gtk_widget_grab_focus(app->remark_entry);
}
void
loqui_app_update_info(LoquiApp *app, 
		      gboolean is_account_changed, Account *account,
		      gboolean is_channel_changed, LoquiChannel *channel)

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
		channel_name = loqui_channel_entry_get_name(LOQUI_CHANNEL_ENTRY(channel));
		topic = loqui_channel_entry_get_topic(LOQUI_CHANNEL_ENTRY(channel));
		
		user_number = loqui_channel_entry_get_member_number(LOQUI_CHANNEL_ENTRY(channel));
		user_number_str = g_strdup_printf("%d", user_number);

		if(!loqui_channel_get_is_private_talk(channel)) {
			channel_mode = loqui_channel_get_mode(channel);	
			op_number = loqui_channel_entry_get_op_number(LOQUI_CHANNEL_ENTRY(channel));
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
	loqui_app_actions_update_sensitivity_related_channel(app);

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
GtkWidget*
loqui_app_new(AccountManager *account_manager)
{
	LoquiApp *app;
	LoquiAppPrivate *priv;
	GError *error = NULL;

	LoquiAccountManagerStore *manager_store;
	GtkWidget *channel_tree;
	GtkWidget *nick_list;

	GtkWidget *vbox;
	GtkWidget *hpaned;
	GtkWidget *vpaned;
	GtkWidget *scrolled_win;
	
	GtkWidget *menu_channelbar;
	GtkWidget *menu_nick_list;

	GdkPixbuf *loqui_icon;

	GtkAction *toggle_command_action;

	app = g_object_new(loqui_app_get_type(), NULL);
	priv = app->priv;

	g_object_ref(account_manager);
	app->account_manager = account_manager;

	gtk_window_set_policy(GTK_WINDOW (app), TRUE, TRUE, TRUE);

	/* set icon */
	if((loqui_icon = gdk_pixbuf_new_from_inline(-1, loqui_pixbuf, FALSE, NULL)) == NULL)
		g_error("Failed to load loqui icon.");
	gtk_window_set_icon(GTK_WINDOW(app), loqui_icon);
	g_object_unref(loqui_icon);

	vbox = gtk_vbox_new(FALSE, 0);
	gtk_container_add(GTK_CONTAINER(app), vbox);

	app->action_group = loqui_app_actions_create_group(app);
	toggle_command_action = gtk_action_group_get_action(app->action_group, "ToggleCommandMode");

	app->channel_entry_group = gtk_action_group_new("channel-entry-group");
	
	app->ui_manager = gtk_ui_manager_new();
	gtk_ui_manager_set_add_tearoffs(app->ui_manager, TRUE);
	gtk_window_add_accel_group(GTK_WINDOW(app),
				   gtk_ui_manager_get_accel_group(app->ui_manager));
	gtk_ui_manager_insert_action_group(app->ui_manager, app->action_group, 0);
	gtk_ui_manager_insert_action_group(app->ui_manager, app->channel_entry_group, 1);

	if(!gtk_ui_manager_add_ui_from_string(app->ui_manager, embedtxt_loqui_app_ui, -1, &error))
		g_error("Failed to load UI XML: %s", error->message);

	gtk_ui_manager_ensure_update(app->ui_manager);
	gtk_box_pack_start(GTK_BOX(vbox), gtk_ui_manager_get_widget(app->ui_manager, "/menubar"), FALSE, FALSE, 0);

	app->tooltips = gtk_tooltips_new();

	priv->handlebox_channelbar = gtk_handle_box_new();
	gtk_box_pack_start(GTK_BOX(vbox), priv->handlebox_channelbar, FALSE, FALSE, 0);

	menu_channelbar = gtk_ui_manager_get_widget(app->ui_manager, "/ChannelListPopup");
	app->channelbar = loqui_channelbar_new(app, menu_channelbar,
					       GTK_TOGGLE_ACTION(gtk_action_group_get_action(app->action_group, LOQUI_ACTION_TOGGLE_SCROLL)));
	gtk_container_add(GTK_CONTAINER(priv->handlebox_channelbar), app->channelbar);

#define SET_SCROLLED_WINDOW(s, w, vpolicy, hpolicy) \
{ \
	s = gtk_scrolled_window_new(NULL, NULL); \
        gtk_scrolled_window_set_policy(GTK_SCROLLED_WINDOW(s), vpolicy, hpolicy); \
	gtk_container_add(GTK_CONTAINER(s), w); \
}

	hpaned = gtk_hpaned_new();
	gtk_box_pack_start_defaults(GTK_BOX(vbox), hpaned);

	app->statusbar = loqui_statusbar_new(app,
					     GTK_TOGGLE_ACTION(gtk_action_group_get_action(app->action_group, LOQUI_ACTION_TOGGLE_SCROLL_COMMON_BUFFER)));
	gtk_box_pack_start(GTK_BOX(vbox), app->statusbar, FALSE, FALSE, 1);	
	
	/* left side */
	vpaned = gtk_vpaned_new();
	gtk_paned_pack1(GTK_PANED(hpaned), vpaned, TRUE, TRUE);
	
	vbox = gtk_vbox_new(FALSE, 0);
	gtk_paned_pack1(GTK_PANED(vpaned), vbox, TRUE, TRUE);

	app->channel_notebook = gtk_notebook_new();
	gtk_notebook_set_show_tabs(GTK_NOTEBOOK(app->channel_notebook), FALSE);
	gtk_notebook_set_show_border(GTK_NOTEBOOK(app->channel_notebook), FALSE);
	gtk_box_pack_start_defaults(GTK_BOX(vbox), app->channel_notebook);

	app->remark_entry = remark_entry_new(app, GTK_TOGGLE_ACTION(toggle_command_action));
	gtk_box_pack_end(GTK_BOX(vbox), app->remark_entry, FALSE, FALSE, 0);

	app->common_textview = loqui_channel_text_view_new(app);
	loqui_channel_text_view_set_auto_switch_scrolling(LOQUI_CHANNEL_TEXT_VIEW(app->common_textview),
							  prefs_general.auto_switch_scrolling_common_buffer);
	g_signal_connect(G_OBJECT(app->common_textview), "notify::is-scroll",
			 G_CALLBACK(loqui_app_channel_text_view_notify_is_scroll_common_buffer_cb), app);
	gtk_paned_pack2(GTK_PANED(vpaned), LOQUI_CHANNEL_TEXT_VIEW(app->common_textview)->scrolled_window, FALSE, TRUE);

	/* right side */
	vpaned = gtk_vpaned_new();
	gtk_paned_pack2(GTK_PANED(hpaned), vpaned, FALSE, TRUE);
	
	menu_nick_list = gtk_ui_manager_get_widget(app->ui_manager, "/NickListPopup");
	nick_list = nick_list_new(app, menu_nick_list);
	SET_SCROLLED_WINDOW(scrolled_win, nick_list, GTK_POLICY_AUTOMATIC, GTK_POLICY_AUTOMATIC);
	gtk_paned_pack1(GTK_PANED(vpaned), scrolled_win, TRUE, TRUE);	

	manager_store = loqui_account_manager_store_new(loqui_app_get_account_manager(app));

	channel_tree = channel_tree_new(app,
					GTK_MENU(gtk_ui_manager_get_widget(app->ui_manager, "/AccountPopup")),
					GTK_MENU(gtk_ui_manager_get_widget(app->ui_manager, "/ChannelPopup")),
					GTK_MENU(gtk_ui_manager_get_widget(app->ui_manager, "/PrivateTalkPopup")));
	gtk_tree_view_set_model(GTK_TREE_VIEW(channel_tree), GTK_TREE_MODEL(manager_store));
	SET_SCROLLED_WINDOW(scrolled_win, channel_tree, GTK_POLICY_AUTOMATIC, GTK_POLICY_AUTOMATIC); 
	gtk_paned_pack2(GTK_PANED(vpaned), scrolled_win, FALSE, TRUE);
	
	app->channel_tree = CHANNEL_TREE(channel_tree);
	app->nick_list = NICK_LIST(nick_list);

	priv->common_buffer = channel_buffer_new();
	channel_buffer_set_whether_common_buffer(priv->common_buffer, TRUE);
	loqui_channel_text_view_set_channel_buffer(LOQUI_CHANNEL_TEXT_VIEW(app->common_textview), priv->common_buffer);

	loqui_app_update_info(app, TRUE, NULL, TRUE, NULL);
	loqui_app_restore_size(app);

#undef SET_SCROLLED_WINDOW

	gtk_widget_show_all(GTK_WIDGET(app));

	loqui_app_set_show_statusbar(app, prefs_general.show_statusbar);
	loqui_app_set_show_channelbar(app, prefs_general.show_channelbar);

	loqui_app_actions_toggle_action_set_active(app, "ToggleStatusbar", prefs_general.show_statusbar);
	loqui_app_actions_toggle_action_set_active(app, "ToggleChannelbar", prefs_general.show_channelbar);
	loqui_app_set_nick_list_sort_type(app, prefs_general.nick_list_sort_type);

	g_signal_connect_after(G_OBJECT(account_manager), "add-account",
			       G_CALLBACK(loqui_app_add_account_after_cb), app);
	g_signal_connect(G_OBJECT(account_manager), "remove-account",
			 G_CALLBACK(loqui_app_remove_account_cb), app);
	g_signal_connect_after(G_OBJECT(account_manager), "remove-account",
			       G_CALLBACK(loqui_app_remove_account_after_cb), app);

	gtk_widget_grab_focus(app->remark_entry);

	return GTK_WIDGET(app);
}
void
loqui_app_set_auto_switch_scrolling_channel_buffers(LoquiApp *app, gboolean auto_switch_scrolling)
{
	LoquiAppPrivate *priv;
	LoquiAccountManagerIter iter;
	LoquiChannelEntry *chent;
	LoquiChannelTextView *chview;
        g_return_if_fail(app != NULL);
        g_return_if_fail(LOQUI_IS_APP(app));

	priv = app->priv;

	loqui_account_manager_iter_init(loqui_app_get_account_manager(app), &iter);
	loqui_account_manager_iter_set_first_channel_entry(&iter);
	while ((chent = loqui_account_manager_iter_channel_entry_next(&iter))) {
		chview = g_object_get_data(G_OBJECT(chent), CHANNEL_TEXT_VIEW_KEY);

		if (chview)
			loqui_channel_text_view_set_auto_switch_scrolling(chview, auto_switch_scrolling);
	}
	prefs_general.auto_switch_scrolling = auto_switch_scrolling;
}
void
loqui_app_set_auto_switch_scrolling_common_buffer(LoquiApp *app, gboolean auto_switch_scrolling)
{
	loqui_channel_text_view_set_auto_switch_scrolling(LOQUI_CHANNEL_TEXT_VIEW(app->common_textview), auto_switch_scrolling);
	prefs_general.auto_switch_scrolling_common_buffer = auto_switch_scrolling;
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

AccountManager *
loqui_app_get_account_manager(LoquiApp *app)
{
        g_return_val_if_fail(app != NULL, NULL);
        g_return_val_if_fail(LOQUI_IS_APP(app), NULL);

	return app->account_manager;
}

GtkWidget *
loqui_app_get_current_channel_text_view(LoquiApp *app)
{
	GtkWidget *chview = NULL;

	if (app->priv->current_chent)
		chview = g_object_get_data(G_OBJECT(app->priv->current_chent), CHANNEL_TEXT_VIEW_KEY);

	return chview;
}

LoquiChannelEntry *
loqui_app_get_current_channel_entry(LoquiApp *app)
{
	return app->priv->current_chent;
}
void
loqui_app_set_current_channel_entry(LoquiApp *app, LoquiChannelEntry *chent)
{
	LoquiAppPrivate *priv;
	gboolean is_account_changed, is_channel_changed;
	GtkTreeModel *model;
	Account *account;
	LoquiChannel *channel;
	LoquiChannelTextView *chview;

        g_return_if_fail(app != NULL);
        g_return_if_fail(LOQUI_IS_APP(app));
	g_return_if_fail(chent != NULL);
	g_return_if_fail(LOQUI_IS_CHANNEL_ENTRY(chent));
	
	priv = app->priv;

	if (priv->current_chent) {
		g_signal_handlers_disconnect_by_func(priv->current_chent, loqui_app_channel_entry_notify_cb, app);

		chview = g_object_get_data(G_OBJECT(priv->current_chent), CHANNEL_TEXT_VIEW_KEY);
		g_signal_handlers_disconnect_by_func(chview,
						     loqui_app_channel_text_view_scrolled_to_end_cb,
						     priv->current_chent);
		g_signal_handlers_disconnect_by_func(chview,
						     loqui_app_channel_text_view_notify_is_scroll_cb,
						     app);
	}
	if (priv->current_channel) {
		g_signal_handlers_disconnect_by_func(priv->current_channel, loqui_app_channel_changed_cb, app);
	}
	if (priv->current_account) {
		g_signal_handlers_disconnect_by_func(priv->current_account, loqui_app_account_changed_cb, app);
	}

	if (IS_ACCOUNT(chent)) {
		channel = NULL;
		account = ACCOUNT(chent);
	} else {
		channel = LOQUI_CHANNEL(chent);
		account = loqui_channel_get_account(channel);
	}

	is_account_changed = (priv->current_account != account) ? TRUE : FALSE;
	is_channel_changed = (priv->current_channel != channel) ? TRUE : FALSE;

	priv->current_account = account;
	priv->current_channel = channel;
	priv->current_chent = chent;

	chview = g_object_get_data(G_OBJECT(chent), CHANNEL_TEXT_VIEW_KEY);
	
	g_signal_connect(G_OBJECT(chview), "scrolled_to_end",
			 G_CALLBACK(loqui_app_channel_text_view_scrolled_to_end_cb), chent);
	g_signal_connect(G_OBJECT(chview), "notify::is-scroll",
			 G_CALLBACK(loqui_app_channel_text_view_notify_is_scroll_cb), app);

	gtk_notebook_set_current_page(GTK_NOTEBOOK(app->channel_notebook),
				      gtk_notebook_page_num(GTK_NOTEBOOK(app->channel_notebook), LOQUI_CHANNEL_TEXT_VIEW(chview)->scrolled_window));

	loqui_channel_text_view_scroll_to_end_if_enabled(LOQUI_CHANNEL_TEXT_VIEW(chview));
	loqui_app_actions_toggle_action_set_active(app, LOQUI_ACTION_TOGGLE_SCROLL,
						   loqui_channel_text_view_get_is_scroll(LOQUI_CHANNEL_TEXT_VIEW(chview)));

	model = g_object_get_data(G_OBJECT(chent), CHANNEL_ENTRY_STORE_KEY);
	gtk_tree_view_set_model(GTK_TREE_VIEW(app->nick_list), GTK_TREE_MODEL(model));

      	loqui_app_update_info(app, 
			      is_account_changed, account,
			      is_channel_changed, channel);

	loqui_channel_entry_set_as_read(chent);

	g_signal_connect(G_OBJECT(chent), "notify",
			 G_CALLBACK(loqui_app_channel_entry_notify_cb), app);

	if (LOQUI_IS_CHANNEL(chent)) {
		g_signal_connect(channel, "mode-changed",
				 G_CALLBACK(loqui_app_channel_changed_cb), app);
	}

	g_signal_connect(G_OBJECT(account), "user_self_changed",
			 G_CALLBACK(loqui_app_account_changed_cb), app);
	g_signal_connect(G_OBJECT(account), "disconnected",
			 G_CALLBACK(loqui_app_account_changed_cb), app);

	channel_tree_select_channel_entry(app->channel_tree, chent);

	gtk_widget_grab_focus(app->remark_entry);

	if (prefs_general.auto_switch_scrolling)
		loqui_app_actions_toggle_action_set_active(app, LOQUI_ACTION_TOGGLE_SCROLL, TRUE);
}

LoquiChannel *
loqui_app_get_current_channel(LoquiApp *app)
{
	return app->priv->current_channel;
}
Account *
loqui_app_get_current_account(LoquiApp *app)
{
	return app->priv->current_account;
}
static void
loqui_app_channel_entry_notify_number_cb(LoquiChannelEntry *chent, GParamSpec *pspec, LoquiApp *app)
{
	if (LOQUI_IS_CHANNEL(chent)) {
		if (loqui_app_is_current_channel(app, LOQUI_CHANNEL(chent)))
			loqui_app_channel_changed_cb(NULL, app);
	}
}
static gboolean
loqui_app_set_current_channel_for_idle(LoquiApp *app)
{
	LoquiAppPrivate *priv;

	priv = app->priv;

	loqui_app_set_current_channel_entry(app, LOQUI_CHANNEL_ENTRY(priv->wait_current_channel));
	priv->wait_current_channel = NULL;
	priv->is_pending_set_current_channel = FALSE;
	return FALSE;
}
static void
loqui_app_set_current_channel_lazy(LoquiApp *app, LoquiChannel *channel)
{
	LoquiAppPrivate *priv;

	priv = app->priv;

	priv->wait_current_channel = channel;

	if (!priv->is_pending_set_current_channel) {
		priv->is_pending_set_current_channel = TRUE;
		g_idle_add((GSourceFunc) loqui_app_set_current_channel_for_idle, app);
	}
}
gboolean
loqui_app_is_current_account(LoquiApp *app, Account *account)
{
	LoquiAppPrivate *priv;

        g_return_val_if_fail(app != NULL, FALSE);
        g_return_val_if_fail(LOQUI_IS_APP(app), FALSE);

	priv = app->priv;

	return (loqui_app_get_current_account(app) == account);
}
gboolean
loqui_app_is_current_channel(LoquiApp *app, LoquiChannel *channel)
{
        g_return_val_if_fail(app != NULL, FALSE);
        g_return_val_if_fail(LOQUI_IS_APP(app), FALSE);

	return (loqui_app_get_current_channel(app) == channel);
}
static void
loqui_app_add_account_after_cb(AccountManager *manager, Account *account, LoquiApp *app)
{
	LoquiAppPrivate *priv;
	LoquiChannelEntryStore *store;
	ChannelBuffer *buffer;
	GtkWidget *chview;

        g_return_if_fail(manager != NULL);
        g_return_if_fail(IS_ACCOUNT_MANAGER(manager));
	g_return_if_fail(account != NULL);
	g_return_if_fail(IS_ACCOUNT(account));
        g_return_if_fail(app != NULL);
        g_return_if_fail(LOQUI_IS_APP(app));

	priv = app->priv;

	store = loqui_channel_entry_store_new(LOQUI_CHANNEL_ENTRY(account));
	g_object_set_data(G_OBJECT(account), CHANNEL_ENTRY_STORE_KEY, store);

	loqui_channel_entry_ui_attach_channel_entry_action(app, LOQUI_CHANNEL_ENTRY(account));
	loqui_channel_entry_ui_add_account(app, account, "/menubar/Buffers", "menubar");
	loqui_channel_entry_ui_add_account(app, account, "/ChannelListPopup", "channelbar");

	g_signal_connect_swapped(G_OBJECT(account), "connected",
				 G_CALLBACK(loqui_app_set_current_channel_entry), app);
	g_signal_connect_after(G_OBJECT(account), "add-channel",
			       G_CALLBACK(loqui_app_add_channel_after_cb), app);
	g_signal_connect(G_OBJECT(account), "remove-channel",
			 G_CALLBACK(loqui_app_remove_channel_cb), app);
	g_signal_connect_after(G_OBJECT(account), "remove-channel",
			       G_CALLBACK(loqui_app_remove_channel_after_cb), app);
	
	buffer = loqui_channel_entry_get_buffer(LOQUI_CHANNEL_ENTRY(account));
	g_signal_connect(G_OBJECT(buffer), "append",
			 G_CALLBACK(loqui_app_channel_buffer_append_cb), app);

	chview = loqui_channel_text_view_new(app);
	g_object_set_data(G_OBJECT(account), CHANNEL_TEXT_VIEW_KEY, chview);
	loqui_channel_text_view_set_channel_buffer(LOQUI_CHANNEL_TEXT_VIEW(chview), buffer);
	loqui_channel_text_view_set_auto_switch_scrolling(LOQUI_CHANNEL_TEXT_VIEW(chview),
							  prefs_general.auto_switch_scrolling);

	gtk_notebook_append_page(GTK_NOTEBOOK(app->channel_notebook),
				 LOQUI_CHANNEL_TEXT_VIEW(chview)->scrolled_window,
				 NULL);
	gtk_widget_show_all(LOQUI_CHANNEL_TEXT_VIEW(chview)->scrolled_window);

	loqui_channel_entry_set_sort_func(LOQUI_CHANNEL_ENTRY(account), priv->sort_func);
	loqui_app_update_channel_entry_accel_key(app);
}
static void
loqui_app_remove_account_cb(AccountManager *manager, Account *account, LoquiApp *app)
{
	LoquiAppPrivate *priv;
	ChannelBuffer *buffer;
	LoquiChannelEntryStore *store;
	GtkWidget *chview;

        g_return_if_fail(manager != NULL);
        g_return_if_fail(IS_ACCOUNT_MANAGER(manager));
        g_return_if_fail(app != NULL);
        g_return_if_fail(LOQUI_IS_APP(app));
	g_return_if_fail(account != NULL);
	g_return_if_fail(IS_ACCOUNT(account));

	priv = app->priv;

	loqui_channel_entry_ui_remove_account(app, account, "/menubar/Buffers", "menubar");
	loqui_channel_entry_ui_remove_account(app, account, "/ChannelListPopup", "channelbar");

	g_signal_handlers_disconnect_by_func(G_OBJECT(account), loqui_app_set_current_channel_entry, app);
	g_signal_handlers_disconnect_by_func(G_OBJECT(account), loqui_app_add_channel_after_cb, app);
	g_signal_handlers_disconnect_by_func(G_OBJECT(account), loqui_app_remove_channel_cb, app);
	g_signal_handlers_disconnect_by_func(G_OBJECT(account), loqui_app_remove_channel_after_cb, app);

	buffer = loqui_channel_entry_get_buffer(LOQUI_CHANNEL_ENTRY(account));
	g_signal_handlers_disconnect_by_func(buffer, loqui_app_channel_buffer_append_cb, app);

	chview = g_object_get_data(G_OBJECT(account), CHANNEL_TEXT_VIEW_KEY);
	gtk_notebook_remove_page(GTK_NOTEBOOK(app->channel_notebook),
				 gtk_notebook_page_num(GTK_NOTEBOOK(app->channel_notebook), LOQUI_CHANNEL_TEXT_VIEW(chview)->scrolled_window));

	store = g_object_get_data(G_OBJECT(account), CHANNEL_ENTRY_STORE_KEY);
	g_object_unref(store);
}
static void
loqui_app_remove_account_after_cb(AccountManager *manager, Account *was_account, LoquiApp *app)
{
        g_return_if_fail(manager != NULL);
        g_return_if_fail(IS_ACCOUNT_MANAGER(manager));
        g_return_if_fail(app != NULL);
        g_return_if_fail(LOQUI_IS_APP(app));

	loqui_app_update_channel_entry_accel_key(app);
}
static void
loqui_app_add_channel_after_cb(Account *account, LoquiChannel *channel, LoquiApp *app)
{
	LoquiAppPrivate *priv;
	LoquiChannelEntryStore *store;
	ChannelBuffer *buffer;
	GtkWidget *chview;

        g_return_if_fail(app != NULL);
        g_return_if_fail(LOQUI_IS_APP(app));
	g_return_if_fail(account != NULL);
	g_return_if_fail(IS_ACCOUNT(account));
	g_return_if_fail(channel != NULL);
	g_return_if_fail(LOQUI_IS_CHANNEL(channel));

	priv = app->priv;

	store = loqui_channel_entry_store_new(LOQUI_CHANNEL_ENTRY(channel));
	g_object_set_data(G_OBJECT(channel), CHANNEL_ENTRY_STORE_KEY, store);

	loqui_channel_entry_ui_attach_channel_entry_action(app, LOQUI_CHANNEL_ENTRY(channel));
	loqui_channel_entry_ui_add_channel(app, channel, "/menubar/Buffers", "menubar");
	loqui_channel_entry_ui_add_channel(app, channel, "/ChannelListPopup", "channelbar");

	g_signal_connect(G_OBJECT(channel), "notify::is-updated",
			 G_CALLBACK(loqui_app_channel_entry_notify_is_updated_cb), app);
	g_signal_connect(G_OBJECT(channel), "notify::op-number",
			 G_CALLBACK(loqui_app_channel_entry_notify_number_cb), app);
	g_signal_connect(G_OBJECT(channel), "notify::member-number",
			 G_CALLBACK(loqui_app_channel_entry_notify_number_cb), app);

	buffer = loqui_channel_entry_get_buffer(LOQUI_CHANNEL_ENTRY(channel));
	g_signal_connect(G_OBJECT(buffer), "append",
			 G_CALLBACK(loqui_app_channel_buffer_append_cb), app);

	chview = loqui_channel_text_view_new(app);
	g_object_set_data(G_OBJECT(channel), CHANNEL_TEXT_VIEW_KEY, chview);
	loqui_channel_text_view_set_channel_buffer(LOQUI_CHANNEL_TEXT_VIEW(chview), buffer);
	gtk_notebook_append_page(GTK_NOTEBOOK(app->channel_notebook),
				 LOQUI_CHANNEL_TEXT_VIEW(chview)->scrolled_window,
				 NULL);
	loqui_channel_text_view_set_auto_switch_scrolling(LOQUI_CHANNEL_TEXT_VIEW(chview),
							  prefs_general.auto_switch_scrolling);
	gtk_widget_show_all(LOQUI_CHANNEL_TEXT_VIEW(chview)->scrolled_window);

	loqui_channel_entry_set_sort_func(LOQUI_CHANNEL_ENTRY(channel), priv->sort_func);

	loqui_app_update_channel_entry_accel_key(app);
	loqui_app_set_current_channel_lazy(app, channel);
}
static void
loqui_app_remove_channel_cb(Account *account, LoquiChannel *channel, LoquiApp *app)
{
	LoquiAppPrivate *priv;
	ChannelBuffer *buffer;
	LoquiChannelEntryStore *store;
	GtkWidget *chview;

        g_return_if_fail(app != NULL);
        g_return_if_fail(LOQUI_IS_APP(app));
	g_return_if_fail(account != NULL);
	g_return_if_fail(IS_ACCOUNT(account));
	g_return_if_fail(channel != NULL);
	g_return_if_fail(LOQUI_IS_CHANNEL(channel));

	priv = app->priv;

	loqui_channel_entry_ui_remove_channel(app, channel, "menubar");
	loqui_channel_entry_ui_remove_channel(app, channel, "channelbar");

	loqui_app_set_current_channel_entry(app, LOQUI_CHANNEL_ENTRY(account));
	g_signal_handlers_disconnect_by_func(channel, loqui_app_channel_entry_notify_is_updated_cb, app);
	g_signal_handlers_disconnect_by_func(channel, loqui_app_channel_entry_notify_number_cb, app);
	
	buffer = loqui_channel_entry_get_buffer(LOQUI_CHANNEL_ENTRY(channel));
	g_signal_handlers_disconnect_by_func(buffer, loqui_app_channel_buffer_append_cb, app);

	chview = g_object_get_data(G_OBJECT(channel), CHANNEL_TEXT_VIEW_KEY);
	gtk_notebook_remove_page(GTK_NOTEBOOK(app->channel_notebook),
				 gtk_notebook_page_num(GTK_NOTEBOOK(app->channel_notebook), LOQUI_CHANNEL_TEXT_VIEW(chview)->scrolled_window));

	store = g_object_get_data(G_OBJECT(channel), CHANNEL_ENTRY_STORE_KEY);
	g_object_unref(store);
}
static void
loqui_app_remove_channel_after_cb(Account *account, LoquiChannel *was_channel, LoquiApp *app)
{
	LoquiAppPrivate *priv;

        g_return_if_fail(app != NULL);
        g_return_if_fail(LOQUI_IS_APP(app));
	g_return_if_fail(account != NULL);
	g_return_if_fail(IS_ACCOUNT(account));

	priv = app->priv;

	loqui_app_update_channel_entry_accel_key(app);
}
static gboolean
loqui_app_update_account_info(LoquiApp *app)
{
	LoquiAppPrivate *priv;

        g_return_val_if_fail(app != NULL, FALSE);
        g_return_val_if_fail(LOQUI_IS_APP(app), FALSE);
	
	priv = app->priv;

	loqui_app_update_info(app, 
			      TRUE, loqui_app_get_current_account(app),
			      FALSE, loqui_app_get_current_channel(app));

	priv->is_pending_update_account_info = FALSE;
	return FALSE;
}
static gboolean
loqui_app_update_channel_info(LoquiApp *app)
{
	LoquiAppPrivate *priv;

        g_return_val_if_fail(app != NULL, FALSE);
        g_return_val_if_fail(LOQUI_IS_APP(app), FALSE);
	
	priv = app->priv;

	loqui_app_update_info(app, 
			      FALSE, loqui_app_get_current_account(app),
			      TRUE, loqui_app_get_current_channel(app));

	priv->is_pending_update_channel_info = FALSE;
	return FALSE;
}
static void
loqui_app_account_changed_cb(GObject *object, gpointer data)
{
	LoquiApp *app;
	LoquiAppPrivate *priv;

        g_return_if_fail(data != NULL);
        g_return_if_fail(LOQUI_IS_APP(data));
	
	app = LOQUI_APP(data);

	priv = app->priv;
	
	if (!priv->is_pending_update_account_info) {
		priv->is_pending_update_account_info = TRUE;
		g_idle_add((GSourceFunc) loqui_app_update_account_info, app);
	}
}
static void
loqui_app_channel_entry_notify_cb(LoquiChannelEntry *chent, GParamSpec *pspec, gpointer data)
{
	loqui_app_channel_changed_cb(G_OBJECT(chent), data);
}
static void
loqui_app_channel_changed_cb(GObject *object, gpointer data)
{
	LoquiApp *app;
	LoquiAppPrivate *priv;

        g_return_if_fail(data != NULL);
        g_return_if_fail(LOQUI_IS_APP(data));
	
	app = LOQUI_APP(data);

	priv = app->priv;
	
	if (!priv->is_pending_update_channel_info) {
		priv->is_pending_update_channel_info = TRUE;
		g_idle_add((GSourceFunc) loqui_app_update_channel_info, app);
	}
}
static void
loqui_app_channel_entry_notify_is_updated_cb(LoquiChannelEntry *chent, GParamSpec *pspec, LoquiApp *app)
{
	LoquiAppPrivate *priv;
	LoquiChannel *channel = NULL;
	gchar *str;
	gboolean updated;
	gint delta;
	
	g_return_if_fail(chent != NULL);
	g_return_if_fail(LOQUI_IS_CHANNEL_ENTRY(chent));
        g_return_if_fail(app != NULL);
        g_return_if_fail(LOQUI_IS_APP(app));

	priv = app->priv;
	
	if (LOQUI_IS_CHANNEL(chent))
		channel = LOQUI_CHANNEL(chent);
	else
		return;
	/* FIXME: handle account */

	updated = loqui_channel_entry_get_is_updated(LOQUI_CHANNEL_ENTRY(channel));
	
	delta = updated ? +1 : -1;
	
	if (loqui_channel_get_is_private_talk(channel))
		priv->updated_private_talk_number += delta;
	else
		priv->updated_channel_number += delta;
	
	if (priv->updated_private_talk_number > 0 && priv->updated_channel_number > 0)
		str = g_strdup_printf(_("Updated: %d private talk(s), %d channel(s)."),
					priv->updated_private_talk_number,
					priv->updated_channel_number);
	else if (priv->updated_private_talk_number > 0)
		str = g_strdup_printf(_("Updated: %d private talk(s)."),
					priv->updated_private_talk_number);
	else if (priv->updated_channel_number > 0)
		str = g_strdup_printf(_("Updated: %d channel(s)."),
					priv->updated_channel_number);
	else
		str = g_strdup("");
	
	loqui_statusbar_set_default(LOQUI_STATUSBAR(app->statusbar), str);
	g_free(str);
		
}
static void
loqui_app_channel_buffer_append_cb(ChannelBuffer *buffer, MessageText *msgtext, LoquiApp *app)
{
	LoquiAppPrivate *priv;
	GtkWidget *chview;

        g_return_if_fail(app != NULL);
        g_return_if_fail(LOQUI_IS_APP(app));

	priv = app->priv;

	if (priv->last_msgtext == msgtext)
		return;
		
	if (prefs_general.save_log)
		loqui_app_append_log(app, msgtext);

	chview = loqui_app_get_current_channel_text_view(app);
	if (chview &&
	    loqui_channel_text_view_get_is_scroll(LOQUI_CHANNEL_TEXT_VIEW(chview)) &&
	    priv->current_chent != NULL &&
	    buffer == loqui_channel_entry_get_buffer(priv->current_chent))
		return;
	
	channel_buffer_append_message_text(priv->common_buffer, msgtext, TRUE, FALSE);

	if (priv->last_msgtext)
		g_object_unref(priv->last_msgtext);
	g_object_ref(msgtext);
	priv->last_msgtext = msgtext;
}

static void
loqui_app_append_log(LoquiApp *app, MessageText *msgtext)
{
	gchar *path;
	gchar *filename;
	gchar *buf;
	gchar *time_str;
	gchar *nick;
	const gchar *account_name;
	GIOChannel *io;
	time_t t;
	
        g_return_if_fail(app != NULL);
        g_return_if_fail(LOQUI_IS_APP(app));
        
        t = time(NULL);
        
        filename = utils_strftime_epoch("log-%Y%m%d.txt", t);
	path = g_build_filename(g_get_home_dir(), PREFS_DIR, LOG_DIR, filename, NULL);
	g_free(filename);
	
	if ((io = g_io_channel_new_file(path, "a", NULL)) == NULL) {
		g_warning("Can't open log file(%s)", path);
		g_free(path);
		return;
	}
	
	time_str = utils_strftime_epoch(prefs_general.time_format, t);
	if (message_text_get_nick(msgtext))
		nick = message_text_get_nick_string(msgtext, TRUE);
	else
		nick = g_strdup("");
	
	account_name = message_text_get_account_name(msgtext);
	
	if (account_name)
		buf = g_strdup_printf("%s[%s] %s\n", time_str, account_name, message_text_get_text(msgtext));
	else
		buf = g_strdup_printf("%s%s%s\n", time_str, nick, message_text_get_text(msgtext));
	g_free(time_str);
	g_free(nick);
		
	if (g_io_channel_write_chars(io, buf, -1, NULL, NULL) == 0)
		g_warning("Can't write log(%s)", path);
	
	g_free(path);
	g_free(buf);
	g_io_channel_unref(io);
}
static void
loqui_app_set_channel_entry_accel_key(LoquiApp *app, LoquiChannelEntry *chent)
{
	guint keyval;
	guint modifiers;
	gint pos;
	gint id;
	gchar *path;

	id = loqui_channel_entry_get_id(chent);
	if (id < 0)
		return;

	pos = loqui_channel_entry_get_position(chent);

	if (0 <= pos && pos < 10) {
		keyval = GDK_0 + pos;
		modifiers = GDK_CONTROL_MASK;
	} else if (10 <= pos && pos < 20) {
		keyval = GDK_0 + pos - 10;
		modifiers = GDK_MOD1_MASK;
	} else {
		keyval = 0;
		modifiers = 0;
	}
	
	if (keyval != 0) {
		path = g_strdup_printf(SHORTCUT_CHANNEL_ENTRY_ACCEL_MAP_PREFIX "%d", id);
		gtk_accel_map_change_entry(path, keyval, modifiers, TRUE);
		g_free(path);
	}
}
static void
loqui_app_update_channel_entry_accel_key(LoquiApp *app)
{
	LoquiAccountManagerIter iter;
	LoquiChannelEntry *chent;

	loqui_account_manager_iter_init(loqui_app_get_account_manager(app), &iter);
	loqui_account_manager_iter_set_first_channel_entry(&iter);
	while ((chent = loqui_account_manager_iter_channel_entry_next(&iter)))
		loqui_app_set_channel_entry_accel_key(app, chent);
}
void
loqui_app_set_nick_list_sort_type(LoquiApp *app, PrefSortType sort_type)
{
	LoquiAppPrivate *priv;
	LoquiAccountManagerIter iter;
	LoquiChannelEntry *chent;

        g_return_if_fail(app != NULL);
        g_return_if_fail(LOQUI_IS_APP(app));
        
	priv = app->priv;

	switch (sort_type) {
	case PREF_SORT_NONE:
		priv->sort_func = NULL;
		break;
	case PREF_SORT_NICK:
		priv->sort_func = (GCompareFunc) loqui_member_sort_funcs_nick;
		break;
	case PREF_SORT_POWER_NICK:
		priv->sort_func = (GCompareFunc) loqui_member_sort_funcs_power_nick;
		break;
	case PREF_SORT_AWAY_NICK:
		priv->sort_func = (GCompareFunc) loqui_member_sort_funcs_away_nick;
		break;
	case PREF_SORT_POWER_AWAY_NICK:
		priv->sort_func = (GCompareFunc) loqui_member_sort_funcs_power_away_nick;
		break;
	case PREF_SORT_AWAY_POWER_NICK:
		priv->sort_func = (GCompareFunc) loqui_member_sort_funcs_away_power_nick;
		break;
	case PREF_SORT_TIME_NICK:
		priv->sort_func = (GCompareFunc) loqui_member_sort_funcs_time_nick;
		break;
	case PREF_SORT_TIME_AWAY_POWER_NICK:
		priv->sort_func = (GCompareFunc) loqui_member_sort_funcs_time_away_power_nick;
		break;
	case PREF_SORT_TIME_POWER_AWAY_NICK:
		priv->sort_func = (GCompareFunc) loqui_member_sort_funcs_time_power_away_nick;
		break;
	default:
		g_print("Invalid sort type: %d", sort_type);
		priv->sort_func = NULL;
	}
	
	loqui_account_manager_iter_init(loqui_app_get_account_manager(app), &iter);
	loqui_account_manager_iter_set_first_channel_entry(&iter);
	while ((chent = loqui_account_manager_iter_channel_entry_next(&iter)))
		loqui_channel_entry_set_sort_func(chent, priv->sort_func);

	prefs_general.nick_list_sort_type = sort_type;
}
