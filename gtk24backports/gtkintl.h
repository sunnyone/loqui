#ifndef __GTKINTL_H__
#define __GTKINTL_H__

#include "config.h"

#define _(String) (String)
#define P_(String) (String)
#define N_(String) (String)
#define textdomain(String) (String)
#define gettext(String) (String)
#define dgettext(Domain,String) (String)
#define dcgettext(Domain,String,Type) (String)
#define bindtextdomain(Domain,Directory) (Domain) 

#endif
