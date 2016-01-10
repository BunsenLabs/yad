AC_INIT([YAD], [0.27.0], [https://sourceforge.net/p/yad-dialog/tickets/], yad, [https://sourceforge.net/projects/yad-dialog/])
AC_CONFIG_AUX_DIR([build])

AM_INIT_AUTOMAKE([1.11 foreign dist-xz no-dist-gzip])
AM_SILENT_RULES([yes])

AC_PREREQ([2.59])

AC_CONFIG_HEADERS([config.h])
AC_CONFIG_SRCDIR([src])

AM_MAINTAINER_MODE

IT_PROG_INTLTOOL([0.40.0])

AC_PROG_CC

AC_ARG_WITH([gtk],
	[gtkver="$withval"], [gtkver="gtk2"])
AC_ARG_WITH([gtk],
	[AS_HELP_STRING([--with-gtk=gtk2|gtk3],[set the GTK+ version to use (default - gtk2)])],
	[case "$with_gtk" in
        gtk2|gtk3) ;;
        *) AC_MSG_ERROR([invalid gtk version specified]) ;;
     esac],
    [with_gtk=gtk2])
AC_MSG_RESULT([$with_gtk])

case "$with_gtk" in
    gtk3) 
        GTK_MODULE="gtk+-3.0"
        GTK_REQUIRED="3.0.0"
        GTK_PRINT="gtk+-unix-print-3.0"
        ;;
    gtk2)
        GTK_MODULE="gtk+-2.0"
        GTK_REQUIRED="2.16.0"
        GTK_PRINT="gtk+-unix-print-2.0"
        ;;
esac
PKG_CHECK_MODULES([GTK], [$GTK_MODULE >= $GTK_REQUIRED $GTK_PRINT])
AC_SUBST([GTK_CFLAGS])
AC_SUBST([GTK_LIBS])

GTK_DISABLE_DEPRECATED

dnl icon browser
AC_ARG_ENABLE([icon-browser],
	[AS_HELP_STRING([--enable-icon-browser],
			[Build YAD icon browser])],
	[build_ib=$enableval], [build_ib=no])
AM_CONDITIONAL([BUILD_IB], [test x$build_ib = xyes])

dnl rgb.txt
AC_ARG_WITH([rgb],
	[AS_HELP_STRING([--with-rgb=PATH],
			[set path to rgb.txt file (default - /etc/X11/rgb.txt)])],
	[AC_DEFINE_UNQUOTED([RGB_FILE], ["$withval"], [Path to rgb.txt file])],
	[AC_DEFINE([RGB_FILE], ["/etc/X11/rgb.txt"], [Path to rgb.txt file])])

# *******************************
# Internationalization
# ******************************* 

GETTEXT_PACKAGE=yad
AC_SUBST([GETTEXT_PACKAGE])
AC_DEFINE_UNQUOTED([GETTEXT_PACKAGE], ["$GETTEXT_PACKAGE"], [Gettext domain])

AM_GLIB_GNU_GETTEXT

# *******************************

AC_SUBST([AM_CFLAGS])
AC_SUBST([AM_CPPFLAGS])
AC_SUBST([AM_LDFLAGS])

# *******************************

AC_CONFIG_FILES([
Makefile 
po/Makefile.in
src/Makefile
data/Makefile
data/icons/Makefile
data/icons/16x16/Makefile
data/icons/24x24/Makefile
data/icons/32x32/Makefile
data/icons/48x48/Makefile
data/icons/96x96/Makefile
data/icons/128x128/Makefile
data/yad.m4
data/yad.spec
])

AC_OUTPUT

echo
echo "Build icon browser - $build_ib"
echo
