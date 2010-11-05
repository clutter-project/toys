#include <stdlib.h>
#include <string.h>
#include <clutter/clutter.h>
#include "foo-object-store.h"
#include "foo-test-object.h"

typedef struct _ModelData
{
  ClutterModel *model;

  guint n_row;
} ModelData;

enum
{
  COLUMN_OBJECT,  /* FOO_TYPE_OBJECT_STORE */
  COLUMN_NUMBER,  /* G_TYPE_INT */
  COLUMN_TEXT,    /* G_TYPE_STRING */

  N_COLUMNS
};

static const struct {
  const gchar *expected_foo;
  gint expected_bar;
} base_model[] = {
  { "String 1", 1 },
  { "String 2", 2 },
  { "String 3", 3 },
  { "String 4", 4 },
  { "String 5", 5 },
  { "String 6", 6 },
  { "String 7", 7 },
  { "String 8", 8 },
  { "String 9", 9 },
};

static const struct {
  const gchar *expected_foo;
  gint expected_bar;
} forward_base[] = {
  { "String 1", 1 },
  { "String 2", 2 },
  { "String 3", 3 },
  { "String 4", 4 },
  { "String 5", 5 },
  { "String 6", 6 },
  { "String 7", 7 },
  { "String 8", 8 },
  { "String 9", 9 },
};

static const struct {
  const gchar *expected_foo;
  gint expected_bar;
} backward_base[] = {
  { "String 9", 9 },
  { "String 8", 8 },
  { "String 7", 7 },
  { "String 6", 6 },
  { "String 5", 5 },
  { "String 4", 4 },
  { "String 3", 3 },
  { "String 2", 2 },
  { "String 1", 1 },
};

static const struct {
  const gchar *expected_foo;
  gint expected_bar;
} filter_odd[] = {
  { "String 1", 1 },
  { "String 3", 3 },
  { "String 5", 5 },
  { "String 7", 7 },
  { "String 9", 9 },
};

static const struct {
  const gchar *expected_foo;
  gint expected_bar;
} filter_even[] = {
  { "String 8", 8 },
  { "String 6", 6 },
  { "String 4", 4 },
  { "String 2", 2 },
};

static inline void
compare_iter (ClutterModelIter *iter,
              const gint        expected_row,
              const gchar      *expected_foo,
              const gint        expected_bar)
{
  gchar *foo = NULL;
  gint bar = 0;
  gint row = 0;

  row = clutter_model_iter_get_row (iter);
  clutter_model_iter_get (iter,
                          COLUMN_TEXT, &foo,
                          COLUMN_NUMBER, &bar,
                          -1);

  if (g_test_verbose ())
    g_print ("Row %d => %d: Got [ '%s', '%d' ], expected [ '%s', '%d' ]\n",
             row, expected_row,
             foo, bar,
             expected_foo, expected_bar);

  g_assert_cmpint (row, ==, expected_row);
  g_assert_cmpstr (foo, ==, expected_foo);
  g_assert_cmpint (bar, ==, expected_bar);

  g_free (foo);
}

static void
on_row_added (ClutterModel     *model,
              ClutterModelIter *iter,
              gpointer          data)
{
  ModelData *model_data = data;

  compare_iter (iter,
                model_data->n_row,
                base_model[model_data->n_row].expected_foo,
                base_model[model_data->n_row].expected_bar);

  model_data->n_row += 1;
}

static gboolean
filter_even_rows (ClutterModel     *model,
                  ClutterModelIter *iter,
                  gpointer          dummy G_GNUC_UNUSED)
{
  gint bar_value;

  clutter_model_iter_get (iter, COLUMN_NUMBER, &bar_value, -1);

  if (bar_value % 2 == 0)
    return TRUE;

  return FALSE;
}

static gboolean
filter_odd_rows (ClutterModel     *model,
                 ClutterModelIter *iter,
                 gpointer          dummy G_GNUC_UNUSED)
{
  gint bar_value;

  clutter_model_iter_get (iter, COLUMN_NUMBER, &bar_value, -1);

  if (bar_value % 2 != 0)
    return TRUE;

  return FALSE;
}

void
test_list_model_filter (void)
{
  ModelData test_data = { NULL, 0 };
  ClutterModelIter *iter;
  gint i;

  test_data.model = foo_object_store_new (N_COLUMNS,
                                          FOO_TYPE_TEST_OBJECT, "object",
                                          G_TYPE_INT,           "number",
                                          G_TYPE_STRING,        "text");
  test_data.n_row = 0;

  for (i = 1; i < 10; i++)
    {
      gchar *foo = g_strdup_printf ("String %d", i);
      GObject *object = g_object_new (FOO_TYPE_TEST_OBJECT,
                                      "number", i,
                                      "text", foo,
                                      NULL);

      clutter_model_append (test_data.model,
                            COLUMN_OBJECT, object,
                            -1);

      g_object_unref (object);
      g_free (foo);
    }

  if (g_test_verbose ())
    g_print ("Forward iteration (filter odd)...\n");

  clutter_model_set_filter (test_data.model, filter_odd_rows, NULL, NULL);

  iter = clutter_model_get_first_iter (test_data.model);
  g_assert (iter != NULL);

  i = 0;
  while (!clutter_model_iter_is_last (iter))
    {
      compare_iter (iter, i,
                    filter_odd[i].expected_foo,
                    filter_odd[i].expected_bar);

      iter = clutter_model_iter_next (iter);
      i += 1;
    }

  g_object_unref (iter);

  if (g_test_verbose ())
    g_print ("Backward iteration (filter even)...\n");

  clutter_model_set_filter (test_data.model, filter_even_rows, NULL, NULL);

  iter = clutter_model_get_last_iter (test_data.model);
  g_assert (iter != NULL);

  i = 0;
  do
    {
      compare_iter (iter, G_N_ELEMENTS (filter_even) - i - 1,
                    filter_even[i].expected_foo,
                    filter_even[i].expected_bar);

      iter = clutter_model_iter_prev (iter);
      i += 1;
    }
  while (!clutter_model_iter_is_first (iter));

  g_object_unref (iter);

  g_object_unref (test_data.model);
}

void
test_list_model_iterate (void)
{
  ModelData test_data = { NULL, 0 };
  ClutterModelIter *iter;
  gint i;

  test_data.model = foo_object_store_new (N_COLUMNS,
                                          FOO_TYPE_TEST_OBJECT, "object",
                                          G_TYPE_INT,           "number",
                                          G_TYPE_STRING,        "text");
  test_data.n_row = 0;

  g_signal_connect (test_data.model, "row-added",
                    G_CALLBACK (on_row_added),
                    &test_data);

  for (i = 1; i < 10; i++)
    {
      gchar *foo = g_strdup_printf ("String %d", i);
      GObject *object = g_object_new (FOO_TYPE_TEST_OBJECT,
                                      "number", i,
                                      "text", foo,
                                      NULL);

      clutter_model_append (test_data.model,
                            COLUMN_OBJECT, object,
                            -1);

      g_object_unref (object);
      g_free (foo);
    }

  if (g_test_verbose ())
    g_print ("Forward iteration...\n");

  iter = clutter_model_get_first_iter (test_data.model);
  g_assert (iter != NULL);

  i = 0;
  while (!clutter_model_iter_is_last (iter))
    {
      compare_iter (iter, i,
                    forward_base[i].expected_foo,
                    forward_base[i].expected_bar);

      iter = clutter_model_iter_next (iter);
      i += 1;
    }

  g_object_unref (iter);

  if (g_test_verbose ())
    g_print ("Backward iteration...\n");

  iter = clutter_model_get_last_iter (test_data.model);
  g_assert (iter != NULL);

  i = 0;
  do
    {
      compare_iter (iter, G_N_ELEMENTS (backward_base) - i - 1,
                    backward_base[i].expected_foo,
                    backward_base[i].expected_bar);

      iter = clutter_model_iter_prev (iter);
      i += 1;
    }
  while (!clutter_model_iter_is_first (iter));

  compare_iter (iter, G_N_ELEMENTS (backward_base) - i - 1,
                backward_base[i].expected_foo,
                backward_base[i].expected_bar);

  g_object_unref (iter);

  g_object_unref (test_data.model);
}

void
test_list_model_populate (void)
{
  ModelData test_data = { NULL, 0 };
  gint i;

  test_data.model = foo_object_store_new (N_COLUMNS,
                                          FOO_TYPE_TEST_OBJECT, "object",
                                          G_TYPE_INT,           "number",
                                          G_TYPE_STRING,        "text");
  test_data.n_row = 0;

  g_signal_connect (test_data.model, "row-added",
                    G_CALLBACK (on_row_added),
                    &test_data);

  for (i = 1; i < 10; i++)
    {
      gchar *foo = g_strdup_printf ("String %d", i);
      GObject *object = g_object_new (FOO_TYPE_TEST_OBJECT,
                                      "number", i,
                                      "text", foo,
                                      NULL);

      clutter_model_append (test_data.model,
                            COLUMN_OBJECT, object,
                            -1);

      g_object_unref (object);
      g_free (foo);
    }

  g_object_unref (test_data.model);
}

int
main (int     argc,
      char  **argv)
{
 
  clutter_init (&argc, &argv);

  test_list_model_populate ();
  test_list_model_iterate ();
  test_list_model_filter ();

  return EXIT_SUCCESS;
}