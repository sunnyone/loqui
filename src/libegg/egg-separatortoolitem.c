#include <gtk/gtkseparatormenuitem.h>
#include "egg-separatortoolitem.h"
#include "intl.h"

static void egg_separator_tool_item_init       (EggSeparatorToolItem *self);
static void egg_separator_tool_item_class_init (EggSeparatorToolItemClass*class);

static void       egg_separator_tool_item_add               (GtkContainer *container,
							     GtkWidget    *child);
static GtkWidget *egg_separator_tool_item_create_menu_proxy (EggToolItem  *self);

static GObjectClass *parent_class = NULL;


GType
egg_separator_tool_item_get_type (void)
{
  static GType type = 0;

  if (!type)
    {
      static const GTypeInfo type_info =
	{
	  sizeof (EggSeparatorToolItemClass),
	  (GBaseInitFunc) 0,
	  (GBaseFinalizeFunc) 0,
	  (GClassInitFunc) egg_separator_tool_item_class_init,
	  (GClassFinalizeFunc) 0,
	  NULL,
	  sizeof (EggSeparatorToolItem),
	  0, /* n_preallocs */
	  (GInstanceInitFunc) egg_separator_tool_item_init
	};

      type = g_type_register_static (EGG_TYPE_TOOL_ITEM,
				     "EggSeparatorToolItem", &type_info, 0);
    }
  return type;
}


static void
egg_separator_tool_item_class_init (EggSeparatorToolItemClass *class)
{
  GtkContainerClass *container_class;
  EggToolItemClass *toolitem_class;

  parent_class = g_type_class_peek_parent (class);
  container_class = (GtkContainerClass *)class;
  toolitem_class = (EggToolItemClass *)class;

  container_class->add = egg_separator_tool_item_add;
  toolitem_class->create_menu_proxy = egg_separator_tool_item_create_menu_proxy;
}

static void
egg_separator_tool_item_init (EggSeparatorToolItem *self)
{
}

static void
egg_separator_tool_item_add (GtkContainer *container, GtkWidget *child)
{
  g_warning("attempt to add a child to an EggSeparatorToolItem");
}

static GtkWidget *
egg_separator_tool_item_create_menu_proxy (EggToolItem *item)
{
  return gtk_separator_menu_item_new ();
}


EggToolItem *
egg_separator_tool_item_new (void)
{
  EggToolItem *self;

  self = g_object_new (EGG_TYPE_SEPARATOR_TOOL_ITEM,
		       NULL);
  
  return self;
}
