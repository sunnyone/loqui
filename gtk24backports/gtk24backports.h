
#ifndef __GTK24BACKPORTS_H__
#define __GTK24BACKPORTS_H__

#include <gtk24toolbar.h>
#include <gtk24ext.h>

#ifdef USE_GTK_2_2
#include <gtkaction.h>
#include <gtkactiongroup.h>
#include <gtkradioaction.h>
#include <gtkradiotoolbutton.h>
#include <gtkseparatortoolitem.h>
#include <gtktoggleaction.h>
#include <gtktoggletoolbutton.h>
#include <gtktoolbutton.h>
#include <gtktoolitem.h>
#include <gtkuimanager.h>
#endif /* USE_GTK_2_2 */

void gtk24backports_init(void);

#endif /* __GTK24BACKPORTS_H__ */
