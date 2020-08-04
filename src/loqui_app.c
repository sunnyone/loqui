/* -*- Mode: C; tab-width: 8; indent-tabs-mode: t; c-basic-offset: 8 -*- */
/*
 * Loqui -- IRC client for Gtk2 <http://launchpad.net/loqui/>
 * Copyright (C) 2002-2003 Yoichi Imai <sunnyone41@gmail.com>
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

#include <loqui_account.h>
#include <libloqui/loqui-account-manager.h>
#include <loqui_channel_entry_utils.h>
#include <libloqui/loqui-utils.h>
#include <libloqui/loqui-message-text-region.h>

#include "loqui_app.h"
#include "channel_tree.h"
#include "nick_list.h"
#include "loqui_channelbar.h"
#include "loqui_statusbar.h"
#include "gtkutils.h"
#include "remark_entry.h"
#include "command_dialog.h"
#include "loqui_profile_account.h"
#include "loqui_app_actions.h"
#include "main.h"
#include "loqui_channel_entry.h"
#include "loqui_channel_entry_action.h"
#include "loqui_channel_entry_store.h"
#include "loqui_channel_text_view.h"
#include "loqui-core-gtk.h"
#include "loqui_account_manager_store.h"
#include "loqui_member_sort_funcs.h"
#include "loqui_account_manager_iter.h"
#include "loqui-tray-icon.h"

#include "embedtxt/loqui_app_ui.h"
#include "icons/pixbufs.h"
#include "loqui_stock.h"

#include <gdk/gdkkeysyms.h>

#include <glib/gi18n.h>

#include <string.h>
#include <time.h>

#include <loqui.h>
#include <libloqui/loqui-general-pref-default.h>
#include <libloqui/loqui-general-pref-groups.h>
#include "loqui-general-pref-gtk-groups.h"
#include "loqui-general-pref-gtk-default.h"

#include "loqui-channel-entry-action-group-ui.h"

#define CHANNEL_ENTRY_STORE_KEY "channel-entry-store"
#define CHANNEL_TEXT_VIEW_KEY "channel-text-view"

#define DISCONNECT_WAIT_WHEN_QUIT 3000

struct _LoquiAppPrivate
{
	GtkWidget *handlebox_channelbar;

	LoquiMessageText *last_msgtext;
	LoquiChannelBufferGtk *common_buffer;

	gboolean is_pending_set_current_channel;
	LoquiChannel *wait_current_channel;

	gboolean has_toplevel_focus;
	gboolean is_obscured;

	gint remained_account_num;

	LoquiPrefPartial *ppref_channel_buffer;

	LoquiChannelEntryActionGroupUI *buffers_ui;
	LoquiChannelEntryActionGroupUI *channelbar_ui;
	LoquiChannelEntryActionGroupUI *trayicon_ui;
	
	GCompareFunc sort_func;
};

static GtkWindowClass *parent_class = NULL;
#define PARENT_TYPE GTK_TYPE_WINDOW

static void loqui_app_class_init(LoquiAppClass *klass);
static void loqui_app_init(LoquiApp *app);
static void loqui_app_finalize(GObject *object);
static void loqui_app_destroy(GtkWidget *widget);
static gboolean loqui_app_delete_event(GtkWidget *widget, GdkEventAny *event);
static gboolean loqui_app_focus_in_event(GtkWidget *widget, GdkEventFocus *event);
static gboolean loqui_app_focus_out_event(GtkWidget *widget, GdkEventFocus *event);
static gboolean loqui_app_visibility_notify_event(GtkWidget *widget, GdkEventVisibility *event);

static void loqui_app_restore_size(LoquiApp *app);
static void loqui_app_save_size(LoquiApp *app);

static void loqui_app_create_tray_icon(LoquiApp *app);

static void loqui_app_channel_entry_added_after(LoquiApp *app, LoquiChannelEntry *chent);
static void loqui_app_channel_entry_removed(LoquiApp *app, LoquiChannelEntry *chent);

static void loqui_app_add_account_after_cb(LoquiAccountManager *manager, LoquiAccount *account, LoquiApp *app);
static void loqui_app_remove_account_cb(LoquiAccountManager *manager, LoquiAccount *account, LoquiApp *app);

static void loqui_app_add_channel_after_cb(LoquiAccount *account, LoquiChannel *channel, LoquiApp *app);
static void loqui_app_remove_channel_cb(LoquiAccount *account, LoquiChannel *channel, LoquiApp *app);

static void loqui_app_channel_text_view_scrolled_to_end_cb(LoquiChannelTextView *chview, LoquiChannelEntry *chent);
static void loqui_app_channel_text_view_notify_is_scroll_cb(LoquiChannelTextView *chview, GParamSpec *pspec, LoquiApp *app);
static void loqui_app_channel_text_view_notify_is_scroll_common_buffer_cb(LoquiChannelTextView *chview, GParamSpec *pspec, LoquiApp *app);

static void loqui_app_channel_entry_append_message_text_after_cb(LoquiChannelEntry *chent, LoquiMessageText *msgtext, LoquiApp *app);

static void loqui_app_append_log(LoquiApp *app, LoquiMessageText *msgtext);

static gboolean loqui_app_set_current_channel_for_idle(LoquiApp *app);
static void loqui_app_set_current_channel_lazy(LoquiApp *app, LoquiChannel *channel);

static void loqui_app_disconnect_all_account_for_quit(LoquiApp *app);

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
loqui_app_class_init(LoquiAppClass *klass)
{
        GObjectClass *object_class = G_OBJECT_CLASS(klass);
	GtkWidgetClass *gtk_widget_class = GTK_WIDGET_CLASS(klass);

        parent_class = g_type_class_peek_parent(klass);
	
        object_class->finalize = loqui_app_finalize;
	gtk_widget_class->destroy = loqui_app_destroy;
	gtk_widget_class->delete_event = loqui_app_delete_event;
	gtk_widget_class->focus_in_event = loqui_app_focus_in_event;
	gtk_widget_class->focus_out_event = loqui_app_focus_out_event;
	gtk_widget_class->visibility_notify_event = loqui_app_visibility_notify_event;
}
static void 
loqui_app_init(LoquiApp *app)
{
	LoquiAppPrivate *priv;

	priv = g_new0(LoquiAppPrivate, 1);
	app->priv = priv;

        gtk_widget_add_events(GTK_WIDGET(app), GDK_VISIBILITY_NOTIFY_MASK);
}
static void
loqui_app_destroy(GtkWidget *widget)
{
	LoquiApp *app;
	LoquiAppPrivate *priv;

        g_return_if_fail(widget != NULL);
        g_return_if_fail(LOQUI_IS_APP(widget));

	app = LOQUI_APP(widget);
	priv = app->priv;

	LOQUI_G_OBJECT_UNREF_UNLESS_NULL(priv->ppref_channel_buffer);

	if (GTK_WIDGET_CLASS(parent_class)->destroy)
                (* GTK_WIDGET_CLASS(parent_class)->destroy) (widget);

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

	loqui_tray_icon_destroy(app->tray_icon);

	g_signal_handlers_disconnect_by_func(G_OBJECT(app->account_manager), loqui_app_add_account_after_cb, app);
	g_signal_handlers_disconnect_by_func(G_OBJECT(app->account_manager), loqui_app_remove_account_cb, app);

	LOQUI_G_OBJECT_UNREF_UNLESS_NULL(app->channel_entry_action_group);
	LOQUI_G_OBJECT_UNREF_UNLESS_NULL(priv->buffers_ui);
	LOQUI_G_OBJECT_UNREF_UNLESS_NULL(priv->channelbar_ui);
	LOQUI_G_OBJECT_UNREF_UNLESS_NULL(priv->trayicon_ui);

	LOQUI_G_OBJECT_UNREF_UNLESS_NULL(app->account_manager);

	if (G_OBJECT_CLASS(parent_class)->finalize)
                (* G_OBJECT_CLASS(parent_class)->finalize) (object);

	g_free(app->priv);
}

static gboolean
loqui_app_delete_event(GtkWidget *widget, GdkEventAny *event)
{
	LoquiApp *app;

        g_return_val_if_fail(widget != NULL, FALSE);
        g_return_val_if_fail(LOQUI_IS_APP(widget), FALSE);

	app = LOQUI_APP(widget);

        if (loqui_pref_get_with_default_boolean(loqui_get_general_pref(),
						LOQUI_GENERAL_PREF_GTK_GROUP_SIZE, "SaveSize",
						LOQUI_GENERAL_PREF_GTK_DEFAULT_SIZE_SAVE_SIZE, NULL)) {
		loqui_app_save_size(LOQUI_APP(widget));
        }
	loqui_app_disconnect_all_account_for_quit(app);

	if (GTK_WIDGET_CLASS(parent_class)->delete_event)
               return (* GTK_WIDGET_CLASS(parent_class)->delete_event) (widget, event);

	return TRUE;
}

static gboolean
loqui_app_focus_in_event(GtkWidget *widget, GdkEventFocus *event)
{
	LoquiApp *app;
	LoquiAppPrivate *priv;

        g_return_val_if_fail(widget != NULL, FALSE);
        g_return_val_if_fail(LOQUI_IS_APP(widget), FALSE);

	app = LOQUI_APP(widget);
	priv = app->priv;
	
	priv->has_toplevel_focus = TRUE;

	loqui_tray_icon_set_hilighted(app->tray_icon, FALSE);
	
	if (GTK_WIDGET_CLASS(parent_class)->focus_in_event)
               return (* GTK_WIDGET_CLASS(parent_class)->focus_in_event) (widget, event);

	return FALSE;
}
static gboolean
loqui_app_focus_out_event(GtkWidget *widget, GdkEventFocus *event)
{
	LoquiApp *app;
	LoquiAppPrivate *priv;

        g_return_val_if_fail(widget != NULL, FALSE);
        g_return_val_if_fail(LOQUI_IS_APP(widget), FALSE);

	app = LOQUI_APP(widget);
	priv = app->priv;

	priv->has_toplevel_focus = FALSE;

	if (GTK_WIDGET_CLASS(parent_class)->focus_out_event)
               return (* GTK_WIDGET_CLASS(parent_class)->focus_out_event) (widget, event);
	return FALSE;
}
static gboolean
loqui_app_visibility_notify_event(GtkWidget *widget, GdkEventVisibility *event)
{
	LoquiApp *app;
	LoquiAppPrivate *priv;

        g_return_val_if_fail(widget != NULL, FALSE);
        g_return_val_if_fail(LOQUI_IS_APP(widget), FALSE);

	app = LOQUI_APP(widget);
	priv = app->priv;

	priv->is_obscured = (event->state == GDK_VISIBILITY_FULLY_OBSCURED || event->state == GDK_VISIBILITY_PARTIAL) ? TRUE : FALSE;

	if (GTK_WIDGET_CLASS(parent_class)->visibility_notify_event)
               return (* GTK_WIDGET_CLASS(parent_class)->visibility_notify_event) (widget, event);

	return FALSE;
}
gboolean
loqui_app_has_toplevel_focus(LoquiApp *app)
{
	LoquiAppPrivate *priv;

        g_return_val_if_fail(app != NULL, FALSE);
        g_return_val_if_fail(LOQUI_IS_APP(app), FALSE);

	priv = app->priv;

	return priv->has_toplevel_focus;
}
gboolean
loqui_app_is_obscured(LoquiApp *app)
{
	LoquiAppPrivate *priv;

        g_return_val_if_fail(app != NULL, FALSE);
        g_return_val_if_fail(LOQUI_IS_APP(app), FALSE);

	priv = app->priv;

	return priv->is_obscured;
}
static void
loqui_app_save_size(LoquiApp *app)
{
	LoquiAppPrivate *priv;

	gint height;
	gint width;

        g_return_if_fail(app != NULL);
        g_return_if_fail(LOQUI_IS_APP(app));

	priv = app->priv;

	gtk_window_get_size(GTK_WINDOW(app), &width, &height);
	loqui_pref_set_integer(loqui_get_general_pref(),
                               LOQUI_GENERAL_PREF_GTK_GROUP_SIZE, "WindowWidth",
                               width);
	loqui_pref_set_integer(loqui_get_general_pref(),
                               LOQUI_GENERAL_PREF_GTK_GROUP_SIZE, "WindowHeight",
                               height);
	
	loqui_pref_set_integer(loqui_get_general_pref(),
                               LOQUI_GENERAL_PREF_GTK_GROUP_SIZE, "CommonBufferHeight",
                               app->common_textview->allocation.height);

	loqui_pref_set_integer(loqui_get_general_pref(),
                               LOQUI_GENERAL_PREF_GTK_GROUP_SIZE, "ChannelTreeHeight",
                               GTK_WIDGET(app->channel_tree)->allocation.height);
	loqui_pref_set_integer(loqui_get_general_pref(),
                               LOQUI_GENERAL_PREF_GTK_GROUP_SIZE, "ChannelTreeWidth",
                               GTK_WIDGET(app->channel_tree)->allocation.width);
}
static void
loqui_app_restore_size(LoquiApp *app)
{
	LoquiAppPrivate *priv;

        g_return_if_fail(app != NULL);
        g_return_if_fail(LOQUI_IS_APP(app));

	priv = app->priv;
        gtk_window_set_default_size(GTK_WINDOW(app),
				    loqui_pref_get_with_default_integer(loqui_get_general_pref(),
									LOQUI_GENERAL_PREF_GTK_GROUP_SIZE, "WindowWidth",
									LOQUI_GENERAL_PREF_GTK_DEFAULT_SIZE_WINDOW_WIDTH, NULL),
				    loqui_pref_get_with_default_integer(loqui_get_general_pref(),
									LOQUI_GENERAL_PREF_GTK_GROUP_SIZE, "WindowHeight",
									LOQUI_GENERAL_PREF_GTK_DEFAULT_SIZE_WINDOW_HEIGHT, NULL));

	gtk_widget_set_usize(app->common_textview, -1,
			     loqui_pref_get_with_default_integer(loqui_get_general_pref(),
								 LOQUI_GENERAL_PREF_GTK_GROUP_SIZE, "CommonBufferHeight",
								 LOQUI_GENERAL_PREF_GTK_DEFAULT_SIZE_COMMON_BUFFER_HEIGHT, NULL));

	gtk_widget_set_usize(GTK_WIDGET(app->channel_tree),
			     loqui_pref_get_with_default_integer(loqui_get_general_pref(),
								 LOQUI_GENERAL_PREF_GTK_GROUP_SIZE, "ChannelTreeWidth",
								 LOQUI_GENERAL_PREF_GTK_DEFAULT_SIZE_CHANNEL_TREE_WIDTH, NULL),
			     loqui_pref_get_with_default_integer(loqui_get_general_pref(),
								 LOQUI_GENERAL_PREF_GTK_GROUP_SIZE, "ChannelTreeHeight",
								 LOQUI_GENERAL_PREF_GTK_DEFAULT_SIZE_CHANNEL_TREE_HEIGHT, NULL));
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
loqui_app_grab_focus_if_key_unused(LoquiApp *app, const gchar *class_name, GdkEventKey *event)
{
	gboolean found = FALSE;
	guint modifiers = event->state;

	switch (event->keyval) {
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
	case GDK_Hyper_R: /* FIXME: modifiers. enough? */
	case GDK_ISO_Left_Tab: /* FIXME: if this doesn't exist, shift + tab does not work... */
	case GDK_Tab:
	case GDK_Menu: /* Application key opens context menu */
		found = TRUE;
		break;
	default:
		found = gtkutils_bindings_has_matched_entry(class_name, modifiers, event->keyval);
	}
	if (!found) {
		gtk_widget_grab_focus(app->remark_entry);
		if (event->keyval != GDK_space) { /* FIXME: entry is activated when space is pressed  ... */
			gtk_widget_event(REMARK_ENTRY(app->remark_entry)->entry, (GdkEvent *) event);
		}
	}
}
static void
loqui_app_create_tray_icon(LoquiApp *app)
{
	LoquiAppPrivate *priv;
	GtkWidget *menu_tray_icon;

	priv = app->priv;

	menu_tray_icon = gtk_ui_manager_get_widget(app->ui_manager, "/TrayIconPopup");
	app->tray_icon = LOQUI_TRAY_ICON(loqui_tray_icon_new(LOQUI_APP(app), GTK_MENU(menu_tray_icon)));
}
static gboolean
loqui_app_quit_idle_cb(LoquiApp *app)
{
	gtk_widget_destroy(GTK_WIDGET(app));
	gtk_main_quit();

	return FALSE;
}
static void
loqui_app_quit_idle(LoquiApp *app)
{
	g_idle_add_full(G_PRIORITY_LOW, (GSourceFunc) loqui_app_quit_idle_cb, app, NULL);
}
static void
loqui_app_account_notify_is_connected_cb_for_quit(LoquiAccount *account, GParamSpec *pspec, LoquiApp *app)
{
	LoquiAppPrivate *priv;

	g_return_if_fail(loqui_account_get_is_connected(account) == FALSE);

	priv = app->priv;
	
	priv->remained_account_num--;

	if (priv->remained_account_num <= 0) {
		loqui_app_quit_idle(app);
	}
}
static gboolean
loqui_app_disconnect_all_timeout_cb(LoquiApp *app)
{
	GList *list, *cur;
	LoquiAccount *account;

	list = loqui_account_manager_get_account_list(loqui_app_get_account_manager(app));

	for (cur = list; cur != NULL; cur = cur->next) {
		account = LOQUI_ACCOUNT(cur->data);

		loqui_account_terminate(account);
	}

	loqui_app_quit_idle(app);

	return FALSE;
}
static void
loqui_app_disconnect_all_account_for_quit(LoquiApp *app)
{
	GList *list, *cur;
	LoquiAccount *account;
	LoquiAppPrivate *priv;

	priv = app->priv;
	list = loqui_account_manager_get_account_list(loqui_app_get_account_manager(app));

	priv->remained_account_num = 0;

	for (cur = list; cur != NULL; cur = cur->next) {
		account = LOQUI_ACCOUNT(cur->data);
		
		if (!loqui_account_get_is_connected(account))
			continue;

		priv->remained_account_num++;

		g_signal_connect(G_OBJECT(account), "notify::is-connected",
				 G_CALLBACK(loqui_app_account_notify_is_connected_cb_for_quit), app);
		loqui_account_disconnect(account);
	}
	g_timeout_add(DISCONNECT_WAIT_WHEN_QUIT, (GSourceFunc) loqui_app_disconnect_all_timeout_cb, app);

	if (app->priv->remained_account_num <= 0) {
		loqui_app_quit_idle(app);
	}
}
static gboolean
loqui_app_transfer_window_delete_event_cb(GtkWidget *widget, GdkEventAny *event, LoquiApp *app)
{
	g_return_val_if_fail(app != NULL, FALSE);
	g_return_val_if_fail(LOQUI_IS_APP(app), FALSE);

	gtk_widget_hide(GTK_WIDGET(app->transfer_window));

	return TRUE;
}
GtkWidget*
loqui_app_new(LoquiAccountManager *account_manager)
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

	app->appinfo = loqui_app_info_new(app);

	gtk_window_set_policy(GTK_WINDOW (app), TRUE, TRUE, TRUE);

	/* set icon */
	if((loqui_icon = gdk_pixbuf_new_from_inline(-1, loqui_pixbuf, FALSE, NULL)) == NULL)
		g_error("Failed to load loqui icon.");
	gtk_window_set_icon(GTK_WINDOW(app), loqui_icon);
	g_object_unref(loqui_icon);

	priv->ppref_channel_buffer = loqui_pref_partial_new();
	loqui_pref_partial_set_pref(priv->ppref_channel_buffer, LOQUI_CORE_GTK(loqui_get_core())->style_pref);
	loqui_pref_partial_set_group_name(priv->ppref_channel_buffer, "BufferText");

	vbox = gtk_vbox_new(FALSE, 0);
	gtk_container_add(GTK_CONTAINER(app), vbox);

	app->action_group = loqui_app_actions_create_group(app);
	toggle_command_action = gtk_action_group_get_action(app->action_group, "ToggleCommandMode");

	app->channel_entry_action_group = loqui_channel_entry_action_group_new(app,
									       loqui_app_get_account_manager(app));

	app->ui_manager = gtk_ui_manager_new();
	gtk_ui_manager_set_add_tearoffs(app->ui_manager, TRUE);
	gtk_window_add_accel_group(GTK_WINDOW(app),
				   gtk_ui_manager_get_accel_group(app->ui_manager));
	gtk_ui_manager_insert_action_group(app->ui_manager, app->action_group, 0);
	gtk_ui_manager_insert_action_group(app->ui_manager, GTK_ACTION_GROUP(app->channel_entry_action_group), 1);

	if(!gtk_ui_manager_add_ui_from_string(app->ui_manager, embedtxt_loqui_app_ui, -1, &error))
		g_error("Failed to load UI XML: %s", error->message);

	gtk_ui_manager_ensure_update(app->ui_manager);
	gtk_box_pack_start(GTK_BOX(vbox), gtk_ui_manager_get_widget(app->ui_manager, "/menubar"), FALSE, FALSE, 0);

	priv->handlebox_channelbar = gtk_handle_box_new();
	gtk_box_pack_start(GTK_BOX(vbox), priv->handlebox_channelbar, FALSE, FALSE, 0);

	priv->buffers_ui = loqui_channel_entry_action_group_ui_new(app->channel_entry_action_group, app->ui_manager, "/menubar/Buffers");
	priv->channelbar_ui = loqui_channel_entry_action_group_ui_new(app->channel_entry_action_group, app->ui_manager, "/ChannelListPopup");
	priv->trayicon_ui = loqui_channel_entry_action_group_ui_new(app->channel_entry_action_group, app->ui_manager, "/TrayIconPopup/BuffersMenu");

	menu_channelbar = loqui_channel_entry_action_group_ui_get_widget(priv->channelbar_ui);
	app->channelbar = loqui_channelbar_new(app, menu_channelbar,
					       GTK_TOGGLE_ACTION(gtk_action_group_get_action(app->action_group, LOQUI_ACTION_TOGGLE_SCROLL)));
	gtk_container_add(GTK_CONTAINER(priv->handlebox_channelbar), app->channelbar);

#define SET_SCROLLED_WINDOW(s, w, vpolicy, hpolicy) \
{ \
	s = gtk_scrolled_window_new(NULL, NULL); \
        gtk_scrolled_window_set_policy(GTK_SCROLLED_WINDOW(s), vpolicy, hpolicy); \
        gtk_scrolled_window_set_shadow_type(GTK_SCROLLED_WINDOW(s), GTK_SHADOW_IN); \
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
							  loqui_pref_get_with_default_boolean(loqui_get_general_pref(),
											      LOQUI_GENERAL_PREF_GTK_GROUP_GENERAL, "AutoSwitchScrollingCommonBuffer",
											      LOQUI_GENERAL_PREF_GTK_DEFAULT_GENERAL_AUTO_SWITCH_SCROLLING_COMMON_BUFFER, NULL));
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

	priv->common_buffer = loqui_channel_buffer_gtk_new(priv->ppref_channel_buffer);
	loqui_channel_buffer_gtk_set_whether_common_buffer(priv->common_buffer, TRUE);
	loqui_channel_buffer_gtk_set_show_channel_name(priv->common_buffer, TRUE);
	loqui_channel_buffer_gtk_set_show_account_name(priv->common_buffer, TRUE);
	loqui_channel_text_view_set_channel_buffer(LOQUI_CHANNEL_TEXT_VIEW(app->common_textview), priv->common_buffer);

	loqui_app_info_update_all(app->appinfo, NULL);
	loqui_app_restore_size(app);

#undef SET_SCROLLED_WINDOW

	gtk_widget_show_all(GTK_WIDGET(app));

	loqui_app_set_show_statusbar(app,
				     loqui_pref_get_with_default_boolean(loqui_get_general_pref(),
									 LOQUI_GENERAL_PREF_GTK_GROUP_VISIBILITY, "ShowStatusbar",
									 LOQUI_GENERAL_PREF_GTK_DEFAULT_VISIBILITY_SHOW_STATUSBAR, NULL));
	loqui_app_set_show_channelbar(app,
				      loqui_pref_get_with_default_boolean(loqui_get_general_pref(),
									  LOQUI_GENERAL_PREF_GTK_GROUP_VISIBILITY, "ShowChannelbar",
									  LOQUI_GENERAL_PREF_GTK_DEFAULT_VISIBILITY_SHOW_CHANNELBAR, NULL));

	loqui_app_actions_toggle_action_set_active(app, "ToggleStatusbar",
						   loqui_pref_get_with_default_boolean(loqui_get_general_pref(),
										       LOQUI_GENERAL_PREF_GTK_GROUP_VISIBILITY, "ShowStatusbar",
										       LOQUI_GENERAL_PREF_GTK_DEFAULT_VISIBILITY_SHOW_STATUSBAR, NULL));
	loqui_app_actions_toggle_action_set_active(app, "ToggleChannelbar",
						   loqui_pref_get_with_default_boolean(loqui_get_general_pref(),
										       LOQUI_GENERAL_PREF_GTK_GROUP_VISIBILITY, "ShowChannelbar",
										       LOQUI_GENERAL_PREF_GTK_DEFAULT_VISIBILITY_SHOW_CHANNELBAR, NULL));
	loqui_app_set_nick_list_sort_type(app,
					  loqui_pref_get_with_default_integer(loqui_get_general_pref(),
									      LOQUI_GENERAL_PREF_GTK_GROUP_GENERAL, "NickListSortType",
									      LOQUI_GENERAL_PREF_GTK_DEFAULT_GENERAL_NICK_LIST_SORT_TYPE, NULL));

	g_signal_connect_after(G_OBJECT(account_manager), "add-account",
			       G_CALLBACK(loqui_app_add_account_after_cb), app);
	g_signal_connect(G_OBJECT(account_manager), "remove-account",
			 G_CALLBACK(loqui_app_remove_account_cb), app);

	gtk_widget_grab_focus(app->remark_entry);

	loqui_app_create_tray_icon(app);

	app->transfer_window = LOQUI_TRANSFER_WINDOW(loqui_transfer_window_new());
	g_signal_connect(G_OBJECT(app->transfer_window), "delete_event",
			 G_CALLBACK(loqui_app_transfer_window_delete_event_cb), app);

	loqui_app_actions_update_sensitivity_related_channel(app);

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
	loqui_pref_set_boolean(loqui_get_general_pref(),
                               LOQUI_GENERAL_PREF_GTK_GROUP_GENERAL, "AutoSwitchScrolling",
                               auto_switch_scrolling);
}
void
loqui_app_set_auto_switch_scrolling_common_buffer(LoquiApp *app, gboolean auto_switch_scrolling)
{
	loqui_channel_text_view_set_auto_switch_scrolling(LOQUI_CHANNEL_TEXT_VIEW(app->common_textview), auto_switch_scrolling);
	loqui_pref_set_boolean(loqui_get_general_pref(),
                               LOQUI_GENERAL_PREF_GTK_GROUP_GENERAL, "AutoSwitchScrollingCommonBuffer",
                               auto_switch_scrolling);
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

	loqui_pref_set_boolean(loqui_get_general_pref(),
                               LOQUI_GENERAL_PREF_GTK_GROUP_VISIBILITY, "ShowStatusbar",
                               show);
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

	loqui_pref_set_boolean(loqui_get_general_pref(),
                               LOQUI_GENERAL_PREF_GTK_GROUP_VISIBILITY, "ShowChannelbar",
                               show);
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

LoquiAccountManager *
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

	if (app->current_channel_entry)
		chview = g_object_get_data(G_OBJECT(app->current_channel_entry), CHANNEL_TEXT_VIEW_KEY);

	return chview;
}

LoquiChannelEntry *
loqui_app_get_current_channel_entry(LoquiApp *app)
{
	return app->current_channel_entry;
}
void
loqui_app_set_current_channel_entry(LoquiApp *app, LoquiChannelEntry *chent)
{
	LoquiAppPrivate *priv;
	GtkTreeModel *model;
	LoquiAccount *account;
	LoquiChannel *channel;
	LoquiChannelTextView *chview;
	LoquiChannelEntry *old_chent;

        g_return_if_fail(app != NULL);
        g_return_if_fail(LOQUI_IS_APP(app));
	g_return_if_fail(chent != NULL);
	g_return_if_fail(LOQUI_IS_CHANNEL_ENTRY(chent));
	
	priv = app->priv;

	old_chent = app->current_channel_entry;
	
	if (old_chent) {
		chview = g_object_get_data(G_OBJECT(old_chent), CHANNEL_TEXT_VIEW_KEY);
		g_assert(chview != NULL);
		g_signal_handlers_disconnect_by_func(chview,
						     loqui_app_channel_text_view_scrolled_to_end_cb,
						     old_chent);
		g_signal_handlers_disconnect_by_func(chview,
						     loqui_app_channel_text_view_notify_is_scroll_cb,
						     app);
	}
	
	loqui_channel_entry_utils_separate(chent, &account, &channel);

	app->current_channel_entry = chent;

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

	loqui_app_info_current_channel_entry_changed(app->appinfo, old_chent, chent);

	loqui_channel_entry_set_as_read(chent);

	channel_tree_select_channel_entry(app->channel_tree, chent);

	loqui_channelbar_update_channel_entry_label(LOQUI_CHANNELBAR(app->channelbar), chent);

	loqui_app_actions_update_sensitivity_related_channel(app);

	gtk_widget_grab_focus(app->remark_entry);

	if (loqui_pref_get_with_default_boolean(loqui_get_general_pref(),
						LOQUI_GENERAL_PREF_GTK_GROUP_GENERAL, "AutoSwitchScrolling",
						LOQUI_GENERAL_PREF_GTK_DEFAULT_GENERAL_AUTO_SWITCH_SCROLLING, NULL))
		loqui_app_actions_toggle_action_set_active(app, LOQUI_ACTION_TOGGLE_SCROLL, TRUE);

}

LoquiChannel *
loqui_app_get_current_channel(LoquiApp *app)
{
	LoquiChannel *channel;

	loqui_channel_entry_utils_separate(app->current_channel_entry, NULL, &channel);

	return channel;
}
LoquiAccount *
loqui_app_get_current_account(LoquiApp *app)
{
	LoquiAccount *account;
	
	loqui_channel_entry_utils_separate(app->current_channel_entry, &account, NULL);

	return account;
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
loqui_app_is_current_account(LoquiApp *app, LoquiAccount *account)
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
loqui_app_channel_entry_added_after(LoquiApp *app, LoquiChannelEntry *chent)
{
	LoquiAppPrivate *priv;
	LoquiChannelEntryStore *store;
	LoquiChannelBuffer *buffer;
	GtkWidget *chview;

        g_return_if_fail(app != NULL);
        g_return_if_fail(LOQUI_IS_APP(app));
	g_return_if_fail(chent != NULL);
	g_return_if_fail(LOQUI_IS_CHANNEL_ENTRY(chent));

	priv = app->priv;

	store = loqui_channel_entry_store_new(chent);
	g_object_set_data(G_OBJECT(chent), CHANNEL_ENTRY_STORE_KEY, store);

	buffer = LOQUI_CHANNEL_BUFFER(loqui_channel_buffer_gtk_new(priv->ppref_channel_buffer));
	loqui_channel_entry_set_buffer(chent, buffer);

	chview = loqui_channel_text_view_new(app);
	g_object_set_data(G_OBJECT(chent), CHANNEL_TEXT_VIEW_KEY, chview);
	loqui_channel_text_view_set_channel_buffer(LOQUI_CHANNEL_TEXT_VIEW(chview), LOQUI_CHANNEL_BUFFER_GTK(buffer));
	loqui_channel_text_view_set_auto_switch_scrolling(LOQUI_CHANNEL_TEXT_VIEW(chview),
							  loqui_pref_get_with_default_boolean(loqui_get_general_pref(),
											      LOQUI_GENERAL_PREF_GTK_GROUP_GENERAL, "AutoSwitchScrolling",
											      LOQUI_GENERAL_PREF_GTK_DEFAULT_GENERAL_AUTO_SWITCH_SCROLLING, NULL));

	gtk_widget_show_all(LOQUI_CHANNEL_TEXT_VIEW(chview)->scrolled_window);
	gtk_notebook_append_page(GTK_NOTEBOOK(app->channel_notebook),
				 LOQUI_CHANNEL_TEXT_VIEW(chview)->scrolled_window,
				 NULL);

	loqui_channel_entry_set_sort_func(LOQUI_CHANNEL_ENTRY(chent), priv->sort_func);

	loqui_app_info_channel_entry_added(app->appinfo, chent);

	g_signal_connect_after(G_OBJECT(chent), "append-message-text",
			       G_CALLBACK(loqui_app_channel_entry_append_message_text_after_cb), app);
}
static void
loqui_app_channel_entry_removed(LoquiApp *app, LoquiChannelEntry *chent)
{
	LoquiChannelEntryStore *store;
	GtkWidget *chview;

        g_return_if_fail(app != NULL);
        g_return_if_fail(LOQUI_IS_APP(app));
	g_return_if_fail(chent != NULL);
	g_return_if_fail(LOQUI_IS_CHANNEL_ENTRY(chent));

	loqui_app_info_channel_entry_removed(app->appinfo, chent);

	g_signal_handlers_disconnect_by_func(chent, loqui_app_channel_entry_append_message_text_after_cb, app);

	chview = g_object_get_data(G_OBJECT(chent), CHANNEL_TEXT_VIEW_KEY);
	gtk_notebook_remove_page(GTK_NOTEBOOK(app->channel_notebook),
				 gtk_notebook_page_num(GTK_NOTEBOOK(app->channel_notebook), LOQUI_CHANNEL_TEXT_VIEW(chview)->scrolled_window));

	store = g_object_get_data(G_OBJECT(chent), CHANNEL_ENTRY_STORE_KEY);
	g_object_unref(store);
}
static void
loqui_app_add_account_after_cb(LoquiAccountManager *manager, LoquiAccount *account, LoquiApp *app)
{
	LoquiAppPrivate *priv;

        g_return_if_fail(manager != NULL);
        g_return_if_fail(LOQUI_IS_ACCOUNT_MANAGER(manager));
	g_return_if_fail(account != NULL);
	g_return_if_fail(LOQUI_IS_ACCOUNT(account));
        g_return_if_fail(app != NULL);
        g_return_if_fail(LOQUI_IS_APP(app));

	priv = app->priv;

	g_signal_connect_after(G_OBJECT(account), "add-channel",
			       G_CALLBACK(loqui_app_add_channel_after_cb), app);
	g_signal_connect(G_OBJECT(account), "remove-channel",
			 G_CALLBACK(loqui_app_remove_channel_cb), app);


	loqui_app_channel_entry_added_after(app, LOQUI_CHANNEL_ENTRY(account));
	loqui_app_info_account_added(app->appinfo, account);
}
static void
loqui_app_remove_account_cb(LoquiAccountManager *manager, LoquiAccount *account, LoquiApp *app)
{
	LoquiAppPrivate *priv;

        g_return_if_fail(manager != NULL);
        g_return_if_fail(LOQUI_IS_ACCOUNT_MANAGER(manager));
        g_return_if_fail(app != NULL);
        g_return_if_fail(LOQUI_IS_APP(app));
	g_return_if_fail(account != NULL);
	g_return_if_fail(LOQUI_IS_ACCOUNT(account));

	priv = app->priv;

	loqui_app_channel_entry_removed(app, LOQUI_CHANNEL_ENTRY(account));
	loqui_app_info_account_removed(app->appinfo, account);

	g_signal_handlers_disconnect_by_func(G_OBJECT(account), loqui_app_add_channel_after_cb, app);
	g_signal_handlers_disconnect_by_func(G_OBJECT(account), loqui_app_remove_channel_cb, app);
}
static void
loqui_app_add_channel_after_cb(LoquiAccount *account, LoquiChannel *channel, LoquiApp *app)
{
	LoquiAppPrivate *priv;

        g_return_if_fail(app != NULL);
        g_return_if_fail(LOQUI_IS_APP(app));
	g_return_if_fail(account != NULL);
	g_return_if_fail(LOQUI_IS_ACCOUNT(account));
	g_return_if_fail(channel != NULL);
	g_return_if_fail(LOQUI_IS_CHANNEL(channel));

	priv = app->priv;

	loqui_app_channel_entry_added_after(app, LOQUI_CHANNEL_ENTRY(channel));
	loqui_app_info_channel_added(app->appinfo, channel);

	if (loqui_pref_get_with_default_boolean(loqui_get_general_pref(),
						LOQUI_GENERAL_PREF_GTK_GROUP_GENERAL, "SelectChannelJoined",
						LOQUI_GENERAL_PREF_GTK_DEFAULT_GENERAL_SELECT_CHANNEL_JOINED, NULL))
		loqui_app_set_current_channel_lazy(app, channel);
	else
		channel_tree_expand_to_channel_entry(app->channel_tree, LOQUI_CHANNEL_ENTRY(channel));

}
static void
loqui_app_remove_channel_cb(LoquiAccount *account, LoquiChannel *channel, LoquiApp *app)
{
	LoquiAppPrivate *priv;

        g_return_if_fail(app != NULL);
        g_return_if_fail(LOQUI_IS_APP(app));
	g_return_if_fail(account != NULL);
	g_return_if_fail(LOQUI_IS_ACCOUNT(account));
	g_return_if_fail(channel != NULL);
	g_return_if_fail(LOQUI_IS_CHANNEL(channel));

	priv = app->priv;

	if (loqui_app_get_current_channel(app) == channel) {
		loqui_app_set_current_channel_entry(app, LOQUI_CHANNEL_ENTRY(account));
	}

	loqui_app_channel_entry_removed(app, LOQUI_CHANNEL_ENTRY(channel));
	loqui_app_info_channel_removed(app->appinfo, channel);
}
static void
loqui_app_channel_entry_append_message_text_after_cb(LoquiChannelEntry *chent, LoquiMessageText *msgtext, LoquiApp *app)
{
	LoquiAppPrivate *priv;
	GtkWidget *chview;

        g_return_if_fail(app != NULL);
        g_return_if_fail(LOQUI_IS_APP(app));

	priv = app->priv;

	if (priv->last_msgtext == msgtext)
		return;
		
	if (loqui_pref_get_with_default_boolean(loqui_get_general_pref(),
						LOQUI_GENERAL_PREF_GTK_GROUP_GENERAL, "SaveLog",
						LOQUI_GENERAL_PREF_GTK_DEFAULT_GENERAL_SAVE_LOG, NULL))
		loqui_app_append_log(app, msgtext);

	chview = loqui_app_get_current_channel_text_view(app);
	if (chview && loqui_channel_text_view_get_is_scroll(LOQUI_CHANNEL_TEXT_VIEW(chview)) &&
	    app->current_channel_entry != NULL &&
	    loqui_channel_entry_get_buffer(chent) == loqui_channel_entry_get_buffer(app->current_channel_entry))
		return;
	
	loqui_channel_buffer_append_message_text(LOQUI_CHANNEL_BUFFER(priv->common_buffer), msgtext);

	if (priv->last_msgtext)
		g_object_unref(priv->last_msgtext);
	g_object_ref(msgtext);
	priv->last_msgtext = msgtext;
}

static void
loqui_app_append_log(LoquiApp *app, LoquiMessageText *msgtext)
{
	gchar *path;
	gchar *filename;
	gchar *buf;
	gchar *time_str;
	gchar *nick;
	const gchar *account_name;
	GIOChannel *io;
	time_t t;
	gchar *time_format;

        g_return_if_fail(app != NULL);
        g_return_if_fail(LOQUI_IS_APP(app));
        
        t = time(NULL);
        
        filename = loqui_utils_strftime_epoch("log-%Y%m%d.txt", t);
	path = g_build_filename(loqui_core_get_user_dir(loqui_get_core()), LOG_DIR, filename, NULL);
	g_free(filename);
	
	if ((io = g_io_channel_new_file(path, "a", NULL)) == NULL) {
		g_warning("Can't open log file(%s)", path);
		g_free(path);
		return;
	}
	
	time_format = loqui_pref_get_with_default_string(loqui_get_general_pref(),
							 LOQUI_GENERAL_PREF_GTK_GROUP_GENERAL, "TimeFormat",
							 LOQUI_GENERAL_PREF_GTK_DEFAULT_GENERAL_TIME_FORMAT, NULL);
	time_str = loqui_utils_strftime_epoch(time_format, t);
	g_free(time_format);

	if (loqui_message_text_get_nick(msgtext))
		nick = loqui_message_text_get_nick_string(msgtext, TRUE);
	else
		nick = g_strdup("");
	
	account_name = loqui_message_text_get_account_name(msgtext);
	
	if (account_name)
		buf = g_strdup_printf("%s[%s] %s\n", time_str, account_name, loqui_message_text_get_text(msgtext));
	else
		buf = g_strdup_printf("%s%s%s\n", time_str, nick, loqui_message_text_get_text(msgtext));
	g_free(time_str);
	g_free(nick);
		
	if (g_io_channel_write_chars(io, buf, -1, NULL, NULL) == 0)
		g_warning("Can't write log(%s)", path);
	
	g_free(path);
	g_free(buf);
	g_io_channel_unref(io);
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

	loqui_pref_set_integer(loqui_get_general_pref(),
                               LOQUI_GENERAL_PREF_GTK_GROUP_GENERAL, "NickListSortType",
                               sort_type);
}
