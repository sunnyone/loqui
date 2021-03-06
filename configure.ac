#                                               -*- Autoconf -*-

AC_PREREQ(2.53)

m4_define([loqui_major_version], [0])
m4_define([loqui_minor_version], [7])
m4_define([loqui_micro_version], [0])
m4_define([loqui_extra_version], [])
m4_define([loqui_version], [loqui_major_version.loqui_minor_version.loqui_micro_version[]loqui_extra_version])

AC_INIT(loqui, loqui_version)
AC_CONFIG_SRCDIR([src/main.c])
AM_CONFIG_HEADER([config.h])

AM_INIT_AUTOMAKE

AM_MAINTAINER_MODE

# Checks for programs.
AC_PROG_CXX
AC_PROG_CC
AC_PROG_CPP
AC_PROG_INSTALL
AC_PROG_LN_S
AC_PROG_MAKE_SET
AC_PROG_LIBTOOL
AC_CHECK_PROG(WINDRES, windres, windres)
AM_CONDITIONAL(HAVE_WINDRES, test x"$WINDRES" != x)

# Checks for libraries.
AM_PATH_GLIB_2_0(2.22.0,, AC_MSG_ERROR(Test for GLib failed. See the 'INSTALL' for help.), gobject gio)
AC_PATH_PROG(GLIB_GENMARSHAL, glib-genmarshal)

AM_PATH_GTK_3_0(3.0.0,, AC_MSG_ERROR(Test for Gtk failed. See the 'INSTALL' for help.))

# Checks for header files.
AC_HEADER_STDC
AC_CHECK_HEADERS([fcntl.h locale.h stdlib.h string.h unistd.h])

# Checks for typedefs, structures, and compiler characteristics.
AC_HEADER_STAT
AC_C_CONST
AC_TYPE_SIZE_T
AC_STRUCT_TM

GETTEXT_PACKAGE="$PACKAGE"
AC_SUBST(GETTEXT_PACKAGE)
AC_DEFINE_UNQUOTED(GETTEXT_PACKAGE,"$GETTEXT_PACKAGE", [Define the gettext package to be used])

ALL_LINGUAS="ja"

AM_GLIB_GNU_GETTEXT
dnl Set PACKAGE_LOCALE_DIR in config.h.
if test "x${prefix}" = "xNONE"; then
  AC_DEFINE_UNQUOTED(PACKAGE_LOCALE_DIR, "${ac_default_prefix}/${DATADIRNAME}/locale", [Define the location where the catalogs will be installed])
else
  AC_DEFINE_UNQUOTED(PACKAGE_LOCALE_DIR, "${prefix}/${DATADIRNAME}/locale")
fi

# Checks for library functions.
AC_FUNC_STRFTIME
AC_CHECK_FUNCS([isascii memset mkdir setlocale strchr strpbrk strrchr strstr strtol])
AC_CHECK_FUNCS(bind_textdomain_codeset)
AC_CHECK_FUNCS(localtime_r)

AC_CONFIG_FILES([
Makefile
libloqui/Makefile
libloqui/protocols/Makefile
libloqui/protocols/jabber/Makefile
src/Makefile
src/icons/Makefile
src/embedtxt/Makefile
po/Makefile.in
m4/Makefile
loqui.spec
])
AC_OUTPUT
