/* GTK - The GIMP Toolkit
 * Copyright (C) 1995-1997 Peter Mattis, Spencer Kimball and Josh MacDonald
 * GtkToolbar copyright (C) Federico Mena
 *
 * Copyright (C) 2002 Anders Carlsson <andersca@gnome.org>
 * Copyright (C) 2002 James Henstridge <james@daa.com.au>
 * Copyright (C) 2003 Soeren Sandmann <sandmann@daimi.au.dk>
 * 
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the
 * Free Software Foundation, Inc., 59 Temple Place - Suite 330,
 * Boston, MA 02111-1307, USA.
 */

/*
 * Modified by the GTK+ Team and others 1997-2000.  See the AUTHORS
 * file for a list of people on the GTK+ Team.  See the ChangeLog
 * files for a list of changes.  These files are distributed with
 * GTK+ at ftp://ftp.gtk.org/pub/gtk/. 
 */

#ifndef __GTK24_TOOLBAR_H__
#define __GTK24_TOOLBAR_H__

#ifdef USE_GTK_2_2

#include <gdk/gdk.h>
#include <gtk/gtkcontainer.h>
#include <gtk/gtkenums.h>
#include <gtk/gtktooltips.h>
#include <gtk/gtktoolbar.h>

#include "gtktoolitem.h"

G_BEGIN_DECLS

#define GTK24_TYPE_TOOLBAR            (gtk24_toolbar_get_type ())
#define GTK24_TOOLBAR(obj)            (G_TYPE_CHECK_INSTANCE_CAST ((obj), GTK24_TYPE_TOOLBAR, Gtk24Toolbar))
#define GTK24_TOOLBAR_CLASS(klass)    (G_TYPE_CHECK_CLASS_CAST ((klass), GTK24_TYPE_TOOLBAR, Gtk24ToolbarClass))
#define GTK24_IS_TOOLBAR(obj)         (G_TYPE_CHECK_INSTANCE_TYPE ((obj), GTK24_TYPE_TOOLBAR))
#define GTK24_IS_TOOLBAR_CLASS(klass) (G_TYPE_CHECK_CLASS_TYPE ((klass), GTK24_TYPE_TOOLBAR))
#define GTK24_TOOLBAR_GET_CLASS(obj)  (G_TYPE_INSTANCE_GET_CLASS ((obj), GTK24_TYPE_TOOLBAR, Gtk24ToolbarClass))

#ifndef GTK_DISABLE_DEPRECATED
typedef enum
{
  GTK24_TOOLBAR_CHILD_SPACE,
  GTK24_TOOLBAR_CHILD_BUTTON,
  GTK24_TOOLBAR_CHILD_TOGGLEBUTTON,
  GTK24_TOOLBAR_CHILD_RADIOBUTTON,
  GTK24_TOOLBAR_CHILD_WIDGET
} Gtk24ToolbarChildType;

typedef struct _Gtk24ToolbarChild      Gtk24ToolbarChild;

struct _Gtk24ToolbarChild
{
  Gtk24ToolbarChildType type;
  GtkWidget *widget;
  GtkWidget *icon;
  GtkWidget *label;
};

#endif /* GTK_DISABLE_DEPRECATED */

typedef enum
{
  GTK24_TOOLBAR_SPACE_EMPTY,
  GTK24_TOOLBAR_SPACE_LINE
} Gtk24ToolbarSpaceStyle;

typedef struct _Gtk24Toolbar           Gtk24Toolbar;
typedef struct _Gtk24ToolbarClass      Gtk24ToolbarClass;
typedef struct _Gtk24ToolbarPrivate    Gtk24ToolbarPrivate;

struct _Gtk24Toolbar
{
  GtkContainer container;

  /*< public >*/
  gint             num_children;
  GList           *children;
  GtkOrientation   orientation;
  GtkToolbarStyle  style;
  GtkIconSize      icon_size;
  
  GtkTooltips     *tooltips;
  
  /*< private >*/
  gint             button_maxw;		/* maximum width of homogeneous children */
  gint             button_maxh;		/* maximum height of homogeneous children */
  
  guint            style_set_connection;
  guint            icon_size_connection;
  
  guint            style_set : 1;
  guint            icon_size_set : 1;

  Gtk24ToolbarPrivate *priv;
};

struct _Gtk24ToolbarClass
{
  GtkContainerClass parent_class;
  
  /* signals */
  void     (* orientation_changed) (Gtk24Toolbar       *toolbar,
				    GtkOrientation    orientation);
  void     (* style_changed)       (Gtk24Toolbar       *toolbar,
				    GtkToolbarStyle   style);
  gboolean (* popup_context_menu)  (Gtk24Toolbar       *toolbar,
				    gint              x,
				    gint              y,
				    gint              button_number);
  
  /* Padding for future expansion */
  void (*_gtk_reserved1) (void);
  void (*_gtk_reserved2) (void);
  void (*_gtk_reserved3) (void);
};

GType           gtk24_toolbar_get_type         (void) G_GNUC_CONST;
GtkWidget*      gtk24_toolbar_new                     (void);
void            gtk24_toolbar_insert                  (Gtk24Toolbar      *toolbar,
						     GtkToolItem     *item,
						     gint             pos);
gint            gtk24_toolbar_get_item_index          (Gtk24Toolbar      *toolbar,
						     GtkToolItem     *item);
gint            gtk24_toolbar_get_n_items             (Gtk24Toolbar      *toolbar);
GtkToolItem *   gtk24_toolbar_get_nth_item            (Gtk24Toolbar      *toolbar,
						     gint             n);
gboolean        gtk24_toolbar_get_show_arrow          (Gtk24Toolbar      *toolbar);
void            gtk24_toolbar_set_show_arrow          (Gtk24Toolbar      *toolbar,
						     gboolean         show_arrow);
GtkOrientation  gtk24_toolbar_get_orientation         (Gtk24Toolbar      *toolbar);
void            gtk24_toolbar_set_orientation         (Gtk24Toolbar      *toolbar,
						     GtkOrientation   orientation);
gboolean        gtk24_toolbar_get_tooltips            (Gtk24Toolbar      *toolbar);
void            gtk24_toolbar_set_tooltips            (Gtk24Toolbar      *toolbar,
						     gboolean         enable);
GtkToolbarStyle gtk24_toolbar_get_style               (Gtk24Toolbar      *toolbar);
void            gtk24_toolbar_set_style               (Gtk24Toolbar      *toolbar,
						     GtkToolbarStyle  style);
void            gtk24_toolbar_unset_style             (Gtk24Toolbar      *toolbar);
GtkIconSize     gtk24_toolbar_get_icon_size           (Gtk24Toolbar      *toolbar);
GtkReliefStyle  gtk24_toolbar_get_relief_style        (Gtk24Toolbar      *toolbar);
gint            gtk24_toolbar_get_drop_index          (Gtk24Toolbar      *toolbar,
						     gint             x,
						     gint             y);
void            gtk24_toolbar_set_drop_highlight_item (Gtk24Toolbar      *toolbar,
						     GtkToolItem     *tool_item,
						     gint             index);

/* internal functions */
gchar *         _gtk24_toolbar_elide_underscores      (const gchar     *original);
void            _gtk24_toolbar_paint_space_line       (GtkWidget       *widget,
						     Gtk24Toolbar      *toolbar,
						     GdkRectangle    *area,
						     GtkAllocation   *allocation);
gint            _gtk24_toolbar_get_default_space_size (void);

/* deprecated */
#ifndef GTK_DISABLE_DEPRECATED
void       gtk24_toolbar_set_icon_size   (Gtk24Toolbar      *toolbar,
                                        GtkIconSize      icon_size);
void       gtk24_toolbar_unset_icon_size (Gtk24Toolbar      *toolbar);

/* Simple button items */
GtkWidget* gtk24_toolbar_append_item   (Gtk24Toolbar      *toolbar,
                                      const char      *text,
                                      const char      *tooltip_text,
                                      const char      *tooltip_private_text,
                                      GtkWidget       *icon,
                                      GtkSignalFunc    callback,
                                      gpointer         user_data);
GtkWidget* gtk24_toolbar_prepend_item  (Gtk24Toolbar      *toolbar,
                                      const char      *text,
                                      const char      *tooltip_text,
                                      const char      *tooltip_private_text,
                                      GtkWidget       *icon,
                                      GtkSignalFunc    callback,
                                      gpointer         user_data);
GtkWidget* gtk24_toolbar_insert_item   (Gtk24Toolbar      *toolbar,
                                      const char      *text,
                                      const char      *tooltip_text,
                                      const char      *tooltip_private_text,
                                      GtkWidget       *icon,
                                      GtkSignalFunc    callback,
                                      gpointer         user_data,
                                      gint             position);

/* Stock Items */
GtkWidget* gtk24_toolbar_insert_stock    (Gtk24Toolbar      *toolbar,
                                        const gchar     *stock_id,
                                        const char      *tooltip_text,
                                        const char      *tooltip_private_text,
                                        GtkSignalFunc    callback,
                                        gpointer         user_data,
                                        gint             position);

/* Space Items */
void       gtk24_toolbar_append_space    (Gtk24Toolbar      *toolbar);
void       gtk24_toolbar_prepend_space   (Gtk24Toolbar      *toolbar);
void       gtk24_toolbar_insert_space    (Gtk24Toolbar      *toolbar,
                                        gint             position);
void       gtk24_toolbar_remove_space    (Gtk24Toolbar      *toolbar,
                                        gint             position);

/* Any element type */
GtkWidget* gtk24_toolbar_append_element  (Gtk24Toolbar      *toolbar,
                                        Gtk24ToolbarChildType type,
                                        GtkWidget       *widget,
                                        const char      *text,
                                        const char      *tooltip_text,
                                        const char      *tooltip_private_text,
                                        GtkWidget       *icon,
                                        GtkSignalFunc    callback,
                                        gpointer         user_data);

GtkWidget* gtk24_toolbar_prepend_element (Gtk24Toolbar      *toolbar,
                                        Gtk24ToolbarChildType type,
                                        GtkWidget       *widget,
                                        const char      *text,
                                        const char      *tooltip_text,
                                        const char      *tooltip_private_text,
                                        GtkWidget       *icon,
                                        GtkSignalFunc    callback,
                                        gpointer         user_data);

GtkWidget* gtk24_toolbar_insert_element  (Gtk24Toolbar      *toolbar,
                                        Gtk24ToolbarChildType type,
                                        GtkWidget       *widget,
                                        const char      *text,
                                        const char      *tooltip_text,
                                        const char      *tooltip_private_text,
                                        GtkWidget       *icon,
                                        GtkSignalFunc    callback,
                                        gpointer         user_data,
                                        gint             position);

/* Generic Widgets */
void       gtk24_toolbar_append_widget   (Gtk24Toolbar      *toolbar,
                                        GtkWidget       *widget,
                                        const char      *tooltip_text,
                                        const char      *tooltip_private_text);
void       gtk24_toolbar_prepend_widget  (Gtk24Toolbar      *toolbar,
                                        GtkWidget       *widget,
                                        const char      *tooltip_text,
                                        const char      *tooltip_private_text);
void       gtk24_toolbar_insert_widget   (Gtk24Toolbar      *toolbar,
                                        GtkWidget       *widget,
                                        const char      *tooltip_text,
                                        const char      *tooltip_private_text,
                                        gint             position);

#endif /* GTK_DISABLE_DEPRECATED */

G_END_DECLS

#else /* GTK_USE_2_2 */
/* TODO: define s/gtk24/gtk/g entries */

#endif /* GTK_USE_2_2 */
#endif /* __GTK24_TOOLBAR_H__ */
