/* -*- Mode: C; tab-width: 8; indent-tabs-mode: t; c-basic-offset: 8 -*- */
/*
 * Loqui -- Chat client for Gtk2 <http://loqui.good-day.net/>
 * Copyright (C) 2004 Yoichi Imai <yoichi@silver-forest.com>
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

#include "loqui_channel_text_view.h"
#include "gtkutils.h"
#include "intl.h"

#include <gdk/gdkkeysyms.h>

#include <loqui.h>
#include "loqui-general-pref-gtk-groups.h"
#include "loqui-general-pref-gtk-default.h"

#define EPS 0.00000001

enum {
	SIGNAL_SCROLLED_TO_END,
        LAST_SIGNAL
};

enum {
	PROP_0,
	PROP_IS_SCROLL,
	PROP_AUTO_SWITCH_SCROLLING,
        LAST_PROP
};

struct _LoquiChannelTextViewPrivate
{
	LoquiApp *app;

	GdkCursor *hand_cursor;
	GdkCursor *normal_cursor;

	gboolean is_hand_cursor;
};

static GtkTextViewClass *parent_class = NULL;

static guint channel_text_view_signals[LAST_SIGNAL] = { 0 };

static void loqui_channel_text_view_class_init(LoquiChannelTextViewClass *klass);
static void loqui_channel_text_view_init(LoquiChannelTextView *chview);
static void loqui_channel_text_view_finalize(GObject *object);
static void loqui_channel_text_view_dispose(GObject *object);

static void loqui_channel_text_view_get_property(GObject *object, guint param_id, GValue *value, GParamSpec *pspec);
static void loqui_channel_text_view_set_property(GObject *object, guint param_id, const GValue *value, GParamSpec *pspec);

static void loqui_channel_text_view_destroy(GtkObject *object);

static void loqui_channel_text_view_vadj_value_changed_cb(GtkAdjustment *adj, gpointer data);
static gboolean loqui_channel_text_view_key_press_event(GtkWidget *widget,
							GdkEventKey *event);
static void loqui_channel_text_view_buffer_insert_text_cb(GtkTextBuffer *textbuf,
							  GtkTextIter *pos,
							  const gchar *text,
							  gint length,
							  gpointer data);
static gboolean loqui_channel_text_view_motion_notify_event(GtkWidget *widget, GdkEventMotion *event);
static gboolean loqui_channel_text_view_visibility_notify_event(GtkWidget *widget, GdkEventVisibility *event);
static gboolean loqui_channel_text_view_button_release_event(GtkWidget *widget, GdkEventButton *event_button);

/* utilities */
static gboolean loqui_channel_text_view_get_buffer_and_iter_at_event_xy(LoquiChannelTextView *chview,
									LoquiChannelBufferGtk **buffer_gtk, GtkTextIter *iter,
									gint event_x, gint event_y);
static gboolean loqui_channel_text_view_get_uri_at_iter(LoquiChannelTextView *chview,
							LoquiChannelBufferGtk *buffer_gtk,
							GtkTextIter *iter,
							GtkTextIter *uri_start_iter,
							GtkTextIter *uri_end_iter);

static void loqui_channel_text_view_iter_activated(LoquiChannelTextView *chview, LoquiChannelBufferGtk *buffer_gtk, GtkTextIter *iter);
static void loqui_channel_text_view_update_hover(LoquiChannelTextView *chview, LoquiChannelBufferGtk *buffer_gtk, GtkTextIter *iter);
static void loqui_channel_text_view_update_cursor(LoquiChannelTextView *chview, gint event_x, gint event_y);

GType
loqui_channel_text_view_get_type(void)
{
	static GType type = 0;
	if (type == 0) {
		static const GTypeInfo our_info =
			{
				sizeof(LoquiChannelTextViewClass),
				NULL,           /* base_init */
				NULL,           /* base_finalize */
				(GClassInitFunc) loqui_channel_text_view_class_init,
				NULL,           /* class_finalize */
				NULL,           /* class_data */
				sizeof(LoquiChannelTextView),
				0,              /* n_preallocs */
				(GInstanceInitFunc) loqui_channel_text_view_init
			};
		
		type = g_type_register_static(GTK_TYPE_TEXT_VIEW,
					      "LoquiChannelTextView",
					      &our_info,
					      0);
	}
	
	return type;
}
static void 
loqui_channel_text_view_finalize(GObject *object)
{
	LoquiChannelTextView *view;

        g_return_if_fail(object != NULL);
        g_return_if_fail(LOQUI_IS_CHANNEL_TEXT_VIEW(object));

        view = LOQUI_CHANNEL_TEXT_VIEW(object);

        if (G_OBJECT_CLASS(parent_class)->finalize)
                (* G_OBJECT_CLASS(parent_class)->finalize)(object);

	g_free(view->priv);
}
static void 
loqui_channel_text_view_dispose(GObject *object)
{
	LoquiChannelTextView *view;

        g_return_if_fail(object != NULL);
        g_return_if_fail(LOQUI_IS_CHANNEL_TEXT_VIEW(object));

        view = LOQUI_CHANNEL_TEXT_VIEW(object);

        if (G_OBJECT_CLASS(parent_class)->dispose)
                (* G_OBJECT_CLASS(parent_class)->dispose)(object);
}
static void
loqui_channel_text_view_get_property(GObject *object, guint param_id, GValue *value, GParamSpec *pspec)
{
        LoquiChannelTextView *view;        

        view = LOQUI_CHANNEL_TEXT_VIEW(object);

        switch (param_id) {
	case PROP_IS_SCROLL:
		g_value_set_boolean(value, view->is_scroll);
		break;
	case PROP_AUTO_SWITCH_SCROLLING:
		g_value_set_boolean(value, view->auto_switch_scrolling);
		break;
        default:
                G_OBJECT_WARN_INVALID_PROPERTY_ID(object, param_id, pspec);
                break;
        }
}
static void
loqui_channel_text_view_set_property(GObject *object, guint param_id, const GValue *value, GParamSpec *pspec)
{
        LoquiChannelTextView *view;        

        view = LOQUI_CHANNEL_TEXT_VIEW(object);

        switch (param_id) {
	case PROP_IS_SCROLL:
		loqui_channel_text_view_set_is_scroll(view, g_value_get_boolean(value));
		break;
	case PROP_AUTO_SWITCH_SCROLLING:
		loqui_channel_text_view_set_auto_switch_scrolling(view, g_value_get_boolean(value));
		break;
        default:
                G_OBJECT_WARN_INVALID_PROPERTY_ID(object, param_id, pspec);
                break;
        }
}

static void
loqui_channel_text_view_class_init(LoquiChannelTextViewClass *klass)
{
        GObjectClass *object_class = G_OBJECT_CLASS(klass);
	GtkWidgetClass *widget_class = GTK_WIDGET_CLASS(klass);

        parent_class = g_type_class_peek_parent(klass);
        
        object_class->finalize = loqui_channel_text_view_finalize;
        object_class->dispose = loqui_channel_text_view_dispose;
        object_class->get_property = loqui_channel_text_view_get_property;
        object_class->set_property = loqui_channel_text_view_set_property;
        GTK_OBJECT_CLASS(klass)->destroy = loqui_channel_text_view_destroy;

	widget_class->key_press_event = loqui_channel_text_view_key_press_event;
	widget_class->motion_notify_event = loqui_channel_text_view_motion_notify_event;
	widget_class->visibility_notify_event = loqui_channel_text_view_visibility_notify_event;
	widget_class->button_release_event = loqui_channel_text_view_button_release_event;

	g_object_class_install_property(object_class,
					PROP_IS_SCROLL,
					g_param_spec_boolean("is_scroll",
							     _("IsScroll"),
							     _("Scrolling or not"),
							     TRUE,
							     G_PARAM_READWRITE));
	g_object_class_install_property(object_class,
					PROP_IS_SCROLL,
					g_param_spec_boolean("auto_switch_scrolling",
							     _("Auto switch scrolling"),
							     _("Switch scrolling automatically"),
							     TRUE,
							     G_PARAM_READWRITE));
	
        channel_text_view_signals[SIGNAL_SCROLLED_TO_END] = g_signal_new("scrolled_to_end",
									 G_OBJECT_CLASS_TYPE(object_class),
									 G_SIGNAL_RUN_FIRST,
									 G_STRUCT_OFFSET(LoquiChannelTextViewClass, scrolled_to_end),
									 NULL, NULL,
									 g_cclosure_marshal_VOID__VOID,
									 G_TYPE_NONE, 0);
}
static void 
loqui_channel_text_view_init(LoquiChannelTextView *chview)
{
	LoquiChannelTextViewPrivate *priv;

	priv = g_new0(LoquiChannelTextViewPrivate, 1);

	chview->priv = priv;
	
	chview->is_scroll = TRUE;

	priv->is_hand_cursor = FALSE;
}
static void 
loqui_channel_text_view_destroy(GtkObject *object)
{
        LoquiChannelTextView *view;

        g_return_if_fail(object != NULL);
        g_return_if_fail(LOQUI_IS_CHANNEL_TEXT_VIEW(object));

        view = LOQUI_CHANNEL_TEXT_VIEW(object);

        if (GTK_OBJECT_CLASS(parent_class)->destroy)
                (* GTK_OBJECT_CLASS(parent_class)->destroy)(object);
}
static gboolean
loqui_channel_text_view_key_press_event(GtkWidget *widget,
					GdkEventKey *event)
{
	GtkTextBuffer *buffer;
	GtkTextIter iter;

	if (event->keyval == GDK_Return ||
	    event->keyval == GDK_KP_Enter) {
		buffer = gtk_text_view_get_buffer(GTK_TEXT_VIEW(widget));
		gtk_text_buffer_get_iter_at_mark(buffer, &iter,
						 gtk_text_buffer_get_insert(buffer));

		g_return_val_if_fail(LOQUI_IS_CHANNEL_BUFFER_GTK(buffer), FALSE);

		loqui_channel_text_view_iter_activated(LOQUI_CHANNEL_TEXT_VIEW(widget),
						       LOQUI_CHANNEL_BUFFER_GTK(buffer),
						       &iter);
		return FALSE;
	}

	loqui_app_grab_focus_if_key_unused(LOQUI_CHANNEL_TEXT_VIEW(widget)->priv->app,
					   "GtkTextView", event);

	if (* GTK_WIDGET_CLASS(parent_class)->key_press_event)
		return (* GTK_WIDGET_CLASS(parent_class)->key_press_event)(widget, event);

	return FALSE;
}
static void
loqui_channel_text_view_vadj_value_changed_cb(GtkAdjustment *adj, gpointer data)
{
	LoquiChannelTextView *chview;
	gboolean reached_to_end;

        g_return_if_fail(data != NULL);
        g_return_if_fail(LOQUI_IS_CHANNEL_TEXT_VIEW(data));

	chview = LOQUI_CHANNEL_TEXT_VIEW(data);

	/* upper - page_size is max virtually. */
	reached_to_end = (ABS(adj->upper - adj->page_size - adj->value) < EPS);

	if (reached_to_end)
		g_signal_emit(G_OBJECT(chview), channel_text_view_signals[SIGNAL_SCROLLED_TO_END], 0);
	
	if (chview->auto_switch_scrolling) {
		loqui_channel_text_view_set_is_scroll(chview, reached_to_end);
	}
}

static void
loqui_channel_text_view_buffer_insert_text_cb(GtkTextBuffer *textbuf,
					      GtkTextIter *pos,
					      const gchar *text,
					      gint length,
					      gpointer data)
{
        LoquiChannelTextView *chview;
	LoquiChannelTextViewPrivate *priv;
	
        g_return_if_fail(data != NULL);
        g_return_if_fail(LOQUI_IS_CHANNEL_TEXT_VIEW(data));
	
	chview = LOQUI_CHANNEL_TEXT_VIEW(data);
	priv = chview->priv;
	
	loqui_channel_text_view_scroll_to_end_if_enabled(chview);
}
static gboolean
loqui_channel_text_view_motion_notify_event(GtkWidget *widget, GdkEventMotion *event)
{
	LoquiChannelTextView *chview;

	chview = LOQUI_CHANNEL_TEXT_VIEW(widget);

	loqui_channel_text_view_update_cursor(chview, event->x, event->y);

	if (GTK_WIDGET_CLASS(parent_class)->motion_notify_event)
               return (* GTK_WIDGET_CLASS(parent_class)->motion_notify_event) (widget, event);

	return FALSE;
}
static gboolean
loqui_channel_text_view_button_release_event(GtkWidget *widget, GdkEventButton *event_button)
{
	LoquiChannelTextView *chview;
	LoquiChannelTextViewPrivate *priv;
	GtkTextBuffer *buffer;
	LoquiChannelBufferGtk *buffer_gtk;
	GtkTextIter iter;
	GtkTextIter selected_start_iter, selected_end_iter;
	gboolean ret = FALSE;

	chview = LOQUI_CHANNEL_TEXT_VIEW(widget);
        priv = chview->priv;
	
	if (GTK_WIDGET_CLASS(parent_class)->button_release_event)
		ret = (* GTK_WIDGET_CLASS(parent_class)->button_release_event) (widget, event_button);

	if (event_button->button != 1)
		return ret;
	
	if (!loqui_channel_text_view_get_buffer_and_iter_at_event_xy(chview, &buffer_gtk, &iter, event_button->x, event_button->y))
		return ret;

	buffer = GTK_TEXT_BUFFER(buffer_gtk);

	/* return if selected */
	gtk_text_buffer_get_selection_bounds(buffer, &selected_start_iter, &selected_end_iter);
	if (!gtk_text_iter_equal(&selected_start_iter, &selected_end_iter))
		return ret;

	loqui_channel_text_view_iter_activated(chview, buffer_gtk, &iter);

	return ret;
}
static gboolean
loqui_channel_text_view_visibility_notify_event(GtkWidget *widget, GdkEventVisibility *event)
{
	gint event_x, event_y;
	LoquiChannelTextView *chview;

        g_return_val_if_fail(widget != NULL, FALSE);
        g_return_val_if_fail(LOQUI_IS_CHANNEL_TEXT_VIEW(widget), FALSE);

	chview = LOQUI_CHANNEL_TEXT_VIEW(widget);

	gdk_window_get_pointer(widget->window, &event_x, &event_y, NULL);
	loqui_channel_text_view_update_cursor(chview, event_x, event_y);
	
	if (GTK_WIDGET_CLASS(parent_class)->visibility_notify_event)
               return (* GTK_WIDGET_CLASS(parent_class)->visibility_notify_event) (widget, event);

	return FALSE;
}
static gboolean
loqui_channel_text_view_get_buffer_and_iter_at_event_xy(LoquiChannelTextView *chview,
							LoquiChannelBufferGtk **buffer_gtk, GtkTextIter *iter,
							gint event_x, gint event_y)
{
	gint x, y;
	GtkTextBuffer *buffer;

        g_return_val_if_fail(chview != NULL, FALSE);
        g_return_val_if_fail(LOQUI_IS_CHANNEL_TEXT_VIEW(chview), FALSE);
	g_return_val_if_fail(buffer_gtk != NULL, FALSE);
	g_return_val_if_fail(iter != NULL, FALSE);

	buffer = gtk_text_view_get_buffer(GTK_TEXT_VIEW(chview));
	if (!LOQUI_IS_CHANNEL_BUFFER_GTK(buffer))
		return FALSE;

	*buffer_gtk = LOQUI_CHANNEL_BUFFER_GTK(buffer);

	gtk_text_view_window_to_buffer_coords(GTK_TEXT_VIEW(chview), 
					      GTK_TEXT_WINDOW_WIDGET,
					      event_x, event_y, &x, &y);

	gtk_text_view_get_iter_at_location(GTK_TEXT_VIEW(chview), iter, x, y);

	return TRUE;
}
static gboolean
loqui_channel_text_view_get_uri_at_iter(LoquiChannelTextView *chview,
					LoquiChannelBufferGtk *buffer_gtk,
					GtkTextIter *iter,
					GtkTextIter *uri_start_iter,
					GtkTextIter *uri_end_iter)
{
	GtkTextBuffer *buffer;
	GtkTextTag *tag_link;
	gboolean in_uri;

	buffer = GTK_TEXT_BUFFER(buffer_gtk);

	tag_link = gtk_text_tag_table_lookup(gtk_text_buffer_get_tag_table(buffer), "link");
	g_return_val_if_fail(tag_link != NULL, FALSE);

	in_uri = gtk_text_iter_has_tag(iter, tag_link);

	if (!in_uri)
		return FALSE;
	
	if (uri_start_iter) {
		*uri_start_iter = *iter;
		if (!gtk_text_iter_begins_tag(uri_start_iter, tag_link) &&
		    !gtk_text_iter_backward_to_tag_toggle(uri_start_iter, tag_link)) {
			debug_puts("Can't find start.");
			return FALSE;
		}
	}

	if (uri_end_iter) {
		*uri_end_iter = *iter;
		if (!gtk_text_iter_ends_tag(uri_end_iter, tag_link) &&
		    !gtk_text_iter_forward_to_tag_toggle(uri_end_iter, tag_link)) {
			debug_puts("Can't find end");
			return FALSE;
		}
	}

	return TRUE;
}
static void
loqui_channel_text_view_iter_activated(LoquiChannelTextView *chview, LoquiChannelBufferGtk *buffer_gtk, GtkTextIter *iter)
{
	gchar *uri_str;
	GtkTextIter uri_start_iter, uri_end_iter;
	gchar *browser_command;

        g_return_if_fail(chview != NULL);
        g_return_if_fail(LOQUI_IS_CHANNEL_TEXT_VIEW(chview));
        g_return_if_fail(buffer_gtk != NULL);
        g_return_if_fail(LOQUI_IS_CHANNEL_BUFFER_GTK(buffer_gtk));

	browser_command = loqui_pref_get_with_default_string(loqui_get_general_pref(),
							     LOQUI_GENERAL_PREF_GTK_GROUP_COMMANDS, "BrowserCommand",
							     LOQUI_GENERAL_PREF_GTK_DEFAULT_COMMANDS_BROWSER_COMMAND, NULL);
	if (browser_command) {
		if (loqui_channel_text_view_get_uri_at_iter(chview, buffer_gtk, iter, &uri_start_iter, &uri_end_iter)) {
			uri_str = gtk_text_iter_get_text(&uri_start_iter, &uri_end_iter);
			gtkutils_exec_command_argument_with_error_dialog(browser_command, uri_str);
			g_free(uri_str);
		}
		g_free(browser_command);
	} else {
		g_warning(_("Failed to get browser command."));
	}
}
/* iter == NULL: forced to remove hover underline */
static void
loqui_channel_text_view_update_hover(LoquiChannelTextView *chview, LoquiChannelBufferGtk *buffer_gtk, GtkTextIter *iter)
{
	gboolean in_uri;
	GtkTextIter uri_start_iter, uri_end_iter;
	GtkTextIter old_uri_start_iter, old_uri_end_iter;
	GtkTextTag *hover_tag;
	GtkTextMark *hover_mark;
	GtkTextBuffer *buffer;
	GtkTextIter old_hover_iter;

	if (iter)
		in_uri = loqui_channel_text_view_get_uri_at_iter(chview, buffer_gtk, iter, &uri_start_iter, &uri_end_iter);
	else
		in_uri = FALSE;

	if (!buffer_gtk->hover_tag_applied && !in_uri)
		return;

	buffer = GTK_TEXT_BUFFER(buffer_gtk);

	hover_tag = gtk_text_tag_table_lookup(gtk_text_buffer_get_tag_table(buffer), "hover");
	g_return_if_fail(hover_tag != NULL);

	hover_mark = gtk_text_buffer_get_mark(buffer, "hover");
	g_return_if_fail(hover_mark != NULL);

	if (buffer_gtk->hover_tag_applied) {
		gtk_text_buffer_get_iter_at_mark(buffer, &old_hover_iter, hover_mark);
		if (loqui_channel_text_view_get_uri_at_iter(chview, buffer_gtk, &old_hover_iter, &old_uri_start_iter, &old_uri_end_iter)) {
			gtk_text_buffer_remove_tag(buffer, hover_tag, &old_uri_start_iter, &old_uri_end_iter);
		}
		buffer_gtk->hover_tag_applied = FALSE;
	}

	if (in_uri) {
		gtk_text_buffer_move_mark(buffer, hover_mark, &uri_start_iter);
		gtk_text_buffer_apply_tag(buffer, hover_tag, &uri_start_iter, &uri_end_iter);
		buffer_gtk->hover_tag_applied = TRUE;
	}
}
static void
loqui_channel_text_view_update_cursor(LoquiChannelTextView *chview, gint event_x, gint event_y)
{
	LoquiChannelTextViewPrivate *priv;
	GtkTextIter iter;
	gboolean should_hand_cursor;
	LoquiChannelBufferGtk *buffer_gtk;
	GtkTextBuffer *buffer;

        g_return_if_fail(chview != NULL);
        g_return_if_fail(LOQUI_IS_CHANNEL_TEXT_VIEW(chview));

        priv = chview->priv;
	
	if (!loqui_channel_text_view_get_buffer_and_iter_at_event_xy(chview, &buffer_gtk, &iter, event_x, event_y))
		return;

	buffer = GTK_TEXT_BUFFER(buffer_gtk);

	should_hand_cursor = loqui_channel_text_view_get_uri_at_iter(chview, buffer_gtk, &iter, NULL, NULL);

	if (should_hand_cursor && !priv->is_hand_cursor) {
		gdk_window_set_cursor(gtk_text_view_get_window(GTK_TEXT_VIEW(chview), GTK_TEXT_WINDOW_TEXT),
				      priv->hand_cursor);
		priv->is_hand_cursor = TRUE;
	} else if (!should_hand_cursor && priv->is_hand_cursor) {
		gdk_window_set_cursor(gtk_text_view_get_window(GTK_TEXT_VIEW(chview), GTK_TEXT_WINDOW_TEXT),
				      priv->normal_cursor);
		priv->is_hand_cursor = FALSE;
	}

	gdk_window_get_pointer(GTK_WIDGET(chview)->window, NULL, NULL, NULL);

	loqui_channel_text_view_update_hover(chview, buffer_gtk, &iter);
}
GtkWidget *
loqui_channel_text_view_new(LoquiApp *app)
{
        LoquiChannelTextView *chview;
	LoquiChannelTextViewPrivate *priv;

	chview = g_object_new(loqui_channel_text_view_get_type(),
			    "editable", FALSE,
			    "wrap_mode", GTK_WRAP_CHAR,
			    NULL);
	
        priv = chview->priv;
	priv->app = app;

	chview->scrolled_window = gtk_scrolled_window_new(NULL, NULL);
        gtk_scrolled_window_set_policy(GTK_SCROLLED_WINDOW(chview->scrolled_window), GTK_POLICY_NEVER, GTK_POLICY_ALWAYS);
	gtk_container_add(GTK_CONTAINER(chview->scrolled_window), GTK_WIDGET(chview));

	g_signal_connect(G_OBJECT(GTK_TEXT_VIEW(chview)->vadjustment), "value-changed",
			 G_CALLBACK(loqui_channel_text_view_vadj_value_changed_cb), chview);

	priv->hand_cursor = gdk_cursor_new(GDK_HAND2);
	priv->normal_cursor = gdk_cursor_new(GDK_XTERM);

        return GTK_WIDGET(chview);
}

void
loqui_channel_text_view_set_channel_buffer(LoquiChannelTextView *chview, LoquiChannelBufferGtk *buffer)
{
	LoquiChannelTextViewPrivate *priv;
	GtkTextBuffer *old_buf;
	GtkStyle *style;
	GdkColor *transparent_color;
	GtkTextTag *transparent_tag;
	
        g_return_if_fail(chview != NULL);
        g_return_if_fail(LOQUI_IS_CHANNEL_TEXT_VIEW(chview));
        g_return_if_fail(buffer != NULL);
        g_return_if_fail(LOQUI_IS_CHANNEL_BUFFER_GTK(buffer));
	
        priv = chview->priv;
       
	old_buf = gtk_text_view_get_buffer(GTK_TEXT_VIEW(chview));
	if (old_buf)
		g_signal_handlers_disconnect_by_func(old_buf, loqui_channel_text_view_buffer_insert_text_cb, chview);
	
	style = gtk_widget_get_style(GTK_WIDGET(chview));
	transparent_color = &style->base[GTK_STATE_NORMAL];
	transparent_tag = gtk_text_tag_table_lookup(gtk_text_buffer_get_tag_table(GTK_TEXT_BUFFER(buffer)),
						    "transparent");
	if (transparent_tag)
		g_object_set(G_OBJECT(transparent_tag), "foreground-gdk", transparent_color, NULL);
	
	gtk_text_view_set_buffer(GTK_TEXT_VIEW(chview), GTK_TEXT_BUFFER(buffer));
	g_signal_connect(G_OBJECT(buffer), "insert-text",
			 G_CALLBACK(loqui_channel_text_view_buffer_insert_text_cb), chview);
	
	loqui_channel_text_view_scroll_to_end(LOQUI_CHANNEL_TEXT_VIEW(chview));
}

void
loqui_channel_text_view_scroll_to_end(LoquiChannelTextView *chview)
{
	GtkTextBuffer *buffer;

        g_return_if_fail(chview != NULL);
        g_return_if_fail(LOQUI_IS_CHANNEL_TEXT_VIEW(chview));

	buffer = gtk_text_view_get_buffer(GTK_TEXT_VIEW(chview));
	if (buffer && LOQUI_IS_CHANNEL_BUFFER_GTK(buffer))
		gtk_text_view_scroll_mark_onscreen(GTK_TEXT_VIEW(chview),
						   gtk_text_buffer_get_mark(buffer, "end"));

	g_signal_emit(G_OBJECT(chview), channel_text_view_signals[SIGNAL_SCROLLED_TO_END], 0);
}
void
loqui_channel_text_view_scroll_to_end_if_enabled(LoquiChannelTextView *chview)
{
        g_return_if_fail(chview != NULL);
        g_return_if_fail(LOQUI_IS_CHANNEL_TEXT_VIEW(chview));

	if (chview->is_scroll)
		loqui_channel_text_view_scroll_to_end(chview);	
}
void
loqui_channel_text_view_set_is_scroll(LoquiChannelTextView *chview, gboolean is_scroll)
{
        g_return_if_fail(chview != NULL);
        g_return_if_fail(LOQUI_IS_CHANNEL_TEXT_VIEW(chview));

	if (chview->is_scroll == is_scroll)
		return;

	chview->is_scroll = is_scroll;
	loqui_channel_text_view_scroll_to_end_if_enabled(chview);

	g_object_notify(G_OBJECT(chview), "is_scroll");
}
gboolean
loqui_channel_text_view_get_is_scroll(LoquiChannelTextView *chview)
{
        g_return_val_if_fail(chview != NULL, FALSE);
        g_return_val_if_fail(LOQUI_IS_CHANNEL_TEXT_VIEW(chview), FALSE);

	return chview->is_scroll;
}
void
loqui_channel_text_view_scroll(LoquiChannelTextView *chview, GtkMovementStep step, gint count)
{
	g_signal_emit_by_name(chview, "move_cursor", step, count, FALSE);
}

void
loqui_channel_text_view_set_auto_switch_scrolling(LoquiChannelTextView *chview, gboolean auto_switch_scrolling)
{
        g_return_if_fail(chview != NULL);
        g_return_if_fail(LOQUI_IS_CHANNEL_TEXT_VIEW(chview));

	if (chview->auto_switch_scrolling == auto_switch_scrolling)
		return;

	chview->auto_switch_scrolling = auto_switch_scrolling;

	g_object_notify(G_OBJECT(chview), "auto_switch_scrolling");
}
gboolean
loqui_channel_text_view_get_auto_switch_scrolling(LoquiChannelTextView *chview)
{
        g_return_val_if_fail(chview != NULL, FALSE);
        g_return_val_if_fail(LOQUI_IS_CHANNEL_TEXT_VIEW(chview), FALSE);

	return chview->auto_switch_scrolling;
}
