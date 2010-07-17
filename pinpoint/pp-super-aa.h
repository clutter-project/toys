/* pp-super-aa.h */

#ifndef _PP_SUPER_AA_H
#define _PP_SUPER_AA_H

#include <glib-object.h>
#include <mx/mx.h>

G_BEGIN_DECLS

#define PP_TYPE_SUPER_AA pp_super_aa_get_type()

#define PP_SUPER_AA(obj) \
  (G_TYPE_CHECK_INSTANCE_CAST ((obj), \
  PP_TYPE_SUPER_AA, PPSuperAA))

#define PP_SUPER_AA_CLASS(klass) \
  (G_TYPE_CHECK_CLASS_CAST ((klass), \
  PP_TYPE_SUPER_AA, PPSuperAAClass))

#define PP_IS_SUPER_AA(obj) \
  (G_TYPE_CHECK_INSTANCE_TYPE ((obj), \
  PP_TYPE_SUPER_AA))

#define PP_IS_SUPER_AA_CLASS(klass) \
  (G_TYPE_CHECK_CLASS_TYPE ((klass), \
  PP_TYPE_SUPER_AA))

#define PP_SUPER_AA_GET_CLASS(obj) \
  (G_TYPE_INSTANCE_GET_CLASS ((obj), \
  PP_TYPE_SUPER_AA, PPSuperAAClass))

typedef struct _PPSuperAA PPSuperAA;
typedef struct _PPSuperAAClass PPSuperAAClass;
typedef struct _PPSuperAAPrivate PPSuperAAPrivate;

struct _PPSuperAA
{
  MxOffscreen parent;

  PPSuperAAPrivate *priv;
};

struct _PPSuperAAClass
{
  MxOffscreenClass parent_class;
};

GType pp_super_aa_get_type (void) G_GNUC_CONST;

ClutterActor *pp_super_aa_new ();

void pp_super_aa_set_resolution (PPSuperAA *aa, gfloat x_res, gfloat y_res);
void pp_super_aa_get_resolution (PPSuperAA *aa, gfloat *x_res, gfloat *y_res);

G_END_DECLS

#endif /* _PP_SUPER_AA_H */
