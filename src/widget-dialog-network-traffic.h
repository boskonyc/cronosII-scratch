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
#ifndef __WIDGET_DIALOG_NETWORK_TRAFFIC_H__
#define __WIDGET_DIALOG_NETWORK_TRAFFIC_H__

#ifdef __cplusplus
extern "C" {
#endif

#include <gnome.h>

#include "widget-dialog.h"

#define C2_DIALOG_NETWORK_TRAFFIC(obj)		(GTK_CHECK_CAST (obj, c2_dialog_network_traffic_get_type (), C2DialogNetworkTraffic))
#define C2_DIALOG_NETWORK_TRAFFIC_CLASS(klass)	(GTK_CHECK_CLASS_CAST (klass, c2_dialog_network_traffic_get_type (), C2DialogNetworkTraffic))

typedef struct _C2DialogNetworkTraffic C2DialogNetworkTraffic;
typedef struct _C2DialogNetworkTrafficClass C2DialogNetworkTrafficClass;

struct _C2DialogNetworkTraffic
{
	C2Dialog dialog;

	GtkWidget *darea;

	GdkPixmap *pixmap;
	GdkGC *blue;
	GdkGC *red;

	gint top_speed;
	GSList *recv, *send;
};

struct _C2DialogNetworkTrafficClass
{
	C2DialogClass parent_class;
};

GtkType
c2_dialog_network_traffic_get_type			(void);

GtkWidget *
c2_dialog_network_traffic_new				(C2Application *application);

#ifdef __cplusplus
}
#endif

#endif
