/* GTK - The GIMP Toolkit
 *
 * Copyright (C) 2010 Red Hat, Inc.
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the
 * Free Software Foundation, Inc., 59 Temple Place - Suite 330,
 * Boston, MA 02111-1307, USA.
 *
 * Author: Colin Walters <walters@verbum.org>
 */

/*
 * Modified by the GTK+ Team and others 1997-2000.  See the AUTHORS
 * file for a list of people on the GTK+ Team.  See the ChangeLog
 * files for a list of changes.  These files are distributed with
 * GTK+ at ftp://ftp.gtk.org/pub/gtk/.
 */

#include "config.h"

#include <stdlib.h>
#include <unistd.h>

#include "gtkapplication.h"
#include "gtkmain.h"
#include "gtkintl.h"
#include "gtkprivate.h"

#include "gtkalias.h"

#include <gdk/gdk.h>
#ifdef GDK_WINDOWING_X11
#include <gdk/x11/gdkx.h>
#endif

/**
 * SECTION:gtkapplication
 * @title: GtkApplication
 * @short_description: Application class
 *
 * #GtkApplication is a class that handles many important aspects
 * of a GTK+ application in a convenient fashion, without enforcing
 * a one-size-fits-all application model.
 *
 * Currently, GtkApplication handles application uniqueness, provides
 * some basic scriptability by exporting 'actions', implements some
 * standard actions itself (such as 'Quit') and provides a main window
 * whose life-cycle is automatically tied to the life-cycle of your
 * application.
 *
 * <example id="gtkapplication"><title>A simple application</title>
 * <programlisting>
 * <xi:include xmlns:xi="http://www.w3.org/2001/XInclude" parse="text" href="../../../../gtk/tests/gtk-example-application.c">
 *  <xi:fallback>FIXME: MISSING XINCLUDE CONTENT</xi:fallback>
 * </xi:include>
 * </programlisting>
 * </example>
 */
enum
{
  PROP_0,
  PROP_WINDOW
};

enum
{
  ACTIVATED,

  LAST_SIGNAL
};

static guint gtk_application_signals[LAST_SIGNAL] = { 0 };

struct _GtkApplicationPrivate
{
  char *appid;
  GtkActionGroup *main_actions;

  GtkWindow *default_window;
  GSList *windows;
};

G_DEFINE_TYPE (GtkApplication, gtk_application, G_TYPE_APPLICATION)

static gboolean
gtk_application_default_quit (GApplication *application,
                              guint         timestamp)
{
  gtk_main_quit ();
  return TRUE;
}

static void
gtk_application_default_run (GApplication *application)
{
  gtk_main ();
}

static void
gtk_application_default_prepare_activation (GApplication *application,
					    GVariant     *arguments,
					    GVariant     *platform_data)
{
  GVariantIter iter;
  gchar *key;
  GVariant *value;

  g_variant_iter_init (&iter, platform_data);
  while (g_variant_iter_next (&iter, "{sv}", &key, &value))
    {
      if (strcmp (key, "startup-notification-id") == 0 &&
          strcmp (g_variant_get_type_string (value), "s") == 0)
        gdk_notify_startup_complete_with_id (g_variant_get_string (value, NULL));
      g_free (key);
      g_variant_unref (value);
    }
  
  g_signal_emit (G_OBJECT (application), gtk_application_signals[ACTIVATED], 0, arguments);
}

static void
gtk_application_default_activated (GApplication *application,
                                   GVariant     *arguments)
{
  GtkApplication *app = GTK_APPLICATION (application);

  /* TODO: should we raise the last focused window instead ? */
  if (app->priv->default_window != NULL)
    gtk_window_present (app->priv->default_window);
}

static void
gtk_application_default_action (GApplication *application,
                                const gchar  *action,
                                guint         timestamp)
{
  GtkApplication *app = GTK_APPLICATION (application);
  GList *actions, *iter;

  actions = gtk_action_group_list_actions (app->priv->main_actions);
  for (iter = actions; iter; iter = iter->next)
    {
      GtkAction *gtkaction = iter->data;
      if (strcmp (action, gtk_action_get_name (gtkaction)) == 0)
        {
          /* TODO set timestamp */
          gtk_action_activate (gtkaction);
          break;
        }
    }
  g_list_free (actions);
}

static GVariant *
gtk_application_format_activation_data (void)
{
  const gchar *startup_id = NULL;
  GdkDisplay *display = gdk_display_get_default ();
  GVariantBuilder builder;

  g_variant_builder_init (&builder, G_VARIANT_TYPE ("a{sv}"));

  /* try and get the startup notification id from GDK, the environment
   * or, if everything else failed, fake one.
   */
#ifdef GDK_WINDOWING_X11
  startup_id = gdk_x11_display_get_startup_notification_id (display);
#endif /* GDK_WINDOWING_X11 */

  if (startup_id)
    g_variant_builder_add (&builder, "{sv}", "startup-notification-id",
                           g_variant_new ("s", startup_id));
  return g_variant_builder_end (&builder);
}

/**
 * gtk_application_new:
 * @argc: (allow-none) (inout): System argument count
 * @argv: (allow-none) (inout): System argument vector
 * @appid: System-dependent application identifier
 *
 * Create a new #GtkApplication, or if one has already been initialized
 * in this process, return the existing instance. This function will as
 * a side effect initialize the display system; see gtk_init().
 *
 * For the behavior if this application is running in another process,
 * see g_application_new().
 *
 * Returns: (transfer full): A newly-referenced #GtkApplication
 *
 * Since: 3.0
 */
GtkApplication*
gtk_application_new (gint          *argc,
                     gchar       ***argv,
                     const gchar   *appid)
{
  GtkApplication *app;
  gint argc_for_app;
  gchar **argv_for_app;
  GVariant *platform_data;

  gtk_init (argc, argv);

  if (argc)
    argc_for_app = *argc;
  else
    argc_for_app = 0;
  if (argv)
    argv_for_app = *argv;
  else
    argv_for_app = NULL;

  app = g_object_new (GTK_TYPE_APPLICATION, "appid", appid, NULL);

  platform_data = gtk_application_format_activation_data ();
  g_application_register_with_data (G_APPLICATION (app), argc_for_app, argv_for_app,
				    platform_data);
  g_variant_unref (platform_data);

  return app;
}

static void
on_action_sensitive (GtkAction      *action,
                     GParamSpec     *pspec,
                     GtkApplication *app)
{
  g_application_set_action_enabled (G_APPLICATION (app),
                                    gtk_action_get_name (action),
                                    gtk_action_get_sensitive (action));
}

/**
 * gtk_application_set_action_group:
 * @app: A #GtkApplication
 * @group: A #GtkActionGroup
 *
 * Set @group as this application's global action group.
 * This will ensure the operating system interface uses
 * these actions as follows:
 *
 * <itemizedlist>
 *   <listitem>In GNOME 2 this exposes the actions for scripting.<listitem>
 *   <listitem>In GNOME 3, this function populates the application menu.</listitem>
 *   <listitem>In Windows prior to version 7, this function does nothing.</listitem>
 *   <listitem>In Windows 7, this function adds "Tasks" to the Jump List.</listitem>
 *   <listitem>In Mac OS X, this function extends the Dock menu.</listitem>
 * </itemizedlist>
 *
 * It is an error to call this function more than once.
 *
 * Since: 3.0
 */
void
gtk_application_set_action_group (GtkApplication *app,
                                  GtkActionGroup *group)
{
  GList *actions, *iter;

  g_return_if_fail (GTK_IS_APPLICATION (app));
  g_return_if_fail (app->priv->main_actions == NULL);

  app->priv->main_actions = g_object_ref (group);
  actions = gtk_action_group_list_actions (group);
  for (iter = actions; iter; iter = iter->next)
    {
      GtkAction *action = iter->data;
      g_application_add_action (G_APPLICATION (app),
                                gtk_action_get_name (action),
                                gtk_action_get_tooltip (action));
      g_signal_connect (action, "notify::sensitive",
                        G_CALLBACK (on_action_sensitive), app);
    }
  g_list_free (actions);
}

static gboolean
gtk_application_on_window_destroy (GtkWidget *window,
                                   gpointer   user_data)
{
  GtkApplication *app = GTK_APPLICATION (user_data);

  app->priv->windows = g_slist_remove (app->priv->windows, window);

  if (app->priv->windows == NULL)
    gtk_application_quit (app);

  return FALSE;
}

static gchar *default_title;

/**
 * gtk_application_add_window:
 * @app: a #GtkApplication
 * @window: a toplevel window to add to @app
 *
 * Adds a window to the #GtkApplication.
 *
 * If the user closes all of the windows added to @app, the default
 * behaviour is to call gtk_application_quit().
 *
 * If your application uses only a single toplevel window, you can
 * use gtk_application_get_window().
 *
 * Since: 3.0
 */
void
gtk_application_add_window (GtkApplication *app,
                            GtkWindow      *window)
{
  app->priv->windows = g_slist_prepend (app->priv->windows, window);

  if (gtk_window_get_title (window) == NULL && default_title != NULL)
    gtk_window_set_title (window, default_title);

  g_signal_connect (window, "destroy",
                    G_CALLBACK (gtk_application_on_window_destroy), app);
}

/**
 * gtk_application_get_window:
 * @app: a #GtkApplication
 *
 * A simple #GtkApplication has a "default window". This window should
 * act as the primary user interaction point with your application.
 * The window returned by this function is of type #GTK_WINDOW_TYPE_TOPLEVEL
 * and its properties such as "title" and "icon-name" will be initialized
 * as appropriate for the platform.
 *
 * If the user closes this window, and your application hasn't created
 * any other windows, the default action will be to call gtk_application_quit().
 *
 * If your application has more than one toplevel window (e.g. an
 * single-document-interface application with multiple open documents),
 * or if you are constructing your toplevel windows yourself (e.g. using
 * #GtkBuilder), use gtk_application_add_window() instead.
 *
 * Returns: (transfer none): The default #GtkWindow for this application
 *
 * Since: 3.0
 */
GtkWindow *
gtk_application_get_window (GtkApplication *app)
{
  if (app->priv->default_window != NULL)
    return app->priv->default_window;

  app->priv->default_window = GTK_WINDOW (gtk_window_new (GTK_WINDOW_TOPLEVEL));
  g_object_ref_sink (app->priv->default_window);

  gtk_application_add_window (app, app->priv->default_window);

  return app->priv->default_window;
}

/**
 * gtk_application_run:
 * @app: a #GtkApplication
 *
 * Runs the main loop; see g_application_run().
 * The default implementation for #GtkApplication uses gtk_main().
 *
 * Since: 3.0
 */
void
gtk_application_run (GtkApplication *app)
{
  g_application_run (G_APPLICATION (app));
}

/**
 * gtk_application_quit:
 * @app: a #GtkApplication
 *
 * Request the application exit.
 * By default, this method will exit the main loop; see gtk_main_quit().
 *
 * Since: 3.0
 */
void
gtk_application_quit (GtkApplication *app)
{
  g_application_quit (G_APPLICATION (app), gtk_get_current_event_time ());
}

static void
gtk_application_get_property (GObject    *object,
                              guint       prop_id,
                              GValue     *value,
                              GParamSpec *pspec)
{
  GtkApplication *app = GTK_APPLICATION (object);

  switch (prop_id)
    {
      case PROP_WINDOW:
        g_value_set_object (value, gtk_application_get_window (app));
        break;
      default:
        G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
    }
}

static void
gtk_application_set_property (GObject      *object,
                              guint         prop_id,
                              const GValue *value,
                              GParamSpec   *pspec)
{
  GtkApplication *app = GTK_APPLICATION (object);

  g_assert (app != NULL);

  switch (prop_id)
    {
      default:
        G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
    }
}

static void
setup_default_window_decorations (void)
{
  const gchar *pid;
  const gchar *filename;
  GKeyFile *keyfile;
  gchar *title;
  gchar *icon_name;

  pid = g_getenv ("GIO_LAUNCHED_DESKTOP_FILE_PID");
  filename = g_getenv ("GIO_LAUNCHED_DESKTOP_FILE");

  keyfile = g_key_file_new ();

  if (pid != NULL && filename != NULL && atoi (pid) == getpid () &&
      g_key_file_load_from_file (keyfile, filename, 0, NULL))
    {
      title = g_key_file_get_locale_string (keyfile, G_KEY_FILE_DESKTOP_GROUP, G_KEY_FILE_DESKTOP_KEY_NAME, NULL, NULL);
      icon_name = g_key_file_get_string (keyfile, G_KEY_FILE_DESKTOP_GROUP, G_KEY_FILE_DESKTOP_KEY_ICON, NULL);

      g_print ("default title: %s\n", title);
      g_print ("default icon: %s\n", icon_name);

      if (default_title == NULL)
        default_title = title;

      if (gtk_window_get_default_icon_name () == NULL)
        gtk_window_set_default_icon_name (icon_name);

      g_free (icon_name);
    }

  g_key_file_free (keyfile);
}

static void
gtk_application_init (GtkApplication *application)
{
  application->priv = G_TYPE_INSTANCE_GET_PRIVATE (application, GTK_TYPE_APPLICATION, GtkApplicationPrivate);

  setup_default_window_decorations ();
}


static GObject*
gtk_application_constructor (GType                  type,
                             guint                  n_construct_properties,
                             GObjectConstructParam *construct_params)
{
  GObject *object;

  /* Last ditch effort here */
  gtk_init (0, NULL);

  object = (* G_OBJECT_CLASS (gtk_application_parent_class)->constructor) (type,
                                                                           n_construct_properties,
                                                                           construct_params);

  return object;
}

static void
gtk_application_class_init (GtkApplicationClass *klass)
{
  GObjectClass *gobject_class;
  GApplicationClass *application_class;

  gobject_class = G_OBJECT_CLASS (klass);
  application_class = G_APPLICATION_CLASS (klass);

  gobject_class->constructor = gtk_application_constructor;
  gobject_class->get_property = gtk_application_get_property;
  gobject_class->set_property = gtk_application_set_property;

  application_class->run = gtk_application_default_run;
  application_class->quit = gtk_application_default_quit;
  application_class->action = gtk_application_default_action;
  application_class->prepare_activation = gtk_application_default_prepare_activation;

  klass->activated = gtk_application_default_activated;

  /**
   * GtkApplication::activated:
   * @arguments: A #GVariant with the signature "aay"
   *
   * This signal is emitted when a non-primary process for a given
   * application is invoked while your application is running; for
   * example, when a file browser launches your program to open a
   * file.  The raw operating system arguments are passed in the
   * variant @arguments.
   */

  gtk_application_signals[ACTIVATED] =
    g_signal_new (g_intern_static_string ("activated"),
                  G_OBJECT_CLASS_TYPE (klass),
                  G_SIGNAL_RUN_LAST,
                  G_STRUCT_OFFSET (GtkApplicationClass, activated),
                  NULL, NULL,
                  g_cclosure_marshal_VOID__BOXED,
                  G_TYPE_NONE, 1,
                  G_TYPE_VARIANT);

  g_type_class_add_private (gobject_class, sizeof (GtkApplicationPrivate));
}

#define __GTK_APPLICATION_C__
#include "gtkaliasdef.c"
