/*  Cronos II - The GNOME mail client
 *  Copyright (C) 2000-2001 Pablo Fernández
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
/**
 * Maintainer(s) of this file:
 * 		* Pablo Fernández
 * Code of this file by:
 * 		* Pablo Fernández
 */
#ifndef __LIBCRONOSII_UTILS_H__
#define __LIBCRONOSII_UTILS_H__

#ifdef __cplusplus
extern "C" {
#endif

#include <glib.h>
#include <stdio.h>
#include <pthread.h>
#include <gtk/gtk.h>
#if defined (HAVE_CONFIG_H) && defined (BUILDING_C2)
#	include <config.h>
#else
#	include <cronosII.h>
#endif

#include "utils-debug.h"

#define C2_HOME								/*$HOME*/ G_DIR_SEPARATOR_S ".c2" G_DIR_SEPARATOR_S

#define C2_CHAR(x)							((gchar*)x)


typedef void *(*C2PthreadFunc)				(void*);
#define C2_PTHREAD_FUNC(x)					((C2PthreadFunc)x)

typedef struct
{
	gpointer v1, v2;
} C2Pthread2;

typedef struct
{
	gpointer v1, v2, v3;
} C2Pthread3;

typedef struct
{
	gpointer v1, v2, v3, v4;
} C2Pthread4;

typedef struct
{
	gpointer v1, v2, v3, v4, v5;
} C2Pthread5;

void
c2_marshal_NONE__INT_INT_INT				(GtkObject *object, GtkSignalFunc func,
											 gpointer func_data, GtkArg * args);

void
c2_marshal_INT__POINTER_POINTER_POINTER		(GtkObject *object, GtkSignalFunc func,
											 gpointer func_data, GtkArg * args);


#ifdef __cplusplus
}
#endif

#endif
