/*
 * build with command:
 *     gcc $(pkg-config --cflags --libs gtkspell3-3.0) -o get-lang get-lang.c
 */

#include <glib.h>
#include <gtkspell/gtkspell.h>

int main (int argc, char *argv[])
{
  GList *lng;
  
  for (lng = gtk_spell_checker_get_language_list (); lng; lng = lng->next)
    g_print ("%s\n", lng->data);

  return 0;
}
