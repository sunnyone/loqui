#ifndef __GTKINTL_H__
#define __GTKINTL_H__

#ifndef _
#define _(String) (String)
#endif

#ifndef P_
#define P_(String) (String)
#endif

#ifndef N_
#define N_(String) (String)
#endif

#ifndef textdomain
#define textdomain(String) (String)
#endif

#ifndef gettext
#define gettext(String) (String)
#endif

#ifndef dgettext
#define dgettext(Domain,String) (String)
#endif

#ifndef dcgettext
#define dcgettext(Domain,String,Type) (String)
#endif

#ifndef bindtextdomain
#define bindtextdomain(Domain,Directory) (Domain) 
#endif

#endif
