/*  Cronos II
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
/*
 * The db module will handle the database request.
 * All interactions with the database of Cronos II should be done through
 * this API.
 * You might want to compare this module with the old Message module.
 */
#ifndef __LIBMODULES_DB_H__
#define __LIBMODULES_DB_H__

#ifdef __cplusplus
extern "C" {
#endif

#include <glib.h>
#include <gtk/gtk.h>

typedef gint mid_t;

typedef enum
{
	C2_DB_NODE_UNREAD,
	C2_DB_NODE_REPLIED,
	C2_DB_NODE_FORWARDED,
	C2_DB_NODE_READED
} C2DBNodeStatus;

typedef enum
{
	C2_SORT_STATUS,
	C2_SORT_SUBJECT,
	C2_SORT_FROM,
	C2_SORT_DATE,
	C2_SORT_ACCOUNT
} C2SortType;

typedef struct
{
	/* Position in the database */
	gint row;
	mid_t mid;

	/* Headers of node (Subject, From, Date, Account) */
	gchar *headers[4];
	
	C2DBNodeStatus status;
	int marked : 1;
} C2DBNode;

typedef struct
{
	gchar *mbox;
	GList *head;
} C2DB;

typedef struct
{
	gchar *mbox;
	mid_t mid;
	gchar *message;
	gchar *header;
	gchar *body;
	GList *mime;
} C2Message;

#define c2_db_new()						(g_new0 (C2DB, 1))
#define c2_db_node_new()				(g_new0 (C2DBNode, 1))

C2DB *
c2_db_load									(const gchar *db_name);

void
c2_db_unload								(C2DB *db_d);

gint
c2_db_message_add							(C2DB *db_d, const gchar *message, gint row);

gint
c2_db_message_remove						(C2DB *db_d, int row);

void
c2_db_sort									(C2DB *db_d, C2SortType c2_type, GtkSortType gtk_type);

C2Message *
c2_db_message_get							(C2DB *db_d, int row);

C2Message *
c2_db_message_get_from_file					(const gchar *filename);

gint
c2_db_message_search_by_mid					(const C2DB *db_d, mid_t mid);

#ifdef __cplusplus
}
#endif

#endif
