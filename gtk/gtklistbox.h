/*
 * Copyright (c) 2013 Red Hat, Inc.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or (at your
 * option) any later version.
 *
 * This program is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY
 * or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU Lesser General Public
 * License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public License
 * along with this program; if not, write to the Free Software Foundation,
 * Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA
 *
 * Author: Alexander Larsson <alexl@redhat.com>
 *
 */

#ifndef __GTK_LIST_BOX_H__
#define __GTK_LIST_BOX_H__

#if !defined (__GTK_H_INSIDE__) && !defined (GTK_COMPILATION)
#error "Only <gtk/gtk.h> can be included directly."
#endif

#include <gtk/gtkscrolledwindow.h>

G_BEGIN_DECLS


#define GTK_TYPE_LIST_BOX (gtk_list_box_get_type ())
#define GTK_LIST_BOX(obj) (G_TYPE_CHECK_INSTANCE_CAST ((obj), GTK_TYPE_LIST_BOX, GtkListBox))
#define GTK_LIST_BOX_CLASS(klass) (G_TYPE_CHECK_CLASS_CAST ((klass), GTK_TYPE_LIST_BOX, GtkListBoxClass))
#define GTK_IS_LIST_BOX(obj) (G_TYPE_CHECK_INSTANCE_TYPE ((obj), GTK_TYPE_LIST_BOX))
#define GTK_IS_LIST_BOX_CLASS(klass) (G_TYPE_CHECK_CLASS_TYPE ((klass), GTK_TYPE_LIST_BOX))
#define GTK_LIST_BOX_GET_CLASS(obj) (G_TYPE_INSTANCE_GET_CLASS ((obj), GTK_TYPE_LIST_BOX, GtkListBoxClass))

typedef struct _GtkListBox GtkListBox;
typedef struct _GtkListBoxClass GtkListBoxClass;
typedef struct _GtkListBoxPrivate GtkListBoxPrivate;

typedef struct _GtkListBoxRow GtkListBoxRow;
typedef struct _GtkListBoxRowClass GtkListBoxRowClass;
typedef struct _GtkListBoxRowPrivate GtkListBoxRowPrivate;

struct _GtkListBox
{
  GtkContainer parent_instance;

  GtkListBoxPrivate * priv;
};

struct _GtkListBoxClass
{
  GtkContainerClass parent_class;

  void (*row_selected) (GtkListBox* list_box, GtkListBoxRow* row);
  void (*row_activated) (GtkListBox* list_box, GtkListBoxRow* row);
  void (*activate_cursor_row) (GtkListBox* list_box);
  void (*toggle_cursor_row) (GtkListBox* list_box);
  void (*move_cursor) (GtkListBox* list_box, GtkMovementStep step, gint count);
  void (*refilter) (GtkListBox* list_box);

  /* Padding for future expansion */
  void (*_gtk_reserved1) (void);
  void (*_gtk_reserved2) (void);
  void (*_gtk_reserved3) (void);
  void (*_gtk_reserved4) (void);
  void (*_gtk_reserved5) (void);
  void (*_gtk_reserved6) (void);
};

#define GTK_TYPE_LIST_BOX_ROW (gtk_list_box_row_get_type ())
#define GTK_LIST_BOX_ROW(obj) (G_TYPE_CHECK_INSTANCE_CAST ((obj), GTK_TYPE_LIST_BOX_ROW, GtkListBoxRow))
#define GTK_LIST_BOX_ROW_CLASS(klass) (G_TYPE_CHECK_CLASS_CAST ((klass), GTK_TYPE_LIST_BOX_ROW, GtkListBoxRowClass))
#define GTK_IS_LIST_BOX_ROW(obj) (G_TYPE_CHECK_INSTANCE_TYPE ((obj), GTK_TYPE_LIST_BOX_ROW))
#define GTK_IS_LIST_BOX_ROW_CLASS(klass) (G_TYPE_CHECK_CLASS_TYPE ((klass), GTK_TYPE_LIST_BOX_ROW))
#define GTK_LIST_BOX_ROW_GET_CLASS(obj) (G_TYPE_INSTANCE_GET_CLASS ((obj), GTK_TYPE_LIST_BOX_ROW, GtkListBoxRowClass))

struct _GtkListBoxRow
{
  GtkBin parent_instance;

  GtkListBoxRowPrivate * priv;
};

struct _GtkListBoxRowClass
{
  GtkBinClass parent_class;

  /* Padding for future expansion */
  void (*_gtk_reserved1) (void);
  void (*_gtk_reserved2) (void);
  void (*_gtk_reserved3) (void);
};

typedef gboolean (*GtkListBoxFilterFunc) (GtkListBoxRow* row, gpointer user_data);
typedef gint (*GtkListBoxSortFunc) (GtkListBoxRow* row1, GtkListBoxRow* row2, gpointer user_data);
typedef void (*GtkListBoxUpdateSeparatorFunc) (GtkListBoxRow* row, GtkListBoxRow* before, gpointer user_data);

GDK_AVAILABLE_IN_3_10
GType      gtk_list_box_row_get_type      (void) G_GNUC_CONST;
GDK_AVAILABLE_IN_3_10
GtkWidget* gtk_list_box_row_new           (void);
GDK_AVAILABLE_IN_3_10
GtkWidget* gtk_list_box_row_get_separator (GtkListBoxRow *row);
GDK_AVAILABLE_IN_3_10
void       gtk_list_box_row_set_separator (GtkListBoxRow *row,
                                           GtkWidget     *separator);
GDK_AVAILABLE_IN_3_10
void       gtk_list_box_row_changed       (GtkListBoxRow *row);


GDK_AVAILABLE_IN_3_10
GType          gtk_list_box_get_type                     (void) G_GNUC_CONST;
GDK_AVAILABLE_IN_3_10
GtkListBoxRow* gtk_list_box_get_selected_row             (GtkListBox                    *list_box);
GDK_AVAILABLE_IN_3_10
GtkListBoxRow* gtk_list_box_get_row_at_index             (GtkListBox                    *list_box,
                                                          int                           index);
GDK_AVAILABLE_IN_3_10
GtkListBoxRow* gtk_list_box_get_row_at_y                 (GtkListBox                    *list_box,
                                                          gint                           y);
GDK_AVAILABLE_IN_3_10
void           gtk_list_box_select_row                   (GtkListBox                    *list_box,
                                                          GtkListBoxRow                 *row);
GDK_AVAILABLE_IN_3_10
void           gtk_list_box_set_adjustment               (GtkListBox                    *list_box,
                                                          GtkAdjustment                 *adjustment);
GDK_AVAILABLE_IN_3_10
GtkAdjustment *gtk_list_box_get_adjustment               (GtkListBox                    *list_box);
GDK_AVAILABLE_IN_3_10
void           gtk_list_box_set_selection_mode           (GtkListBox                    *list_box,
                                                          GtkSelectionMode               mode);
GDK_AVAILABLE_IN_3_10
GtkSelectionMode gtk_list_box_get_selection_mode         (GtkListBox                    *list_box);
GDK_AVAILABLE_IN_3_10
void           gtk_list_box_set_filter_func              (GtkListBox                    *list_box,
                                                          GtkListBoxFilterFunc           filter_func,
                                                          gpointer                       user_data,
                                                          GDestroyNotify                 destroy);
GDK_AVAILABLE_IN_3_10
void           gtk_list_box_set_separator_func           (GtkListBox                    *list_box,
                                                          GtkListBoxUpdateSeparatorFunc  update_separator,
                                                          gpointer                       user_data,
                                                          GDestroyNotify                 destroy);
GDK_AVAILABLE_IN_3_10
void           gtk_list_box_refilter                     (GtkListBox                    *list_box);
GDK_AVAILABLE_IN_3_10
void           gtk_list_box_resort                       (GtkListBox                    *list_box);
GDK_AVAILABLE_IN_3_10
void           gtk_list_box_reseparate                   (GtkListBox                    *list_box);
GDK_AVAILABLE_IN_3_10
void           gtk_list_box_set_sort_func                (GtkListBox                    *list_box,
                                                          GtkListBoxSortFunc             sort_func,
                                                          gpointer                       user_data,
                                                          GDestroyNotify                 destroy);
GDK_AVAILABLE_IN_3_10
void           gtk_list_box_set_activate_on_single_click (GtkListBox                    *list_box,
                                                          gboolean                       single);
GDK_AVAILABLE_IN_3_10
void           gtk_list_box_drag_unhighlight_row         (GtkListBox                    *list_box);
GDK_AVAILABLE_IN_3_10
void           gtk_list_box_drag_highlight_row           (GtkListBox                    *list_box,
                                                          GtkListBoxRow                 *row);
GDK_AVAILABLE_IN_3_10
GtkWidget*     gtk_list_box_new                          (void);


G_END_DECLS

#endif
