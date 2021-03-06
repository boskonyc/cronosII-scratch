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
#ifndef __WIDGET_SELECT_LIST_H__
#define __WIDGET_SELECT_LIST_H__

#ifdef __cplusplus
extern "C" {
#endif

#include <gtk/gtkclist.h>

#if defined (HAVE_CONFIG_H) && defined (BUILDING_C2)
#	include "widget-application.h"
#else
#	include <cronosII.h>
#endif

#define C2_SELECT_LIST(obj)					GTK_CHECK_CAST (obj, c2_select_list_get_type (), C2SelectList)
#define C2_SELECT_LIST_CLASS(klass)			GTK_CHECK_CLASS_CAST (klass, c2_select_list_get_type (), C2SelectListClass)
#define C2_IS_SELECT_LIST(obj)				GTK_CHECK_TYPE (obj, c2_select_list_get_type ())
#define C2_SELECT_LIST_CLASS_FW(obj)		C2_SELECT_LIST_CLASS (((GtkObject*)(obj))->klass)

typedef struct _C2SelectList C2SelectList;
typedef struct _C2SelectListClass C2SelectListClass;

struct _C2SelectList
{
	GtkCList clist;

	GtkWidget *pixmap_on;
	GtkWidget *pixmap_off;
};

struct _C2SelectListClass
{
	GtkCListClass parent_class;

	void (*select_item) (C2SelectList *sl, gint row, gpointer data);
	void (*unselect_item) (C2SelectList *sl, gint row, gpointer data);
};

GtkType
c2_select_list_get_type						(void);

GtkWidget *
c2_select_list_new_from_glade				(const gchar *str1, const gchar *str2, gint n1, gint n2);

GtkWidget *
c2_select_list_new							(gint columns);

#define c2_select_list_append_item(sl,row,data) \
										c2_select_list_insert_item  (sl, \
										(GTK_CLIST (sl)->rows >= 0) ? GTK_CLIST (sl)->rows : 0, \
										row, data)

#define c2_select_list_prepend_item(sl,row,data) \
										c2_select_list_insert_item  (sl, 0, row, data)

#define c2_select_list_last(sl)				(GTK_CLIST (sl)->rows-1)

void
c2_select_list_insert_item					(C2SelectList *sl, gint row, gchar **cont, gpointer data);

void
c2_select_list_set_active					(C2SelectList *sl, gint row, gboolean active);

GSList *
c2_select_list_get_active_items				(C2SelectList *sl);

GSList *
c2_select_list_get_active_items_data		(C2SelectList *sl);

#ifdef __cplusplus
}
#endif

#endif
