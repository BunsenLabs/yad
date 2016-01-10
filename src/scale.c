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

#include "yad.h"

static GtkWidget *scale;

static void
value_changed_cb (GtkWidget * w, gpointer data)
{
  g_print ("%.0f\n", gtk_range_get_value (GTK_RANGE (w)));
}

GtkWidget *
scale_create_widget (GtkWidget * dlg)
{
  GtkWidget *w;
  GtkAdjustment *adj;
  gint page;

  if (options.scale_data.min_value >= options.scale_data.max_value)
    {
      g_printerr (_("Maximum value must be greater than minimum value.\n"));
      return NULL;
    }

  /* check for initial value */
  if (options.scale_data.have_value)
    {
      if (options.scale_data.value < options.scale_data.min_value)
        {
          g_printerr (_("Initial value less than minimal.\n"));
          options.scale_data.value = options.scale_data.min_value;
        }
      else if (options.scale_data.value > options.scale_data.max_value)
        {
          g_printerr (_("Initial value greater than maximum.\n"));
          options.scale_data.value = options.scale_data.max_value;
        }
    }
  else
    options.scale_data.value = options.scale_data.min_value;


  page = options.scale_data.page == -1 ? options.scale_data.step * 10 : options.scale_data.page;
  /* this type conversion needs only for gtk-2.0 */
  adj = (GtkAdjustment *) gtk_adjustment_new ((double) options.scale_data.value,
                                              (double) options.scale_data.min_value,
                                              (double) options.scale_data.max_value,
                                              (double) options.scale_data.step, (double) page, 0.0);
  if (options.common_data.vertical)
    {
#if GTK_CHECK_VERSION(3,0,0)
      w = scale = gtk_scale_new (GTK_ORIENTATION_VERTICAL, GTK_ADJUSTMENT (adj));
#else    
      w = scale = gtk_vscale_new (GTK_ADJUSTMENT (adj));
#endif
      gtk_range_set_inverted (GTK_RANGE (w), !options.scale_data.invert);
    }
  else
    {
#if GTK_CHECK_VERSION(3,0,0)
      w = scale = gtk_scale_new (GTK_ORIENTATION_HORIZONTAL, GTK_ADJUSTMENT (adj));
#else    
      w = scale = gtk_hscale_new (GTK_ADJUSTMENT (adj));
#endif
      gtk_range_set_inverted (GTK_RANGE (w), options.scale_data.invert);
    }
  gtk_widget_set_name (w, "yad-scale-widget");
  gtk_scale_set_digits (GTK_SCALE (w), 0);

  if (options.scale_data.print_partial)
    g_signal_connect (G_OBJECT (w), "value-changed", G_CALLBACK (value_changed_cb), NULL);

  if (options.scale_data.hide_value)
    gtk_scale_set_draw_value (GTK_SCALE (w), FALSE);

  /* add marks */
  if (options.scale_data.marks)
    {
      GtkPositionType pos;
      GSList *m = options.scale_data.marks;

      pos = options.common_data.vertical ? GTK_POS_LEFT : GTK_POS_BOTTOM;
      for (; m; m = m->next)
        {
          YadScaleMark *mark = (YadScaleMark *) m->data;
          gtk_scale_add_mark (GTK_SCALE (w), mark->value, pos, mark->name);
        }
    }

  return w;
}

void
scale_print_result (void)
{
  g_print ("%.0f\n", gtk_range_get_value (GTK_RANGE (scale)));
}
