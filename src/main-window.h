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
#ifndef __CRONOSII_MAIN_WINDOW_H__
#define __CRONOSII_MAIN_WINDOW_H__

#ifdef __cplusplus
extern "C" {
#endif

#include <gnome.h>
#include <pthread.h>
#include <glade/glade.h>

#if defined (HAVE_CONFIG_H) && defined (BUILDING_C2)
#	include <libcronosII/mailbox.h>
#else
#	include <cronosII.h>
#endif
	
enum
{
	C2_HEADER_TITLES_FROM,
	C2_HEADER_TITLES_TO,
	C2_HEADER_TITLES_CC,
	C2_HEADER_TITLES_BCC,
	C2_HEADER_TITLES_SUBJECT,
	C2_HEADER_TITLES_ACCOUNT,
	C2_HEADER_TITLES_DATE,
	C2_HEADER_TITLES_PRIORITY,
	C2_HEADER_TITLES_LAST
};

typedef struct
{
	GladeXML *xml;
	GladeXML *ctree_menu;

	pthread_mutex_t appbar_lock;
	pthread_mutex_t index_lock;
	pthread_mutex_t text_lock;

	C2Mailbox *selected_mbox;
} C2MainWindow;

C2MainWindow WMain;

void
c2_window_new									(void);

#ifdef __cplusplus
}
#endif

#endif
