/*
 * Copyright (C) 2007 Neil J. Patel
 * Copyright (C) 2007 OpenedHand Ltd
 *
 * Author: Neil J. Patel  <njp@o-hand.com>
 */

#include <config.h>
#include <glib.h>
#include <glib/gstdio.h>
#include <clutter/clutter.h>

#include "fluttr-auth.h"
#include "fluttr-behave.h"
#include "fluttr-library.h"
#include "fluttr-library-row.h"
#include "fluttr-list.h"
#include "fluttr-list-view.h"
#include "fluttr-photo.h"
#include "fluttr-settings.h"
#include "fluttr-set-view.h"
#include "fluttr-set.h"
#include "fluttr-viewer.h"

#include <libnflick/nflick.h>

typedef enum {
	FLUTTR_VIEW_SETS,
	FLUTTR_VIEW_PHOTOS,
	FLUTTR_VIEW_PHOTO

} FluttrView;

typedef struct {
	FluttrLibrary		*library;
	
	ClutterActor 		*stage;
	ClutterActor 		*auth;
	ClutterActor		*sets;
	ClutterActor		*list;
	ClutterActor		*viewer;
	
	/* Current view info */
	FluttrView		 view;
	gulong			 sig;
	
	/* Flickr info */
	gchar 			*username;
	gchar			*fullname;
	gchar 			*token;
	gchar 			*usernsid;
	
	/* The swapping timeline */
	ClutterActor		*in;
	ClutterActor		*out;
	ClutterTimeline		*timeline;
	ClutterAlpha		*alpha;
	ClutterBehaviour	*behave;

} Fluttr;

static void      	browse_input_cb (ClutterStage *stage, 
				  	 ClutterEvent *event,
		      	 	  	 Fluttr	      *fluttr);

static void   	 	create_background (ClutterActor *bg,
				    	   guint width, 
				    	   guint height);

static gboolean 	check_credentials (Fluttr *fluttr);

static void		auth_successful (FluttrAuth *auth, gchar *null, 
					 Fluttr *fluttr);
static void		auth_error (FluttrAuth *auth, gchar *msg, 
				    Fluttr *fluttr);
				    
static void		list_get_successful (FluttrAuth *auth, 
					     NFlickWorker *worker, 
					     Fluttr *fluttr);
static void		list_get_error (FluttrAuth *auth, gchar *msg, 
				    	Fluttr *fluttr);
static void		_swap_alpha_func (ClutterBehaviour *behave,
		  			  guint		    alpha_value,
		  			  gpointer	   data);
		              	  	  
static gboolean
_auth_timeout (Fluttr *fluttr)
{
	fluttr_auth_go (FLUTTR_AUTH (fluttr->auth));
	return FALSE;
}

int
main (int argc, char **argv)
{
 	Fluttr *fluttr = g_new0 (Fluttr, 1);
 	ClutterActor *stage, *background, *list;
	ClutterColor stage_color = { 0x00, 0x00, 0x00, 0xff };
	FluttrSettings *settings = NULL;
	gchar *filename;
	
	g_thread_init (NULL);
	clutter_init (&argc, &argv);		
	
	/* Check that there are enough arguments */
	if (argc < 2 && !(check_credentials (fluttr))) {
		g_print ("\n\nYou need to start Fluttr with your Flickr "\
			  "authorisation code, which is available here:\n"\
			  "http://www.flickr.com/auth-72157600141007022\n\n");
		return 0;
	}
	
	/* Make sure the thumbnail path is created */
	filename = g_build_filename (g_get_home_dir (),
				     ".fluttr-thumbs",
				     NULL);	
	g_mkdir (filename, 0775);
	g_free (filename);
	
	/* Create a new library */
	fluttr->library = NULL;
	fluttr->view = FLUTTR_VIEW_SETS;
	
	stage = clutter_stage_get_default ();
	fluttr->stage = stage;
	clutter_actor_set_size (stage, 800, 440);
	clutter_stage_set_color (CLUTTER_STAGE (stage), &stage_color);

	/*g_object_set (stage, "fullscreen", TRUE, NULL);*/

	
	if (fluttr->username == NULL) {
		/* Authorise the mini-token */
		g_print ("Authenticating : %s\n", argv[1]);		
		fluttr->auth = fluttr_auth_new (argv[1]);
		g_signal_connect (G_OBJECT (fluttr->auth), "successful",
				  G_CALLBACK (auth_successful), fluttr);
		g_signal_connect (G_OBJECT (fluttr->auth), "error",
				  G_CALLBACK (auth_error), fluttr);
				  
		clutter_actor_set_size (fluttr->auth, 800, 440);
		clutter_actor_set_position (fluttr->auth, 0, 0);
		clutter_group_add (CLUTTER_GROUP (fluttr->stage), fluttr->auth);
	
		g_timeout_add (1500, (GSourceFunc)_auth_timeout, 
			       (gpointer)fluttr);
	}
	
	/* Background */
	background = clutter_texture_new ();
	clutter_actor_set_position (background, 0, 0);
	create_background (background, CLUTTER_STAGE_WIDTH (), 
				       CLUTTER_STAGE_HEIGHT ());	
	clutter_group_add (CLUTTER_GROUP (stage), background);		
	
	/* Set up the list worker */
	list = fluttr_list_new ();
	fluttr->list = list;
	g_object_set (G_OBJECT (list),
		      "username", fluttr->username,
		      "fullname", fluttr->fullname,
		      "token", fluttr->token,
		      "usernsid", fluttr->usernsid,
		      NULL);	
	g_signal_connect (G_OBJECT (list), "successful",
				  G_CALLBACK (list_get_successful), fluttr);
	g_signal_connect (G_OBJECT (list), "error",
				  G_CALLBACK (list_get_error), fluttr);
				  
	clutter_actor_set_size (list, 800, 480);
	clutter_actor_set_position (list, 0, 0);
	clutter_group_add (CLUTTER_GROUP (fluttr->stage), list);
	
	
	/* If we have a username etc, we want to start the list fetcher */
	if (fluttr->username != NULL) {
		/* We update the settings singleton */
		settings = fluttr_settings_get_default ();
		g_object_set (G_OBJECT (settings),
			      "username", fluttr->username,
			      "fullname", fluttr->fullname,
			      "token", fluttr->token,
			      "usernsid", fluttr->usernsid,
			      NULL);
		fluttr_list_go (FLUTTR_LIST (fluttr->list));
	}
	
	/* Sets view */
	ClutterActor *sets = fluttr_set_view_new ();
	fluttr->sets = sets;
	clutter_group_add (CLUTTER_GROUP (fluttr->stage), sets);
	clutter_actor_set_size (sets, 
				CLUTTER_STAGE_WIDTH (),
				CLUTTER_STAGE_HEIGHT());
	clutter_actor_set_position (sets, 0, 0);
	
	/* The list view */
	fluttr->list = fluttr_list_view_new ();
	clutter_group_add (CLUTTER_GROUP (fluttr->stage), fluttr->list);
	clutter_actor_set_size (fluttr->list, CLUTTER_STAGE_WIDTH (),
				CLUTTER_STAGE_HEIGHT ());
	clutter_actor_set_position (fluttr->list, 0, 0);
	clutter_actor_set_opacity (fluttr->list, 0);	
	
	clutter_actor_show_all (fluttr->stage);	    
	
	/* The viewer */
	fluttr->viewer = fluttr_viewer_new ();
	clutter_group_add (CLUTTER_GROUP (fluttr->stage), fluttr->viewer);
	clutter_actor_set_size (fluttr->viewer, CLUTTER_STAGE_WIDTH (),
						CLUTTER_STAGE_HEIGHT ());
	clutter_actor_set_position (fluttr->viewer, 0, 0);
	clutter_actor_set_opacity (fluttr->viewer, 0);	
	
	clutter_actor_show_all (fluttr->stage);	  
	
	/* Setup the view swa behaviour */
	fluttr->in = fluttr->out = NULL;
	fluttr->timeline = clutter_timeline_new (40, 60);
	fluttr->alpha = clutter_alpha_new_full (fluttr->timeline,
					      alpha_linear_inc_func,
					      NULL, NULL);
	fluttr->behave = fluttr_behave_new (fluttr->alpha,
					    _swap_alpha_func,
					    (gpointer)fluttr);	
	
	/* Receive all input events */
	g_signal_connect (stage, 
		          "input-event",
		          G_CALLBACK (browse_input_cb),
		          (gpointer)fluttr);	
	
	clutter_main();	
	return 0;
}

/* Fade out text, change text, then fade in, all within one play of the timeline
   just to keep things interesting :) */
static void
_swap_alpha_func (ClutterBehaviour *behave,
		  guint		    alpha_value,
		  gpointer	    data)
{
        Fluttr	   *fluttr = (Fluttr*)data;
        gfloat factor;
	factor = (gfloat) alpha_value / CLUTTER_ALPHA_MAX_ALPHA;
	ClutterActor *stage = clutter_stage_get_default ();
	
	clutter_actor_set_opacity (CLUTTER_ACTOR (fluttr->in), 255 * factor);
	clutter_actor_set_opacity (CLUTTER_ACTOR (fluttr->out),
				   255- (255*factor));
	
	if (CLUTTER_ACTOR_IS_VISIBLE (CLUTTER_ACTOR(stage)))
		clutter_actor_queue_redraw (CLUTTER_ACTOR(stage));	
}

/* If available, load setting from the users key file */
static gboolean
check_credentials (Fluttr *fluttr)
{
	gchar *path;
	gchar *res = NULL;
	GKeyFile *keyf = NULL;
	
	path = g_build_filename (g_get_home_dir (), ".fluttr", NULL);
	
	if ( (g_file_test (path, G_FILE_TEST_IS_REGULAR | G_FILE_TEST_EXISTS))){
		
		keyf = g_key_file_new ();
		g_key_file_load_from_file (keyf, path, 0, NULL);
		
		/* Try loading the username form the file, if it works, then we
		   already have the credentials, else we set everything to NULL
		*/
		res = g_key_file_get_string (keyf, "fluttr", "username", NULL);
		if (res == NULL) {
			/* Somethings wrong, so just set the varibales to NULL*/
			fluttr->username = NULL;
			fluttr->fullname = NULL;
			fluttr->token = NULL;
			fluttr->usernsid = NULL;
			g_free (path);
			return FALSE;			
		}
		
		fluttr->username = g_strdup (res);
		
		res = NULL;
		res = g_key_file_get_string (keyf, "fluttr", "fullname", NULL);
		fluttr->fullname = g_strdup (res);
		
		res = NULL;
		res = g_key_file_get_string (keyf, "fluttr", "token", NULL);
		fluttr->token = g_strdup (res);
		
		res = NULL;
		res = g_key_file_get_string (keyf, "fluttr", "usernsid", NULL);
		fluttr->usernsid = g_strdup (res);				
		

		g_print ("Loaded Credentials:\n\t%s\n\t%s\n\t%s\n\t%s\n",
			 fluttr->username,
			 fluttr->fullname,
			 fluttr->token,
			 fluttr->usernsid);
		 		
		g_free (path);		
		return TRUE;
	} else {
		/* Doesn't exist, so just set the varibales to NULL */
		fluttr->username = NULL;
		fluttr->fullname = NULL;
		fluttr->token = NULL;
		fluttr->usernsid = NULL;
		g_free (path);		
		return FALSE;
	}
}

/* Authorisation Call backs */
static void		
auth_successful (FluttrAuth *auth, gchar *null, Fluttr *fluttr)
{
	gchar *c;
	GKeyFile *kf = g_key_file_new();
	gchar *file = g_build_filename(g_get_home_dir(), ".fluttr", NULL);
	FluttrSettings *settings;

	/* Load the details */
	g_object_get (G_OBJECT (fluttr->auth), 
                      "username", &fluttr->username, 
                      "fullname", &fluttr->fullname, 
                      "token", &fluttr->token, 
                      "usernsid", &fluttr->usernsid, 
                      NULL);		      

	/* Save the details for next time */
                      
	g_key_file_set_string (kf, "fluttr", "username", fluttr->username);
	g_key_file_set_string (kf, "fluttr", "fullname", fluttr->fullname);
	g_key_file_set_string (kf, "fluttr", "token", fluttr->token);
	g_key_file_set_string (kf, "fluttr", "usernsid", fluttr->usernsid);
	
	c = g_key_file_to_data(kf, NULL, NULL);
	g_key_file_free(kf);

	g_file_set_contents(file, c, -1, NULL);
	g_free(c);
	g_free(file);                      
	
	g_print ("Auth Successful:\n\t%s\n\t%s\n\t%s\n\t%s\n",
		 fluttr->username,
		 fluttr->fullname,
		 fluttr->token,
		 fluttr->usernsid);

	/* Start the list fetcher */
	g_object_set (G_OBJECT (fluttr->list),
		      "username", fluttr->username,
		      "fullname", fluttr->fullname,
		      "token", fluttr->token,
		      "usernsid", fluttr->usernsid,
		      NULL);		

	/* We update the settings singleton */
	settings = fluttr_settings_get_default ();
	g_object_set (G_OBJECT (settings),
		      "username", fluttr->username,
		      "fullname", fluttr->fullname,
		      "token", fluttr->token,
		      "usernsid", fluttr->usernsid,
		      NULL);
	fluttr_list_go (FLUTTR_LIST (fluttr->list));	
}

static void		
auth_error (FluttrAuth *auth, gchar *msg, Fluttr *fluttr)
{
	g_critical ("Auth Unsuccessful : %s\n", msg);
}

/* get list callbacks */

/* Go through the list of sets, and poplulate the Fluttr library with 
   FluttrLibraryRows */
static void		
list_get_successful (FluttrAuth *auth, NFlickWorker *worker, Fluttr *fluttr)
{
	GList *list = NULL;
	GList *l = NULL;
	list = nflick_set_list_worker_take_list ((NFlickSetListWorker*) worker);
	gint i = 0;
	gint j = 0;
	
	g_print ("\n");
	for (l = list; l != NULL; l = l->next) {
		ClutterActor *set = fluttr_set_new (l->data);
		GList *photos = NULL;
		GList *photo;	
		gchar *id = NULL;
		g_object_get (G_OBJECT (l->data), "combotext", &id, NULL);
		g_print ("%s : ", id);
		
		g_object_get (G_OBJECT (l->data), "list", &photos, NULL);
		
		g_print ("%d loaded\n", g_list_length (photos));
		
		for (photo = photos; photo!=NULL ;photo = photo->next) {
			NFlickPhotoData *p = (NFlickPhotoData*)photo->data;
			
			fluttr_set_append_photo (FLUTTR_SET (set),
						 p->Id, 
						 p->Name);
			
			i++;
		}
		fluttr_set_view_add_set (FLUTTR_SET_VIEW (fluttr->sets), 
					 FLUTTR_SET (set));
		j++;
	}
	fluttr_set_view_advance (FLUTTR_SET_VIEW (fluttr->sets), 0);
	
	g_print ("%d Photo(s) in %d set(s)\n", i, j);
}

static void
list_get_error (FluttrAuth *auth, gchar *msg, Fluttr *fluttr)
{
	g_critical ("Auth Unsuccessful : %s\n", msg);
}

static void 
photo_input_cb (ClutterStage *stage,
		 ClutterEvent *event,
		 Fluttr	      *fluttr)
{
	FluttrPhoto *photo = NULL;
	
	
	/* First check for app wide keybinding */
	if (event->type == CLUTTER_KEY_RELEASE)	{
		ClutterKeyEvent* kev = (ClutterKeyEvent *) event;

		switch (clutter_key_event_symbol (kev)) {
		case CLUTTER_Left:
		case CLUTTER_Right:
		case CLUTTER_Up:
		case CLUTTER_Down:
		case CLUTTER_Return:
		case CLUTTER_space:
		case CLUTTER_KP_Enter:
		case CLUTTER_Escape:
			fluttr_list_view_advance_col 
					(FLUTTR_LIST_VIEW (fluttr->list), 0);
			photo = fluttr_list_view_get_active 
					(FLUTTR_LIST_VIEW (fluttr->list));
			fluttr->view = FLUTTR_VIEW_PHOTOS;
			fluttr_viewer_show (FLUTTR_VIEWER (fluttr->viewer),
					    FALSE);
			fluttr_photo_show_options (photo, FALSE);
			
			break;
		default:
			break;
		}
	}
}

static void
_show_viewer (FluttrPhoto *photo, gchar *null, Fluttr *fluttr)
{
	fluttr_viewer_show (FLUTTR_VIEWER (fluttr->viewer), TRUE);
	
	g_signal_handler_disconnect (G_OBJECT (photo), fluttr->sig);
}

static void 
list_input_cb (ClutterStage *stage,
		 ClutterEvent *event,
		 Fluttr	      *fluttr)
{
	FluttrPhoto *photo = NULL;
	
	
	/* First check for app wide keybinding */
	if (event->type == CLUTTER_KEY_RELEASE)	{
		ClutterKeyEvent* kev = (ClutterKeyEvent *) event;

		switch (clutter_key_event_symbol (kev)) {
		case CLUTTER_Left:
			fluttr_list_view_advance_col 
					(FLUTTR_LIST_VIEW (fluttr->list), -1);
			break;
		case CLUTTER_Right:
			fluttr_list_view_advance_col 
					(FLUTTR_LIST_VIEW (fluttr->list), 1);
			break;
		case CLUTTER_Up:
			fluttr_list_view_advance_row 
					(FLUTTR_LIST_VIEW (fluttr->list), -1);
			break;
		case CLUTTER_Down:
			fluttr_list_view_advance_row 
					(FLUTTR_LIST_VIEW (fluttr->list), 1);
			break;
		case CLUTTER_Return:
		case CLUTTER_space:
		case CLUTTER_KP_Enter:
			fluttr_list_view_activate (FLUTTR_LIST_VIEW 
								(fluttr->list));
			photo = fluttr_list_view_get_active 
					(FLUTTR_LIST_VIEW (fluttr->list));
			fluttr_photo_show_options (photo, TRUE);
			fluttr->view = FLUTTR_VIEW_PHOTO;
			
			fluttr_viewer_go (FLUTTR_VIEWER (fluttr->viewer),photo);
			fluttr->sig = g_signal_connect (photo, "activated",
					  G_CALLBACK (_show_viewer), fluttr);
			break;
		case CLUTTER_Escape:
			fluttr->in = fluttr->sets;
			fluttr->out = fluttr->list;
			if (!clutter_timeline_is_playing (fluttr->timeline))
				clutter_timeline_start (fluttr->timeline);
			/*
			clutter_actor_set_opacity (fluttr->list, 0);
			clutter_actor_set_opacity (fluttr->sets, 255);
			*/
			fluttr->view = FLUTTR_VIEW_SETS; 
			break;
		default:
			break;
		}
	}
}

static void 
sets_input_cb (ClutterStage *stage,
		 ClutterEvent *event,
		 Fluttr	      *fluttr)
{
	FluttrSet *set = NULL;
	
	
	/* First check for app wide keybinding */
	if (event->type == CLUTTER_KEY_RELEASE)	{
		ClutterKeyEvent* kev = (ClutterKeyEvent *) event;

		switch (clutter_key_event_symbol (kev)) {
		case CLUTTER_Left:
			fluttr_set_view_advance_col 
					(FLUTTR_SET_VIEW (fluttr->sets), -1);
			break;
		case CLUTTER_Right:
			fluttr_set_view_advance_col 
					(FLUTTR_SET_VIEW (fluttr->sets), 1);
			break;
		case CLUTTER_Up:
			fluttr_set_view_advance_row 
					(FLUTTR_SET_VIEW (fluttr->sets), -1);
			break;
		case CLUTTER_Down:
			fluttr_set_view_advance_row 
					(FLUTTR_SET_VIEW (fluttr->sets), 1);
			break;
		case CLUTTER_Return:
		case CLUTTER_space:
		case CLUTTER_KP_Enter:
			fluttr_set_view_activate (FLUTTR_SET_VIEW 
								(fluttr->sets));
			set = fluttr_set_view_get_active (FLUTTR_SET_VIEW 
								(fluttr->sets));
			if (set) {
				g_object_set (G_OBJECT (fluttr->list),
					      "set", set, NULL);
				fluttr->in = fluttr->list;
				fluttr->out = fluttr->sets;
				if (!clutter_timeline_is_playing (
							fluttr->timeline))
					clutter_timeline_start (
							fluttr->timeline);
				fluttr->view = FLUTTR_VIEW_PHOTOS; 
				fluttr_list_view_advance 
					(FLUTTR_LIST_VIEW (fluttr->list), 0);
			}
			break;
		case CLUTTER_Escape:
			clutter_main_quit();
			break;
		default:
			break;
		}
	}
}


static void 
browse_input_cb (ClutterStage *stage,
		 ClutterEvent *event,
		 Fluttr	      *fluttr)
{
	/* First check for app wide keybinding */
	if (event->type == CLUTTER_KEY_RELEASE)	{
		ClutterKeyEvent* kev = (ClutterKeyEvent *) event;

		switch (clutter_key_event_symbol (kev)) {
		
		case CLUTTER_Escape:
			if (fluttr->view == FLUTTR_VIEW_SETS)
				clutter_main_quit();
			break;
		default:
			break;
		}
	}
	/* if we have got here, we can pass the input onto the right place */
	if (fluttr->view == FLUTTR_VIEW_SETS)
		sets_input_cb (stage, event, fluttr);
	else if (fluttr->view == FLUTTR_VIEW_PHOTOS)
		list_input_cb (stage, event, fluttr);
	else
		photo_input_cb (stage, event, fluttr);
}

static void 
create_background (ClutterActor *bg, guint width, guint height)
{
  	GdkPixbuf *pixbuf = NULL;
  	
  	pixbuf = gdk_pixbuf_new_from_file_at_scale (PKGDATADIR \
  						    	"/background.svg",
  						    width,
  						    height,
  						    FALSE,
  						    NULL);
	if (pixbuf)
		clutter_texture_set_pixbuf (CLUTTER_TEXTURE (bg), pixbuf);
	else
		g_print ("Could not load pixbuf\n");		
}

