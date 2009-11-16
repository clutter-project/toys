/*
 * Copyright (C) 2007 Neil J. Patel
 * Copyright (C) 2007 OpenedHand Ltd
 *
 * Author: Neil J. Patel  <njp@o-hand.com>
 */


#include <config.h>
#include <glib-object.h>

#ifndef _HAVE_FLUTTR_SETTINGS_H
#define _HAVE_FLUTTR_SETTINGS_H

G_BEGIN_DECLS

#define FLUTTR_TYPE_SETTINGS fluttr_settings_get_type()

#define FLUTTR_SETTINGS(obj) (G_TYPE_CHECK_INSTANCE_CAST ((obj), \
	FLUTTR_TYPE_SETTINGS, \
	FluttrSettings))

#define FLUTTR_SETTINGS_CLASS(klass) (G_TYPE_CHECK_CLASS_CAST ((klass),\
	FLUTTR_TYPE_SETTINGS, \
	FluttrSettingsClass))

#define FLUTTR_IS_SETTINGS(obj) \
	(G_TYPE_CHECK_INSTANCE_TYPE ((obj), \
	FLUTTR_TYPE_SETTINGS))

#define FLUTTR_IS_SETTINGS_CLASS(klass) \
	(G_TYPE_CHECK_CLASS_TYPE ((klass), \
	FLUTTR_TYPE_SETTINGS))

#define FLUTTR_SETTINGS_GET_CLASS(obj) (G_TYPE_INSTANCE_GET_CLASS ((obj),\
	FLUTTR_TYPE_SETTINGS, \
	FluttrSettingsClass))

typedef struct _FluttrSettings FluttrSettings;
typedef struct _FluttrSettingsClass FluttrSettingsClass;
typedef struct _FluttrSettingsPrivate FluttrSettingsPrivate;

struct _FluttrSettings
{
	GObject         parent;
	
	/* private */
	FluttrSettingsPrivate   *priv;
};

struct _FluttrSettingsClass 
{
	/*< private >*/
	GObjectClass parent_class;
}; 

GType                           
fluttr_settings_get_type (void);

FluttrSettings*
fluttr_settings_new (void);

FluttrSettings*
fluttr_settings_get_default (void);

G_END_DECLS

#endif

