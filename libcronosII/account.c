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
#include <glib.h>
#include <stdarg.h>

#include "account.h"
#include "error.h"
#include "pop3.h"
#include "smtp.h"

enum
{
	UPDATE_CHECK,
	LAST_SIGNAL
};

static guint c2_account_signals[LAST_SIGNAL] = { 0 };

static GtkObjectClass *parent_class = NULL;

GtkType
c2_account_get_type (void)
{
	static GtkType c2_account_type = 0;

	if (!c2_account_type)
	{
		static const GtkTypeInfo c2_account_info = {
			"C2Account",
			sizeof (C2Account),
			sizeof (C2AccountClass),
			(GtkClassInitFunc) c2_account_class_init,
			(GtkObjectInitFunc) c2_account_init,
			/* reserved_1 */ NULL,
			/* reserved_2 */ NULL,
			(GtkClassInitFunc) NULL
		};

		c2_account_type = gtk_type_unique (gtk_object_get_type (), &c2_account_info);

	}

	return c2_account_type;
}

static void
c2_account_class_init (C2AccountClass *klass)
{
	GtkObjectClass *object_class;

	object_class = (GtkObjectClass *) klass;

	parent_class = gtk_type_class (gtk_object_get_type ());

	c2_account_signals[UPDATE_CHECK] =
		gtk_signal_new ("update_check",
					GTK_RUN_FIRST,
					object_class->type,
					GTK_SIGNAL_OFFSET (C2AccountClass, update_check),
					gtk_marshal_NONE__INT, GTK_TYPE_NONE, 4,
					GTK_TYPE_INT, GTK_TYPE_INT, GTK_TYPE_LONG, GTK_TYPE_LONG);

	gtk_object_class_add_signals (object_class, c2_account_signals, LAST_SIGNAL);

	object_class->destroy = c2_account_destroy;

	klass->update_check = NULL;
}

static void
c2_account_init (C2Account *account)
{
	account->name = NULL;
	account->per_name = NULL;
	account->organization = NULL;
	account->email = NULL;
	account->reply_to = NULL;
	account->protocol.pop3 = NULL;
	account->smtp = NULL;
	account->signature.string = NULL;
	account->next = NULL;
}

C2Account *
c2_account_new (const gchar *name, const gchar *per_name, const gchar *organization, const gchar *email,
				const gchar *reply_to, gboolean active, C2AccountType account_type, ...,
				C2SMTPType smtp_type, ..., C2AccountSignatureType type, const gchar *signature,
				gboolean signature_automatic)
{
	C2Account *account;
	gchar *user, *pass, *host;
	gint port;
	gchar *file;
	va_list args;
	
	c2_return_val_if_fail (name || email, NULL, C2EDATA);

	account = gtk_type_new (C2_TYPE_ACCOUNT);
	account->name = g_strdup (name);
	account->per_name = g_strdup (per_name);
	account->organization = g_strdup (organization);
	account->email = g_strdup (email);
	account->smtp = c2_smtp_new (smtp_addr, smtp_port);
	
	account->options.active = active;

	account->signature.string = g_strdup (signature);
	account->signature.automatically = autosign;

	account->type = type;

	va_start (args, type);

	switch (account->type)
	{
		case C2_ACCOUNT_POP3:
			user = va_arg (args, gchar *);
			pass = va_arg (args, gchar *);
			host = va_arg (args, gchar *);
			port = va_arg (args, gint);
			
			account->protocol.pop3 = c2_pop3_new (user, pass, host, port);
	}

	va_end (args);

	return account;
}

void
c2_account_free (C2Account *account)
{
	c2_return_if_fail (account, C2EDATA);

	g_free (account->name);
	g_free (account->per_name);
	g_free (account->email);
	if (account->type == C2_ACCOUNT_POP3)
		c2_pop3_free (account->protocol.pop3);
	g_free (account->signature.string);
	g_free (account);
}

void
c2_account_free_all (C2Account *head)
{
	C2Account *l, *s;
	
	for (l = head; l != NULL;)
	{
		s = l->next;
		c2_account_free (l);
		l = s;
	}
}

/**
 * c2_account_copy
 * @account: C2Account object to copy.
 *
 * This function will copy a C2Account object.
 * 
 * Return Value:
 * The copy of the object.
 **/
C2Account *
c2_account_copy (C2Account *account)
{
	C2Account *copy = NULL;
	
	c2_return_val_if_fail (account, NULL, C2EDATA);

	if (account->type == C2_ACCOUNT_POP3)
	{
		copy = c2_account_new (account->name, account->per_name, account->email,
								account->smtp->address, account->smtp->port,
								account->options.active, account->signature.string,
								account->signature.automatically, account->type,
								account->protocol.pop3->user, account->protocol.pop3->pass,
								account->protocol.pop3->host, account->protocol.pop3->port);
		c2_pop3_set_flags (copy->protocol.pop3, account->protocol.pop3->flags);
		c2_smtp_set_flags (copy->smtp, account->smtp->flags);
	}

	return copy;
}
