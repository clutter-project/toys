/*
 * Copyright (C) 2007 Neil J. Patel
 * Copyright (C) 2007 OpenedHand Ltd
 *
 * Author: Neil J. Patel  <njp@o-hand.com>
 */

#include <config.h>
#include <glib.h>
#include <clutter/clutter.h>

#ifndef _HAVE_FLUTTR_AUTH_H
#define _HAVE_FLUTTR_AUTH_H


G_BEGIN_DECLS

#define FLUTTR_TYPE_AUTH fluttr_auth_get_type()

#define FLUTTR_AUTH(obj) (G_TYPE_CHECK_INSTANCE_CAST ((obj), \
	FLUTTR_TYPE_AUTH, \
	FluttrAuth))

#define FLUTTR_AUTH_CLASS(klass) (G_TYPE_CHECK_CLASS_CAST ((klass),\
	FLUTTR_TYPE_AUTH, \
	FluttrAuthClass))

#define FLUTTR_IS_AUTH(obj) \
	(G_TYPE_CHECK_INSTANCE_TYPE ((obj), \
	FLUTTR_TYPE_AUTH))

#define FLUTTR_IS_AUTH_CLASS(klass) \
	(G_TYPE_CHECK_CLASS_TYPE ((klass), \
	FLUTTR_TYPE_AUTH))

#define FLUTTR_AUTH_GET_CLASS(obj) (G_TYPE_INSTANCE_GET_CLASS ((obj),\
	FLUTTR_TYPE_AUTH, \
	FluttrAuthClass))

typedef struct _FluttrAuth FluttrAuth;
typedef struct _FluttrAuthClass FluttrAuthClass;
typedef struct _FluttrAuthPrivate FluttrAuthPrivate;

struct _FluttrAuth
{
	ClutterGroup         parent;
	
	/* private */
	FluttrAuthPrivate   *priv;
};

struct _FluttrAuthClass 
{
	/*< private >*/
	ClutterGroupClass parent_class;

	void (*successful) (FluttrAuth *auth, gchar *null);
	void (*error) (FluttrAuth *auth, gchar *msg);
	void (*_fluttr_auth_3) (void);
	void (*_fluttr_auth_4) (void);
}; 

GType fluttr_auth_get_type (void) G_GNUC_CONST;

ClutterActor* 
fluttr_auth_new (const char *mini_token);

void
fluttr_auth_go (FluttrAuth *auth);

G_END_DECLS

#endif
