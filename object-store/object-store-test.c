/*
 * Copyright (c) 2010, Intel Corporation.
 *
 * This program is free software; you can redistribute it and/or modify it
 * under the terms and conditions of the GNU General Public License,
 * version 2, as published by the Free Software Foundation.
 *
 * This program is distributed in the hope it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
 * FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License for
 * more details.
 *
 * You should have received a copy of the GNU General Public License along with
 * this program; if not, write to the Free Software Foundation, Inc.,
 * 51 Franklin St - Fifth Floor, Boston, MA 02110-1301 USA.
 *
 * Author: Rob Staudinger <robsta@linux.intel.com>
 */

#include <stdbool.h>
#include <stdlib.h>
#include <mx/mx.h>
#include "foo-object-store.h"
#include "foo-test-object.h"

typedef struct
{
  ClutterActor  *view;
  MxEntry       *entry;
  ClutterModel  *store;
} ObjectStoreTest;

/*
 * Look up item by index.
 * Returned object needs to be g_object_unref'd.
 * May return NULL.
 */
FooTestObject *
store_get_object (ClutterModel *store,
                  unsigned      index)
{
  ClutterModelIter  *iter;
  FooTestObject     *object = NULL;

  iter = clutter_model_get_iter_at_row (store, index);
  if (iter &&
      !clutter_model_iter_is_last (iter))
  {
    /* Column #0 of the model holds the actual object. */
    clutter_model_iter_get (iter,
                            0, &object,
                            -1);
  }

  return object;
}

/*
 * Add object to the store.
 */
static void
store_add_object (ClutterModel  *store,
                  char const    *text)
{
  FooTestObject *object;

  object = foo_test_object_new ();
  foo_test_object_set_text (object, text);

  /* Column #0 holds the actual object, the other cols are mapped to
   * its properties. */
  clutter_model_append (store, 0, object, -1);
  g_object_unref (object);
}

static void
_update_clicked (MxButton         *button,
                 ObjectStoreTest  *app)
{
  char const *input;

  input = mx_entry_get_text (app->entry);
  if (input == NULL ||
      input[0] == '\0')
  {
    g_warning ("Please enter text");
    return;

  } else if (input[0] == '-') {

    /* Remove item */
    int index = g_ascii_isdigit (input[1]) ?
                  atoi (&input[1]) :
                  -1;
    if (index < 0)
    {
      g_warning ("Invalid number, can not remove");
      return;
    }

    clutter_model_remove (app->store, index);

  } else if (g_ascii_isdigit (input[0])) {

    /* Update item */
    unsigned index = atoi (input);
    char **tokens = g_strsplit (input, ":", 2);
    char const *text = tokens[1];
    FooTestObject *object = store_get_object (app->store, index);

    if (object == NULL)
    {
      g_warning ("Failed to find object");
      return;
    }

    foo_test_object_set_text (FOO_TEST_OBJECT (object), text);
    g_object_unref (object);

  } else {

    /* Add item */
    store_add_object (app->store, input);
  }

  mx_entry_set_text (app->entry, "");
}

static void
_dump_clicked (MxButton         *button,
               ObjectStoreTest  *app)
{
  ClutterModelIter *iter;

  for (iter = clutter_model_get_first_iter (app->store);
       !clutter_model_iter_is_last (iter);
       iter = clutter_model_iter_next (iter))
  {
    FooTestObject *object = NULL;
    char          *text = NULL;

    clutter_model_iter_get (iter,
                            0, &object,
                            1, &text,
                            -1);
    g_debug ("%p %s\n", object, text);
    g_object_unref (object);
    g_free (text);
  }
}

int
main (int     argc,
      char  **argv)
{
  ClutterActor    *stage;
  MxBoxLayout     *vbox;
  MxBoxLayout     *hbox;
  ClutterActor    *button;
  ClutterActor    *label;
  ObjectStoreTest  app = { 0, };

  clutter_init (&argc, &argv);

  stage = clutter_stage_get_default ();
  clutter_actor_set_size (stage, 320.0, 240.0);

  vbox = (MxBoxLayout *) mx_box_layout_new ();
  clutter_actor_set_size (CLUTTER_ACTOR (vbox), 320.0, 240.0);
  mx_box_layout_set_orientation (vbox, MX_ORIENTATION_VERTICAL);
  clutter_container_add_actor (CLUTTER_CONTAINER (stage), CLUTTER_ACTOR (vbox));

  /* Create model */
  app.store = foo_object_store_new (2,
                                    FOO_TYPE_TEST_OBJECT, "foo",  /* column #0 */
                                    G_TYPE_STRING, "text");       /* column #1 */

  /*
   * Create view
   */
  app.view = mx_list_view_new ();

  /* Use MxButton to render the model's items */
  mx_list_view_set_item_type (MX_LIST_VIEW (app.view), MX_TYPE_BUTTON);

  /* Map column #1 to attribute "label" of view's GtkButton */
  mx_list_view_add_attribute (MX_LIST_VIEW (app.view), "label", 1);

  /* Connect to model */
  mx_list_view_set_model (MX_LIST_VIEW (app.view), app.store);

  mx_box_layout_add_actor_with_properties (vbox, app.view, -1,
                                           "expand", true,
                                           "x-fill", true,
                                           "y-fill", true,
                                           NULL);

  hbox = (MxBoxLayout *) mx_box_layout_new ();
  mx_box_layout_set_orientation (hbox, MX_ORIENTATION_HORIZONTAL);
  mx_box_layout_add_actor_with_properties (vbox, CLUTTER_ACTOR (hbox), -1,
                                           "expand", false,
                                           "x-fill", true,
                                           NULL);

  app.entry = (MxEntry *) mx_entry_new ();
  mx_box_layout_add_actor_with_properties (hbox, CLUTTER_ACTOR (app.entry), -1,
                                           "expand", true,
                                           "x-fill", true,
                                           NULL);

  button = mx_button_new_with_label ("Update");
  g_signal_connect (button, "clicked",
                    G_CALLBACK (_update_clicked), &app);
  clutter_container_add_actor (CLUTTER_CONTAINER (hbox), button);

  button = mx_button_new_with_label ("Dump");
  g_signal_connect (button, "clicked",
                    G_CALLBACK (_dump_clicked), &app);
  clutter_container_add_actor (CLUTTER_CONTAINER (hbox), button);

  label = mx_label_new_with_text ("Enter text and update to add item\n"
                                  "Enter <number>:<text> to change item <number>\n"
                                  "Enter -<number> to delete item <number>");
  clutter_container_add_actor (CLUTTER_CONTAINER (vbox), label);

  clutter_actor_show_all (stage);
  clutter_main ();

  return EXIT_SUCCESS;
}
