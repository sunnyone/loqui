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
#include "gtk24ext.h"

void
gtk_check_menu_item_set_draw_as_radio (GtkCheckMenuItem *check_menu_item,
                                       gboolean          draw_as_radio)
{
  g_return_if_fail (GTK_IS_CHECK_MENU_ITEM (check_menu_item));
  
  draw_as_radio = draw_as_radio != FALSE;

  if (draw_as_radio != gtk_check_menu_item_get_draw_as_radio(check_menu_item))
    {
      g_object_set_data(G_OBJECT(check_menu_item), "draw_as_radio", GINT_TO_POINTER((int) draw_as_radio));

      gtk_widget_queue_draw (GTK_WIDGET (check_menu_item));

      g_object_notify (G_OBJECT (check_menu_item), "draw_as_radio");
    }
}
gboolean
gtk_check_menu_item_get_draw_as_radio (GtkCheckMenuItem *check_menu_item)
{
  g_return_val_if_fail (GTK_IS_CHECK_MENU_ITEM (check_menu_item), FALSE);
  
  return GPOINTER_TO_INT(g_object_get_data(G_OBJECT(check_menu_item), "draw_as_radio"));
}

void
gtk_button_set_focus_on_click (GtkButton *button,
                               gboolean   focus_on_click)
{
  g_return_if_fail (GTK_IS_BUTTON (button));
  
  focus_on_click = focus_on_click != FALSE;

  if (gtk_button_get_focus_on_click(button) != focus_on_click)
    {
      g_object_set_data(G_OBJECT(button), "focus_on_click", GINT_TO_POINTER((gint) focus_on_click+100));
      
      g_object_notify (G_OBJECT (button), "focus_on_click");
    }
}

gboolean
gtk_button_get_focus_on_click (GtkButton *button)
{
  gint val;
  g_return_val_if_fail (GTK_IS_BUTTON (button), FALSE);
  
  /* uninitialized: 0, false: 100, true: 101 */
  val = GPOINTER_TO_INT(g_object_get_data(G_OBJECT(button), "focus_on_click"));
  /* default value is true, so if uninitialized, return true */
  if (val == 0)
    return TRUE;

  return (gboolean) val - 100;
}

void
gtk_widget_queue_resize_no_redraw (GtkWidget *widget)
{
  g_return_if_fail (GTK_IS_WIDGET (widget));

  _gtk_size_group_queue_resize (widget);
}
gboolean
gtk_widget_get_no_show_all (GtkWidget *widget)
{
  g_return_val_if_fail (GTK_IS_WIDGET (widget), FALSE);
  
  return (GTK_WIDGET_FLAGS (widget) & GTK_NO_SHOW_ALL) != 0;
}
void
gtk_widget_set_no_show_all (GtkWidget *widget,
                            gboolean   no_show_all)
{
  g_return_if_fail (GTK_IS_WIDGET (widget));

  no_show_all = (no_show_all != FALSE);

  if (no_show_all == ((GTK_WIDGET_FLAGS (widget) & GTK_NO_SHOW_ALL) != 0))
    return;

  if (no_show_all)
    GTK_WIDGET_SET_FLAGS (widget, GTK_NO_SHOW_ALL);
  else
    GTK_WIDGET_UNSET_FLAGS (widget, GTK_NO_SHOW_ALL);

  g_object_notify (G_OBJECT (widget), "no_show_all");
}

#endif /* USE_GTK_2_2 */
