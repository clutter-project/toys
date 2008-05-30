#ifndef _POPUP_FACTORY
#define _POPUP_FACTORY

#include <tidy/tidy-list-view.h>

G_BEGIN_DECLS

#define POPUP_TYPE_FACTORY (popup_factory_get_type ())

typedef struct _PopupFactoryPrivate PopupFactoryPrivate;

#define POPUP_FACTORY(obj) (G_TYPE_CHECK_INSTANCE_CAST ((obj), POPUP_TYPE_FACTORY, PopupFactory))

typedef struct {
  TidyListView parent;

  PopupFactoryPrivate *priv;
} PopupFactory;

typedef struct {
  TidyListViewClass parent_class;
  
  void (*show_menu) (PopupFactory *factory);
  void (*hide_menu) (PopupFactory *factory);
} PopupFactoryClass;

GType popup_factory_get_type (void);

G_END_DECLS

#endif
