# $Id$

INCLUDES =								\
	-I$(top_srcdir)							\
	-DG_LOG_DOMAIN=\"xfce4-TileRole-plugin\"			\
	-DPACKAGE_LOCALE_DIR=\"$(localedir)\"				\
	$(PLATFORM_CPPFLAGS)

BACKEND_SRC = 								\
	eval.c								\
	eval.h								\
	lexer.c								\
	lexer.h								\
	parser.c							\
	parser.h							\
	parsetree.c							\
	parsetree.h							\
	constants.h

plugin_PROGRAMS =							\
	xfce4-TileRole-plugin

plugindir =								\
	$(libexecdir)/xfce4/panel-plugins


xfce4_TileRole_plugin_SOURCES =					\
	TileRole.c							\
	$(BACKEND_SRC)

xfce4_TileRole_plugin_CFLAGS =					\
	$(LIBXFCE4UTIL_CFLAGS)						\
	$(LIBXFCE4UI_CFLAGS)						\
	$(LIBXFCE4PANEL_CFLAGS)						\
	$(PLATFORM_CFLAGS)

xfce4_TileRole_plugin_LDADD =						\
	$(LIBXFCE4UTIL_LIBS)						\
	$(LIBXFCE4UI_LIBS)						\
	$(LIBXFCE4PANEL_LIBS)

desktopdir =								\
	$(datadir)/xfce4/panel-plugins

desktop_in_in_files =							\
	TileRole.desktop.in.in

desktop_in_files =							\
	$(desktop_in_in_files:.desktop.in.in=.desktop.in)

%.desktop.in: %.desktop.in.in
	sed -e "s,\@libexecdir\@,$(libexecdir),g" < $< > $@

desktop_DATA =								\
	$(desktop_in_files:.desktop.in=.desktop)

@INTLTOOL_DESKTOP_RULE@

EXTRA_DIST =								\
	$(desktop_in_in_files)						

CLEANFILES =								\
	$(desktop_in_files)						\
	$(desktop_DATA)

# vi:set ts=8 sw=8 noet ai nocindent syntax=automake:
