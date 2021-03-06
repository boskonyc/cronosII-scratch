/*  Cronos II - The GNOME mail client
 *  Copyright (C) 2000-2001 Pablo Fern�ndez
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
#include <config.h>
#include <gnome.h>
#include <signal.h>

#ifdef USE_GCONF
#	include <gconf/gconf.h>
#endif

#include <libcronosII/mailbox.h>
#include <libcronosII/error.h>
#include <libcronosII/hash.h>
#include <libcronosII/utils.h>
#include <libcronosII/utils-str.h>

#include "main.h"
#include "preferences.h"
#include "widget-application-utils.h"
#include "widget-composer.h"
#include "widget-window.h"
#include "widget-window-main.h"

static gint
on_main_idle								(gpointer);

#ifdef USE_DEBUG
static gint
on_intensive_test_timeout					(gpointer data);
#endif

C2Application *
global_application = NULL;

gchar *error_list[] =
{
	N_("Failed to load message: %s."),
	N_("Failed to save message: %s."),
	N_("Failed to load mailbox �%s�: %s."),
	N_("Failed to create mailbox �%s�: %s."),
	N_("Failed to save file: %s."),

	N_("Message saved successfully."),
	N_("File saved successfully."),

	N_("Action cancelled by user."),
	N_("There is no selected mailbox."),
	N_("Unknown reason")
};

static struct {
	gboolean open_composer;
	
	gchar *account;
	gchar *to;
	gchar *cc;
	gchar *bcc;
	gchar *subject;
	gchar *body;

	gchar *mailto;

	gchar *mailbox;

	gboolean check;
	gchar *open_file;
	gboolean open_main_window;
	gboolean be_server;
	
	gboolean hide_wmain;
	gboolean raise_wmain;

	gboolean exit;

#ifdef USE_DEBUG
	gchar *intensive_test;
	gboolean intensive_test_list;
	gchar *debug_modules;
	gboolean debug_modules_list;
#endif
} flags =
{
	FALSE,
	
	NULL,
	NULL,
	NULL,
	NULL,
	NULL,
	NULL,
	
	NULL,

	NULL,

	FALSE,
	NULL,
	FALSE,
	FALSE,
	FALSE,
	FALSE,
	FALSE

#ifdef USE_DEBUG
	,NULL,
	FALSE,
	NULL,
	FALSE
#endif
};

void on_sigsegv (int signal)
{
	GtkWidget *dialog;

	system ("date");

#ifndef USE_DEBUG /* XXX REMOVE THIS!!! */
	dialog = gnome_error_dialog (_("An internal error has crashed Cronos II!\n"
 							       "\n"
								   "Your unsaved data is going to be saved\n"
								   "so you do not lose any information.\n"
								   "\n"
								   "We apologise for this inconvenient.\n"));
	gnome_dialog_run_and_close (dialog);

	gtk_signal_emit_by_name (GTK_OBJECT (global_application), "emergency_data_save");
#endif
	c2_preferences_set_application_crashed (TRUE);
	abort ();
}

static void
c2_init (gint argc, gchar **argv)
{
	static struct poptOption options[] = {
		{
			"compose", 'c', POPT_ARG_NONE,
			&(flags.open_composer), 0,
			N_("Compose a new email."), NULL
		},
		{
			"account", 'a', POPT_ARG_STRING,
			&(flags.account), 0,
			N_("Set the Account field."),
			N_("Account")
		},
		{
			"to", 't', POPT_ARG_STRING,
			&(flags.to), 0,
			N_("Set the To field."),
			N_("Address")
		},
		{
			"cc", 0, POPT_ARG_STRING,
			&(flags.cc), 0,
			N_("Set the CC field."),
			N_("Address")
		},
		{
			"bcc", 0, POPT_ARG_STRING,
			&(flags.bcc), 0,
			N_("Set the BCC field."),
			N_("Address")
		},
		{
			"subject", 'u', POPT_ARG_STRING,
			&(flags.subject), 0,
			N_("Set the Subject field."),
			N_("Subject")
		},
		{
			"body", 'b', POPT_ARG_STRING,
			&(flags.body), 0,
			N_("Set the Body."),
			N_("Text")
		},
		{
			"link", 'l', POPT_ARG_STRING,
			&(flags.mailto), 0,
			N_("Compose a new email decoding the argument as a mailto: link"),
			N_("mailto:email@somewhere.")
		},
		{
			"mailbox", 'm', POPT_ARG_STRING,
			&(flags.mailbox), 0,
			N_("Set the active mailbox at start (default=Inbox)"),
			N_("Inbox")
		},
		{
			"check", 'k', POPT_ARG_NONE,
			&(flags.check), 0,
			N_("Check account for mail."), NULL
		},
		{
			"file", 'f', POPT_ARG_STRING,
			&(flags.open_file), 0,
			N_("Open the specified file with the Mail Viewer"), NULL
		},
		{
			"wmain", 0, POPT_ARG_NONE,
			&(flags.open_main_window), 0,
			N_("Open the main window (default)"), NULL
		},
		{
			"server", 's', POPT_ARG_NONE,
			&(flags.be_server), 0,
			N_("Don't open any windows; instead act as a server for quick startup of new Cronos II instances"),
			NULL
		},
		{
			"hide_wmain", 0, POPT_ARG_NONE,
			&(flags.hide_wmain), 0,
			N_("Hides the main window."), NULL
		},
		{
			"raise_wmain", 0, POPT_ARG_NONE,
			&(flags.raise_wmain), 0,
			N_("Shows the main window."), NULL
		},
		{
			"exit", 0, POPT_ARG_NONE,
			&(flags.exit), 0,
			N_("Finishes Cronos II and its multisession server"),
			NULL
		}
#ifdef USE_DEBUG
		, {
			"intensive-test", 0, POPT_ARG_STRING,
			&(flags.intensive_test), 0,
			N_("Runs an intensive test"),
			NULL
		},
		{
			"debug-modules-list", 0, POPT_ARG_NONE,
			&(flags.debug_modules_list), 0,
			N_("Shows the list of modules that can be debugged"),
			NULL
		},
		{
			"debug-modules", 0, POPT_ARG_STRING,
			&(flags.debug_modules), 0,
			N_("Separated list of commas with the names of the modules you want to debug. See --debug-module-list for a list"),
			NULL
		}
#endif
	};

	g_thread_init (NULL);
	
	/* Language bindings */
	gtk_set_locale ();
#ifdef ENABLE_NLS
#ifdef HAVE_SETLOCALE_H
	setlocale (LC_ALL, "");
#endif
	bindtextdomain (PACKAGE, GNOMELOCALEDIR);
	textdomain (PACKAGE);
#endif
	
	gnome_init_with_popt_table ("Cronos II", VERSION, argc, argv, options, 0, NULL);
	
	glade_gnome_init ();
	c2_hash_init ();
	
	signal (SIGSEGV, on_sigsegv);
}

#define CREATE_WINDOW_MAIN \
	{ \
		c2_application_command (application, C2_COMMAND_WINDOW_MAIN_NEW, flags.mailbox); \
		something_opened = TRUE; \
	}

gint
main (gint argc, gchar **argv)
{
	gboolean something_opened = FALSE;
	gchar *version;
	gchar *str;
	gint i;

#ifdef USE_DEBUG
	/* Redirect the error output */
	setbuf (stdout, NULL);

	/* Default to NO ALL the debugging symbols */
	_debug_db			= _debug_db_cronosII		= _debug_db_imap		= _debug_db_spool =
	_debug_imap			= _debug_mailbox		= _debug_message		= _debug_mime =
	_debug_net_object		= _debug_pop3			= _debug_request		= _debug_smtp =
	_debug_widget_application	= _debug_widget_composer	= _debug_widget_index		= _debug_widget_mailbox_list =
	_debug_widget_mail		= _debug_widget_part		= _debug_widget_transfer_item	= 0;
#endif

	/* Initialization of GNOME and Glade */
	c2_init (argc, argv);

#ifdef USE_DEBUG
	if (flags.intensive_test_list)
	{
		printf ("\nAvailable intensive tests are:\n"
				"%18s\t\t%s.\n"
				"\n",
				"mailbox", "Makes a hard tests creating, manipulating and deleting mailboxes");
		exit (0);
	}

	if (flags.debug_modules_list)
	{
		printf (_("If you found some problem with Cronos II when doing some specific thing you can debug an specific portion "
			"of the program and send the output to cronosII-hackers@lists.sf.net so we can fix the problem for you.\n"
			"\n"
			"Recommendation: Debugging produces a lot of output, we recommend you to redirect the output that Cronos II "
			"generates by adding >cronosII.out 2>&1 to the end of the command.\n"
			"\n"
			"This is a list of the available debug-modules symbols:\n"
			"\n"
			"db\n"
			"db-cronosII\n"
			"db-imap\n"
			"db-spool\n"
			"imap\n"
			"mailbox\n"
			"message\n"
			"mime\n"
			"net-object\n"
			"pop3\n"
			"request\n"
			"smtp\n"
			"widget-application\n"
			"widget-composer\n"
			"widget-mailbox-list\n"
			"widget-mail\n"
			"widget-part\n"
			"widget-transfer-item\n"));
		exit (0);
	}

	if (flags.debug_modules)
	{
		for (i = 0;; i++)
		{
			str = c2_str_get_word (i, flags.debug_modules, ',');
			if (!str || !strlen (str))
				break;

			if (c2_streq (str, "widget-application"))
				_debug_widget_application = TRUE;
			else if (c2_streq (str, "widget-composer"))
				_debug_widget_composer = TRUE;
			else if (c2_streq (str, "widget-index"))
				_debug_widget_index = TRUE;
			else if (c2_streq (str, "widget-mailbox-list"))
				_debug_widget_mailbox_list = TRUE;
			else if (c2_streq (str, "widget-part"))
				_debug_widget_part = TRUE;
			else if (c2_streq (str, "widget-transfer-item"))
				_debug_widget_transfer_item = TRUE;
			else if (c2_streq (str, "db"))
				_debug_db = TRUE;
			else if (c2_streq (str, "db-cronosII"))
				_debug_db_cronosII = TRUE;
			else if (c2_streq (str, "db-imap"))
				_debug_db_imap = TRUE;
			else if (c2_streq (str, "db-spool"))
				_debug_db_spool = TRUE;
			else if (c2_streq (str, "imap"))
				_debug_imap = TRUE;
			else if (c2_streq (str, "mailbox"))
				_debug_mailbox = TRUE;
			else if (c2_streq (str, "message"))
				_debug_message = TRUE;
			else if (c2_streq (str, "mime"))
				_debug_mime = TRUE;
			else if (c2_streq (str, "net-object"))
				_debug_net_object = TRUE;
			else if (c2_streq (str, "pop3"))
				_debug_pop3 = TRUE;
			else if (c2_streq (str, "request"))
				_debug_request = TRUE;
			else if (c2_streq (str, "smtp"))
				_debug_smtp = TRUE;
			else
				fprintf (stderr, _("Unknown debugging symbol: %s\n"), str);
			
			g_free (str);
		}
	}
#endif

	gdk_threads_enter ();

	c2_font_bold	= _("-adobe-helvetica-bold-r-normal-*-*-120-*-*-p-*-*-1"); /* Translators: Replace the last * with the iso code of the language you are translating (i.e. spanish: "iso8859-1") */
	c2_font_italic	= _("-adobe-helvetica-medium-o-normal-*-*-120-*-*-p-*-*-1");

	/* Get the Version of the Application */
	version = c2_preferences_get_application_version ();
	if (!version)
	{
		c2_install_new ();
		gtk_main ();
	}

	/* Create the Application object */
	if (!(application = c2_application_new (PACKAGE, flags.be_server)))			
		return 1;
	global_application = application;

	if (flags.be_server)
	{
		something_opened = TRUE;
	} else if (c2_preferences_get_advanced_security_password_ask ())
	{
		if (!c2_application_dialog_password (application))
			return 1;
	}
	
	/* Open specified windows */
	if (flags.open_main_window)
	{
		CREATE_WINDOW_MAIN;
	}
	
	/* Composer */
	if (flags.open_composer || flags.account || flags.to || flags.cc ||
		flags.bcc || flags.subject || flags.body || flags.mailto)
	{
		gchar *headers = NULL;
		gchar *values = NULL;
		gchar *buf = NULL;
		gboolean interpret_as_link = FALSE;
		
		if (flags.mailto)
		{
			interpret_as_link = TRUE;
			headers = flags.mailto;
		} else
		{
			if (flags.account)
			{
				if (headers)
				{
					buf = g_strdup_printf ("%s\r%s", headers, C2_COMPOSER_ACCOUNT);
					g_free (headers);
				} else
					buf = g_strdup (C2_COMPOSER_ACCOUNT);
				headers = buf;
				
				if (values)
				{
					buf = g_strdup_printf ("%s\r%s", values, flags.account);
					g_free (values);
				} else
					buf = g_strdup (flags.account);
				values = buf;
			}
					
			if (flags.to)
			{
				if (headers)
				{
					buf = g_strdup_printf ("%s\r%s", headers, C2_COMPOSER_TO);
					g_free (headers);
				} else
					buf = g_strdup (C2_COMPOSER_TO);
				headers = buf;
				
				if (values)
				{
					buf = g_strdup_printf ("%s\r%s", values, flags.to);
					g_free (values);
				} else
					buf = g_strdup (flags.to);
				values = buf;
			}
			
			if (flags.cc)
			{
				if (headers)
				{
					buf = g_strdup_printf ("%s\r%s", headers, C2_COMPOSER_CC);
					g_free (headers);
				} else
					buf = g_strdup (C2_COMPOSER_CC);
				headers = buf;
				
				if (values)
				{
					buf = g_strdup_printf ("%s\r%s", values, flags.cc);
					g_free (values);
				} else
					buf = g_strdup (flags.cc);
				values = buf;
				
			}
			
			if (flags.bcc)
			{
				if (headers)
				{
					buf = g_strdup_printf ("%s\r%s", headers, C2_COMPOSER_BCC);
					g_free (headers);
				} else
					buf = g_strdup (C2_COMPOSER_BCC);
				headers = buf;
				
				if (values)
				{
					buf = g_strdup_printf ("%s\r%s", values, flags.bcc);
					g_free (values);
				} else
					buf = g_strdup (flags.bcc);
				values = buf;
			}
			
			if (flags.subject)
			{
				if (headers)
				{
					buf = g_strdup_printf ("%s\r%s", headers, C2_COMPOSER_SUBJECT);
					g_free (headers);
				} else
					buf = g_strdup (C2_COMPOSER_SUBJECT);
				headers = buf;
				
				if (values)
				{
					buf = g_strdup_printf ("%s\r%s", values, flags.subject);
					g_free (values);
				} else
					buf = g_strdup (flags.subject);
				values = buf;
			}
			
			if (flags.body)
			{
				if (headers)
				{
					buf = g_strdup_printf ("%s\r%s", headers, C2_COMPOSER_BODY);
					g_free (headers);
				} else
					buf = g_strdup (C2_COMPOSER_BODY);
				headers = buf;
				
				if (values)
				{
					buf = g_strdup_printf ("%s\r%s", values, flags.body);
					g_free (values);
				} else
					buf = g_strdup (flags.body);
				values = buf;
			}
		}

		c2_application_command (application, C2_COMMAND_COMPOSER_NEW, interpret_as_link, headers, values);
			
		something_opened = TRUE;
	}

	/* Open file */
	if (flags.open_file)
	{
		c2_application_command (application, C2_COMMAND_OPEN_FILE, flags.open_file);
		something_opened = TRUE;
	}

	/* Check mail */
	if (flags.check)
	{
		c2_application_command (application, C2_COMMAND_CHECK_MAIL);
		something_opened = TRUE;
	}

	if (flags.hide_wmain)
	{
		c2_application_command (application, C2_COMMAND_WINDOW_MAIN_HIDE);
		something_opened = TRUE;
	}

	if (flags.raise_wmain)
	{
		c2_application_command (application, C2_COMMAND_WINDOW_MAIN_RAISE);
		something_opened = TRUE;
	}

	if (flags.exit)
	{
		c2_application_command (application, C2_COMMAND_EXIT);
		something_opened = TRUE;
	}
	
	/* If nothing opened we will open the defaults window */
	if (!something_opened)
	{
		CREATE_WINDOW_MAIN;
	}

#ifdef USE_DEBUG
	if (flags.intensive_test)
	{
		printf ("Starting intensive test '%s' in 2 seconds\n", flags.intensive_test);
		gtk_timeout_add (2000, on_intensive_test_timeout, application);
	}
#endif

	if (application->acting_as_server)
	{
		/* Release Information Dialog */
		if (c2_preferences_get_extra_release_information_show () ||
			c2_preferences_get_extra_default_mailer_check ())
			gtk_idle_add (on_main_idle, application);
	
		gtk_main ();
		gdk_threads_leave ();
	
		gnome_config_sync ();
		c2_hash_destroy ();
	}

	return 0;
}

static gint
on_main_idle (gpointer data)
{
	C2Application *application = C2_APPLICATION (data);

	if (c2_preferences_get_extra_release_information_show ())
		c2_application_dialog_release_information (application);

	if (c2_preferences_get_extra_default_mailer_check ())
		c2_application_dialog_default_mailer_check (application);

	return FALSE;
}

#ifdef USE_DEBUG
static void
intensive_test_mailbox (C2Application *application)
{
	C2Mailbox *test1, *test2;
	gint config_id;
	gchar *query;
	C2Message *message;
	gint i;
	GList *mlist = NULL;
	
/*	printf ("Checking if mailbox 'test1' exists: ");
	fflush (stdout);

	test1 = c2_mailbox_get_by_name (application->mailbox, "test1");
	if (C2_IS_MAILBOX (test1))
	{
		printf ("yes\n");
		printf ("Removing it...\n");
		if (c2_mailbox_remove (application->mailbox, test1))
			printf ("Success\n");
		else
			printf ("Failed: %s\n", c2_error_object_get (GTK_OBJECT (test1)));
	} else
		printf ("no\n");

	sleep (2);

	printf ("Creating mailbox 'test1'\n");
	
	gdk_threads_enter ();
	test1 = c2_mailbox_new_with_parent (&application->mailbox, "test1", NULL, C2_MAILBOX_CRONOSII,
								C2_MAILBOX_SORT_DATE, GTK_SORT_ASCENDING);
	gdk_threads_leave ();
	
	config_id = gnome_config_get_int_with_default ("/"PACKAGE"/Mailboxes/quantity=0", NULL)+1;
	query = g_strdup_printf ("/"PACKAGE"/Mailbox %d/", config_id);
	gnome_config_push_prefix (query);
	gnome_config_set_string ("name", test1->name);
	gnome_config_set_string ("id", test1->id);
	gnome_config_set_int ("type", test1->type);
	gnome_config_set_int ("sort_by", test1->sort_by);
	gnome_config_set_int ("sort_type", test1->sort_type);
	gnome_config_pop_prefix ();
	g_free (query);
	gnome_config_set_int ("/"PACKAGE"/Mailboxes/quantity", config_id);

*/	/* Do the same for test2 */
/*	test2 = c2_mailbox_get_by_name (application->mailbox, "test2");
	if (C2_IS_MAILBOX (test2))
	{
		printf ("yes\n");
		printf ("Removing it...\n");
		if (c2_mailbox_remove (application->mailbox, test2))
			printf ("Success\n");
		else
			printf ("Failed: %s\n", c2_error_object_get (GTK_OBJECT (test2)));
	} else
		printf ("no\n");

	sleep (1);

	printf ("Creating mailbox 'test2'\n");
	
	gdk_threads_enter ();
	test2 = c2_mailbox_new_with_parent (&application->mailbox, "test2", NULL, C2_MAILBOX_CRONOSII,
								C2_MAILBOX_SORT_DATE, GTK_SORT_ASCENDING);
	gdk_threads_leave ();
	
	config_id = gnome_config_get_int_with_default ("/"PACKAGE"/Mailboxes/quantity=0", NULL)+1;
	query = g_strdup_printf ("/"PACKAGE"/Mailbox %d/", config_id);
	gnome_config_push_prefix (query);
	gnome_config_set_string ("name", test1->name);
	gnome_config_set_string ("id", test1->id);
	gnome_config_set_int ("type", test1->type);
	gnome_config_set_int ("sort_by", test1->sort_by);
	gnome_config_set_int ("sort_type", test1->sort_type);
	gnome_config_pop_prefix ();
	g_free (query);
	gnome_config_set_int ("/"PACKAGE"/Mailboxes/quantity", config_id);

*/	/* We are going to add a message */
	message = c2_message_new ();
	c2_message_set_message (message, "From: Intensive Test <intensive@test.org>\n"
									 "To: You <y@u.net>\n"
									 "Subject: Just an intensive test\n"
									 "\n"
									 "This is a mail produced by an intensive test.\n");

	printf ("Making a list of 2000 messages\n");
	/* Add this message 2000 times */
	for (i = 0; i < 2000; i++)
		mlist = g_list_prepend (mlist, message);
	
	test1 = c2_mailbox_get_by_name (application->mailbox, "Drafts");
	printf ("Freezing the test1 mailbox\n");
	c2_db_freeze (test1);
	printf ("Adding the messages\n");
	c2_db_message_add_list (test1, mlist);
	printf ("Thawing the test1 mailbox\n");
	c2_db_thaw (test1);
	printf ("Done adding the 2000 messages to test1\n");
	
	
	
	c2_preferences_commit ();
}

static gint
on_intensive_test_timeout (gpointer data)
{
	pthread_t thread;
	
	printf ("Starting intensive test '%s'\n\n\n\n", flags.intensive_test);

	if (c2_streq (flags.intensive_test, "mailbox"))
		pthread_create (&thread, NULL, C2_PTHREAD_FUNC (intensive_test_mailbox), data);
	else
		printf ("Intensive test error: '%s' is not a valid intensive test.\n", flags.intensive_test);
	
	return FALSE;
}
#endif
