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
#ifndef __LIBMODULES_POP_H__
#define __LIBMODULES_POP_H__

#ifdef __cplusplus
extern "C" {
#endif

#include <glib.h>

typedef struct
{
	gchar *host;
	gint port;

	gboolean keep_copy;

	gint sock;
} C2Pop;

C2Pop *
c2_pop_new (const gchar *host, gint port);

void
c2_pop_free (C2Pop *pop);

gint
c2_pop_fetchmail (C2Pop *pop);

#ifdef __cplusplus
}
#endif

#endif