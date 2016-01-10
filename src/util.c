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

#include <stdlib.h>
#include <string.h>
#include <errno.h>

#include <sys/ipc.h>
#include <sys/shm.h>

#include "yad.h"

#define SETTINGS_FILE "yad.conf"

YadSettings settings;

void
read_settings (void)
{
  GKeyFile *kf;
  gchar *filename;

  /* set defaults */
  settings.width = settings.height = -1;
  settings.timeout = 0;
  settings.to_indicator = "none";
  settings.show_remain = FALSE;
  settings.rules_hint = TRUE;
  settings.always_selected = FALSE;
#if !GTK_CHECK_VERSION(2,22,0)
  settings.dlg_sep = FALSE;
#endif
  settings.combo_always_editable = FALSE;
  settings.show_gtk_palette = FALSE;
  settings.expand_palette = FALSE;
  settings.term = "xterm -e %s";
  settings.ignore_unknown = TRUE;
  settings.max_tab = 100;

  settings.print_settings = NULL;
  settings.page_setup = NULL;

  filename = g_build_filename (g_get_user_config_dir (), SETTINGS_FILE, NULL);

  if (g_file_test (filename, G_FILE_TEST_EXISTS))
    {
      kf = g_key_file_new ();

      if (g_key_file_load_from_file (kf, filename, G_KEY_FILE_NONE, NULL))
        {
#if !GTK_CHECK_VERSION(2,22,0)
          if (g_key_file_has_key (kf, "General", "dialog_separator", NULL))
            settings.dlg_sep = g_key_file_get_boolean (kf, "General", "dialog_separator", NULL);
#endif
          if (g_key_file_has_key (kf, "General", "width", NULL))
            settings.width = g_key_file_get_integer (kf, "General", "width", NULL);
          if (g_key_file_has_key (kf, "General", "height", NULL))
            settings.height = g_key_file_get_integer (kf, "General", "height", NULL);
          if (g_key_file_has_key (kf, "General", "timeout", NULL))
            settings.timeout = g_key_file_get_integer (kf, "General", "timeout", NULL);
          if (g_key_file_has_key (kf, "General", "timeout_indicator", NULL))
            settings.to_indicator = g_key_file_get_string (kf, "General", "timeout_indicator", NULL);
          if (g_key_file_has_key (kf, "General", "show_remain", NULL))
            settings.show_remain = g_key_file_get_boolean (kf, "General", "show_remain", NULL);
          if (g_key_file_has_key (kf, "General", "rules_hint", NULL))
            settings.rules_hint = g_key_file_get_boolean (kf, "General", "rules_hint", NULL);
          if (g_key_file_has_key (kf, "General", "always_selected", NULL))
            settings.always_selected = g_key_file_get_boolean (kf, "General", "always_selected", NULL);
          if (g_key_file_has_key (kf, "General", "combo_always_editable", NULL))
            settings.combo_always_editable = g_key_file_get_boolean (kf, "General", "combo_always_editable", NULL);
          if (g_key_file_has_key (kf, "General", "show_gtk_palette", NULL))
            settings.show_gtk_palette = g_key_file_get_boolean (kf, "General", "show_gtk_palette", NULL);
          if (g_key_file_has_key (kf, "General", "expand_palette", NULL))
            settings.expand_palette = g_key_file_get_boolean (kf, "General", "expand_palette", NULL);
          if (g_key_file_has_key (kf, "General", "terminal", NULL))
            settings.term = g_key_file_get_string (kf, "General", "terminal", NULL);
          if (g_key_file_has_key (kf, "General", "ignore_unknown_options", NULL))
            settings.ignore_unknown = g_key_file_get_boolean (kf, "General", "ignore_unknown_options", NULL);
          if (g_key_file_has_key (kf, "General", "max_tab", NULL))
            settings.max_tab = g_key_file_get_integer (kf, "General", "max_tab", NULL);

          settings.print_settings = gtk_print_settings_new_from_key_file (kf, NULL, NULL);
          settings.page_setup = gtk_page_setup_new_from_key_file (kf, NULL, NULL);
        }

      g_key_file_free (kf);
    }
  else
    write_settings ();

  g_free (filename);
}

void
write_settings (void)
{
  GKeyFile *kf;
  gchar *context;

  kf = g_key_file_new ();

#if !GTK_CHECK_VERSION(2,22,0)
  g_key_file_set_boolean (kf, "General", "dialog_separator", settings.dlg_sep);
  g_key_file_set_comment (kf, "General", "dialog_separator", "Enable separator between dialog and buttons", NULL);
#endif
  g_key_file_set_integer (kf, "General", "width", settings.width);
  g_key_file_set_comment (kf, "General", "width", "Default dialog width", NULL);
  g_key_file_set_integer (kf, "General", "height", settings.height);
  g_key_file_set_comment (kf, "General", "height", "Default dialog height", NULL);
  g_key_file_set_integer (kf, "General", "timeout", settings.timeout);
  g_key_file_set_comment (kf, "General", "timeout", "Default timeout (0 for no timeout)", NULL);
  g_key_file_set_string (kf, "General", "timeout_indicator", settings.to_indicator);
  g_key_file_set_comment (kf, "General", "timeout_indicator",
                          "Position of timeout indicator (top, bottom, left, right, none)", NULL);
  g_key_file_set_boolean (kf, "General", "show_remain", settings.show_remain);
  g_key_file_set_comment (kf, "General", "show_remain", "Show remain seconds in timeout indicator", NULL);
  g_key_file_set_boolean (kf, "General", "rules_hint", settings.rules_hint);
  g_key_file_set_comment (kf, "General", "rules_hint", "Enable rules hints in list widget", NULL);
  g_key_file_set_boolean (kf, "General", "always_selected", settings.always_selected);
  g_key_file_set_comment (kf, "General", "always_selected", "List widget always have a selection", NULL);
  g_key_file_set_boolean (kf, "General", "combo_always_editable", settings.combo_always_editable);
  g_key_file_set_comment (kf, "General", "combo_always_editable", "Combo-box in entry dialog is always editable", NULL);
  g_key_file_set_boolean (kf, "General", "show_gtk_palette", settings.show_gtk_palette);
  g_key_file_set_comment (kf, "General", "show_gtk_palette", "Show GtkColorSelection palette", NULL);
  g_key_file_set_boolean (kf, "General", "expand_palette", settings.expand_palette);
  g_key_file_set_comment (kf, "General", "expand_palette", "Expand list of predefined colors in color dialog", NULL);
  g_key_file_set_string (kf, "General", "terminal", settings.term);
  g_key_file_set_comment (kf, "General", "terminal", "Default terminal command (use %s for command template)", NULL);
  g_key_file_set_boolean (kf, "General", "ignore_unknown_options", settings.ignore_unknown);
  g_key_file_set_comment (kf, "General", "ignore_unknown_options", "Ingnore unknown command-line options", NULL);
  g_key_file_set_integer (kf, "General", "max_tab", settings.max_tab);
  g_key_file_set_comment (kf, "General", "max_tab", "Maximum number of tabs in notebook", NULL);

  if (settings.print_settings)
    gtk_print_settings_to_key_file (settings.print_settings, kf, NULL);
  if (settings.page_setup)
    gtk_page_setup_to_key_file (settings.page_setup, kf, NULL);

  context = g_key_file_to_data (kf, NULL, NULL);

  g_key_file_free (kf);

  if (g_mkdir_with_parents (g_get_user_config_dir (), 0755) != -1)
    {
      gchar *filename = g_build_filename (g_get_user_config_dir (), SETTINGS_FILE, NULL);
      g_file_set_contents (filename, context, -1, NULL);
      g_free (filename);
    }
  else
    g_printerr ("yad: cannot write settings file: %s\n", strerror (errno));

  g_free (context);
}

GdkPixbuf *
get_pixbuf (gchar * name, YadIconSize size)
{
  gint w, h;
  GdkPixbuf *pb = NULL;
  GError *err = NULL;

  if (g_file_test (name, G_FILE_TEST_EXISTS))
    {
      pb = gdk_pixbuf_new_from_file (name, &err);
      if (!pb)
        {
          g_printerr ("yad_get_pixbuf(): %s\n", err->message);
          g_error_free (err);
        }
    }
  else
    {
      if (size == YAD_BIG_ICON)
        gtk_icon_size_lookup (GTK_ICON_SIZE_DIALOG, &w, &h);
      else
        gtk_icon_size_lookup (GTK_ICON_SIZE_MENU, &w, &h);

      pb = gtk_icon_theme_load_icon (settings.icon_theme, name, MIN (w, h), GTK_ICON_LOOKUP_GENERIC_FALLBACK, NULL);
    }

  if (!pb)
    {
      if (size == YAD_BIG_ICON)
        pb = settings.big_fallback_image;
      else
        pb = settings.small_fallback_image;
    }

  return pb;
}

#ifdef __clang__
extern inline void
#else
inline void
#endif
strip_new_line (gchar * str)
{
  gint nl = strlen (str) - 1;

  if (str[nl] == '\n')
    str[nl] = '\0';
}

gchar **
split_arg (const gchar * str)
{
  gchar **res;
  gchar *p_col;

  res = g_new0 (gchar *, 3);

  p_col = g_strrstr (str, ":");
  if (p_col && p_col[1])
    {
      res[0] = g_strndup (str, p_col - str);
      res[1] = g_strdup (p_col + 1);
    }
  else
    res[0] = g_strdup (str);

  return res;
}

YadNTabs *
get_tabs (key_t key, gboolean create)
{
  YadNTabs *t = NULL;
  int shmid, i;

  /* get shared memory */
  if (create)
    {
      if ((shmid = shmget (key, (settings.max_tab + 1) * sizeof (YadNTabs), IPC_CREAT | IPC_EXCL | 0644)) == -1)
        {
          g_printerr ("yad: cannot create shared memory for key %ld: %s\n", key, strerror (errno));
          return NULL;
        }
    }
  else
    {
      if ((shmid = shmget (key, (settings.max_tab + 1) * sizeof (YadNTabs), 0)) == -1)
        {
          if (errno != ENOENT)
            g_printerr ("yad: cannot get shared memory for key %ld: %s\n", key, strerror (errno));
          return NULL;
        }
    }

  /* attach shared memory */
  if ((t = shmat (shmid, NULL, 0)) == (YadNTabs *) -1)
    {
      g_printerr ("yad: cannot attach shared memory for key %ld: %s\n", key, strerror (errno));
      return NULL;
    }

  /* initialize memory */
  if (create)
    {
      for (i = 0; i < settings.max_tab + 1; i++)
        {
          t[i].pid = -1;
          t[i].xid = 0;
        }
      t[0].pid = shmid;
    }

  return t;
}

GtkWidget *
get_label (gchar *str, guint border)
{
  GtkWidget *a, *t, *i, *l;
  GtkStockItem it;
  gchar **vals;

  if (!str)
    return gtk_label_new ("");

  l = i = NULL;

  a = gtk_alignment_new (0.0, 0.5, 0, 0);
  gtk_container_set_border_width (GTK_CONTAINER (a), border);

#if !GTK_CHECK_VERSION(3,0,0)
  t = gtk_hbox_new (FALSE, 0);
#else
  t = gtk_box_new (GTK_ORIENTATION_HORIZONTAL, 0);
#endif
  gtk_container_add (GTK_CONTAINER (a), t);

  vals = g_strsplit_set (str, options.common_data.item_separator, 3);
  if (gtk_stock_lookup (vals[0], &it))
    {
      l = gtk_label_new_with_mnemonic (it.label);
      gtk_misc_set_alignment (GTK_MISC (l), 0.0, 0.5);

      i = gtk_image_new_from_pixbuf (get_pixbuf (it.stock_id, YAD_SMALL_ICON));
    }
  else
    {
      if (vals[0] && *vals[0])
        {
          l = gtk_label_new (NULL);
          if (!options.data.no_markup)
            gtk_label_set_markup_with_mnemonic (GTK_LABEL (l), vals[0]);
          else
            gtk_label_set_text_with_mnemonic (GTK_LABEL (l), vals[0]);
          gtk_misc_set_alignment (GTK_MISC (l), 0.0, 0.5);
        }

      if (vals[1] && *vals[1])
        i = gtk_image_new_from_pixbuf (get_pixbuf (vals[1], YAD_SMALL_ICON));
    }

  if (i)
    gtk_box_pack_start (GTK_BOX (t), i, FALSE, FALSE, 1);
  if (l)
    gtk_box_pack_start (GTK_BOX (t), l, FALSE, FALSE, 1);

  /* !!! must check both 1 and 2 values for !NULL */
  if (vals[1] && vals[2] && *vals[2])
    {
      if (!options.data.no_markup)
        gtk_widget_set_tooltip_markup (t, vals[2]);
      else
        gtk_widget_set_tooltip_text (t, vals[2]);
    }

  g_strfreev (vals);

  gtk_widget_show_all (a);

  return a;
}

char *
escape_str (char *str)
{
  char *res, *buf = str;
  unsigned i = 0, len;

  if (!str)
    return NULL;

  len = strlen (str);
  res = (char *) calloc (len + 1, sizeof (char));

  while (*buf)
    {
      switch (*buf)
        {
        case '\n':
          len += 1;
          res = (char *) realloc (res, len + 1);
          strcpy (res + i, "\\n");
          i += 2;
          break;
        case '\t':
          len += 1;
          res = (char *) realloc (res, len + 1);
          strcpy (res + i, "\\t");
          i += 2;
          break;
        default:
          *(res + i) = *buf;
          i++;
          break;
        }
      buf++;
    }
  res[i] = '\0';

  return res;
}
