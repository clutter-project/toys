#include <webkit/webkitpopupfactory.h>

#include <clutter/clutter.h>
#include "popup-factory.h"

static void popup_factory_iface_init (WebKitPopupFactoryInterface *iface);

enum {
  NAME_COLUMN,
  LAST_COLUMN
};

enum {
  SHOW_MENU,
  HIDE_MENU,
  LAST_SIGNAL
};

G_DEFINE_TYPE_WITH_CODE (PopupFactory, popup_factory, TIDY_TYPE_LIST_VIEW,
			 G_IMPLEMENT_INTERFACE (WEBKIT_TYPE_POPUP_FACTORY,
						popup_factory_iface_init));
#define GET_PRIVATE(o) (G_TYPE_INSTANCE_GET_PRIVATE ((o), POPUP_TYPE_FACTORY, PopupFactoryPrivate))

struct _PopupFactoryPrivate
{
  ClutterModel *model;
};

static guint32 signals[LAST_SIGNAL] = {0, };

static void
popup_factory_class_init (PopupFactoryClass *klass)
{
  g_type_class_add_private (klass, sizeof (PopupFactoryPrivate));

  signals[SHOW_MENU] = g_signal_new ("show-menu",
				     G_TYPE_FROM_CLASS (klass),
				     G_SIGNAL_NO_RECURSE | 
				     G_SIGNAL_RUN_LAST,
				     G_STRUCT_OFFSET ( PopupFactoryClass,
						      show_menu),
				     NULL, NULL,
				     g_cclosure_marshal_VOID__VOID,
				     G_TYPE_NONE, 0);
  
  signals[HIDE_MENU] = g_signal_new ("hide-menu",
				     G_TYPE_FROM_CLASS (klass),
				     G_SIGNAL_NO_RECURSE | 
				     G_SIGNAL_RUN_LAST,
				     G_STRUCT_OFFSET ( PopupFactoryClass,
						      hide_menu),
				     NULL, NULL,
				     g_cclosure_marshal_VOID__VOID,
				     G_TYPE_NONE, 0);
}

static void
popup_factory_init (PopupFactory *factory)
{
  PopupFactoryPrivate *priv = factory->priv = GET_PRIVATE (factory);
  
  priv->model = clutter_list_model_new (LAST_COLUMN, G_TYPE_STRING, "name");
  g_object_set (G_OBJECT (factory),
		"model", priv->model,
		NULL);
}
	
static void
factory_clear (WebKitPopupFactory *factory)
{
  PopupFactoryPrivate *priv = POPUP_FACTORY (factory)->priv;

  if (priv->model) {
    g_object_unref (priv->model);
  }

  priv->model = clutter_list_model_new (LAST_COLUMN, G_TYPE_STRING, "name");
}

static void
factory_append_separator (WebKitPopupFactory *factory)
{
}

static void
factory_append_item (WebKitPopupFactory *factory,
		     const char         *text)
{
  PopupFactoryPrivate *priv = POPUP_FACTORY (factory)->priv;

  clutter_model_append (priv->model, NAME_COLUMN, text, -1);
}

static void
factory_show (WebKitPopupFactory *factory, int index)
{
  PopupFactoryPrivate *priv = POPUP_FACTORY (factory)->priv;
  
  tidy_list_view_set_model (TIDY_LIST_VIEW (factory), priv->model);
  g_signal_emit (factory, signals[SHOW_MENU], 0);
}

static void
factory_hide (WebKitPopupFactory *factory)
{
  g_signal_emit (factory, signals[HIDE_MENU], 0);
  tidy_list_view_set_model (TIDY_LIST_VIEW (factory), NULL);
}

static void
popup_factory_iface_init (WebKitPopupFactoryInterface *iface)
{
  iface->clear = factory_clear;
  iface->append_separator = factory_append_separator;
  iface->append_item = factory_append_item;
  iface->show = factory_show;
  iface->hide = factory_hide;
}
