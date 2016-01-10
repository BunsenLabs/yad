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

#include <errno.h>

#include <glib/gprintf.h>

#include "yad.h"

static GtkWidget *font;

static void
realize_cb (GtkWidget * w, gpointer d)
{
  gtk_font_selection_set_font_name (GTK_FONT_SELECTION (w), options.common_data.font);
}

GtkWidget *
font_create_widget (GtkWidget * dlg)
{
  GtkWidget *w;

  w = font = gtk_font_selection_new ();
  gtk_widget_set_name (w, "yad-font-widget");

  if (options.font_data.preview)
    gtk_font_selection_set_preview_text (GTK_FONT_SELECTION (w), options.font_data.preview);

  /* font must be set after widget inserted in toplevel */
  if (options.common_data.font)
    g_signal_connect_after (G_OBJECT (w), "realize", G_CALLBACK (realize_cb), NULL);

  return w;
}

void
font_print_result (void)
{
  g_printf ("%s\n", gtk_font_selection_get_font_name (GTK_FONT_SELECTION (font)));
}
