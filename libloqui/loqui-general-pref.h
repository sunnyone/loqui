#ifndef __LOQUI_GENERAL_PREF__
#define __LOQUI_GENERAL_PREF__

#include "loqui.h"
#include "loqui-core.h"
#include "loqui-pref.h"

#define LOQUI_GET_GENERAL_PREF() loqui_core_get_general_pref(loqui_get_core())

#define LOQUI_GENERAL_PREF_SET_STRING(group, key, val) loqui_pref_set_string(LOQUI_GET_GENERAL_PREF(), group, key, val)
#define LOQUI_GENERAL_PREF_GET_STRING(group, key) loqui_pref_get_string(LOQUI_GET_GENERAL_PREF(), group, key, NULL)
#define LOQUI_GENERAL_PREF_SET_STRING_DEFAULT(group, key, val) loqui_pref_set_string_default(LOQUI_GET_GENERAL_PREF(), group, key, val)

#define LOQUI_GENERAL_PREF_SET_BOOLEAN(group, key, val) loqui_pref_set_boolean(LOQUI_GET_GENERAL_PREF(), group, key, val)
#define LOQUI_GENERAL_PREF_GET_BOOLEAN(group, key) loqui_pref_get_boolean(LOQUI_GET_GENERAL_PREF(), group, key, NULL)
#define LOQUI_GENERAL_PREF_SET_BOOLEAN_DEFAULT(group, key, val) loqui_pref_set_boolean_default(LOQUI_GET_GENERAL_PREF(), group, key, val)

#define LOQUI_GENERAL_PREF_SET_INTEGER(group, key, val) loqui_pref_set_boolean(LOQUI_GET_GENERAL_PREF(), group, key, val)
#define LOQUI_GENERAL_PREF_GET_INTEGER(group, key) loqui_pref_get_boolean(LOQUI_GET_GENERAL_PREF(), group, key, NULL)
#define LOQUI_GENERAL_PREF_SET_INTEGER_DEFAULT(group, key, val) loqui_pref_set_boolean_default(LOQUI_GET_GENERAL_PREF(), group, key, val)

#define LOQUI_GENERAL_PREF_SET_STRING_LIST(group, key, val, length) loqui_pref_set_string_list(LOQUI_GET_GENERAL_PREF(), group, key, val, length)
#define LOQUI_GENERAL_PREF_GET_STRING_LIST(group, key, length) loqui_pref_get_string_list(LOQUI_GET_GENERAL_PREF(), group, key, length, NULL)
#define LOQUI_GENERAL_PREF_SET_STRING_LIST_DEFAULT(group, key, val, length) loqui_pref_set_string_list_default(LOQUI_GET_GENERAL_PREF(), group, key, val, length)

#endif /* __LOQUI_GENERAL_PREF__ */
