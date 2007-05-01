/*
 * Copyright (C) 2007 Neil J. Patel
 * Copyright (C) 2007 OpenedHand Ltd
 *
 * Author: Neil J. Patel  <njp@o-hand.com>
 */

#include <config.h>
#include <glib.h>
#include <clutter/clutter.h>

#include "fluttr-auth.h"
#include "fluttr-list.h"

#include "nflick-photo-data.h"
#include "nflick-photo-set.h"
#include "nflick-set-list-worker.h"
#include "nflick-worker.h"

typedef struct {
	ClutterActor 		*stage;
	ClutterActor 		*auth;
	ClutterActor		*list;
	
	/* Flickr info */
	gchar 			*username;
	gchar			*fullname;
	gchar 			*token;
	gchar 			*usernsid;

} Fluttr;

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
 	ClutterActor *stage, *list;
	ClutterColor stage_color = { 0x00, 0x00, 0x00, 0xff };
	
	g_thread_init (NULL);
	clutter_init (&argc, &argv);		
	
	/* Check that there are enough arguments */
	if (argc < 2 && !(check_credentials (fluttr))) {
		g_print ("\n\nYou need to start Fluttr with your Flickr "\
			  "authorisation code, which is available here:\n"\
			  "http://www.flickr.com/auth-72157600141007022\n\n");
		return 0;
	}
	
	stage = clutter_stage_get_default ();
	fluttr->stage = stage;
	clutter_actor_set_size (stage, 800, 440);
	clutter_stage_set_color (CLUTTER_STAGE (stage), &stage_color);
	
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
	
		g_timeout_add (1000, (GSourceFunc)_auth_timeout, 
			       (gpointer)fluttr);
	}
	
	
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
		fluttr_list_go (FLUTTR_LIST (fluttr->list));
	}

	clutter_actor_show_all (fluttr->stage);	    
	
	clutter_main();	
	return 0;
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
	fluttr_list_go (FLUTTR_LIST (fluttr->list));	
}

static void		
auth_error (FluttrAuth *auth, gchar *msg, Fluttr *fluttr)
{
	g_critical ("Auth Unsuccessful : %s\n", msg);
}

/* get list callbacks */

static void		
list_get_successful (FluttrAuth *auth, NFlickWorker *worker, Fluttr *fluttr)
{
	GList *list = NULL;
	GList *l = NULL;
	list = nflick_set_list_worker_take_list ((NFlickSetListWorker*) worker);
	gint i = 0;
	gint j = 0;
	
	for (l = list; l != NULL; l = l->next) {
		gchar *id = NULL;
		g_object_get (G_OBJECT (l->data), "combotext", &id, NULL);
		g_print ("%s\n", id);	
	
		GList *photos = NULL;
		GList *photo;
		g_object_get (G_OBJECT (l->data), "list", &photos, NULL);
		g_print ("%d\n", g_list_length (photos));
		for (photo = photos; photo!=NULL ;photo = photo->next) {
			NFlickPhotoData *p = (NFlickPhotoData*)photo->data;
			if (p->Id)
				g_print ("\t%s ", p->Id);
			if (p->Name)
				g_print (p->Name);
			g_print ("\n");
			i++;
		}
		j++;
	}
	g_print ("%d Photo(s)\n in %d set(s)", i, j);
}

static void
list_get_error (FluttrAuth *auth, gchar *msg, Fluttr *fluttr)
{
	g_critical ("Auth Unsuccessful : %s\n", msg);
}

