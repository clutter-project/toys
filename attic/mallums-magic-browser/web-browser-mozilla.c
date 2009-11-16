#include <clutter/clutter.h>

#include "web-browser-mozilla.h"

#ifdef WITH_MOZILLA
#  include "clutter-mozembed.h"
#else
#  include <tidy/tidy.h>
#  include <webkit/webkit.h>
#  include "scroll-frame.h"
#  include "popup-factory.h"
#endif

static void
tabs_cb (ClutterActor *button,
         ClutterEvent *event,
         MmBrowser    *browser);

G_DEFINE_TYPE (MmBrowser, mm_browser, CLUTTER_TYPE_GROUP)
#define BROWSER_PRIVATE(o) \
  (G_TYPE_INSTANCE_GET_PRIVATE ((o), MM_TYPE_BROWSER, MmBrowserPrivate))

typedef struct _MmBrowserPage
{
  MmBrowser *browser;
  ClutterActor *web;
  ClutterActor *overlay;
  ClutterActor *scroll;
  ClutterActor *popup_menu;

#ifndef WITH_MOZILLA
  PopupFactory *factory;

  WebKitWebView *view;
  WebkitAdjustment *hadj, *vadj;
#endif

  char *address;

  gboolean over_link;

  int start_x;
  int start_y;
} MmBrowserPage;

struct _MmBrowserPrivate
{
  ClutterTimeline *fade_timeline;
  ClutterTimeline *scale_timeline;
  ClutterTimeline *scroll_timeline;
  ClutterTimeline *move_timeline;

  ClutterActor *toolbar, *toolbar_bg;
  ClutterActor *tab_control;
  ClutterActor *new_tab;
  ClutterActor *prev_tab;
  ClutterActor *next_tab;

  ClutterActor *next_prev_group;

  ClutterActor *back;
  ClutterActor *forward;
  ClutterActor *entry;
  ClutterActor *tabs;
  ClutterActor *progress;

  ClutterActor *page_group;

  GList *pages;
  GList *current_page;

  gboolean showing_tabs;
  gboolean maybe_scroll;

  int popup_x;
  int popup_y;
};

#define WEB_WIDTH 800
#define WEB_HEIGHT 480

#define JITTER 5

static void
mm_browser_finalize (GObject *object)
{
  G_OBJECT_CLASS (mm_browser_parent_class)->finalize (object);
}

static void
mm_browser_dispose (GObject *object)
{
  G_OBJECT_CLASS (mm_browser_parent_class)->dispose (object);
}

static void
mm_browser_class_init (MmBrowserClass *klass)
{
  GObjectClass *o_class = (GObjectClass *) klass;
  ClutterActorClass *a_class = (ClutterActorClass *) klass;

  g_type_class_add_private (klass, sizeof (MmBrowserPrivate));

  o_class->finalize = mm_browser_finalize;
  o_class->dispose = mm_browser_dispose;
}

static void
set_back_and_forward (MmBrowser *browser)
{
  MmBrowserPrivate *priv = browser->priv;
  MmBrowserPage *page;

  /* Get top page */
  page = priv->current_page->data;

#ifdef WITH_MOZILLA
  if (clutter_mozembed_can_go_back (CLUTTER_MOZEMBED (page->web))) {
#else
  if (webkit_web_view_can_go_back (page->view)) {
#endif
    clutter_actor_animate (priv->back, CLUTTER_LINEAR, 500,
                           "opacity", 0xff, NULL);
  } else {
    clutter_actor_animate (priv->back, CLUTTER_LINEAR, 500,
                           "opacity", 0x55, NULL);
  }

#ifdef WITH_MOZILLA
  if (clutter_mozembed_can_go_forward (CLUTTER_MOZEMBED (page->web))) {
#else
  if (webkit_web_view_can_go_forward (page->view)) {
#endif
    clutter_actor_animate (priv->forward, CLUTTER_LINEAR, 500,
                           "opacity", 0xff, NULL);
  } else {
    clutter_actor_animate (priv->forward, CLUTTER_LINEAR, 500,
                           "opacity", 0x55, NULL);
  }
}

static void
load_started_cb (MmBrowser      *browser)
{
  MmBrowserPrivate *priv = browser->priv;
  ClutterTimeline *tl;

  clutter_actor_animate (priv->progress, CLUTTER_LINEAR, 500,
                         "opacity", 0xff, NULL);
}

#ifdef WITH_MOZILLA
static void
load_finished_cb (MmBrowser       *browser,
                  ClutterMozEmbed *mozembed)
#else
static void
load_finished_cb (MmBrowser      *browser,
                  WebKitWebFrame *frame,
                  WebKitWebView  *web_view)
#endif
{
  MmBrowserPrivate *priv = browser->priv;
  const gchar *location;
  MmBrowserPage *page;
  ClutterTimeline *tl;

  clutter_actor_animate (priv->progress, CLUTTER_LINEAR, 500,
                         "opacity", 0x00, NULL);

#ifdef WITH_MOZILLA
  location = clutter_mozembed_get_location (mozembed);
#else
  location = webkit_web_frame_get_uri (frame);
#endif

  clutter_text_set_text (CLUTTER_TEXT (priv->entry), location);
  page = priv->current_page->data;

  g_free (page->address);
  page->address = g_strdup (location);

  set_back_and_forward (browser);
}

static gboolean
web_event_capture_cb (ClutterActor  *actor,
                      ClutterEvent  *event,
                      MmBrowserPage *page)
{
  MmBrowser *browser = page->browser;
  MmBrowserPrivate *priv = browser->priv;

  switch (event->type) {
  case CLUTTER_BUTTON_PRESS:
    if (priv->showing_tabs == TRUE)
      {
        tabs_cb (NULL, NULL, browser);
        return TRUE;
      }

    return FALSE;

  case CLUTTER_BUTTON_RELEASE:
    return FALSE;

  case CLUTTER_MOTION:
#if 0
    if (priv->maybe_scroll == TRUE) {
      ClutterMotionEvent *mev = (ClutterMotionEvent *) event;
      int dx = mev->x - page->start_x;
      int dy = mev->y - page->start_y;

      gtk_adjustment_set_value (page->hscroll,
                                MIN (page->hscroll->value - dx,
                                     page->hscroll->upper - WEB_WIDTH));
      gtk_adjustment_set_value (page->vscroll,
                                MIN (page->vscroll->value - dy,
                                     page->vscroll->upper - WEB_HEIGHT));

      page->start_x = mev->x;
      page->start_y = mev->y;
    } else {
      return FALSE;
    }
#endif
    return FALSE;

  case CLUTTER_ENTER:
  case CLUTTER_LEAVE:
  default:
    /* Let the actor handle all the other events */
    return FALSE;
  }

  return TRUE;
}

#ifndef WITH_MOZILLA
static void
hovering_over_link_cb (WebKitWebView *view,
                       const char    *string1, /* What is this string? */
                       const char    *url,
                       MmBrowserPage *page)
{
  if (string1 == NULL && url == NULL) {
    page->over_link = FALSE;
  } else {
    page->over_link = TRUE;
  }
}

static void
show_popup_menu (WebKitPopupFactory *factory,
                 MmBrowser          *browser)
{
  MmBrowserPrivate *priv = browser->priv;
  MmBrowserPage *page;

  page = priv->current_page->data;
  clutter_actor_raise_top (page->popup_menu);
  clutter_actor_show_all (page->popup_menu);
}

static void
hide_popup_menu (WebKitPopupFactory *factory,
                 MmBrowser          *browser)
{
  MmBrowserPrivate *priv = browser->priv;
  MmBrowserPage *page;

  page = priv->current_page->data;
  clutter_actor_hide (page->popup_menu);
}

static gboolean
popup_button_release_cb (ClutterActor       *actor,
                         ClutterButtonEvent *event,
                         MmBrowser          *browser)
{
  MmBrowserPrivate *priv = browser->priv;
  MmBrowserPage *page;

  page = priv->current_page->data;

  if ((ABS (event->x - priv->popup_x) < JITTER) &&
      (ABS (event->y - priv->popup_y) < JITTER)) {
    int row;

    row = tidy_list_view_get_row_at_pos (TIDY_LIST_VIEW (page->factory),
                                         event->x, event->y);
    if (row == -1) {
      return FALSE;
    }

    webkit_popup_factory_activate (WEBKIT_POPUP_FACTORY (page->factory), row);
    webkit_popup_factory_close (WEBKIT_POPUP_FACTORY (page->factory));
    return TRUE;
  }

  return FALSE;
}

static gboolean
popup_button_press_cb (ClutterActor       *actor,
                       ClutterButtonEvent *event,
                       MmBrowser          *browser)
{
  MmBrowserPrivate *priv = browser->priv;

  if (event->button != 1) {
    return FALSE;
  }

  priv->popup_x = event->x;
  priv->popup_y = event->y;

  return TRUE;
}

static void
create_popup_factory (MmBrowser     *browser,
		      MmBrowserPage *page)
{
  MmBrowserPrivate *priv = browser->priv;
  ClutterActor *bground, *scroll;
  ClutterColor black = {0xbb, 0xbb, 0xbb, 0xdd};

  page->popup_menu = clutter_group_new ();

  bground = clutter_rectangle_new_with_color (&black);
  clutter_container_add_actor (CLUTTER_CONTAINER (page->popup_menu), bground);
  clutter_actor_set_size (bground, WEB_WIDTH, 125);
  clutter_actor_show (bground);

  page->factory = g_object_new (POPUP_TYPE_FACTORY,
				"rules-hint", FALSE,
				"show-headers", FALSE,
				NULL);
  tidy_stylable_set_style (TIDY_STYLABLE (page->factory), tidy_style_new ());
  tidy_stylable_set (TIDY_STYLABLE (page->factory),
		     "font-name", "Impact 20", NULL);

  g_signal_connect (page->factory, "show-menu",
                    G_CALLBACK (show_popup_menu), browser);
  g_signal_connect (page->factory, "hide-menu",
                    G_CALLBACK (hide_popup_menu), browser);
  g_signal_connect (page->factory, "button-press-event",
                    G_CALLBACK (popup_button_press_cb), browser);
  g_signal_connect (page->factory, "button-release-event",
                    G_CALLBACK (popup_button_release_cb), browser);
  webkit_web_view_set_popup_factory (page->view, WEBKIT_POPUP_FACTORY (page->factory));
  clutter_actor_set_size (CLUTTER_ACTOR (page->factory), WEB_WIDTH, 125);
  clutter_actor_show (CLUTTER_ACTOR (page->factory));

  scroll = tidy_finger_scroll_new (TIDY_FINGER_SCROLL_MODE_KINETIC);
  clutter_container_add_actor (CLUTTER_CONTAINER (page->popup_menu), scroll);
  clutter_container_add_actor (CLUTTER_CONTAINER (scroll),
                               CLUTTER_ACTOR (page->factory));
  clutter_actor_set_size (scroll, WEB_WIDTH, 125);

  clutter_actor_set_position (page->popup_menu, 0, WEB_HEIGHT - 125);
  clutter_container_add_actor (CLUTTER_CONTAINER (clutter_stage_get_default ()),
                               page->popup_menu);

  clutter_actor_show_all (scroll);
}

static void
page_start_editing_cb (WebkitActor *actor,
		       MmBrowser   *browser)
{
  MmBrowserPrivate *priv = browser->priv;
  MmBrowserPage *page;

  /* Get top page */
  page = priv->current_page->data;

  webkit_web_view_zoom_to_selected_node (page->view);
}

static void
page_stop_editing_cb (WebkitActor *actor,
		      MmBrowser   *browser)
{
  MmBrowserPrivate *priv = browser->priv;
  MmBrowserPage *page;

  /* Get top page */
  page = priv->current_page->data;

  webkit_web_view_zoom_to_default (page->view);
}
#endif

static void
add_new_page (MmBrowser *browser)
{
  MmBrowserPrivate *priv = browser->priv;
  MmBrowserPage *page;
  ClutterActor *frame;

  page = g_new (MmBrowserPage, 1);
  page->address = NULL;
  page->browser = browser;

#ifdef WITH_MOZILLA
  page->scroll = page->web = clutter_mozembed_new ();
  clutter_actor_set_size (page->web, WEB_WIDTH, WEB_HEIGHT);
  clutter_mozembed_open (CLUTTER_MOZEMBED (page->web), "about:blank");
  g_signal_connect_swapped (page->web, "net-start",
                            G_CALLBACK (load_started_cb), browser);
  g_signal_connect_swapped (page->web, "net-stop",
                            G_CALLBACK (load_finished_cb), browser);
#else
  page->hadj = webkit_adjustment_new (0,0,0,0,0,0);
  page->vadj = webkit_adjustment_new (0,0,0,0,0,0);

  page->web = webkit_web_view_new (WEB_WIDTH, WEB_HEIGHT);
  webkit_web_view_set_scroll_adjustments (WEBKIT_WEB_VIEW (page->web),
					  page->hadj, page->vadj);

  clutter_actor_set_reactive (page->web, TRUE);
  clutter_actor_set_size (page->web, WEB_WIDTH, WEB_HEIGHT);
  g_signal_connect (page->web, "captured-event",
                    G_CALLBACK (webkit_event_capture_cb), page);
  page->view = WEBKIT_WEB_VIEW (page->web);
  clutter_actor_show (page->web);

  frame = g_object_new (SCROLL_TYPE_FRAME, NULL);
  /* clutter_actor_set_size (frame, WEB_WIDTH, WEB_HEIGHT); */
  clutter_actor_show (frame);

  scroll_frame_add_webkit (SCROLL_FRAME (frame), page->view);

  page->scroll = tidy_finger_scroll_new (TIDY_FINGER_SCROLL_MODE_KINETIC);
  tidy_stylable_set_style (TIDY_STYLABLE (page->scroll), tidy_style_new ());
  tidy_stylable_set (TIDY_STYLABLE (page->scroll),
		     "xthickness", 5, "ythickness", 5, NULL);
  clutter_actor_set_size (page->scroll, WEB_WIDTH, WEB_HEIGHT);
  clutter_container_add_actor (CLUTTER_CONTAINER (page->scroll), frame);

  webkit_web_view_open (page->view, "about:blank");
  g_signal_connect_swapped (page->view, "load-started",
                            G_CALLBACK (load_started_cb), browser);
  g_signal_connect_swapped (page->view, "load-finished",
                            G_CALLBACK (load_finished_cb), browser);
  g_signal_connect (page->view, "hovering-over-link",
                    G_CALLBACK (hovering_over_link_cb), page);
  g_signal_connect (page->view, "start-editing",
		    G_CALLBACK (page_start_editing_cb), browser);
  g_signal_connect (page->view, "stop-editing",
		    G_CALLBACK (page_stop_editing_cb), browser);

  create_popup_factory (browser, page);
#endif

  clutter_actor_set_anchor_point_from_gravity (page->scroll,
                                               CLUTTER_GRAVITY_CENTER);
  clutter_actor_set_position (page->scroll, WEB_WIDTH / 2, WEB_HEIGHT / 2);

  clutter_container_add_actor (CLUTTER_CONTAINER (priv->page_group),
                               page->scroll);

  priv->pages = g_list_append (priv->pages, page);

  /* Fixme...obviously */
  priv->current_page = g_list_last (priv->pages);
}

static ClutterActor *
make_button (const char *image)
{
  return clutter_texture_new_from_file (image, NULL);
}

#if 0
static void
key_release_cb (ClutterEntry *entry,
                ClutterEvent *event,
                MmBrowser    *browser)
{
  if (event->type == CLUTTER_KEY_RELEASE) {
    ClutterKeyEvent *kev = (ClutterKeyEvent *) event;

    clutter_entry_handle_key_event (CLUTTER_ENTRY (browser->priv->entry), kev);
  }
}
#endif
static void
entry_activated_cb (ClutterText  *entry,
                    MmBrowser    *browser)
{
  ClutterActor *stage = clutter_stage_get_default ();
  MmBrowserPrivate *priv = browser->priv;
  char *address = g_strdup (clutter_text_get_text (entry));
  MmBrowserPage *page;

  mm_browser_open (browser, address);

  page = priv->current_page->data;
  clutter_stage_set_key_focus (CLUTTER_STAGE (stage), page->web);
  g_free (address);
}

static gboolean
entry_clicked_cb (ClutterActor       *actor,
		  ClutterButtonEvent *event,
		  MmBrowser          *browser)
{
  ClutterActor *stage = clutter_stage_get_default ();
  MmBrowserPrivate *priv = browser->priv;

  clutter_stage_set_key_focus (CLUTTER_STAGE (stage), priv->entry);
  
  return FALSE;
}

static void
back_cb (ClutterActor *button,
         ClutterEvent *event,
         MmBrowser    *browser)
{
  MmBrowserPrivate *priv = browser->priv;
  MmBrowserPage *page;

  if (priv->showing_tabs == TRUE)
    return;

  /* Get top page */
  page = priv->current_page->data;
#ifdef WITH_MOZILLA
  clutter_mozembed_back (CLUTTER_MOZEMBED (page->web));
#else
  webkit_web_view_go_back (page->view);
#endif
  set_back_and_forward (browser);
}

static void
forward_cb (ClutterActor *button,
            ClutterEvent *event,
            MmBrowser    *browser)
{
  MmBrowserPrivate *priv = browser->priv;
  MmBrowserPage *page;

  if (priv->showing_tabs == TRUE)
    return;

  /* Get top page */
  page = priv->current_page->data;
#ifdef WITH_MOZILLA
  clutter_mozembed_forward (CLUTTER_MOZEMBED (page->web));
#else
  webkit_web_view_go_forward (page->view);
#endif
  set_back_and_forward (browser);
}

static void
hide_on_effect_complete (ClutterActor *actor,
			 gpointer      userdata)
{
  clutter_actor_hide (actor);
}

static void
tabs_cb (ClutterActor *button,
         ClutterEvent *event,
         MmBrowser    *browser)
{
  MmBrowserPrivate *priv = browser->priv;
  MmBrowserPage *page, *prev = NULL, *next = NULL;

  if (priv->showing_tabs == FALSE) {
    page = priv->current_page->data;

    /* Layout previous page */
    if (priv->current_page->prev) {
      prev = priv->current_page->prev->data;

      clutter_actor_set_scale (prev->scroll, 0.4, 0.4);
      clutter_actor_set_position (prev->scroll, 0, 240);
      clutter_actor_set_opacity (prev->scroll, 0x00);

      clutter_actor_show (prev->scroll);
    } else {
      g_print ("No prev\n");
    }

    /* Layout next page */
    if (priv->current_page->next) {
      next = priv->current_page->next->data;

      clutter_actor_set_scale (next->scroll, 0.4, 0.4);
      clutter_actor_set_position (next->scroll, 800, 240);
      clutter_actor_set_opacity (next->scroll, 0x00);

      clutter_actor_show (next->scroll);
    }

    clutter_actor_animate (page->scroll, CLUTTER_LINEAR, 100,
                           "scale-x", 0.4, "scale-y", 0.4, NULL);
    clutter_actor_show (priv->tab_control);
    clutter_actor_animate (priv->tab_control, CLUTTER_LINEAR, 500,
                           "opacity", 0xff, NULL);
    if (prev != NULL) {
      clutter_actor_show (prev->scroll);
      clutter_actor_animate (prev->scroll, CLUTTER_LINEAR, 500,
                             "opacity", 0xff, NULL);
    }

    if (next != NULL) {
      clutter_actor_show (next->scroll);
      clutter_actor_animate (next->scroll, CLUTTER_LINEAR, 500,
                             "opacity", 0xff, NULL);
    }

    priv->showing_tabs = TRUE;
  } else {
    page = priv->current_page->data;

    if (priv->current_page->prev) {
      prev = priv->current_page->prev->data;

      g_signal_connect_swapped (clutter_actor_animate (prev->scroll,
                                                       CLUTTER_LINEAR, 500,
                                                       "opacity", 0x00, NULL),
                                "completed",
                                G_CALLBACK (hide_on_effect_complete),
                                prev->scroll);
    }

    if (priv->current_page->next) {
      next = priv->current_page->next->data;

      g_signal_connect_swapped (clutter_actor_animate (next->scroll,
                                                       CLUTTER_LINEAR, 500,
                                                       "opacity", 0x00, NULL),
                                "completed",
                                G_CALLBACK (hide_on_effect_complete),
                                next->scroll);
    }

    clutter_actor_animate (page->scroll, CLUTTER_LINEAR, 100,
                           "scale-x", 1.0, "scale-y", 1.0, NULL);
    g_signal_connect_swapped (clutter_actor_animate (priv->tab_control,
                                                     CLUTTER_LINEAR, 500,
                                                     "opacity", 0x00, NULL),
                              "completed",
                              G_CALLBACK (hide_on_effect_complete),
                              priv->tab_control);
    priv->showing_tabs = FALSE;
  }
}

static void
select_previous_tab (ClutterActor *button,
                     ClutterEvent *event,
                     MmBrowser    *browser)
{
  ClutterActor *stage = clutter_stage_get_default ();
  MmBrowserPrivate *priv = browser->priv;
  MmBrowserPage *pages[4], *current;
  int i;

  pages[2] = priv->current_page->data;

  if (priv->current_page->next) {
    pages[3] = priv->current_page->next->data;
  } else {
    pages[3] = NULL;
  }

  if (priv->current_page->prev) {
    pages[1] = priv->current_page->prev->data;

    if (priv->current_page->prev->prev) {
      pages[0] = priv->current_page->prev->prev->data;
    } else {
      pages[0] = NULL;
    }
  } else {
    /* Current page was the first page, so we can't screll */
    return;
  }

  /* Scroll all four pages */
  for (i = 0; i < 4; i++) {
    int x;

    if (pages[i] == NULL) {
      continue;
    }

    clutter_actor_get_position (pages[i]->scroll, &x, NULL);
    clutter_actor_animate (pages[i]->scroll, CLUTTER_LINEAR, 400,
                           "x", x + 400, NULL);
  }

  priv->current_page = priv->current_page->prev;
  current = priv->current_page->data;
  clutter_stage_set_key_focus (CLUTTER_STAGE (stage), current->scroll);
  clutter_text_set_text (CLUTTER_TEXT (priv->entry), 
                          current->address ? current->address : "");
}

static void
select_next_tab (ClutterActor *button,
                 ClutterEvent *event,
                 MmBrowser    *browser)
{
  ClutterActor *stage = clutter_stage_get_default ();
  MmBrowserPrivate *priv = browser->priv;
  MmBrowserPage *pages[4], *current;
  int i;

  pages[1] = priv->current_page->data;

  if (priv->current_page->prev) {
    pages[0] = priv->current_page->prev->data;
  } else {
    pages[0] = NULL;
  }

  if (priv->current_page->next) {
    pages[2] = priv->current_page->next->data;

    if (priv->current_page->next->next) {
      pages[3] = priv->current_page->next->next->data;
    } else {
      pages[3] = NULL;
    }
  } else {
    /* Current page was last page, so we can't scroll */
    return;
  }

  /* Scroll all four pages */
  for (i = 0; i < 4; i++) {
    int x;

    if (pages[i] == NULL) {
      continue;
    }

    clutter_actor_get_position (pages[i]->scroll, &x, NULL);
    clutter_actor_animate (pages[i]->scroll, CLUTTER_LINEAR, 400,
                           "x", x - 400, NULL);
  }

  priv->current_page = priv->current_page->next;
  current = priv->current_page->data;
  clutter_stage_set_key_focus (CLUTTER_STAGE (stage), current->web);
  clutter_text_set_text (CLUTTER_TEXT (priv->entry), 
                          current->address ? current->address : "");
}

static void
create_new_tab (ClutterActor *button,
                ClutterEvent *event,
                MmBrowser    *browser)
{
  g_print ("New tab\n");
}

static void
mm_browser_init (MmBrowser *self)
{
  MmBrowserPrivate *priv;
  ClutterColor white = {0x33, 0x33, 0x33, 0xff};
  ClutterColor progress_color = {0x00, 0x55, 0xdd, 0xff};
  ClutterActor *stage = clutter_stage_get_default ();
  ClutterAlpha *alpha;
  ClutterBehaviour *behave;
  ClutterKnot progress_knots[] = {{265, 11}, {515, 11}};
  MmBrowserPage *page;

  priv = self->priv = BROWSER_PRIVATE (self);

  alpha = clutter_alpha_new_full (priv->move_timeline, CLUTTER_EASE_IN_OUT_CUBIC);
  behave = clutter_behaviour_path_new_with_knots (alpha, progress_knots, 2);

  priv->pages = NULL;
  priv->showing_tabs = FALSE;

  priv->page_group = clutter_group_new ();
  clutter_container_add_actor (CLUTTER_CONTAINER (self), priv->page_group);
  clutter_actor_set_position (priv->page_group, 0, 0);
  clutter_actor_set_reactive (priv->page_group, TRUE);

  add_new_page (self);
  add_new_page (self);
  add_new_page (self);

  priv->current_page = priv->current_page->prev;
  clutter_actor_show (((MmBrowserPage *) priv->current_page->data)->scroll);
  clutter_actor_raise_top (((MmBrowserPage *) priv->current_page->data)->scroll);

  priv->tab_control = clutter_group_new ();
  clutter_container_add_actor (CLUTTER_CONTAINER (priv->page_group), priv->tab_control);
  clutter_actor_set_position (priv->tab_control, 0, 350);
  clutter_actor_set_size (priv->tab_control, 800, 34);
  
  priv->prev_tab = make_button ("assets/go-previous.png");
  clutter_container_add_actor (CLUTTER_CONTAINER (priv->tab_control),
                               priv->prev_tab);
  clutter_actor_set_reactive (priv->prev_tab, TRUE);
  clutter_actor_set_position (priv->prev_tab, 20, 2);
  g_signal_connect (priv->prev_tab, "button-release-event",
                    G_CALLBACK (select_previous_tab), self);

  priv->next_tab = make_button ("assets/go-next.png");
  clutter_container_add_actor (CLUTTER_CONTAINER (priv->tab_control),
                               priv->next_tab);
  clutter_actor_set_reactive (priv->next_tab, TRUE);
  clutter_actor_set_position (priv->next_tab, 748, 2);
  g_signal_connect (priv->next_tab, "button-release-event",
                    G_CALLBACK (select_next_tab), self);

#if 0  
  priv->new_tab = make_button ("assets/document-new.png");
  clutter_container_add_actor (CLUTTER_CONTAINER (priv->tab_control),
                               priv->new_tab);
  clutter_actor_set_reactive (priv->new_tab, TRUE);
  clutter_actor_set_position (priv->new_tab, 384, 2);
  g_signal_connect (priv->new_tab, "button-release-event",
                    G_CALLBACK (create_new_tab), self);
#endif
  clutter_actor_set_opacity (priv->tab_control, 0x00);
  clutter_actor_show_all (priv->tab_control);

  clutter_actor_show (priv->page_group);

  priv->toolbar = clutter_group_new ();
  clutter_container_add_actor (CLUTTER_CONTAINER (self), priv->toolbar);
  clutter_actor_set_position (priv->toolbar, 0, 430);

  priv->toolbar_bg = clutter_texture_new_from_file ("assets/toolbar-bg.png", NULL);
  clutter_group_add (CLUTTER_GROUP (priv->toolbar), priv->toolbar_bg);

  priv->progress = clutter_rectangle_new_with_color (&progress_color);
  clutter_container_add_actor (CLUTTER_CONTAINER (priv->toolbar), 
                               priv->progress);
  clutter_actor_set_size (priv->progress, 30, 28);
  clutter_actor_set_position (priv->progress, 265, 11);
  clutter_actor_set_opacity (priv->progress, 0x00);
  clutter_behaviour_apply (behave, priv->progress);


  priv->back = make_button ("assets/back.png");
  clutter_container_add_actor (CLUTTER_CONTAINER (priv->toolbar), priv->back);
  clutter_actor_set_reactive (CLUTTER_ACTOR (priv->back), TRUE);
  clutter_actor_set_position (priv->back, 140, 2);
  g_signal_connect (priv->back, "button-release-event",
                    G_CALLBACK (back_cb), self);

  priv->forward = make_button ("assets/forward.png");
  clutter_container_add_actor (CLUTTER_CONTAINER (priv->toolbar), priv->forward);
  clutter_actor_set_reactive (CLUTTER_ACTOR (priv->forward), TRUE);
  clutter_actor_set_position (priv->forward, 200, 2);
  g_signal_connect (priv->forward, "button-release-event",
                    G_CALLBACK (forward_cb), self);

  priv->tabs = make_button ("assets/tabs.png");
  clutter_container_add_actor (CLUTTER_CONTAINER (priv->toolbar), priv->tabs);
  clutter_actor_set_reactive (CLUTTER_ACTOR (priv->tabs), TRUE);
  clutter_actor_set_position (priv->tabs, 8, 2);
  g_signal_connect (priv->tabs, "button-release-event",
                    G_CALLBACK (tabs_cb), self);


  priv->entry = clutter_text_new_full ("Sans 28px", "", &white);
  g_object_set (G_OBJECT (priv->entry),
                "editable", TRUE, "activatable", TRUE, NULL);
  clutter_container_add_actor (CLUTTER_CONTAINER (priv->toolbar), priv->entry);
  clutter_actor_set_reactive (priv->entry, TRUE);
  clutter_actor_set_position (priv->entry, 265, 11);
  clutter_actor_set_size (priv->entry, 515, 50);
#if 0
  g_signal_connect (priv->entry, "key-release-event",
                    G_CALLBACK (key_release_cb), self);
#endif
  g_signal_connect (priv->entry, "activate",
                    G_CALLBACK (entry_activated_cb), self);
  g_signal_connect (priv->entry, "button-release-event",
		    G_CALLBACK (entry_clicked_cb), self);

  set_back_and_forward (self);

  page = priv->current_page->data;
  clutter_stage_set_key_focus (CLUTTER_STAGE (stage), page->web);

  clutter_actor_show_all (priv->toolbar);

  /* clutter_actor_raise_top (priv->page_group); */
}

MmBrowser *
mm_browser_new (void)
{
  return g_object_new (MM_TYPE_BROWSER, NULL);
}

void
mm_browser_open (MmBrowser  *browser,
                 const char *address)
{
  MmBrowserPrivate *priv = browser->priv;
  MmBrowserPage *page;

  /* Get top page */
  page = priv->current_page->data;
#ifdef WITH_MOZILLA
  clutter_mozembed_open (CLUTTER_MOZEMBED (page->web), address);
#else
  webkit_web_view_open (page->view, address);
#endif
}

/***************************************************************************/

int
main (int    argc,
      char **argv)
{
        ClutterActor *stage;
        ClutterActor *background;
        MmBrowser *browser;
        ClutterColor col = {0x24, 0x29, 0x29, 0xff};

        clutter_init (&argc, &argv);

        stage = clutter_stage_get_default ();
        clutter_actor_set_size (stage, 800, 480);
        clutter_stage_set_color (CLUTTER_STAGE(stage), &col);

        browser = mm_browser_new ();
        clutter_actor_set_position (CLUTTER_ACTOR (browser), 0, 0);
        clutter_group_add (CLUTTER_GROUP (stage), CLUTTER_ACTOR (browser));
        clutter_actor_show_all (stage);

        if (argc < 2) {
          mm_browser_open (browser, "http://news.google.co.uk/");
        } else {
          mm_browser_open (browser, argv[1]);
        }

        clutter_main ();
        return 0;
}
