/*  Cronos II - The GNOME mail client
 *  Copyright (C) 2000-2001 Pablo Fern�ndez Navarro
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
#include <libcronosII/error.h>

#include "widget-window.h"

static void
class_init									(C2WindowClass *klass);

static void
init										(C2Window *window);

static void
destroy										(GtkObject *object);

static gint
on_report_timeout							(gpointer data);

static gint
on_activity_timeout							(gpointer data);

enum
{
	LAST_SIGNAL
};

static guint signals[LAST_SIGNAL] = { 0 };

static GnomeAppClass *parent_class = NULL;

GtkType
c2_window_get_type (void)
{
	static GtkType type = 0;

	if (!type)
	{
		static GtkTypeInfo info =
		{
			"C2Window",
			sizeof (C2Window),
			sizeof (C2WindowClass),
			(GtkClassInitFunc) class_init,
			(GtkObjectInitFunc) init,
			(GtkArgSetFunc) NULL,
			(GtkArgGetFunc) NULL
		};

		type = gtk_type_unique (gnome_app_get_type (), &info);
	}

	return type;
}

static void
class_init (C2WindowClass *klass)
{
	GtkObjectClass *object_class = (GtkObjectClass *) klass;
	
	parent_class = gtk_type_class (gnome_app_get_type ());

	gtk_object_class_add_signals (object_class, signals, LAST_SIGNAL);

	object_class->destroy = destroy;
}

static void
init (C2Window *window)
{
	window->application = NULL;
	window->xml = NULL;
	pthread_mutex_init (&window->status_lock, NULL);
	pthread_mutex_init (&window->progress_lock, NULL);
}

static void
destroy (GtkObject *object)
{
	C2Window *window;

	window = C2_WINDOW (object);
#ifdef USE_DEBUG
	g_print ("Destroing window %s\n", window->type);
#endif

	c2_application_window_remove (window->application, GTK_WINDOW (window));

	g_free (window->type);

	pthread_mutex_destroy (&window->status_lock);
	pthread_mutex_destroy (&window->progress_lock);

	if (window->xml)
		gtk_object_unref (GTK_OBJECT (window->xml));
}

GtkWidget *
c2_window_new (C2Application *application, const gchar *title, const gchar *type)
{
	C2Window *window;

	window = gtk_type_new (c2_window_get_type ());

#ifdef USE_DEBUG
	g_print ("Creating new C2Window ('%s').\n", type);
#endif

	c2_window_construct (window, application, title, type);

	return GTK_WIDGET (window);
}

void
c2_window_construct (C2Window *window, C2Application *application, const gchar *title, const gchar *type)
{
	window->application = application;
	window->type = g_strdup (type);

	gnome_app_construct (GNOME_APP (window), application->name, title ? title : "Cronos II");

	c2_application_window_add (application, GTK_WINDOW (window));
}

void
c2_window_set_contents (C2Window *window, GtkWidget *widget)
{
	gnome_app_set_contents (GNOME_APP (window), widget);
}

void
c2_window_set_contents_from_glade (C2Window *window, const gchar *dscp)
{
	GtkWidget *widget = glade_xml_get_widget (window->xml, dscp);

	if (widget)
		c2_window_set_contents (window, widget);
}

void
c2_window_report (C2Window *window, C2WindowReportType type, const gchar *fmt, ...)
{
	va_list args;
	gchar *msg, *rmsg;

	va_start (args, fmt);
	msg = g_strdup_vprintf (fmt, args);
	va_end (args);

	if (!pthread_mutex_trylock (&window->status_lock))
	{
		GtkWidget *appbar = glade_xml_get_widget (window->xml, "appbar");

		if (!appbar)
		{
			g_free (msg);
			return;
		}

		if (c2_errno && (type != C2_WINDOW_REPORT_MESSAGE))
			rmsg = g_strdup_printf ("%s: %s\n", msg, c2_error_get (c2_errno));
		else
			rmsg = g_strdup (msg);

		g_free (msg);

		gnome_appbar_push (GNOME_APPBAR (appbar), rmsg);
		gtk_timeout_add (10000, on_report_timeout, window);
		g_free (rmsg);
		pthread_mutex_unlock (&window->status_lock);
	}

	g_free (msg);
}

static gint
on_report_timeout (gpointer data)
{
	C2Window *window = C2_WINDOW (data);

	if (!pthread_mutex_trylock (&window->status_lock))
	{
		GtkWidget *appbar = glade_xml_get_widget (window->xml, "appbar");

		gnome_appbar_pop (GNOME_APPBAR (appbar));
		pthread_mutex_unlock (&window->status_lock);
	}
	
	return FALSE;
}

void
c2_window_set_activity (C2Window *window, gboolean state)
{
	if (state)
	{
		if (!pthread_mutex_trylock (&window->progress_lock))
		{
			GtkWidget *appbar = glade_xml_get_widget (window->xml, "appbar");

			if (!appbar)
			{
				pthread_mutex_unlock (&window->progress_lock);
				return;
			}

			gtk_progress_set_activity_mode (gnome_appbar_get_progress (GNOME_APPBAR (appbar)), TRUE);
			gtk_timeout_add (10, on_activity_timeout, window);
		}
	} else
	{
		GtkWidget *appbar = glade_xml_get_widget (window->xml, "appbar");

		gtk_progress_set_activity_mode (gnome_appbar_get_progress (GNOME_APPBAR (appbar)), FALSE);
		gtk_progress_set_value (gnome_appbar_get_progress (GNOME_APPBAR (appbar)), 0);
		pthread_mutex_unlock (&window->progress_lock);
	}
}

static gint
on_activity_timeout (gpointer data)
{
	C2Window *window = C2_WINDOW (data);

	if (!pthread_mutex_trylock (&window->progress_lock))
	{
		pthread_mutex_unlock (&window->progress_lock);
	} else
	{
		GtkWidget *appbar = glade_xml_get_widget (window->xml, "appbar");
		static gboolean adding = TRUE;
		gfloat value = gtk_progress_get_value (gnome_appbar_get_progress (GNOME_APPBAR (appbar)));
	
		if (adding)
			value += 0.05;
		else
			value -= 0.05;
		
		if (adding && value > 1)
		{
			value -= 0.05;
			adding = !adding;
		} else if (!adding && value < 0)
		{
			value += 0.05;
			adding = !adding;
		}
		gtk_progress_set_value (gnome_appbar_get_progress (GNOME_APPBAR (appbar)), value);
		return TRUE;
	}
	
	return FALSE;
}