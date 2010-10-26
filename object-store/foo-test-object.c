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

#include "foo-test-object.h"

G_DEFINE_TYPE (FooTestObject, foo_test_object, G_TYPE_OBJECT)

#define GET_PRIVATE(o) \
  (G_TYPE_INSTANCE_GET_PRIVATE ((o), FOO_TYPE_TEST_OBJECT, FooTestObjectPrivate))

enum
{
  PROP_0,
  PROP_TEXT
};

typedef struct
{
  char *text;
} FooTestObjectPrivate;

static void
_get_property (GObject    *object,
               unsigned    property_id,
               GValue     *value,
               GParamSpec *pspec)
{
  switch (property_id)
  {
  case PROP_TEXT:
    g_value_set_string (value,
                        foo_test_object_get_text (
                          FOO_TEST_OBJECT (object)));
    break;
  default:
    G_OBJECT_WARN_INVALID_PROPERTY_ID (object, property_id, pspec);
  }
}

static void
_set_property (GObject      *object,
               unsigned      property_id,
               const GValue *value,
               GParamSpec   *pspec)
{
  switch (property_id)
  {
  case PROP_TEXT:
    foo_test_object_set_text (FOO_TEST_OBJECT (object),
                              g_value_get_string (value));
    break;
  default:
    G_OBJECT_WARN_INVALID_PROPERTY_ID (object, property_id, pspec);
  }
}

static void
_finalize (GObject *object)
{
  G_OBJECT_CLASS (foo_test_object_parent_class)->finalize (object);
}

static void
foo_test_object_class_init (FooTestObjectClass *klass)
{
  GObjectClass *object_class = G_OBJECT_CLASS (klass);

  g_type_class_add_private (klass, sizeof (FooTestObjectPrivate));

  object_class->get_property = _get_property;
  object_class->set_property = _set_property;
  object_class->finalize = _finalize;

  g_object_class_install_property (object_class,
                                   PROP_TEXT,
                                   g_param_spec_string ("text", "", "",
                                                        NULL,
                                                        G_PARAM_READWRITE));
}

static void
foo_test_object_init (FooTestObject *self)
{
}

FooTestObject *
foo_test_object_new (void)
{
  return g_object_new (FOO_TYPE_TEST_OBJECT, NULL);
}

char const *
foo_test_object_get_text (FooTestObject *self)
{
  FooTestObjectPrivate *priv = GET_PRIVATE (self);

  g_return_val_if_fail (FOO_IS_TEST_OBJECT (self), NULL);

  return priv->text;
}

void
foo_test_object_set_text (FooTestObject *self,
                          char const    *text)
{
  FooTestObjectPrivate *priv = GET_PRIVATE (self);

  g_return_if_fail (FOO_IS_TEST_OBJECT (self));

  if (0 != g_strcmp0 (text, priv->text))
  {
    if (priv->text)
    {
      g_free (priv->text);
      priv->text = NULL;
    }

    if (text)
    {
      priv->text = g_strdup (text);
    }

    g_object_notify (G_OBJECT (self), "text");
  }
}
