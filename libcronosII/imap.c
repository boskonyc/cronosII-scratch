/*  Cronos II - The GNOME mail client
 *  Copyright (C) 2000-2001 Pablo Fernández Navarro
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.
 */
#include <stdio.h>
#include <glib.h>
#include "utils.h"
#include "imap.h"
#include "i18n.h"

#define NET_READ_FAILED  _("Internal socket read operation failed, connection is most likely broken")
#define NET_WRITE_FAILED _("Internal socket write operation failed, connection is most likely broken")

/* C2 IMAP Module in the process of being engineered by Pablo and Bosko =) */
/* TODO: Implement a hash table in IMAP object for handing server replies */
/* (in progress) TODO: Function for reading server replies */
/* TODO: Login (at least plain-text for now) */
/* TODO: Get list of folders */
/* TODO: Get list of messages */
/* TODO: Get and delete messages */
/* TODO: Create, rename, and remove folders */
/* TODO: Create a test module */

/* Private GtkObject functions */
static void
class_init									(C2IMAPClass *klass);

static void
init										(C2IMAP *imap);

static void
destroy										(GtkObject *object);

/* Private IMAP functions */
static void
c2_imap_on_net_traffic (gpointer *data, gint source, GdkInputCondition condition);

static gint
c2_imap_plaintext_login (C2IMAP *imap);

/* Misc. functions */
static tag_t
c2_imap_get_tag								(C2IMAP *imap);

static void
c2_imap_set_error(C2IMAP *imap, const gchar *error);

enum
{
	LOGIN,
	LOGIN_FAILED,
	MAILBOX_LIST,
	INCOMING_MAIL,
	LOGOUT,
	NET_ERROR,
	LAST_SIGNAL
};

static guint signals[LAST_SIGNAL] = { 0 };

static C2NetObjectClass *parent_class = NULL;

GtkType
c2_imap_get_type (void)
{
	static GtkType type = 0;

	if (!type)
	{
		static GtkTypeInfo info =
		{
			"C2IMAP",
			sizeof (C2IMAP),
			sizeof (C2IMAPClass),
			(GtkClassInitFunc) class_init,
			(GtkObjectInitFunc) init,
			(GtkArgSetFunc) NULL,
			(GtkArgGetFunc) NULL
		};

		type = gtk_type_unique (c2_net_object_get_type (), &info);
	}

	return type;
}

static void
class_init (C2IMAPClass *klass)
{
	GtkObjectClass *object_class = (GtkObjectClass*) klass;

	signals[LOGIN] =
		gtk_signal_new ("login",
						GTK_RUN_FIRST,
						object_class->type,
						GTK_SIGNAL_OFFSET (C2IMAPClass, login),
						gtk_marshal_NONE__NONE, GTK_TYPE_NONE, 0);
	signals[MAILBOX_LIST] =
		gtk_signal_new ("mailbox_list",
						GTK_RUN_FIRST,
						object_class->type,
						GTK_SIGNAL_OFFSET (C2IMAPClass, mailbox_list),
						gtk_marshal_NONE__POINTER, GTK_TYPE_NONE, 1,
						GTK_TYPE_POINTER);
	signals[INCOMING_MAIL] =
		gtk_signal_new ("incoming_mail",
						GTK_RUN_FIRST,
						object_class->type,
						GTK_SIGNAL_OFFSET (C2IMAPClass, incoming_mail),
						gtk_marshal_NONE__NONE, GTK_TYPE_NONE, 0);
	signals[LOGOUT] =
		gtk_signal_new ("logout",
						GTK_RUN_FIRST,
						object_class->type,
						GTK_SIGNAL_OFFSET (C2IMAPClass, logout),
						gtk_marshal_NONE__NONE, GTK_TYPE_NONE, 0);
	signals[NET_ERROR] =
		gtk_signal_new("net_error",
						GTK_RUN_FIRST,
						object_class->type,
						GTK_SIGNAL_OFFSET (C2IMAPClass, net_error),
						gtk_marshal_NONE__NONE, GTK_TYPE_NONE, 0);
	
	gtk_object_class_add_signals (object_class, signals, LAST_SIGNAL);

	klass->login = NULL;
	klass->login_failed = NULL;
	klass->mailbox_list = NULL;
	klass->incoming_mail = NULL;
	klass->logout = NULL;
}

static void
init (C2IMAP *imap)
{
	imap->auth = 0;
	imap->cmnd = 0;
	imap->mailboxes = NULL;
	imap->selected_mailbox = NULL;
	imap->host = NULL;
	imap->user = NULL;
	imap->pass = NULL;
	imap->hash = NULL;
	imap->login = NULL;
	
	pthread_mutex_init(&imap->lock, NULL);
}

C2IMAP *
c2_imap_new (gchar *host, gint port, gchar *user, gchar *pass, 
		C2IMAPAuthenticationType auth, gboolean ssl)
{
	C2IMAP *imap;

	imap = gtk_type_new (c2_imap_get_type ());
	imap->user = g_strdup(user);
	imap->pass = g_strdup(pass);
	imap->host = g_strdup(host);
	imap->hash = g_hash_table_new(g_str_hash, g_int_equal);
	
	switch(auth)
	{
		case C2_IMAP_AUTHENTICATION_PLAINTEXT:
			imap->login = c2_imap_plaintext_login;
			break;
	}

	c2_net_object_construct (C2_NET_OBJECT (imap), host, port, ssl);

	gdk_input_add(C2_NET_OBJECT(imap)->sock, GDK_INPUT_READ,
								(GdkInputFunction)c2_imap_on_net_traffic, imap);
	
	return imap;
}

/**
 * c2_imap_init
 * @imap: IMAP object.
 *
 * This function will start an IMAP object,
 * it will connect the object, make it login, etc.
 **/
void
c2_imap_init (C2IMAP *imap)
{
	pthread_mutex_lock(&imap->lock);
	
	if(c2_net_object_run(C2_NET_OBJECT(imap)) < 0)
	{
		gtk_signal_emit(GTK_OBJECT(imap), NET_ERROR);
		c2_imap_set_error(imap, _("Error connecting to IMAP server."));
		pthread_mutex_unlock(&imap->lock);
		return;
	}
	if(imap->login(imap) < 0)
	{
		gtk_signal_emit(GTK_OBJECT(imap), LOGIN_FAILED);
		c2_imap_set_error(imap, _("Failed to login to IMAP server."));
		pthread_mutex_unlock(&imap->lock);
		return;
	}
	
	g_print ("%s (%s@%s)\n", __PRETTY_FUNCTION__, imap->user, C2_NET_OBJECT (imap)->host);
}

static void
destroy(GtkObject *object)
{
	g_free(C2_IMAP(object)->user);
	g_free(C2_IMAP(object)->pass);
	pthread_mutex_destroy(&C2_IMAP(object)->lock);
}

/* function that gets fired off every time there is incoming net data */
static void
c2_imap_on_net_traffic (gpointer *data, gint source, GdkInputCondition condition)
{
	C2IMAP *imap = C2_IMAP(data);
	gchar *buf, *buf2, *ptr = NULL;
	gchar *final = NULL;
	tag_t tag = 0;
	
	pthread_mutex_lock(&imap->lock);
	
	for(;;)
	{
		if(c2_net_object_read(C2_NET_OBJECT(imap), &buf) < 0)
		{
			printf(_("Error reading from socket on IMAP host %s! Reader thread aborting!\n"),
						imap->host);
			c2_imap_set_error(imap, NET_READ_FAILED);
			c2_net_object_disconnect(C2_NET_OBJECT(imap));
			gtk_signal_emit(GTK_OBJECT(imap), NET_ERROR);
			return; 
		}
		if(!final) final = g_strdup(buf);
		else 
		{
			buf2 = final;
			final = g_strconcat(final, buf, NULL);
			g_free(buf2);
		}
		
		/* The IMAP server returned our tag, end of response */
		if(c2_strneq(buf, "CronosII-", 9)) 
			break;
		g_free(buf);
	}
	
	tag = atoi(buf+9);
	g_free(buf);
	/* now insert 'final' into the hash...*/
	printf("Now inserting data: %s in hash table for tag #%i\n", final, tag);
	g_hash_table_insert(imap->hash, &tag, final);
	
	pthread_mutex_unlock(&imap->lock);
}

static tag_t
c2_imap_get_tag (C2IMAP *imap)
{
	tag_t tag;

	/* No, there's no error here, cmnd is an unsigned int of 10 bits,
	 * thus, the greatest number is 1023 and the lowest 0...
	 */
	
	tag = imap->cmnd++;

	return tag;
}

static void
c2_imap_set_error(C2IMAP *imap, const gchar *error)
{
	GtkObject *object = GTK_OBJECT(imap);
	gchar *buf;
	
	if((buf = gtk_object_get_data(object, "error")))
		g_free(buf);

	buf = g_strdup(error);
	gtk_object_set_data(object, "error", buf);
}

static gint
c2_imap_plaintext_login (C2IMAP *imap)
{	
	tag_t tag;
	
	tag = c2_imap_get_tag (imap);

	if(c2_net_object_send(C2_NET_OBJECT(imap), "CronosII-%04d LOGIN %s %s\r\n", 
												tag, imap->user, imap->pass) < 0)
	{
		/* Bosko, are you sure you want to increment the tag again here? -pablo */
		c2_imap_get_tag (imap);
		pthread_mutex_unlock(&imap->lock);
		return -1;
	}
	
	/* be careful to unlock the mutex before waiting on replies */
	pthread_mutex_unlock(&imap->lock);
	
	/* here we should block or wait until our local imap
	 * hash has a reply with the 'tag' tag. */
	
	return 0;
}
