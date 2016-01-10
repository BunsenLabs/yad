/*
 * This file is part of YAD.
 *
 * YAD is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 3 of the License, or
 * (at your option) any later version.
 *
 * YAD is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with YAD. If not, see <http://www.gnu.org/licenses/>.
 *
 * Copyright (C) 2008-2014, Victor Ananjevsky <ananasik@gmail.com>
 */

#ifndef _YAD_H_
#define _YAD_H_

#include <config.h>

#include <sys/types.h>
#include <sys/ipc.h>

#include <gdk/gdkx.h>

#include <gtk/gtk.h>
#include <glib/gi18n.h>

#if GTK_CHECK_VERSION(3,0,0)
#include <gtk/gtkx.h>
#endif

G_BEGIN_DECLS

#define YAD_SETTINGS_FILE "yad.conf"

#define YAD_RESPONSE_OK         0
#define YAD_RESPONSE_CANCEL     1
#define YAD_RESPONSE_TIMEOUT   	70
#define YAD_RESPONSE_ESC        -4      /* 252 */
#define YAD_URL_REGEX "(http|https|ftp)://[a-zA-Z0-9./_%#&-]+"

typedef enum {
  YAD_MODE_MESSAGE,
  YAD_MODE_CALENDAR,
  YAD_MODE_COLOR,
  YAD_MODE_DND,
  YAD_MODE_ENTRY,
  YAD_MODE_FILE,
  YAD_MODE_FONT,
  YAD_MODE_FORM,
  YAD_MODE_ICONS,
  YAD_MODE_LIST,
  YAD_MODE_MULTI_PROGRESS,
  YAD_MODE_NOTEBOOK,
  YAD_MODE_NOTIFICATION,
  YAD_MODE_PRINT,
  YAD_MODE_PROGRESS,
  YAD_MODE_SCALE,
  YAD_MODE_TEXTINFO,
  YAD_MODE_ABOUT,
  YAD_MODE_VERSION,
} YadDialogMode;

typedef enum {
  YAD_FIELD_SIMPLE = 0,
  YAD_FIELD_HIDDEN,
  YAD_FIELD_READ_ONLY,
  YAD_FIELD_NUM,
  YAD_FIELD_CHECK,
  YAD_FIELD_COMBO,
  YAD_FIELD_COMBO_ENTRY,
  YAD_FIELD_FILE,
  YAD_FIELD_FILE_SAVE,
  YAD_FIELD_MFILE,
  YAD_FIELD_DIR,
  YAD_FIELD_DIR_CREATE,
  YAD_FIELD_MDIR,
  YAD_FIELD_FONT,
  YAD_FIELD_COLOR,
  YAD_FIELD_DATE,
  YAD_FIELD_SCALE,
  YAD_FIELD_BUTTON,
  YAD_FIELD_FULL_BUTTON,
  YAD_FIELD_LABEL,
  YAD_FIELD_TEXT,
} YadFieldType;

typedef enum {
  YAD_COLUMN_TEXT = 0,
  YAD_COLUMN_NUM,
  YAD_COLUMN_FLOAT,
  YAD_COLUMN_CHECK,
  YAD_COLUMN_RADIO,
  YAD_COLUMN_IMAGE,
  YAD_COLUMN_HIDDEN,
  YAD_COLUMN_ATTR_FORE,
  YAD_COLUMN_ATTR_BACK,
  YAD_COLUMN_ATTR_FONT,
} YadColumnType;

typedef enum {
  YAD_PRINT_TEXT = 0,
  YAD_PRINT_IMAGE,
  YAD_PRINT_RAW,
} YadPrintType;

typedef enum {
  YAD_PROGRESS_NORMAL = 0,
  YAD_PROGRESS_RTL,
  YAD_PROGRESS_PULSE,
} YadProgressType;

typedef enum {
  YAD_BIG_ICON = 0,
  YAD_SMALL_ICON,
} YadIconSize;

typedef struct {
  gchar *name;
  gchar *cmd;
  gint response;
} YadButton;

typedef struct {
  gchar *name;
  YadFieldType type;
} YadField;

typedef struct {
  gchar *name;
  YadColumnType type;
} YadColumn;

typedef struct {
  gchar *name;
  YadProgressType type;
} YadProgressBar;

typedef struct {
  gchar *name;
  gint value;
} YadScaleMark;

typedef struct {
  gchar *dialog_title;
  gchar *window_icon;
  gint width;
  gint height;
  gchar *geometry;
  guint timeout;
  gchar *to_indicator;
  gchar *dialog_text;
  GtkJustification text_align;
  gchar *dialog_image;
  gboolean image_on_top;
  gchar *icon_theme;
#if !GTK_CHECK_VERSION(3,0,0)
  gboolean dialog_sep;
#endif
  gchar *expander;
  gint borders;
  GSList *buttons;
  gboolean no_buttons;
  gboolean no_markup;
  gboolean always_print;
  gboolean selectable_labels;
  GtkButtonBoxStyle buttons_layout;
  /* window settings */
  gboolean sticky;
  gboolean fixed;
  gboolean ontop;
  gboolean center;
  gboolean mouse;
  gboolean undecorated;
  gboolean skip_taskbar;
  gboolean maximized;
  gboolean fullscreen;
} YadData;

typedef struct {
  gint day;
  gint month;
  gint year;
  gchar *details;
} YadCalendarData;

typedef struct {
  gchar *init_color;
  gboolean use_palette;
  gchar *palette;
  gboolean extra;
} YadColorData;

typedef struct {
  gboolean tooltip;
} YadDNDData;

typedef struct {
  gchar *entry_text;
  gchar *entry_label;
  gboolean hide_text;
  gboolean completion;
  gboolean numeric;
  gchar *licon;
  gchar *licon_action;
  gchar *ricon;
  gchar *ricon_action;
} YadEntryData;

typedef struct {
  gboolean directory;
  gboolean save;
  gboolean confirm_overwrite;
  gchar *confirm_text;
  gchar **filter;
} YadFileData;

typedef struct {
  gchar *preview;
} YadFontData;

typedef struct {
  GSList *fields;
  guint columns;
  gboolean scroll;
} YadFormData;

typedef struct {
  gchar *directory;
  gboolean compact;
  gboolean generic;
  gboolean descend;
  gboolean sort_by_name;
  gboolean single_click;
  guint width;
  gchar *term;
} YadIconsData;

typedef struct {
  GSList *columns;
  gboolean no_headers;
  gboolean checkbox;
  gboolean radiobox;
  gboolean print_all;
  gint print_column;
  gint hide_column;
  gint expand_column;
  gint search_column;
  gint tooltip_column;
  guint limit;
  PangoEllipsizeMode ellipsize;
  gchar *dclick_action;
  gboolean regex_search;
  gboolean clickable;
} YadListData;

typedef struct {
  GSList *bars;
} YadMultiProgressData;

typedef struct {
  GSList *tabs;
  guint borders;
  GtkPositionType pos;
  key_t key;
} YadNotebookData;

typedef struct {
  gboolean middle;
  gboolean hidden;
  gchar *menu;
} YadNotificationData;

typedef struct {
  YadPrintType type;
  gboolean headers;
} YadPrintData;

typedef struct {
  gchar *progress_text;
  gboolean pulsate;
  gboolean autoclose;
#ifndef G_OS_WIN32
  gboolean autokill;
#endif
  guint percentage;
  gboolean rtl;
  gchar *log;
  gboolean log_expanded;
  gboolean log_on_top;
  gint log_height;
} YadProgressData;

typedef struct {
  gint value;
  gint min_value;
  gint max_value;
  gint step;
  gint page;
  gboolean print_partial;
  gboolean hide_value;
  gboolean have_value;
  gboolean invert;
  GSList *marks;
} YadScaleData;

typedef struct {
  gchar *fore;
  gchar *back;
  gboolean wrap;
  GtkJustification justify;
  gint margins;
  gboolean tail;
  gboolean uri;
} YadTextData;

typedef struct {
  gchar *uri;
  gchar *font;
  gchar *separator;
  gchar *item_separator;
  gboolean editable;
  gboolean multi;
  gboolean vertical;
  gchar *command;
  gchar *date_format;
  gdouble align;
  gboolean listen;
  gboolean preview;
  gboolean quoted_output;
} YadCommonData;

typedef struct {
  YadDialogMode mode;

  YadData data;
  YadCommonData common_data;

  YadCalendarData calendar_data;
  YadColorData color_data;
  YadDNDData dnd_data;
  YadEntryData entry_data;
  YadFileData file_data;
  YadFontData font_data;
  YadFormData form_data;
  YadIconsData icons_data;
  YadListData list_data;
  YadMultiProgressData multi_progress_data;
  YadNotebookData notebook_data;
  YadNotificationData notification_data;
  YadPrintData print_data;
  YadProgressData progress_data;
  YadScaleData scale_data;
  YadTextData text_data;

  gchar *gtkrc_file;

  gchar *rest_file;
  gchar **extra_data;

  key_t plug;
  guint tabnum;

#ifndef G_OS_WIN32
  guint kill_parent;
  gboolean print_xid;
#endif
} YadOptions;

extern YadOptions options;

typedef struct {
  guint width;
  guint height;
  guint timeout;
  gchar *to_indicator;
  gboolean show_remain;
  gboolean rules_hint;
  gboolean always_selected;
#if !GTK_CHECK_VERSION(3,0,0)
  gboolean dlg_sep;
#endif
  gboolean combo_always_editable;
  gboolean show_gtk_palette;
  gboolean expand_palette;
  gboolean ignore_unknown;
  GtkIconTheme *icon_theme;
  GdkPixbuf *big_fallback_image;
  GdkPixbuf *small_fallback_image;
  gchar *term;
  guint max_tab;

  GtkPrintSettings *print_settings;
  GtkPageSetup *page_setup;

} YadSettings;

extern YadSettings settings;

typedef struct {
  pid_t pid;
  Window xid;
} YadNTabs;

/* pointer to shared memory for tabbed dialog */
/* 0 item used for special info: */
/*   pid - memory id */
/*   xid - count of registered tabs (for sync) */
extern YadNTabs *tabs;
extern gint t_sem;

void yad_options_init (void);
GOptionContext *yad_create_context (void);
void yad_set_mode (void);

GtkWidget *calendar_create_widget (GtkWidget * dlg);
GtkWidget *color_create_widget (GtkWidget * dlg);
GtkWidget *entry_create_widget (GtkWidget * dlg);
GtkWidget *file_create_widget (GtkWidget * dlg);
GtkWidget *font_create_widget (GtkWidget * dlg);
GtkWidget *form_create_widget (GtkWidget * dlg);
GtkWidget *icons_create_widget (GtkWidget * dlg);
GtkWidget *list_create_widget (GtkWidget * dlg);
GtkWidget *multi_progress_create_widget (GtkWidget * dlg);
GtkWidget *notebook_create_widget (GtkWidget * dlg);
GtkWidget *progress_create_widget (GtkWidget * dlg);
GtkWidget *scale_create_widget (GtkWidget * dlg);
GtkWidget *text_create_widget (GtkWidget * dlg);

void confirm_overwrite_cb (GtkDialog * dlg, gint id, gpointer data);
void notebook_swallow_childs (void);

void calendar_print_result (void);
void color_print_result (void);
void entry_print_result (void);
void file_print_result (void);
void font_print_result (void);
void form_print_result (void);
void list_print_result (void);
void notebook_print_result (void);
void scale_print_result (void);
void text_print_result (void);

void dnd_init (GtkWidget * w);

gint yad_notification_run (void);
gint yad_print_run (void);
gint yad_about (void);

void notebook_close_childs (void);

void read_settings (void);
void write_settings (void);

GdkPixbuf *get_pixbuf (gchar * name, YadIconSize size);

#ifdef __clang__
extern inline void strip_new_line (gchar * str);
#else
inline void strip_new_line (gchar * str);
#endif

gchar **split_arg (const gchar * str);

YadNTabs *get_tabs (key_t key, gboolean create);

GtkWidget *get_label (gchar *str, guint border);

char *escape_str (char *str);

G_END_DECLS

#endif /* _YAD_H_ */
