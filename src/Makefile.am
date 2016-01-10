AM_CPPFLAGS = 					\
	-DLOCALEDIR=\""$(datadir)/locale"\"	\
	@DISABLE_DEPRECATED@

bin_PROGRAMS = yad

yad_SOURCES = 			\
	about.c			\
	calendar.c		\
	color.c			\
	dnd.c           	\
	entry.c			\
	file.c			\
	font.c			\
	form.c			\
	icons.c			\
	list.c			\
	multi-progress.c	\
	notebook.c		\
	notification.c		\
	option.c		\
	print.c			\
	progress.c		\
	scale.c			\
	text.c			\
	util.c			\
	main.c			\
	yad.h			\
	calendar.xpm

yad_CFLAGS = $(GTK_CFLAGS)
yad_LDADD = $(GTK_LIBS)

if BUILD_IB
bin_PROGRAMS += yad-icon-browser

yad_icon_browser_SOURCES = browser.c
yad_icon_browser_CFLAGS = $(GTK_CFLAGS)
yad_icon_browser_LDADD = $(GTK_LIBS)
endif
