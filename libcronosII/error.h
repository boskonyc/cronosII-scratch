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
#ifndef __LIBCRONOSII_ERROR_H__
#define __LIBCRONOSII_ERROR_H__

#ifdef __cplusplus
extern "C" {
#endif

#include <gtk/gtk.h>
#include <errno.h>
	
#define c2_return_if_fail(condition, errnum) if (!(condition)) { \
		c2_error_set (errnum); \
		g_return_if_fail (condition); \
	}

#define c2_return_val_if_fail(condition, value, errnum) if (!(condition)) { \
		c2_error_set (errnum); \
		g_return_val_if_fail (condition, value); \
	}

#define c2_return_if_fail_obj(condition, errnum, obj) if (!(condition)) { \
		c2_error_object_set (obj, errnum); \
		g_return_if_fail (condition); \
	}

#define c2_return_val_if_fail_obj(condition, value, errnum, obj) if (!(condition)) { \
		c2_error_object_set (obj, errnum); \
		g_return_val_if_fail (condition, value); \
	}

enum
{
	C2SUCCESS,
	C2EDATA,
	C2ENOMSG,
	C2EBUSY,
	C2ERSLV,
	C2USRCNCL,
	C2SRVCNCL,
	C2INTERNAL,
	C2NOBJMAX,
	
	C2CUSTOM,
	C2ELAST
};

/* Own errno variable to keep track of our errors */
gint c2_errno;
const gchar *c2_errstr;

const gchar *
c2_error_get								(void);

void
c2_error_set								(gint err);

void
c2_error_set_custom							(gchar *err);

const gchar *
c2_error_object_get							(GtkObject *object);

gint
c2_error_object_get_id						(GtkObject *object);

void
c2_error_object_set							(GtkObject *object, gint err);

void
c2_error_object_set_custom					(GtkObject *object, gchar *err);

#ifdef __cplusplus
}
#endif

#endif
