#ifndef __GCR_H__
#define __GCR_H__
#include <clutter/clutter.h>

/* this needs to be called before clutter_main()  */
void gcr_prepare (const gchar *path);
void gcr_start   (void);
void gcr_stop    (void);

#endif
