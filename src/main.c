/*  Cronos II Mail Client
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
#include <config.h>
#include <gnome.h>

#include <libcronosII/mailbox.h>
#include <libcronosII/error.h>

#include "c2-app.h"
#include "main-window.h"

static gint
c2_config_init											(void);

static void
c2_init (gint argc, gchar **argv)
{
	static struct poptOption options[] = {
		{"checkmail", 'c', POPT_ARG_NONE,
			NULL, 0,
			N_("Get new mail on startup"), NULL},
		{"compose", 'm', POPT_ARG_STRING,
			NULL, 0,
			N_("Compose a new email to EMAIL@ADDRESS"), "EMAIL@ADDRESS"}
	};
	gnome_init_with_popt_table ("Cronos II", VERSION, argc, argv, options, 0, NULL);
	glade_gnome_init ();
}

gint
main (gint argc, gchar **argv)
{
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

	/* Initialization of GNOME and Glade */
	c2_init (argc, argv);

	if (!c2_config_init ())
	{
		c2_app_init ();
		c2_window_new ();
		
		gdk_threads_enter ();
		gtk_main ();
		gdk_threads_leave ();
	}

	return 0;
}

static void
load_mailboxes								(void);

/**
 * c2_config_init
 *
 * Will load the configuration.
 *
 * Return Value:
 * 0 if success or 1.
 **/
static gint
c2_config_init (void)
{
	gchar *tmp;
	
	c2_app.tooltips = gtk_tooltips_new ();
	c2_app.open_windows = NULL;
	c2_app.tmp_files = NULL;

	/* Check if the configuration exists */
	tmp = gnome_config_get_string ("/cronosII/CronosII/Version");
	if (!tmp)
	{
#if FALSE /* TODO */
		gdk_threads_enter ();
		c2_install_new ();
		gdk_threads_leave ();
#endif
	}
	
	/* Get mailboxes */
	load_mailboxes ();

	c2_app.options_check_timeout = gnome_config_get_int_with_default
									("/cronoII/Options/check_timeout=" DEFAULT_OPTIONS_CHECK_TIMEOUT, NULL);
	c2_app.options_mark_timeout = gnome_config_get_int_with_default
									("/cronosII/Options/mark_timeout=" DEFAULT_OPTIONS_MARK_TIMEOUT, NULL);
	c2_app.options_prepend_character = gnome_config_get_string_with_default
									("/cronosII/Options/prepend_character=" DEFAULT_OPTIONS_PREPEND_CHARACTER, NULL);
	c2_app.options_empty_garbage = gnome_config_get_int_with_default
									("/cronosII/Options/empty_garbage=" DEFAULT_OPTIONS_EMPTY_GARBAGE, NULL);
	c2_app.options_use_outbox = gnome_config_get_int_with_default
									("/cronosII/Options/use_outbox=" DEFAULT_OPTIONS_USE_OUTBOX, NULL);
	c2_app.options_check_at_start = gnome_config_get_int_with_default
									("/cronosII/Options/check_at_start=" DEFAULT_OPTIONS_CHECK_AT_START, NULL);
	c2_app.options_default_mime = gnome_config_get_int_with_default
									("/cronosII/Options/default_mime=" DEFAULT_OPTIONS_DEFAULT_MIME, NULL);	
	
	c2_app.account = NULL;

	c2_app.interface_title = gnome_config_get_string_with_default
									("/cronosII/Interface/title=" DEFAULT_INTERFACE_TITLE, NULL);
	c2_app.interface_toolbar = gnome_config_get_int_with_default
									("/cronosII/Interface/toolbar=" DEFAULT_INTERFACE_TOOLBAR, NULL);
	c2_app.interface_date_fmt = gnome_config_get_string_with_default
									("/cronosII/Interface/date_fmt=" DEFAULT_INTERFACE_DATE_FMT, NULL);

	c2_app.fonts_message_body = gnome_config_get_string_with_default
									("/cronosII/Fonts/message_body=" DEFAULT_FONTS_MESSAGE_BODY, NULL);
	c2_app.fonts_unreaded_message = gnome_config_get_string_with_default
									("/cronosII/Fonts/unreaded_message=" DEFAULT_FONTS_UNREADED_MESSAGE, NULL);
	c2_app.fonts_readed_message = gnome_config_get_string_with_default
									("/cronosII/Fonts/readed_message=" DEFAULT_FONTS_READED_MESSAGE, NULL);
	c2_app.fonts_source = gnome_config_get_int_with_default
									("/cronosII/Fonts/source=" DEFAULT_FONTS_SOURCE, NULL);
	
	c2_app.colors_replying_original_message.red = gnome_config_get_int_with_default
			("/cronosII/Colors/replying_original_message_red=" DEFAULT_COLORS_REPLYING_ORIGINAL_MESSAGE_RED, NULL);
	c2_app.colors_replying_original_message.green = gnome_config_get_int_with_default
		("/cronosII/Colors/replying_original_message_green=" DEFAULT_COLORS_REPLYING_ORIGINAL_MESSAGE_GREEN, NULL);
	c2_app.colors_replying_original_message.blue = gnome_config_get_int_with_default
			("/cronosII/Colors/replying_original_message_blue=" DEFAULT_COLORS_REPLYING_ORIGINAL_MESSAGE_BLUE, NULL);
	c2_app.colors_message_bg.red = gnome_config_get_int_with_default
				("/cronosII/Colors/message_bg_red=" DEFAULT_COLORS_MESSAGE_BG_RED, NULL);
	c2_app.colors_message_bg.green = gnome_config_get_int_with_default
				("/cronosII/Colors/replying_original_message_green=" DEFAULT_COLORS_MESSAGE_BG_GREEN, NULL);
	c2_app.colors_message_bg.blue = gnome_config_get_int_with_default
				("/cronosII/Colors/message_bg_blue=" DEFAULT_COLORS_MESSAGE_BG_BLUE, NULL);	
	c2_app.colors_message_fg.red = gnome_config_get_int_with_default
				("/cronosII/Colors/message_fg_red=" DEFAULT_COLORS_MESSAGE_FG_RED, NULL);
	c2_app.colors_message_fg.green = gnome_config_get_int_with_default
				("/cronosII/Colors/message_fg_green=" DEFAULT_COLORS_MESSAGE_FG_GREEN, NULL);
	c2_app.colors_message_fg.blue = gnome_config_get_int_with_default
				("/cronosII/Colors/message_fg_blue=" DEFAULT_COLORS_MESSAGE_FG_BLUE, NULL);
	c2_app.colors_message_source = gnome_config_get_int_with_default
									("/cronosII/Colors/message_source=" DEFAULT_COLORS_MESSAGE_SOURCE, NULL);

	c2_app.paths_saving = gnome_config_get_string_with_default
									("/cronosII/Paths/saving=" DEFAULT_PATHS_SAVING, NULL);
	c2_app.paths_download = gnome_config_get_string_with_default
									("/cronosII/Paths/download=" DEFAULT_PATHS_DOWNLOAD, NULL);
	c2_app.paths_get = gnome_config_get_string_with_default
									("/cronosII/Paths/get=" DEFAULT_PATHS_GET, NULL);

	c2_app.advanced_http_proxy_addr = gnome_config_get_string_with_default
									("/cronosII/Advanced/http_proxy_addr=" DEFAULT_ADVANCED_HTTP_PROXY_ADDR, NULL);
	c2_app.advanced_http_proxy_port = gnome_config_get_int_with_default
									("/cronosII/Advanced/http_proxy_port=" DEFAULT_ADVANCED_HTTP_PROXY_PORT, NULL);
	c2_app.advanced_http_proxy = gnome_config_get_int_with_default
									("/cronosII/Advanced/http_proxy=" DEFAULT_ADVANCED_HTTP_PROXY, NULL);
	c2_app.advanced_ftp_proxy_addr = gnome_config_get_string_with_default
									("/cronosII/Advanced/ftp_proxy_addr=" DEFAULT_ADVANCED_FTP_PROXY_ADDR, NULL);
	c2_app.advanced_ftp_proxy_port = gnome_config_get_int_with_default
									("/cronosII/Advanced/ftp_proxy_port=" DEFAULT_ADVANCED_FTP_PROXY_PORT, NULL);
	c2_app.advanced_ftp_proxy = gnome_config_get_int_with_default
									("/cronosII/Advanced/ftp_proxy=" DEFAULT_ADVANCED_FTP_PROXY, NULL);
	c2_app.advanced_persistent_smtp_addr = gnome_config_get_string_with_default
							("/cronosII/Advanced/persistent_smtp_addr=" DEFAULT_ADVANCED_PERSISTENT_SMTP_ADDR, NULL);
	c2_app.advanced_persistent_smtp_port = gnome_config_get_int_with_default
							("/cronosII/Advanced/persistent_smtp_port=" DEFAULT_ADVANCED_PERSISTENT_SMTP_PORT, NULL);
	c2_app.advanced_persistent_smtp = gnome_config_get_int_with_default
									("/c2/Advanced/persistent_smtp=" DEFAULT_ADVANCED_PERSISTENT_SMTP, NULL);
	c2_app.advanced_use_internal_browser = gnome_config_get_int_with_default
							("/cronosII/Advanced/use_internal_browser=" DEFAULT_ADVANCED_USE_INTERNAL_BROWSER, NULL);

	c2_app.rc_hpan = gnome_config_get_int_with_default ("/cronosII/Rc/hpan=" DEFAULT_RC_HPAN, NULL);
	c2_app.rc_vpan = gnome_config_get_int_with_default ("/cronosII/Rc/vpan=" DEFAULT_RC_VPAN, NULL);
	c2_app.rc_clist[0] = gnome_config_get_int_with_default ("/cronosII/Rc/clist[0]=" DEFAULT_RC_CLIST_0, NULL);
	c2_app.rc_clist[1] = gnome_config_get_int_with_default ("/cronosII/Rc/clist[1]=" DEFAULT_RC_CLIST_1, NULL);
	c2_app.rc_clist[2] = gnome_config_get_int_with_default ("/cronosII/Rc/clist[2]=" DEFAULT_RC_CLIST_2, NULL);
	c2_app.rc_clist[3] = gnome_config_get_int_with_default ("/cronosII/Rc/clist[3]=" DEFAULT_RC_CLIST_3, NULL);
	c2_app.rc_clist[4] = gnome_config_get_int_with_default ("/cronosII/Rc/clist[4]=" DEFAULT_RC_CLIST_4, NULL);
	c2_app.rc_clist[5] = gnome_config_get_int_with_default ("/cronosII/Rc/clist[5]=" DEFAULT_RC_CLIST_5, NULL);
	c2_app.rc_clist[6] = gnome_config_get_int_with_default ("/cronosII/Rc/clist[6]=" DEFAULT_RC_CLIST_6, NULL);
	c2_app.rc_clist[7] = gnome_config_get_int_with_default ("/cronosII/Rc/clist[7]=" DEFAULT_RC_CLIST_7, NULL);
	c2_app.rc_width = gnome_config_get_int_with_default ("/cronosII/Rc/width=" DEFAULT_RC_WIDTH, NULL);
	c2_app.rc_height = gnome_config_get_int_with_default ("/cronosII/Rc/height=" DEFAULT_RC_HEIGHT, NULL);
	c2_app.rc_showable_headers[C2_SHOWABLE_HEADERS_PREVIEW] = gnome_config_get_int_with_default
								("/cronosII/Rc/showable_headers_preview=" DEFAULT_RC_SHOWABLE_HEADERS_PREVIEW, NULL);
	c2_app.rc_showable_headers[C2_SHOWABLE_HEADERS_MESSAGE] = gnome_config_get_int_with_default
								("/cronosII/Rc/showable_headers_message=" DEFAULT_RC_SHOWABLE_HEADERS_MESSAGE, NULL);
	c2_app.rc_showable_headers[C2_SHOWABLE_HEADERS_COMPOSE] = gnome_config_get_int_with_default
								("/cronosII/Rc/showable_headers_compose=" DEFAULT_RC_SHOWABLE_HEADERS_COMPOSE, NULL);
	c2_app.rc_showable_headers[C2_SHOWABLE_HEADERS_SAVE] = gnome_config_get_int_with_default
								("/cronosII/Rc/showable_headers_save=" DEFAULT_RC_SHOWABLE_HEADERS_SAVE, NULL);
	c2_app.rc_showable_headers[C2_SHOWABLE_HEADERS_PRINT] = gnome_config_get_int_with_default
								("/cronosII/Rc/showable_headers_print=" DEFAULT_RC_SHOWABLE_HEADERS_PRINT, NULL);
	
	return 0;
}

static void
load_mailboxes (void)
{
	int i;

	for (c2_app.mailbox = NULL, i = 0;; i++)
	{
		gchar *name;
		gchar *id;
		C2MailboxType type;
		C2MailboxSortBy sort_by;
		GtkSortType sort_type;
		gchar *host, *user, *pass;
		gint port;
		gchar *db;
		
		gchar *query = g_strdup_printf ("/cronosII/Mailboxes/%d", i);
		
		gnome_config_push_prefix (query);
		if (!(name = gnome_config_get_string ("::Name")))
		{
			gnome_config_pop_prefix ();
			c2_app.mailbox = c2_mailbox_get_head ();
			g_free (query);
			break;
		}

		id = gnome_config_get_string ("::Id");
		type = gnome_config_get_int ("::Type");
		sort_by = gnome_config_get_int ("::Sort By");
		sort_type = gnome_config_get_int ("::Sort Type");

		switch (type)
		{
			case C2_MAILBOX_CRONOSII:
				c2_mailbox_new (name, id, type, sort_by, sort_type);
				break;
			case C2_MAILBOX_IMAP:
				host = gnome_config_get_string ("::Host");
				port = gnome_config_get_int ("::Port");
				user = gnome_config_get_string ("::User");
				pass = gnome_config_get_string ("::Pass");	
				c2_mailbox_new (name, id, type, sort_by, sort_type, host, port, user, pass);
				g_free (host);
				g_free (user);
				g_free (pass);
				break;
#ifdef USE_MYSQL
			case C2_MAILBOX_MYSQL:
				host = gnome_config_get_string ("::Host");
				port = gnome_config_get_int ("::Port");
				db = gnome_config_get_string ("::Db");
				user = gnome_config_get_string ("::User");
				pass = gnome_config_get_string ("::Pass");	
				c2_mailbox_new (name, id, type, sort_by, sort_type, host, port, db, user, pass);
				g_free (host);
				g_free (db);
				g_free (user);
				g_free (pass);
				break;
#endif
		}
		g_free (name);
		g_free (id);
		gnome_config_pop_prefix ();
		g_free (query);
	}
}
