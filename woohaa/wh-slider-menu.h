#ifndef _HAVE_WOOHAA_SLIDER_MENU_H
#define _HAVE_WOOHAA_SLIDER_MENU_H

#include <clutter/clutter.h>
#include <glib-object.h>

G_BEGIN_DECLS
#define WOOHAA_TYPE_SLIDER_MENU woohaa_slider_menu_get_type()

#define WOOHAA_SLIDER_MENU(obj) \
  (G_TYPE_CHECK_INSTANCE_CAST ((obj), \
  WOOHAA_TYPE_SLIDER_MENU, WoohaaSliderMenu))

#define WOOHAA_SLIDER_MENU_CLASS(klass) \
  (G_TYPE_CHECK_CLASS_CAST ((klass), \
  WOOHAA_TYPE_SLIDER_MENU, WoohaaSliderMenuClass))

#define WOOHAA_IS_SLIDER_MENU(obj) \
  (G_TYPE_CHECK_INSTANCE_TYPE ((obj), \
  WOOHAA_TYPE_SLIDER_MENU))

#define WOOHAA_IS_SLIDER_MENU_CLASS(klass) \
  (G_TYPE_CHECK_CLASS_TYPE ((klass), \
  WOOHAA_TYPE_SLIDER_MENU))

#define WOOHAA_SLIDER_MENU_GET_CLASS(obj) \
  (G_TYPE_INSTANCE_GET_CLASS ((obj), \
  WOOHAA_TYPE_SLIDER_MENU, WoohaaSliderMenuClass))

typedef struct _WoohaaSliderMenu        WoohaaSliderMenu;
typedef struct _WoohaaSliderMenuClass   WoohaaSliderMenuClass;
typedef struct _WoohaaSliderMenuPrivate WoohaaSliderMenuPrivate;

struct _WoohaaSliderMenu
{
  /*< private >*/
  ClutterActor             parent;
  WoohaaSliderMenuPrivate *priv;
}; 

struct _WoohaaSliderMenuClass 
{
  /*< private >*/
  ClutterActorClass parent_class;

  /* Future padding */
  void (* __reserved1) (void);
  void (* __reserved2) (void);
  void (* __reserved3) (void);
  void (* __reserved4) (void);
  void (* __reserved5) (void);
  void (* __reserved6) (void);
}; 

typedef void (*WoohaaSliderMenuSelectedFunc) (WoohaaSliderMenu *menu,
					      ClutterActor     *actor,
					      gpointer          userdata);

GType         woohaa_slider_menu_get_type  (void) G_GNUC_CONST;
ClutterActor *woohaa_slider_menu_new       (const gchar *font);

void woohaa_slider_menu_add_option (WoohaaSliderMenu            *menu, 
				    const gchar                 *text,
				    WoohaaSliderMenuSelectedFunc selected,
				    gpointer                     userdata);
void woohaa_slider_menu_activate   (WoohaaSliderMenu             *menu,
				    gint                         entry_num);
void woohaa_slider_menu_advance    (WoohaaSliderMenu             *menu, 
				    gint                          n);

G_END_DECLS

#endif
