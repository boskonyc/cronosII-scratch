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
#include <gtk/gtk.h>
#include <time.h>

#include <libcronosII/message.h>
#include <libcronosII/mime.h>
#include <libcronosII/db.h>

gint
main (gint argc, gchar **argv)
{
	C2Message *message, *fmessage;
	
	gtk_init (&argc, &argv);

	message = c2_db_message_get_from_file ("message.elm");
	message->mime = c2_mime_new (message);

	g_print ("Fixed Message:\n"
			 "%s\n"
			 "\n"
			 "%s\n",
			 (fmessage = c2_message_fix_broken_message (message))->header, fmessage->body);

	return 0;
}
