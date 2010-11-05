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

#ifndef FOO_MODEL_OBJECT_H
#define FOO_MODEL_OBJECT_H

#include <glib-object.h>

G_BEGIN_DECLS

#define FOO_TYPE_TEST_OBJECT foo_test_object_get_type()

#define FOO_TEST_OBJECT(obj) \
  (G_TYPE_CHECK_INSTANCE_CAST ((obj), FOO_TYPE_TEST_OBJECT, FooTestObject))

#define FOO_TEST_OBJECT_CLASS(klass) \
  (G_TYPE_CHECK_CLASS_CAST ((klass), FOO_TYPE_TEST_OBJECT, FooTestObjectClass))

#define FOO_IS_TEST_OBJECT(obj) \
  (G_TYPE_CHECK_INSTANCE_TYPE ((obj), FOO_TYPE_TEST_OBJECT))

#define FOO_IS_TEST_OBJECT_CLASS(klass) \
  (G_TYPE_CHECK_CLASS_TYPE ((klass), FOO_TYPE_TEST_OBJECT))

#define FOO_TEST_OBJECT_GET_CLASS(obj) \
  (G_TYPE_INSTANCE_GET_CLASS ((obj), FOO_TYPE_TEST_OBJECT, FooTestObjectClass))

typedef struct
{
  GObject parent;
} FooTestObject;

typedef struct
{
  GObjectClass parent;
} FooTestObjectClass;

GType
foo_test_object_get_type (void);

FooTestObject *
foo_test_object_new (void);

int
foo_test_object_get_number (FooTestObject *self);

void
foo_test_object_set_number (FooTestObject *self,
                            int            number);

char const *
foo_test_object_get_text (FooTestObject *self);

void
foo_test_object_set_text (FooTestObject *self,
                           char const   *text);

G_END_DECLS

#endif /* FOO_TEST_OBJECT_H */
