/* GTK - The GIMP Toolkit
 * Copyright (C) 1995-1997 Peter Mattis, Spencer Kimball and Josh MacDonald
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

/* 2004-03-16
   Modified for Gtk24 backports by Yoichi Imai <yoichi@silver-forest.com>
   Available from Loqui <http://loqui.good-day.net/>
*/

#include "config.h"

#ifdef USE_GTK_2_2

#include <gtk/gtk.h>
#include "gtk24backports.h"
#include "gtk24ext.h"
#include "gtkintl.h"

#define PROP_NO_SHOW_ALL 100
#define PROP_FOCUS_ON_CLICK 200
#define PROP_DRAW_AS_RADIO 300

static gboolean gtk24backports_initialized = FALSE;

static GtkWidgetClass *gtk_widget_class = NULL;
static GtkButtonClass *gtk_button_class = NULL;
static GtkCheckMenuItemClass *gtk_check_menu_item_class = NULL;

#define G_ERROR_IF_GTK24BACKPORTS_UNINITIALIZED() if (!gtk24backports_initialized) g_error("Gtk24 backports uninitialized")

static void (*gtk_widget_set_property_orig) (GObject *object, guint property_id, const GValue *value, GParamSpec *pspec) = NULL;
static void (*gtk_widget_get_property_orig) (GObject *object, guint property_id, GValue *value, GParamSpec *pspec) = NULL;
static void (*gtk_widget_show_orig) (GtkWidget *widget) = NULL;
static void (*gtk_widget_hide_orig) (GtkWidget *widget) = NULL;

static void (*gtk_button_set_property_orig) (GObject *object, guint property_id, const GValue *value, GParamSpec *pspec) = NULL;
static void (*gtk_button_get_property_orig) (GObject *object, guint property_id, GValue *value, GParamSpec *pspec) = NULL;

static void (*gtk_check_menu_item_set_property_orig) (GObject *object, guint property_id, const GValue *value, GParamSpec *pspec) = NULL;
static void (*gtk_check_menu_item_get_property_orig) (GObject *object, guint property_id, GValue *value, GParamSpec *pspec) = NULL;

static void gtk_widget_set_property_backports (GObject         *object,
					       guint            prop_id,
					       const GValue    *value,
					       GParamSpec      *pspec);
static void gtk_widget_get_property_backports (GObject         *object,
					       guint            prop_id,
					       GValue          *value,
					       GParamSpec      *pspec);

static void gtk_widget_show_backports(GtkWidget *widget);
static void gtk_widget_hide_backports(GtkWidget *widget);

static void gtk_button_set_property_backports (GObject         *object,
					       guint            prop_id,
					       const GValue    *value,
					       GParamSpec      *pspec);
static void gtk_button_get_property_backports (GObject         *object,
					       guint            prop_id,
					       GValue          *value,
					       GParamSpec      *pspec);
static gboolean gtk_button_button_press_event_backports(GtkWidget *widget, GdkEventButton *event);

static void gtk_check_menu_item_set_property_backports (GObject         *object,
							guint            prop_id,
							const GValue    *value,
							GParamSpec      *pspec);
static void gtk_check_menu_item_get_property_backports (GObject         *object,
							guint            prop_id,
							GValue          *value,
							GParamSpec      *pspec);
static void gtk_real_check_menu_item_draw_indicator_backports(GtkCheckMenuItem *check_menu_item,
							      GdkRectangle     *area);

void gtk24backports_init(void)
{

	gtk_widget_class = g_type_class_ref(GTK_TYPE_WIDGET);

	g_object_class_install_property (G_OBJECT_CLASS(gtk_widget_class),
					 PROP_NO_SHOW_ALL,
					 g_param_spec_boolean("no_show_all",
							      P_("No show all"),
							      P_("Whether gtk_widget_show_all() should not affect this widget"),
							      FALSE,
							      G_PARAM_READWRITE));

	gtk_widget_set_property_orig = G_OBJECT_CLASS(gtk_widget_class)->set_property;
	G_OBJECT_CLASS(gtk_widget_class)->set_property = gtk_widget_set_property_backports;
	gtk_widget_get_property_orig = G_OBJECT_CLASS(gtk_widget_class)->get_property;
	G_OBJECT_CLASS(gtk_widget_class)->get_property = gtk_widget_get_property_backports;

	gtk_widget_show_orig = gtk_widget_class->show;
	gtk_widget_class->show = gtk_widget_show_backports;
	gtk_widget_hide_orig = gtk_widget_class->hide;
	gtk_widget_class->hide = gtk_widget_hide_backports;
	
	gtk_button_class = g_type_class_ref(GTK_TYPE_BUTTON);
	
	g_object_class_install_property (G_OBJECT_CLASS(gtk_button_class),
					 PROP_FOCUS_ON_CLICK,
					 g_param_spec_boolean ("focus_on_click",
							       P_("Focus on click"),
							       P_("Whether the button grabs focus when it is clicked with the mouse"),
							       TRUE,
							       G_PARAM_READWRITE));
	
	gtk_button_set_property_orig = G_OBJECT_CLASS(gtk_button_class)->set_property;
	G_OBJECT_CLASS(gtk_button_class)->set_property = gtk_button_set_property_backports;
	gtk_button_get_property_orig = G_OBJECT_CLASS(gtk_button_class)->get_property;
	G_OBJECT_CLASS(gtk_button_class)->get_property = gtk_button_get_property_backports;
	GTK_WIDGET_CLASS(gtk_button_class)->button_press_event = gtk_button_button_press_event_backports;

	gtk_check_menu_item_class = g_type_class_ref(GTK_TYPE_CHECK_MENU_ITEM);

	g_object_class_install_property (G_OBJECT_CLASS(gtk_check_menu_item_class),
					 PROP_DRAW_AS_RADIO,
					 g_param_spec_boolean ("draw_as_radio",
							       P_("Draw as radio menu item"),
							       P_("Whether the menu item looks like a radio menu item"),
							       FALSE,
							       G_PARAM_READWRITE));

	gtk_check_menu_item_set_property_orig = G_OBJECT_CLASS(gtk_check_menu_item_class)->set_property;
	G_OBJECT_CLASS(gtk_check_menu_item_class)->set_property = gtk_check_menu_item_set_property_backports;
	gtk_check_menu_item_get_property_orig = G_OBJECT_CLASS(gtk_check_menu_item_class)->get_property;
	G_OBJECT_CLASS(gtk_check_menu_item_class)->get_property = gtk_check_menu_item_get_property_backports;
	gtk_check_menu_item_class->draw_indicator = gtk_real_check_menu_item_draw_indicator_backports;

	gtk24backports_initialized = TRUE;
}

static void
gtk_widget_set_property_backports (GObject         *object,
				   guint            prop_id,
				   const GValue    *value,
				   GParamSpec      *pspec)
{
	G_ERROR_IF_GTK24BACKPORTS_UNINITIALIZED();

	if (prop_id == PROP_NO_SHOW_ALL) {
		gtk_widget_set_no_show_all(GTK_WIDGET(object), g_value_get_boolean (value));
		return;
	}

	gtk_widget_set_property_orig(object, prop_id, value, pspec);
}
static void
gtk_widget_get_property_backports (GObject         *object,
				   guint            prop_id,
				   GValue          *value,
				   GParamSpec      *pspec)
{
	G_ERROR_IF_GTK24BACKPORTS_UNINITIALIZED();

	if (prop_id == PROP_NO_SHOW_ALL) {
		g_value_set_boolean (value, gtk_widget_get_no_show_all(GTK_WIDGET(object)));
		return;
	}
	gtk_widget_get_property_orig(object, prop_id, value, pspec);
}
static void
gtk_widget_show_backports(GtkWidget *widget)
{
	G_ERROR_IF_GTK24BACKPORTS_UNINITIALIZED();

	if ((GTK_WIDGET_FLAGS (widget) & GTK_NO_SHOW_ALL) != 0)
		return;

	gtk_widget_show_orig(widget);
}
static void
gtk_widget_hide_backports(GtkWidget *widget)
{
	G_ERROR_IF_GTK24BACKPORTS_UNINITIALIZED();
	
	if ((GTK_WIDGET_FLAGS (widget) & GTK_NO_SHOW_ALL) != 0)
		return;

	gtk_widget_hide_orig(widget);
}
static void
gtk_button_set_property_backports (GObject         *object,
				   guint            prop_id,
				   const GValue    *value,
				   GParamSpec      *pspec)
{
	G_ERROR_IF_GTK24BACKPORTS_UNINITIALIZED();

	if (prop_id == PROP_FOCUS_ON_CLICK) {
		gtk_button_set_focus_on_click(GTK_BUTTON(object), g_value_get_boolean (value));
		return;
	}

	gtk_button_set_property_orig(object, prop_id, value, pspec);
}
static void
gtk_button_get_property_backports (GObject         *object,
				   guint            prop_id,
				   GValue          *value,
				   GParamSpec      *pspec)
{
	G_ERROR_IF_GTK24BACKPORTS_UNINITIALIZED();

	if (prop_id == PROP_FOCUS_ON_CLICK) {
		g_value_set_boolean (value, gtk_button_get_focus_on_click(GTK_BUTTON(object)));
		return;
	}
	gtk_button_get_property_orig(object, prop_id, value, pspec);
}
static gboolean
gtk_button_button_press_event_backports(GtkWidget *widget, GdkEventButton *event)
{
  GtkButton *button;

  G_ERROR_IF_GTK24BACKPORTS_UNINITIALIZED();

  if (event->type == GDK_BUTTON_PRESS)
    {
      button = GTK_BUTTON (widget);

      if (gtk_button_get_focus_on_click(button) && !GTK_WIDGET_HAS_FOCUS (widget))
        gtk_widget_grab_focus (widget);

      if (event->button == 1)
        gtk_button_pressed (button);
    }

  return TRUE;
}

static void
gtk_check_menu_item_set_property_backports (GObject         *object,
					    guint            prop_id,
					    const GValue    *value,
					    GParamSpec      *pspec)
{
	G_ERROR_IF_GTK24BACKPORTS_UNINITIALIZED();

	if (prop_id == PROP_DRAW_AS_RADIO) {
		gtk_check_menu_item_set_draw_as_radio(GTK_CHECK_MENU_ITEM(object), g_value_get_boolean(value));
		return;
	}

	gtk_check_menu_item_set_property_orig(object, prop_id, value, pspec);
}
static void
gtk_check_menu_item_get_property_backports (GObject         *object,
					    guint            prop_id,
					    GValue          *value,
					    GParamSpec      *pspec)
{
	G_ERROR_IF_GTK24BACKPORTS_UNINITIALIZED();

	if (prop_id == PROP_DRAW_AS_RADIO) {
		g_value_set_boolean (value, gtk_check_menu_item_get_draw_as_radio(GTK_CHECK_MENU_ITEM(object)));
		return;
	}

	gtk_check_menu_item_get_property_orig(object, prop_id, value, pspec);
}
static void
gtk_real_check_menu_item_draw_indicator_backports (GtkCheckMenuItem *check_menu_item,
						   GdkRectangle     *area)
{
  GtkWidget *widget;
  GtkStateType state_type;
  GtkShadowType shadow_type;
  gint x, y;

  if (GTK_WIDGET_DRAWABLE (check_menu_item))
    {
      guint offset;
      guint toggle_size;
      guint toggle_spacing;
      guint horizontal_padding;
      guint indicator_size;
      
      widget = GTK_WIDGET (check_menu_item);

/*      gtk_widget_style_get (GTK_WIDGET (check_menu_item),
                            "toggle_spacing", &toggle_spacing,
                            "horizontal_padding", &horizontal_padding,
                            "indicator_size", &indicator_size,
                            NULL); */
      
      /* FIXME: add properties (currently uses default values) */
      toggle_spacing = 5;
      horizontal_padding = 3;
      indicator_size = 12;

      toggle_size = GTK_MENU_ITEM (check_menu_item)->toggle_size;
      offset = GTK_CONTAINER (check_menu_item)->border_width + widget->style->xthickness;
      
      offset = GTK_CONTAINER (check_menu_item)->border_width +
        widget->style->xthickness + 2; 

      if (gtk_widget_get_direction (widget) == GTK_TEXT_DIR_LTR)
        {
          x = widget->allocation.x + offset + horizontal_padding +
            (toggle_size - toggle_spacing - indicator_size) / 2;
        }
      else 
        {
          x = widget->allocation.x + widget->allocation.width -
            offset - horizontal_padding - toggle_size + toggle_spacing +
            (toggle_size - toggle_spacing - indicator_size) / 2;
        }
      y = widget->allocation.y + (widget->allocation.height - indicator_size) / 2;

      if (check_menu_item->active ||
          check_menu_item->always_show_toggle ||
          (GTK_WIDGET_STATE (check_menu_item) == GTK_STATE_PRELIGHT))
        {
          state_type = GTK_WIDGET_STATE (widget);
          
          if (check_menu_item->inconsistent)
            shadow_type = GTK_SHADOW_ETCHED_IN;
          else if (check_menu_item->active)
            shadow_type = GTK_SHADOW_IN;
          else 
            shadow_type = GTK_SHADOW_OUT;
          
          if (!GTK_WIDGET_IS_SENSITIVE (widget))
            state_type = GTK_STATE_INSENSITIVE;

          if (gtk_check_menu_item_get_draw_as_radio(check_menu_item))
            {
              gtk_paint_option (widget->style, widget->window,
                                state_type, shadow_type,
                                area, widget, "option",
                                x, y, indicator_size, indicator_size);
            }
          else
            {
              gtk_paint_check (widget->style, widget->window,
                               state_type, shadow_type,
                               area, widget, "check",
                               x, y, indicator_size, indicator_size);
            }
        }
    }
}

#else /* USE_GTK2_2 */
void gtk24backports_init(void)
{
	/* do nothing */
}
#endif /* USE_GTK2_2 */
