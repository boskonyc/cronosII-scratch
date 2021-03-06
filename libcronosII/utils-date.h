/*  Cronos II - The GNOME mail client
 *  Copyright (C) 2000-2001 Pablo Fern�ndez L�pez
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
 * 		* Pablo Fern�ndez L�pez
 * Code of this file by:
 * 		* Pablo Fern�ndez L�pez
 */
#ifndef __LIBCRONOSII_UTILS_DATE_H__
#define __LIBCRONOSII_UTILS_DATE_H__

#ifdef __cplusplus
extern "C" {
#endif

#include <time.h>

/* Utils macros */
#define C2_SECONDS_IN_MS(sec)				(sec*1000)

gint
c2_date_get_month							(const gchar *strmnt);

time_t
c2_date_parse								(const gchar *strtime);

time_t
c2_date_parse_fmt2							(const gchar *strtime);

time_t
c2_date_parse_fmt3							(const gchar *strtime);

#ifdef __cplusplus
}
#endif

#endif
