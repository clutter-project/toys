
#ifndef ODO_DISTORT_FUNCS_H
#define ODO_DISTORT_FUNCS_H

#include <clutter/clutter.h>
#include "odo-texture.h"

G_BEGIN_DECLS

typedef struct
{
  gfloat           radius;
  gfloat           angle;
  gfloat           turn;
  gfloat           amplitude;
} OdoDistortData;

void
cloth_func (OdoTexture *otex,
            CoglTextureVertex *vertex,
            gfloat             width,
            gfloat             height,
            gpointer           data);

void
bowtie_func (OdoTexture        *otex,
             CoglTextureVertex *vertex,
             gfloat             width,
             gfloat             height,
             gpointer           data);

void
page_turn_func (OdoTexture        *otex,
                CoglTextureVertex *vertex,
                gfloat             width,
                gfloat             height,
                gpointer           data);

G_END_DECLS

#endif
