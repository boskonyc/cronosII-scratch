/*  Cronos II Mail Client /src/widget-message-transfer.c
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
#include <gnome.h>
#include <glade/glade.h>

#include <libcronosII/error.h>
#include <libcronosII/utils.h>

#include "c2-app.h"
#include "widget-message-transfer.h"

static void
class_init										(C2MessageTransferClass *klass);

static void
init											(C2MessageTransfer *obj);

static void
destroy											(GtkObject *obj);

static C2MessageTransferQueue *
queue_new										(const C2Account *account, C2MessageTransferType type, 
												 C2MessageTransferAction action, ...);

static C2MessageTransferQueue *
queue_append									(C2MessageTransferQueue *head, C2MessageTransferQueue *obj);

static void
queue_free										(C2MessageTransferQueue *head);

static gint
run												(C2MessageTransfer *mt);

static void
reset											(C2MessageTransfer *mt);

static gint
check											(C2Pthread2 *data);

static void
clist_select_row								(GtkCList *clist, gint row, gint column,
												 GdkEvent *event, C2MessageTransfer *mt);

static void
clist_unselect_row								(GtkCList *clist, gint row, gint column,
												 GdkEvent *event, C2MessageTransfer *mt);

static void
ok_btn_clicked									(GtkWidget *button, C2MessageTransfer *mt);

static void
cancel_btn_clicked								(GtkWidget *button, C2MessageTransfer *mt);

enum
{
	APPEND,
	TASK_DONE,
	LAST_SIGNAL
};

static guint signals[LAST_SIGNAL] = { 0 };

static GnomeDialogClass *parent_class = NULL;

GtkWidget *
c2_message_transfer_new (void)
{
	C2MessageTransfer *mt = gtk_type_new (c2_message_transfer_get_type ());
	GtkWidget *box;

	mt->xml = glade_xml_new (C2_APP_GLADE_FILE ("cronosII"), "dlg_message_transfer_box");
	box = glade_xml_get_widget (mt->xml, "dlg_message_transfer_box");

	gtk_box_pack_start (GTK_BOX (GNOME_DIALOG (mt)->vbox), box, TRUE, TRUE, 0);

	gtk_widget_set_usize (GTK_WIDGET (mt), -1, 420);

	box = glade_xml_get_widget (mt->xml, "task_clist");
	gtk_signal_connect (GTK_OBJECT (box), "select_row",
							GTK_SIGNAL_FUNC (clist_select_row), mt);
	gtk_signal_connect (GTK_OBJECT (box), "unselect_row",
							GTK_SIGNAL_FUNC (clist_unselect_row), mt);
	gnome_dialog_button_connect (GNOME_DIALOG (GTK_WIDGET (mt)), 0, ok_btn_clicked, mt);
	gnome_dialog_button_connect (GNOME_DIALOG (GTK_WIDGET (mt)), 1, cancel_btn_clicked, mt);	

	return GTK_WIDGET (mt);
}

void
c2_message_transfer_freeze (C2MessageTransfer *mt)
{
	mt->queue = NULL;
	reset (mt);
	/* XXX If we get some problems, like gtk+ freezing when
	 * XXX some checking stuff is executed, there's a good
	 * XXX chance the problem is here (we are using a blocking
	 * XXX function.
	 */
	pthread_mutex_lock (&mt->queue_lock);
}

void
c2_message_transfer_thaw (C2MessageTransfer *mt)
{
	pthread_mutex_unlock (&mt->queue_lock);

	gtk_progress_configure (GTK_PROGRESS (glade_xml_get_widget (mt->xml, "task_progress")),
							mt->tasks[DONE], 0, mt->tasks[TOTAL]);
	
	/* If it is not already running, make it run */
	if (!mt->active)
	{
		pthread_t thread;

		pthread_create (&thread, NULL, C2_PTHREAD_FUNC (run), mt);
	}
}

void
c2_message_transfer_append (C2MessageTransfer *mt, const C2Account *account, C2MessageTransferType type,
							C2MessageTransferAction action, ...)
{
	GtkWidget *tasks = glade_xml_get_widget (mt->xml, "task_progress");
	C2MessageTransferQueue *queue;
	va_list args;
	
	queue = queue_new (account, type, action);
	mt->queue = queue_append (mt->queue, queue);

	mt->tasks[TOTAL]++;
}

static C2MessageTransferQueue *
queue_new (const C2Account *account, C2MessageTransferType type, C2MessageTransferAction action, ...)
{
	C2MessageTransferQueue *queue = g_new0 (C2MessageTransferQueue, 1);

	queue->subtasks[DONE] = queue->subtasks[TOTAL] = queue->subtask[DONE], queue->subtask[TOTAL] = 0;
	queue->action = action;
	queue->type = type;
	queue->account = account;
	queue->edata = NULL;

	/* TODO read the ... */

	return queue;
}

static C2MessageTransferQueue *
queue_append (C2MessageTransferQueue *head, C2MessageTransferQueue *obj)
{
	C2MessageTransferQueue *s;
	
	c2_return_val_if_fail (obj, head, C2EDATA);

	for (s = head; s; s = s->next)
		if (!s->next)
			break;

	if (s)
		s->next = obj;
	else
		head = obj;

	return head;
}

static void
queue_free (C2MessageTransferQueue *head)
{
}

static gint
run (C2MessageTransfer *mt)
{
	C2MessageTransferQueue *s;
	GtkCList *clist = GTK_CLIST (glade_xml_get_widget (mt->xml, "task_clist"));
	GnomePixmap *pixmap = NULL;
	gchar *info = NULL;
	gboolean success = TRUE;
	gint row;
	const gchar *line[] = { NULL, NULL, NULL };
	C2Pthread2 *data;
	pthread_t thread;

	/* We need to block here because we don't want
	 * two run() running at the same time.
	 */
	c2_return_if_fail (!mt->active, C2EDATA);

	mt->active = TRUE;
	reset (mt);

	gdk_threads_enter ();
	pixmap = GNOME_PIXMAP (gnome_pixmap_new_from_file (PKGDATADIR "/pixmaps/blue_arrow.png"));
	gtk_clist_freeze (clist);
	for (s = mt->queue, row = 0; s; s = s->next, row++)
	{
		line[1] = s->account->name;
		
		if (s->action == C2_MESSAGE_TRANSFER_CHECK)
			line[2] = _("Waiting to get messages.");
		else
			line[2] = _("Waiting to deliver a message.");
		
		gtk_clist_append (clist, line);
		gtk_clist_set_pixmap (clist, row, 0, pixmap->pixmap, pixmap->mask);
		gtk_clist_set_row_data (clist, row, s);
	}
	gtk_clist_thaw (clist);
	gdk_threads_leave ();

	for (s = mt->queue, row = 0; s; s = s->next)
	{
		if (s->action == C2_MESSAGE_TRANSFER_CHECK)
		{
			/* We need to fire the function in other thread,
			 * the data we'll need is:
			 * o C2MessageTransfer,
			 * o C2MessageTransferQueue (or row)
			 */
			data = g_new0 (C2Pthread2, 1);
			data->v1 = (gpointer) mt;
			data->v2 = (gpointer) s;
			pthread_create (&thread, NULL, C2_PTHREAD_FUNC (check), data);
			
			if (c2_app.options_mt_mode == C2_MESSAGE_TRANSFER_MONOTHREAD)
				pthread_join (thread, NULL);
		} else
		{
			g_print (PACKAGE " v." VERSION " doesn't supports sending of messages, "
					 "if you are a developer you can code it yourself:\n"
					 "file: " __FILE__"\n"
					 "line: %d\n"
					 "function: " __PRETTY_FUNCTION__ "\n"
					 "\n",
					 __LINE__);
		}
	}

	if (pthread_mutex_trylock (&mt->queue_lock))
	{
		mt->active = FALSE;
		return 0;
	}

	mt->active = FALSE;

	if (GTK_TOGGLE_BUTTON (glade_xml_get_widget (mt->xml, "auto_close_btn"))->active)
	{
		gdk_threads_enter ();
		gtk_widget_hide (GTK_WIDGET (mt));
		gdk_threads_leave ();
	}
	
	pthread_mutex_unlock (&mt->queue_lock);

	return 0;
}

static void
reset (C2MessageTransfer *mt)
{
	static C2MessageTransferQueue *queue = NULL;

	gtk_progress_configure (GTK_PROGRESS (glade_xml_get_widget (mt->xml, "task_progress")), 0, 0, 0);
	gtk_progress_configure (GTK_PROGRESS (glade_xml_get_widget (mt->xml, "subtasks_progress")), 0, 0, 0);
	gtk_progress_configure (GTK_PROGRESS (glade_xml_get_widget (mt->xml, "subtask_progress")), 0, 0, 0);
	gtk_widget_hide (glade_xml_get_widget (mt->xml, "subtask_progress"));
	gtk_clist_clear (GTK_CLIST (glade_xml_get_widget (mt->xml, "task_clist")));

	if (queue)
		queue_free (queue);

	queue = mt->queue;
}

static gint
check (C2Pthread2 *data)
{
	C2MessageTransfer *mt = data->v1;
	C2MessageTransferQueue *queue = data->v2;

	sleep (5);
	printf ("Checking %d\n", (gint) queue);
}

static void
clist_select_row (GtkCList *clist, gint row, gint column, GdkEvent *event, C2MessageTransfer *mt)
{
	GtkWidget *info_box = glade_xml_get_widget (mt->xml, "info_box");
	GtkWidget *tasks_frame = glade_xml_get_widget (mt->xml, "tasks_frame");
	GtkWidget *subtasks_frame = glade_xml_get_widget (mt->xml, "subtasks_frame");
	GtkWidget *subtasks_progress = glade_xml_get_widget (mt->xml, "subtasks_progress");
	GtkWidget *subtask_frame = glade_xml_get_widget (mt->xml, "subtask_frame");
	GtkWidget *subtask_progress = glade_xml_get_widget (mt->xml, "subtask_progress");
	C2MessageTransferQueue *queue = (C2MessageTransferQueue *) gtk_clist_get_row_data (
									GTK_CLIST (glade_xml_get_widget (mt->xml, "task_clist")), row);


	gtk_progress_configure (GTK_PROGRESS (subtasks_progress), queue->subtasks[DONE], 0,
							queue->subtasks[TOTAL]);
	gtk_progress_configure (GTK_PROGRESS (subtask_progress), queue->subtask[DONE], 0,
							queue->subtask[TOTAL]);

	gtk_widget_show (info_box);
	gtk_widget_hide (tasks_frame);
	gtk_widget_show (subtasks_frame);
	gtk_widget_show (subtasks_progress);
	gtk_widget_show (subtask_frame);
	gtk_widget_show (subtask_progress);
}

static void
clist_unselect_row (GtkCList *clist, gint row, gint column, GdkEvent *event, C2MessageTransfer *mt)
{
	GtkWidget *info_box = glade_xml_get_widget (mt->xml, "info_box");

	gtk_widget_hide (info_box);
}

static void
ok_btn_clicked (GtkWidget *button, C2MessageTransfer *mt)
{
	gtk_widget_hide (GTK_WIDGET (mt));
}

static void
cancel_btn_clicked (GtkWidget *button, C2MessageTransfer *mt)
{
#ifdef USE_DEBUG
	g_print ("Cancel hasn't been coded: %s:%d\n", __FILE__, __LINE__);
#endif
}

GtkType
c2_message_transfer_get_type (void)
{
	static GtkType type = 0;

	if (!type)
	{
		GtkTypeInfo info =
		{
			"C2MessageTransfer",
			sizeof (C2MessageTransfer),
			sizeof (C2MessageTransferClass),
			(GtkClassInitFunc) class_init,
			(GtkObjectInitFunc) init,
			(GtkArgSetFunc) NULL,
			(GtkArgGetFunc) NULL
		};

		type = gtk_type_unique (gnome_dialog_get_type (), &info);
	}

	return type;
}

static void
class_init (C2MessageTransferClass *klass)
{
	GtkObjectClass *object_class;

	object_class = (GtkObjectClass *) klass;

	parent_class = gtk_type_class (gnome_dialog_get_type ());

	signals[APPEND] =
		gtk_signal_new ("append",
				GTK_RUN_FIRST,
				object_class->type,
				GTK_SIGNAL_OFFSET (C2MessageTransferClass, append),
				gtk_marshal_NONE__ENUM, GTK_TYPE_NONE, 2,
				GTK_TYPE_ENUM, GTK_TYPE_POINTER);
	signals[TASK_DONE] =
		gtk_signal_new ("task_done",
				GTK_RUN_FIRST,
				object_class->type,
				GTK_SIGNAL_OFFSET (C2MessageTransferClass, task_done),
				gtk_marshal_NONE__INT, GTK_TYPE_NONE, 1,
				GTK_TYPE_INT);

	gtk_object_class_add_signals (object_class, signals, LAST_SIGNAL);

	klass->append = NULL;
	klass->task_done = NULL;

	object_class->destroy = destroy;
}

static void
init (C2MessageTransfer *obj)
{
	gchar *buttons[] =
	{
		GNOME_STOCK_BUTTON_OK,
		GNOME_STOCK_BUTTON_CANCEL,
		NULL
	};
	
	obj->tasks[DONE] = obj->tasks[TOTAL] = 0;

	obj->queue = NULL;
	obj->xml = NULL;
	obj->close = FALSE;
	obj->cancel = FALSE;

	pthread_mutex_init (&obj->queue_lock, NULL);

	gnome_dialog_constructv (GNOME_DIALOG (obj), _("Message Transfer"), (const gchar **) buttons);
}

static void
destroy (GtkObject *obj)
{
	C2MessageTransfer *mt = C2_MESSAGE_TRANSFER (obj);

	queue_free (mt->queue);

	gtk_object_destroy (GTK_OBJECT (mt->xml));
}