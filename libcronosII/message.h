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
#ifndef __LIBC2_MESSAGE_H__
#define __LIBC2_MESSAGE_H__

#ifdef __cplusplus
extern "C" {
#endif

#include <gtk/gtk.h>
#include <time.h>

#ifdef HAVE_CONFIG_H
#	include "db.h"
#	include "mailbox.h"
#else
#	include <cronosII.h>
#endif

#define C2_TYPE_MESSAGE							(c2_message_get_type ())
#define C2_MESSAGE(obj)							(GTK_CHECK_CAST (obj, C2_TYPE_MESSAGE, C2Message))
#define C2_MESSAGE_CLASS(klass)					(GTK_CHECK_CLASS_CAST (klass, C2_TYPE_MESSAGE, C2MessageClass))
#define C2_IS_MESSAGE(obj)						(GTK_CHECK_TYPE(obj, C2_TYPE_MESSAGE))
#define C2_IS_MESSAGE_CLASS(klass)				(GTK_CHECK_CLASS_TYPE (klass, C2_TYPE_MESSAGE))

typedef struct _C2Message C2Message;
typedef struct _C2MessageClass C2MessageClass;
typedef enum _C2MessageState C2MessageState;
typedef enum _C2MessageAction C2MessageAction;

enum _C2MessageState
{
	C2_MESSAGE_READED		= ' ',
	C2_MESSAGE_UNREADED		= 'N',
	C2_MESSAGE_REPLIED		= 'R',
	C2_MESSAGE_FORWARDED	= 'F'
};

enum _C2MessageAction
{
	C2_MESSAGE_DELETE,
	C2_MESSAGE_EXPUNGE,
	C2_MESSAGE_MOVE,
	C2_MESSAGE_COPY
};

struct _C2Message
{
	GtkObject object;
	
	gchar *message;
	gchar *header;
	const gchar *body;
	GList *mime;
};

struct _C2MessageClass
{
	GtkObjectClass parent_class;
	
	void (*message_die) (C2Message *message);
};

GtkType
c2_message_get_type									(void);

C2Message *
c2_message_new										(void);

gchar *
c2_message_get_message_header						(C2Message *message);

const gchar *
c2_message_get_message_body							(C2Message *message);

gchar *
c2_message_get_header_field							(C2Message *message, const gchar *field);

#ifdef __cplusplus
}
#endif

#endif