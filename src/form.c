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

#include <glib/gprintf.h>

#include "yad.h"

#include "calendar.xpm"

static GSList *fields = NULL;
static guint n_fields;

static void button_clicked_cb (GtkButton * b, gchar * action);

/* expand %N in command to fields values */
static GString *
expand_action (gchar * cmd)
{
  GString *xcmd;
  guint i = 0;

  xcmd = g_string_new ("");
  while (cmd[i])
    {
      if (cmd[i] == '%')
        {
          i++;
          if (g_ascii_isdigit (cmd[i]))
            {
              YadField *fld;
              gchar *buf;
              guint num, j = i;

              /* get field num */
              while (g_ascii_isdigit (cmd[j]))
                j++;
              buf = g_strndup (cmd + i, j - i);
              num = g_ascii_strtoll (buf, NULL, 10);
              g_free (buf);
              if (num > 0 && num <= n_fields)
                num--;
              else
                continue;
              /* get field value */
              fld = g_slist_nth_data (options.form_data.fields, num);
              switch (fld->type)
                {
                case YAD_FIELD_SIMPLE:
                case YAD_FIELD_HIDDEN:
                case YAD_FIELD_READ_ONLY:
                case YAD_FIELD_FILE_SAVE:
                case YAD_FIELD_DIR_CREATE:
                case YAD_FIELD_MFILE:
                case YAD_FIELD_MDIR:
                case YAD_FIELD_DATE:
                  g_string_append (xcmd, gtk_entry_get_text (GTK_ENTRY (g_slist_nth_data (fields, num))));
                  break;
                case YAD_FIELD_NUM:
                  g_string_append_printf (xcmd, "%f", gtk_spin_button_get_value
                                          (GTK_SPIN_BUTTON (g_slist_nth_data (fields, num))));
                  break;
                case YAD_FIELD_CHECK:
                  g_string_append (xcmd, gtk_toggle_button_get_active
                                   (GTK_TOGGLE_BUTTON (g_slist_nth_data (fields, num))) ? "TRUE" : "FALSE");
                  break;
                case YAD_FIELD_COMBO:
                case YAD_FIELD_COMBO_ENTRY:
                  g_string_append (xcmd,
#if GTK_CHECK_VERSION(2,24,0)
                                   gtk_combo_box_text_get_active_text (GTK_COMBO_BOX_TEXT
                                                                       (g_slist_nth_data (fields, num)))
#else
                                   gtk_combo_box_get_active_text (GTK_COMBO_BOX (g_slist_nth_data (fields, num)))
#endif
                                   );
                  break;
                case YAD_FIELD_SCALE:
                  g_string_append_printf (xcmd, "%d", (gint) gtk_range_get_value
                                          (GTK_RANGE (g_slist_nth_data (fields, num))));
                  break;
                case YAD_FIELD_FILE:
                case YAD_FIELD_DIR:
                  g_string_append (xcmd, gtk_file_chooser_get_filename (GTK_FILE_CHOOSER (g_slist_nth_data (fields, num))));
                  break;
                case YAD_FIELD_FONT:
                  g_string_append (xcmd, gtk_font_button_get_font_name (GTK_FONT_BUTTON (g_slist_nth_data (fields, num))));
                  break;
                case YAD_FIELD_COLOR:
                  {
                    GdkColor c;

                    gtk_color_button_get_color (GTK_COLOR_BUTTON (g_slist_nth_data (fields, num)), &c);
                    buf = gdk_color_to_string (&c);
                    g_free (buf);
                    break;
                  }
                case YAD_FIELD_TEXT:
                  {
                    gchar *txt;
                    GtkTextBuffer *tb;
                    GtkTextIter b, e;

                    tb = gtk_text_view_get_buffer (GTK_TEXT_VIEW (g_slist_nth_data (fields, num)));
                    gtk_text_buffer_get_bounds (tb, &b, &e);
                    buf = gtk_text_buffer_get_text (tb, &b, &e, FALSE);
                    txt = escape_str (buf);
                    g_string_append (xcmd, txt);
                    g_free (txt);
                    g_free (buf);
                  }
                default:;
                }
              i = j;
            }
          else
            {
              g_string_append_c (xcmd, cmd[i]);
              i++;
            }
        }
      else
        {
          g_string_append_c (xcmd, cmd[i]);
          i++;
        }
    }
  g_string_append_c (xcmd, '\0');

  return xcmd;
}

static void
set_field_value (guint num, gchar * value)
{
  GtkWidget *w;
  gchar **s;
  YadField *fld = g_slist_nth_data (options.form_data.fields, num);

  w = GTK_WIDGET (g_slist_nth_data (fields, num));
  if (g_ascii_strcasecmp (value, "@disabled@") == 0)
    {
      gtk_widget_set_sensitive (w, FALSE);
      return;
    }
  else
    gtk_widget_set_sensitive (w, TRUE);

  switch (fld->type)
    {
    case YAD_FIELD_READ_ONLY:
      gtk_widget_set_sensitive (w, FALSE);
    case YAD_FIELD_SIMPLE:
    case YAD_FIELD_HIDDEN:
    case YAD_FIELD_MFILE:
    case YAD_FIELD_MDIR:
    case YAD_FIELD_FILE_SAVE:
    case YAD_FIELD_DIR_CREATE:
    case YAD_FIELD_DATE:
      gtk_entry_set_text (GTK_ENTRY (w), value);
      break;

    case YAD_FIELD_NUM:
      s = g_strsplit (value, options.common_data.item_separator, -1);
      if (s[0])
        {
          gdouble val = g_ascii_strtod (s[0], NULL);
          w = g_slist_nth_data (fields, num);
          if (s[1])
            {
              gdouble min, max;
              gchar **s1 = g_strsplit (s[1], "..", 2);
              min = g_ascii_strtod (s1[0], NULL);
              max = g_ascii_strtod (s1[1], NULL);
              g_strfreev (s1);
              gtk_spin_button_set_range (GTK_SPIN_BUTTON (w), min, max);
              if (s[2])
                {
                  gdouble step = g_ascii_strtod (s[2], NULL);
                  gtk_spin_button_set_increments (GTK_SPIN_BUTTON (w), step, step * 10);
                  if (s[3])
                    {
                      guint prec = (guint) g_ascii_strtoull (s[3], NULL, 0);
                      if (prec > 20)
                        prec = 20;
                      gtk_spin_button_set_digits (GTK_SPIN_BUTTON (w), prec);
                    }
                }
            }
          /* set initial value must be after setting range and step */
          gtk_spin_button_set_value (GTK_SPIN_BUTTON (w), val);
        }
      g_strfreev (s);
      break;

    case YAD_FIELD_CHECK:
      if (g_ascii_strcasecmp (value, "TRUE") == 0)
        gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (w), TRUE);
      else
        gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (w), FALSE);
      break;

    case YAD_FIELD_COMBO:
    case YAD_FIELD_COMBO_ENTRY:
      {
        gint i = 0, def = 0;

        s = g_strsplit (value, options.common_data.item_separator, -1);
        while (s[i])
          {
            gchar *buf;

            if (s[i][0] == '^')
              {
                buf = g_strcompress (s[i] + 1);
                def = i;
              }
            else
              buf = g_strcompress (s[i]);
#if GTK_CHECK_VERSION(2,24,0)
            gtk_combo_box_text_append_text (GTK_COMBO_BOX_TEXT (w), buf);
#else
            gtk_combo_box_append_text (GTK_COMBO_BOX (w), buf);
#endif
            g_free (buf);
            i++;
          }
        gtk_combo_box_set_active (GTK_COMBO_BOX (w), def);
        g_strfreev (s);
        break;
      }

    case YAD_FIELD_DIR:
      gtk_file_chooser_set_current_folder (GTK_FILE_CHOOSER (w), value);
    case YAD_FIELD_FILE:
      gtk_file_chooser_set_filename (GTK_FILE_CHOOSER (w), value);
      break;

    case YAD_FIELD_FONT:
      gtk_font_button_set_font_name (GTK_FONT_BUTTON (w), value);
      break;

    case YAD_FIELD_SCALE:
      gtk_range_set_value (GTK_RANGE (w), atoi (value));
      break;

    case YAD_FIELD_COLOR:
      {
        GdkColor c;

        gdk_color_parse (value, &c);
        gtk_color_button_set_color (GTK_COLOR_BUTTON (w), &c);
        break;
      }

    case YAD_FIELD_BUTTON:
    case YAD_FIELD_FULL_BUTTON:
      g_signal_connect (G_OBJECT (w), "clicked", G_CALLBACK (button_clicked_cb), value);
      break;

    case YAD_FIELD_TEXT:
      {
        GtkTextBuffer *tb = gtk_text_view_get_buffer (GTK_TEXT_VIEW (w));
        gchar *txt = g_strcompress (value);
        gtk_text_buffer_set_text (tb, txt, -1);
        g_free (txt);
        break;
      }

    default: ;
    }
}

static void
button_clicked_cb (GtkButton * b, gchar * action)
{
  static gchar *newline = NULL;

  if (!newline)
    newline = g_strcompress ("\n");

  if (action && action[0])
    {
      if (action[0] == '@')
        {
          gchar *data;
          gint exit = 1;
          GString *cmd = expand_action (action + 1);
          g_spawn_command_line_sync (cmd->str, &data, NULL, &exit, NULL);
          if (exit == 0)
            {
              guint i = 0;
              gchar **lines = g_strsplit (data, "\n", 0);
              while (lines[i] && lines[i][0])
                {
                  gint fn;
                  gchar **ln;

                  if (!lines[i][0])
                    continue;

                  ln = g_strsplit (lines[i], ":", 2);
                  fn = g_ascii_strtoll (ln[0], NULL, 10);
                  if (fn && ln[1])
                    set_field_value (fn - 1, ln[1]);
                  g_strfreev (ln);
                  i++;
                }
            }
          g_free (data);
          g_string_free (cmd, TRUE);
        }
      else
        {
          GString *cmd = expand_action (action);
          g_spawn_command_line_async (cmd->str, NULL);
          g_string_free (cmd, TRUE);
        }
    }
}

static void
form_activate_cb (GtkEntry * entry, gpointer data)
{
  if (options.plug == -1)
    gtk_dialog_response (GTK_DIALOG (data), YAD_RESPONSE_OK);
}

static void
select_files_cb (GtkEntry * entry, GtkEntryIconPosition pos, GdkEventButton * event, gpointer data)
{
  GtkWidget *dlg;
  static gchar *path = NULL;

  if (event->button == 1)
    {
      YadFieldType type = GPOINTER_TO_INT (data);

      if (!path)
        {
          const gchar *val = gtk_entry_get_text (entry);

          if (g_file_test (val, G_FILE_TEST_IS_DIR))
            path = g_strdup (val);
          else
            path = val ? g_path_get_dirname (val) : g_get_current_dir ();
        }

      if (type == YAD_FIELD_MFILE)
        {
          dlg = gtk_file_chooser_dialog_new (_("Select files"),
                                             GTK_WINDOW (gtk_widget_get_toplevel (GTK_WIDGET (entry))),
                                             GTK_FILE_CHOOSER_ACTION_OPEN,
                                             GTK_STOCK_CANCEL, GTK_RESPONSE_CANCEL,
                                             GTK_STOCK_OK, GTK_RESPONSE_ACCEPT, NULL);
        }
      else
        {
          dlg = gtk_file_chooser_dialog_new (_("Select folders"),
                                             GTK_WINDOW (gtk_widget_get_toplevel (GTK_WIDGET (entry))),
                                             GTK_FILE_CHOOSER_ACTION_SELECT_FOLDER,
                                             GTK_STOCK_CANCEL, GTK_RESPONSE_CANCEL,
                                             GTK_STOCK_OK, GTK_RESPONSE_ACCEPT, NULL);
        }
      gtk_file_chooser_set_select_multiple (GTK_FILE_CHOOSER (dlg), TRUE);
      gtk_file_chooser_set_current_folder (GTK_FILE_CHOOSER (dlg), path);

      if (gtk_dialog_run (GTK_DIALOG (dlg)) == GTK_RESPONSE_ACCEPT)
        {
          GSList *files, *ptr;
          GString *str;

          str = g_string_new ("");
          files = ptr = gtk_file_chooser_get_uris (GTK_FILE_CHOOSER (dlg));

          while (ptr)
            {
              if (ptr->data)
                {
                  gchar *fn = g_filename_from_uri ((gchar *) ptr->data, NULL, NULL);
                  g_string_append (str, fn);
                  g_string_append (str, options.common_data.item_separator);
                  g_free (fn);
                }
              ptr = ptr->next;
            }

          str->str[str->len-1] = '\0'; // remove last item separator
          gtk_entry_set_text (entry, str->str);

          g_slist_free (files);
          g_string_free (str, TRUE);
        }

      g_free (path);
      path = gtk_file_chooser_get_current_folder (GTK_FILE_CHOOSER (dlg));
      gtk_widget_destroy (dlg);
    }
}

static void
create_files_cb (GtkEntry * entry, GtkEntryIconPosition pos, GdkEventButton * event, gpointer data)
{
  GtkWidget *dlg;
  static gchar *path = NULL;

  if (event->button == 1)
    {
      YadFieldType type = GPOINTER_TO_INT (data);

      if (!path)
        {
          const gchar *val = gtk_entry_get_text (entry);

          if (g_file_test (val, G_FILE_TEST_IS_DIR))
            path = g_strdup (val);
          else
            path = val ? g_path_get_dirname (val) : g_get_current_dir ();
        }

      if (type == YAD_FIELD_FILE_SAVE)
        {
          dlg = gtk_file_chooser_dialog_new (_("Select or create file"),
                                             GTK_WINDOW (gtk_widget_get_toplevel (GTK_WIDGET (entry))),
                                             GTK_FILE_CHOOSER_ACTION_SAVE,
                                             GTK_STOCK_CANCEL, GTK_RESPONSE_CANCEL,
                                             GTK_STOCK_OK, GTK_RESPONSE_ACCEPT, NULL);
        }
      else
        {
          dlg = gtk_file_chooser_dialog_new (_("Select or create folder"),
                                             GTK_WINDOW (gtk_widget_get_toplevel (GTK_WIDGET (entry))),
                                             GTK_FILE_CHOOSER_ACTION_CREATE_FOLDER,
                                             GTK_STOCK_CANCEL, GTK_RESPONSE_CANCEL,
                                             GTK_STOCK_OK, GTK_RESPONSE_ACCEPT, NULL);
        }
      gtk_file_chooser_set_current_folder (GTK_FILE_CHOOSER (dlg), path);

      if (gtk_dialog_run (GTK_DIALOG (dlg)) == GTK_RESPONSE_ACCEPT)
        {
          gchar *file = gtk_file_chooser_get_filename (GTK_FILE_CHOOSER (dlg));

          gtk_entry_set_text (entry, file);
          g_free (file);
        }

      g_free (path);
      path = gtk_file_chooser_get_current_folder (GTK_FILE_CHOOSER (dlg));
      gtk_widget_destroy (dlg);
    }
}

static void
date_selected_cb (GtkCalendar * c, gpointer data)
{
  gtk_dialog_response (GTK_DIALOG (data), GTK_RESPONSE_ACCEPT);
}

static void
select_date_cb (GtkEntry * entry, GtkEntryIconPosition pos, GdkEventButton * event, gpointer data)
{
  GtkWidget *dlg, *cal;

  if (event->button == 1)
    {
      GDate *d;

      dlg = gtk_dialog_new_with_buttons (_("Select date"),
                                         GTK_WINDOW (gtk_widget_get_toplevel (GTK_WIDGET (entry))),
                                         GTK_DIALOG_MODAL | GTK_DIALOG_DESTROY_WITH_PARENT,
                                         GTK_STOCK_CANCEL, GTK_RESPONSE_CANCEL,
                                         GTK_STOCK_OK, GTK_RESPONSE_ACCEPT, NULL);
      cal = gtk_calendar_new ();
      gtk_widget_show (cal);
      g_signal_connect (G_OBJECT (cal), "day-selected-double-click", G_CALLBACK (date_selected_cb), dlg);
      gtk_box_pack_start (GTK_BOX (gtk_dialog_get_content_area (GTK_DIALOG (dlg))), cal, TRUE, TRUE, 5);

      d = g_date_new ();
      g_date_set_parse (d, gtk_entry_get_text (entry));
      if (g_date_valid (d))
        {
          gtk_calendar_select_day (GTK_CALENDAR (cal), g_date_get_day (d));
          gtk_calendar_select_month (GTK_CALENDAR (cal), g_date_get_month (d) - 1, g_date_get_year (d));
        }
      g_date_free (d);

      if (gtk_dialog_run (GTK_DIALOG (dlg)) == GTK_RESPONSE_ACCEPT)
        {
          guint day, month, year;
          gchar *format = options.common_data.date_format;
          gchar time_string[128];

          if (format == NULL)
            format = "%x";

          gtk_calendar_get_date (GTK_CALENDAR (cal), &day, &month, &year);
          d = g_date_new_dmy (year, month + 1, day);
          g_date_strftime (time_string, 127, format, d);
          gtk_entry_set_text (entry, time_string);
          g_date_free (d);
        }
      gtk_widget_destroy (dlg);
    }
}

GtkWidget *
form_create_widget (GtkWidget * dlg)
{
  GtkWidget *sw, *vp, *tbl;
  GtkWidget *w = NULL;

  if (options.form_data.fields)
    {
      GtkWidget *l, *e;
      GdkPixbuf *pb;
      guint i, col, row, rows;

      n_fields = g_slist_length (options.form_data.fields);

      row = col = 0;
      rows = n_fields / options.form_data.columns;
      if (n_fields % options.form_data.columns > 0)
        rows++;

#if !GTK_CHECK_VERSION(3,0,0)
      tbl = gtk_table_new (n_fields, 2 * options.form_data.columns, FALSE);
#else
      tbl = gtk_grid_new ();
      gtk_grid_set_row_spacing (GTK_GRID (tbl), 5);
      gtk_grid_set_column_spacing (GTK_GRID (tbl), 5);
#endif

      if (options.form_data.scroll)
        {
          /* create scrolled container */
          GtkAdjustment *hadj, *vadj;

          sw = gtk_scrolled_window_new (NULL, NULL);
          gtk_scrolled_window_set_shadow_type (GTK_SCROLLED_WINDOW (sw), GTK_SHADOW_NONE);
          gtk_scrolled_window_set_policy (GTK_SCROLLED_WINDOW (sw), GTK_POLICY_AUTOMATIC, GTK_POLICY_AUTOMATIC);

          hadj = gtk_scrolled_window_get_hadjustment (GTK_SCROLLED_WINDOW (sw));
          vadj = gtk_scrolled_window_get_vadjustment (GTK_SCROLLED_WINDOW (sw));

          vp = gtk_viewport_new (hadj, vadj);
          gtk_viewport_set_shadow_type (GTK_VIEWPORT (vp), GTK_SHADOW_NONE);
          gtk_container_add (GTK_CONTAINER (sw), vp);

          gtk_container_add (GTK_CONTAINER (vp), tbl);
          w = sw;
        }
      else
        w = tbl;

      /* create form */
      for (i = 0; i < n_fields; i++)
        {
          YadField *fld = g_slist_nth_data (options.form_data.fields, i);

          /* add field label */
          if (fld->type != YAD_FIELD_CHECK && fld->type != YAD_FIELD_BUTTON &&
              fld->type != YAD_FIELD_FULL_BUTTON &&
              fld->type != YAD_FIELD_LABEL && fld->type != YAD_FIELD_TEXT)
            {
              gchar *buf = g_strcompress (fld->name);
              l = gtk_label_new (NULL);
              if (!options.data.no_markup)
                gtk_label_set_markup (GTK_LABEL (l), buf);
              else
                gtk_label_set_text (GTK_LABEL (l), buf);
              gtk_widget_set_name (l, "yad-form-flabel");
              gtk_misc_set_alignment (GTK_MISC (l), options.common_data.align, 0.5);
#if !GTK_CHECK_VERSION(3,0,0)
              gtk_table_attach (GTK_TABLE (tbl), l, col * 2, 1 + col * 2, row, row + 1, GTK_FILL, 0, 5, 5);
#else
              gtk_grid_attach (GTK_GRID (tbl), l, col * 2, row, 1, 1);
#endif
              g_free (buf);
            }

          /* add field entry */
          switch (fld->type)
            {
            case YAD_FIELD_SIMPLE:
            case YAD_FIELD_HIDDEN:
            case YAD_FIELD_READ_ONLY:
              e = gtk_entry_new ();
              gtk_widget_set_name (e, "yad-form-entry");
              g_signal_connect (G_OBJECT (e), "activate", G_CALLBACK (form_activate_cb), dlg);
              if (fld->type == YAD_FIELD_HIDDEN)
                gtk_entry_set_visibility (GTK_ENTRY (e), FALSE);
              else if (fld->type == YAD_FIELD_READ_ONLY)
                gtk_widget_set_sensitive (e, FALSE);
#if !GTK_CHECK_VERSION(3,0,0)
              gtk_table_attach (GTK_TABLE (tbl), e, 1 + col * 2, 2 + col * 2, row, row + 1,
                                GTK_EXPAND | GTK_FILL, 0, 5, 5);
#else
              gtk_grid_attach (GTK_GRID (tbl), e, 1 + col * 2, row, 1, 1);
              gtk_widget_set_hexpand (e, TRUE);
#endif
              fields = g_slist_append (fields, e);
              break;

            case YAD_FIELD_NUM:
              e = gtk_spin_button_new_with_range (0.0, 65525.0, 1.0);
              gtk_entry_set_alignment (GTK_ENTRY (e), 1.0);
              gtk_widget_set_name (e, "yad-form-spin");
#if !GTK_CHECK_VERSION(3,0,0)
              gtk_table_attach (GTK_TABLE (tbl), e, 1 + col * 2, 2 + col * 2, row, row + 1,
                                GTK_EXPAND | GTK_FILL, 0, 5, 5);
#else
              gtk_grid_attach (GTK_GRID (tbl), e, 1 + col * 2, row, 1, 1);
              gtk_widget_set_hexpand (e, TRUE);
#endif
              fields = g_slist_append (fields, e);
              break;

            case YAD_FIELD_CHECK:
              {
                gchar *buf = g_strcompress (fld->name);
                e = gtk_check_button_new_with_label (buf);
                gtk_widget_set_name (e, "yad-form-check");
#if !GTK_CHECK_VERSION(3,0,0)
                gtk_table_attach (GTK_TABLE (tbl), e, col * 2, 2 + col * 2, row, row + 1,
                                  GTK_EXPAND | GTK_FILL, 0, 5, 5);
#else
                gtk_grid_attach (GTK_GRID (tbl), e, col * 2, row, 2, 1);
                gtk_widget_set_hexpand (e, TRUE);
#endif
                fields = g_slist_append (fields, e);
                g_free (buf);
              }
              break;

            case YAD_FIELD_COMBO:
#if GTK_CHECK_VERSION(2,24,0)
              e = gtk_combo_box_text_new ();
#else
              e = gtk_combo_box_new_text ();
#endif
              gtk_widget_set_name (e, "yad-form-combo");
#if !GTK_CHECK_VERSION(3,0,0)
              gtk_table_attach (GTK_TABLE (tbl), e, 1 + col * 2, 2 + col * 2, row, row + 1,
                                GTK_EXPAND | GTK_FILL, 0, 5, 5);
#else
              gtk_grid_attach (GTK_GRID (tbl), e, 1 + col * 2, row, 1, 1);
              gtk_widget_set_hexpand (e, TRUE);
#endif
              fields = g_slist_append (fields, e);
              break;

            case YAD_FIELD_COMBO_ENTRY:
#if GTK_CHECK_VERSION(2,24,0)
              e = gtk_combo_box_text_new_with_entry ();
#else
              e = gtk_combo_box_entry_new_text ();
#endif
              gtk_widget_set_name (e, "yad-form-edit-combo");
#if !GTK_CHECK_VERSION(3,0,0)
              gtk_table_attach (GTK_TABLE (tbl), e, 1 + col * 2, 2 + col * 2, row, row + 1,
                                GTK_EXPAND | GTK_FILL, 0, 5, 5);
#else
              gtk_grid_attach (GTK_GRID (tbl), e, 1 + col * 2, row, 1, 1);
              gtk_widget_set_hexpand (e, TRUE);
#endif
              fields = g_slist_append (fields, e);
              break;

            case YAD_FIELD_FILE:
              e = gtk_file_chooser_button_new (_("Select file"), GTK_FILE_CHOOSER_ACTION_OPEN);
              gtk_widget_set_name (e, "yad-form-file");
              gtk_file_chooser_set_current_folder (GTK_FILE_CHOOSER (e), g_get_current_dir ());
#if !GTK_CHECK_VERSION(3,0,0)
              gtk_table_attach (GTK_TABLE (tbl), e, 1 + col * 2, 2 + col * 2, row, row + 1,
                                GTK_EXPAND | GTK_FILL, 0, 5, 5);
#else
              gtk_grid_attach (GTK_GRID (tbl), e, 1 + col * 2, row, 1, 1);
              gtk_widget_set_hexpand (e, TRUE);
#endif
              fields = g_slist_append (fields, e);
              break;

            case YAD_FIELD_DIR:
              e = gtk_file_chooser_button_new (_("Select folder"), GTK_FILE_CHOOSER_ACTION_SELECT_FOLDER);
              gtk_widget_set_name (e, "yad-form-file");
              gtk_file_chooser_set_current_folder (GTK_FILE_CHOOSER (e), g_get_current_dir ());
#if !GTK_CHECK_VERSION(3,0,0)
              gtk_table_attach (GTK_TABLE (tbl), e, 1 + col * 2, 2 + col * 2, row, row + 1,
                                GTK_EXPAND | GTK_FILL, 0, 5, 5);
#else
              gtk_grid_attach (GTK_GRID (tbl), e, 1 + col * 2, row, 1, 1);
              gtk_widget_set_hexpand (e, TRUE);
#endif
              fields = g_slist_append (fields, e);
              break;

            case YAD_FIELD_FONT:
              e = gtk_font_button_new ();
              gtk_widget_set_name (e, "yad-form-font");
#if !GTK_CHECK_VERSION(3,0,0)
              gtk_table_attach (GTK_TABLE (tbl), e, 1 + col * 2, 2 + col * 2, row, row + 1,
                                GTK_EXPAND | GTK_FILL, 0, 5, 5);
#else
              gtk_grid_attach (GTK_GRID (tbl), e, 1 + col * 2, row, 1, 1);
              gtk_widget_set_hexpand (e, TRUE);
#endif
              fields = g_slist_append (fields, e);
              break;

            case YAD_FIELD_COLOR:
              e = gtk_color_button_new ();
              gtk_widget_set_name (e, "yad-form-color");
#if !GTK_CHECK_VERSION(3,0,0)
              gtk_table_attach (GTK_TABLE (tbl), e, 1 + col * 2, 2 + col * 2, row, row + 1,
                                GTK_EXPAND | GTK_FILL, 0, 5, 5);
#else
              gtk_grid_attach (GTK_GRID (tbl), e, 1 + col * 2, row, 1, 1);
              gtk_widget_set_hexpand (e, TRUE);
#endif
              fields = g_slist_append (fields, e);
              break;

            case YAD_FIELD_MFILE:
            case YAD_FIELD_MDIR:
              e = gtk_entry_new ();
              gtk_widget_set_name (e, "yad-form-entry");
              gtk_entry_set_icon_from_stock (GTK_ENTRY (e), GTK_ENTRY_ICON_SECONDARY, "gtk-directory");
              g_signal_connect (G_OBJECT (e), "icon-press", G_CALLBACK (select_files_cb),
                                GINT_TO_POINTER (fld->type));
              g_signal_connect (G_OBJECT (e), "activate", G_CALLBACK (form_activate_cb), dlg);
#if !GTK_CHECK_VERSION(3,0,0)
              gtk_table_attach (GTK_TABLE (tbl), e, 1 + col * 2, 2 + col * 2, row, row + 1,
                                GTK_EXPAND | GTK_FILL, 0, 5, 5);
#else
              gtk_grid_attach (GTK_GRID (tbl), e, 1 + col * 2, row, 1, 1);
              gtk_widget_set_hexpand (e, TRUE);
#endif
              fields = g_slist_append (fields, e);
              break;

            case YAD_FIELD_FILE_SAVE:
            case YAD_FIELD_DIR_CREATE:
              e = gtk_entry_new ();
              gtk_widget_set_name (e, "yad-form-entry");
              gtk_entry_set_icon_from_stock (GTK_ENTRY (e), GTK_ENTRY_ICON_SECONDARY, "gtk-directory");
              g_signal_connect (G_OBJECT (e), "icon-press", G_CALLBACK (create_files_cb),
                                GINT_TO_POINTER (fld->type));
              g_signal_connect (G_OBJECT (e), "activate", G_CALLBACK (form_activate_cb), dlg);
#if !GTK_CHECK_VERSION(3,0,0)
              gtk_table_attach (GTK_TABLE (tbl), e, 1 + col * 2, 2 + col * 2, row, row + 1,
                                GTK_EXPAND | GTK_FILL, 0, 5, 5);
#else
              gtk_grid_attach (GTK_GRID (tbl), e, 1 + col * 2, row, 1, 1);
              gtk_widget_set_hexpand (e, TRUE);
#endif
              fields = g_slist_append (fields, e);
              break;

            case YAD_FIELD_DATE:
              e = gtk_entry_new ();
              gtk_widget_set_name (e, "yad-form-entry");
              pb = gdk_pixbuf_new_from_xpm_data (calendar_xpm);
              gtk_entry_set_icon_from_pixbuf (GTK_ENTRY (e), GTK_ENTRY_ICON_SECONDARY, pb);
              g_object_unref (pb);
              g_signal_connect (G_OBJECT (e), "icon-press", G_CALLBACK (select_date_cb), e);
              g_signal_connect (G_OBJECT (e), "activate", G_CALLBACK (form_activate_cb), dlg);
#if !GTK_CHECK_VERSION(3,0,0)
              gtk_table_attach (GTK_TABLE (tbl), e, 1 + col * 2, 2 + col * 2, row, row + 1,
                                GTK_EXPAND | GTK_FILL, 0, 5, 5);
#else
              gtk_grid_attach (GTK_GRID (tbl), e, 1 + col * 2, row, 1, 1);
              gtk_widget_set_hexpand (e, TRUE);
#endif
              fields = g_slist_append (fields, e);
              break;

            case YAD_FIELD_SCALE:
#if !GTK_CHECK_VERSION(3,0,0)
              e = gtk_hscale_new_with_range (0.0, 100.0, 1.0);
#else
              e = gtk_scale_new_with_range (GTK_ORIENTATION_HORIZONTAL, 0.0, 100.0, 1.0);
#endif
              gtk_widget_set_name (e, "yad-form-scale");
#if !GTK_CHECK_VERSION(3,0,0)
              gtk_table_attach (GTK_TABLE (tbl), e, 1 + col * 2, 2 + col * 2, row, row + 1,
                                GTK_EXPAND | GTK_FILL, 0, 5, 5);
#else
              gtk_grid_attach (GTK_GRID (tbl), e, 1 + col * 2, row, 1, 1);
              gtk_widget_set_hexpand (e, TRUE);
#endif
              fields = g_slist_append (fields, e);
              break;

            case YAD_FIELD_BUTTON:
            case YAD_FIELD_FULL_BUTTON:
              e = gtk_button_new ();
              gtk_container_add (GTK_CONTAINER (e), get_label (fld->name, 2));
              gtk_widget_set_name (e, "yad-form-button");
              gtk_button_set_alignment (GTK_BUTTON (e), 0.5, 0.5);
              if (fld->type == YAD_FIELD_BUTTON)
                gtk_button_set_relief (GTK_BUTTON (e), GTK_RELIEF_NONE);
#if !GTK_CHECK_VERSION(3,0,0)
              gtk_table_attach (GTK_TABLE (tbl), e, col * 2, 2 + col * 2, row, row + 1,
                                GTK_EXPAND | GTK_FILL, 0, 5, 5);
#else
              gtk_grid_attach (GTK_GRID (tbl), e, col * 2, row, 2, 1);
              gtk_widget_set_hexpand (e, TRUE);
#endif
              fields = g_slist_append (fields, e);
              break;

            case YAD_FIELD_LABEL:
              if (fld->name[0])
                {
                  gchar *buf = g_strcompress (fld->name);
                  e = gtk_label_new (NULL);
                  gtk_widget_set_name (e, "yad-form-label");
                  if (options.data.no_markup)
                    gtk_label_set_text (GTK_LABEL (e), buf);
                  else
                    gtk_label_set_markup (GTK_LABEL (e), buf);
                  gtk_label_set_line_wrap (GTK_LABEL (e), TRUE);
                  gtk_label_set_selectable (GTK_LABEL (e), options.data.selectable_labels);
                  gtk_misc_set_alignment (GTK_MISC (e), options.common_data.align, 0.5);
                  //g_signal_connect_after (G_OBJECT (e), "size-allocate", G_CALLBACK (text_size_allocate_cb), NULL);
                  g_free (buf);
                }
              else
                {
#if GTK_CHECK_VERSION(3,0,0)
                  e = gtk_separator_new (GTK_ORIENTATION_HORIZONTAL);
#else
                  e = gtk_hseparator_new ();
#endif
                  gtk_widget_set_name (e, "yad-form-separator");
                }
#if !GTK_CHECK_VERSION(3,0,0)
              gtk_table_attach (GTK_TABLE (tbl), e, col * 2, 2 + col * 2, row, row + 1,
                                GTK_EXPAND | GTK_FILL, 0, 5, 5);
#else
              gtk_grid_attach (GTK_GRID (tbl), e, col * 2, row, 2, 1);
              gtk_widget_set_hexpand (e, TRUE);
#endif
              fields = g_slist_append (fields, e);
              break;

            case YAD_FIELD_TEXT:
              {
                GtkWidget *l, *sw, *b;
                gchar *ltxt = g_strcompress (fld->name);

#if !GTK_CHECK_VERSION(3,0,0)
                b = gtk_vbox_new (FALSE, 2);
#else
                b = gtk_box_new (GTK_ORIENTATION_VERTICAL, 2);
#endif

                l = gtk_label_new ("");
                gtk_misc_set_alignment (GTK_MISC (l), 0.0, 0.5);
                if (options.data.no_markup)
                  gtk_label_set_text (GTK_LABEL (l), ltxt);
                else
                  gtk_label_set_markup (GTK_LABEL (l), ltxt);
                g_free (ltxt);
                gtk_box_pack_start (GTK_BOX (b), l, FALSE, FALSE, 0);

                sw = gtk_scrolled_window_new (NULL, NULL);
                gtk_scrolled_window_set_shadow_type (GTK_SCROLLED_WINDOW (sw), GTK_SHADOW_ETCHED_IN);
                gtk_scrolled_window_set_policy (GTK_SCROLLED_WINDOW (sw), GTK_POLICY_AUTOMATIC, GTK_POLICY_AUTOMATIC);
                gtk_box_pack_start (GTK_BOX (b), sw, TRUE, TRUE, 0);

                e = gtk_text_view_new ();
                gtk_widget_set_name (e, "yad-form-text");
                gtk_text_view_set_editable (GTK_TEXT_VIEW (e), TRUE);
                gtk_text_view_set_wrap_mode (GTK_TEXT_VIEW (e), GTK_WRAP_WORD_CHAR);
                gtk_container_add (GTK_CONTAINER (sw), e);

#if !GTK_CHECK_VERSION(3,0,0)
                gtk_table_attach (GTK_TABLE (tbl), b, col * 2, 2 + col * 2, row, row + 1,
                                  GTK_EXPAND | GTK_FILL, GTK_EXPAND | GTK_FILL, 5, 5);
#else
                gtk_grid_attach (GTK_GRID (tbl), b, col * 2, row, 2, 1);
                gtk_widget_set_hexpand (b, TRUE);
                gtk_widget_set_vexpand (b, TRUE);
#endif
                fields = g_slist_append (fields, e);

                break;
              }
            }

          /* increase row and column */
          row++;
          if (row >= rows)
            {
              row = 0;
              col++;
            }
        }

      /* fill entries with data */
      if (options.extra_data)
        {
          i = 0;
          while (options.extra_data[i] && i < n_fields)
            {
              set_field_value (i, options.extra_data[i]);
              i++;
            }
        }
    }

  return w;
}

void
form_print_result (void)
{
  guint i;

  for (i = 0; i < n_fields; i++)
    {
      YadField *fld = g_slist_nth_data (options.form_data.fields, i);

      switch (fld->type)
        {
        case YAD_FIELD_SIMPLE:
        case YAD_FIELD_HIDDEN:
        case YAD_FIELD_READ_ONLY:
        case YAD_FIELD_MFILE:
        case YAD_FIELD_MDIR:
        case YAD_FIELD_FILE_SAVE:
        case YAD_FIELD_DIR_CREATE:
        case YAD_FIELD_DATE:
          if (options.common_data.quoted_output)
            {
              gchar *buf = g_shell_quote (gtk_entry_get_text (GTK_ENTRY (g_slist_nth_data (fields, i))));
              g_printf ("%s%s", buf, options.common_data.separator);
              g_free (buf);
            }
          else
            g_printf ("%s%s", gtk_entry_get_text (GTK_ENTRY (g_slist_nth_data (fields, i))),
                      options.common_data.separator);
          break;
        case YAD_FIELD_NUM:
          if (options.common_data.quoted_output)
            g_printf ("'%f'%s", gtk_spin_button_get_value (GTK_SPIN_BUTTON (g_slist_nth_data (fields, i))),
                      options.common_data.separator);
          else
            g_printf ("%f%s", gtk_spin_button_get_value (GTK_SPIN_BUTTON (g_slist_nth_data (fields, i))),
                      options.common_data.separator);
          break;
        case YAD_FIELD_CHECK:
          if (options.common_data.quoted_output)
            g_printf ("'%s'%s", gtk_toggle_button_get_active (GTK_TOGGLE_BUTTON (g_slist_nth_data (fields, i))) ? "TRUE" :
                      "FALSE", options.common_data.separator);
          else
            g_printf ("%s%s", gtk_toggle_button_get_active (GTK_TOGGLE_BUTTON (g_slist_nth_data (fields, i))) ? "TRUE" :
                      "FALSE", options.common_data.separator);
          break;
        case YAD_FIELD_COMBO:
        case YAD_FIELD_COMBO_ENTRY:
          if (options.common_data.quoted_output)
            {
#if GTK_CHECK_VERSION(2,24,0)
              gchar *buf = g_shell_quote (gtk_combo_box_text_get_active_text
                                          (GTK_COMBO_BOX_TEXT (g_slist_nth_data (fields, i))));
#else
              gchar *buf = g_shell_quote (gtk_combo_box_get_active_text
                                          (GTK_COMBO_BOX (g_slist_nth_data (fields, i))));
#endif
              g_printf ("%s%s", buf, options.common_data.separator);
              g_free (buf);
            }
          else
            {
              g_printf ("%s%s",
#if GTK_CHECK_VERSION(2,24,0)
                        gtk_combo_box_text_get_active_text (GTK_COMBO_BOX_TEXT (g_slist_nth_data (fields, i))),
#else
                        gtk_combo_box_get_active_text (GTK_COMBO_BOX (g_slist_nth_data (fields, i))),
#endif
                        options.common_data.separator);
            }
          break;
        case YAD_FIELD_FILE:
        case YAD_FIELD_DIR:
          if (options.common_data.quoted_output)
            {
              gchar *buf = g_shell_quote (gtk_file_chooser_get_filename (GTK_FILE_CHOOSER (g_slist_nth_data (fields, i))));
              g_printf ("%s%s", buf, options.common_data.separator);
              g_free (buf);
            }
          else
            g_printf ("%s%s", gtk_file_chooser_get_filename (GTK_FILE_CHOOSER (g_slist_nth_data (fields, i))),
                      options.common_data.separator);
          break;
        case YAD_FIELD_FONT:
          if (options.common_data.quoted_output)
            g_printf ("'%s'%s", gtk_font_button_get_font_name (GTK_FONT_BUTTON (g_slist_nth_data (fields, i))),
                      options.common_data.separator);
          else
            g_printf ("%s%s", gtk_font_button_get_font_name (GTK_FONT_BUTTON (g_slist_nth_data (fields, i))),
                      options.common_data.separator);
          break;
        case YAD_FIELD_COLOR:
          {
            GdkColor c;

            gtk_color_button_get_color (GTK_COLOR_BUTTON (g_slist_nth_data (fields, i)), &c);
            if (options.common_data.quoted_output)
              g_printf ("'%s'%s", gdk_color_to_string (&c), options.common_data.separator);
            else
              g_printf ("%s%s", gdk_color_to_string (&c), options.common_data.separator);
            break;
          }
        case YAD_FIELD_SCALE:
          if (options.common_data.quoted_output)
            g_printf ("'%d'%s", (gint) gtk_range_get_value (GTK_RANGE (g_slist_nth_data (fields, i))),
                      options.common_data.separator);
          else
            g_printf ("%d%s", (gint) gtk_range_get_value (GTK_RANGE (g_slist_nth_data (fields, i))),
                      options.common_data.separator);
          break;
        case YAD_FIELD_BUTTON:
        case YAD_FIELD_FULL_BUTTON:
        case YAD_FIELD_LABEL:
          if (options.common_data.quoted_output)
            g_printf ("''%s", options.common_data.separator);
          else
            g_printf ("%s", options.common_data.separator);
          break;
        case YAD_FIELD_TEXT:
          {
            gchar *txt;
            GtkTextBuffer *tb;
            GtkTextIter b, e;

            tb = gtk_text_view_get_buffer (GTK_TEXT_VIEW (g_slist_nth_data (fields, i)));
            gtk_text_buffer_get_bounds (tb, &b, &e);
            txt = escape_str (gtk_text_buffer_get_text (tb, &b, &e, FALSE));
            g_printf ("%s%s", txt, options.common_data.separator);
            g_free (txt);
          }
        }
    }
  g_printf ("\n");
}
