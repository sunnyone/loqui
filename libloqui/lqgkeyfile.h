/* gkeyfile.h - desktop entry file parser
 *
 *  Copyright 2004 Red Hat, Inc.
 *
 *  Ray Strode <halfline@hawaii.rr.com>
 *
 * GLib is free software; you can redistribute it and/or modify it
 * under the terms of the GNU Lesser General Public License as
 * published by the Free Software Foundation; either version 2 of the
 * License, or (at your option) any later version.
 *
 * GLib is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with GLib; see the file COPYING.LIB.  If not,
 * write to the Free Software Foundation, Inc., 59 Temple Place - Suite 330,
 *   Boston, MA 02111-1307, USA.
 */
/* backported for glib-2.4. 2004-12-30 Yoichi Imai <yoichi@silver-forest.com>
   locale-related functions do not work. */

#ifndef __LQG_KEY_FILE_H__
#define __LQG_KEY_FILE_H__

#include <glib/gerror.h>

#ifndef G_GNUC_MALLOC
#define G_GNUC_MALLOC 
#endif

G_BEGIN_DECLS

typedef enum
{
  LQG_KEY_FILE_ERROR_UNKNOWN_ENCODING,
  LQG_KEY_FILE_ERROR_PARSE,
  LQG_KEY_FILE_ERROR_NOT_FOUND,
  LQG_KEY_FILE_ERROR_KEY_NOT_FOUND,
  LQG_KEY_FILE_ERROR_GROUP_NOT_FOUND,
  LQG_KEY_FILE_ERROR_INVALID_VALUE
} LqGKeyFileError;

#define LQG_KEY_FILE_ERROR lqg_key_file_error_quark()

GQuark lqg_key_file_error_quark (void);

typedef struct _LqGKeyFile LqGKeyFile;

typedef enum
{
  LQG_KEY_FILE_NONE              = 0,
  LQG_KEY_FILE_KEEP_COMMENTS     = 1 << 0,
  LQG_KEY_FILE_KEEP_TRANSLATIONS = 1 << 1
} LqGKeyFileFlags;

LqGKeyFile *lqg_key_file_new                    (void);
void      lqg_key_file_free                   (LqGKeyFile             *key_file);
void      lqg_key_file_set_list_separator     (LqGKeyFile             *key_file,
					     gchar                 separator);
gboolean  lqg_key_file_load_from_file         (LqGKeyFile             *key_file,
					     const gchar          *file,
					     LqGKeyFileFlags         flags,
					     GError              **error);
gboolean  lqg_key_file_load_from_data         (LqGKeyFile             *key_file,
					     const gchar          *data,
					     gsize                 length,
					     LqGKeyFileFlags         flags,
					     GError              **error);
gboolean lqg_key_file_load_from_data_dirs    (LqGKeyFile             *key_file,
					     const gchar          *file,
					     gchar               **full_path,
					     LqGKeyFileFlags         flags,
					     GError              **error);
gchar    *lqg_key_file_to_data                (LqGKeyFile             *key_file,
					     gsize                *length,
					     GError              **error) G_GNUC_MALLOC;
gchar    *lqg_key_file_get_start_group        (LqGKeyFile             *key_file) G_GNUC_MALLOC;
gchar   **lqg_key_file_get_groups             (LqGKeyFile             *key_file,
					     gsize                *length) G_GNUC_MALLOC;
gchar   **lqg_key_file_get_keys               (LqGKeyFile             *key_file,
					     const gchar          *group_name,
					     gsize                *length,
					     GError              **error) G_GNUC_MALLOC;
gboolean  lqg_key_file_has_group              (LqGKeyFile             *key_file,
					     const gchar          *group_name);
gboolean  lqg_key_file_has_key                (LqGKeyFile             *key_file,
					     const gchar          *group_name,
					     const gchar          *key,
					     GError              **error);
gchar    *lqg_key_file_get_value              (LqGKeyFile             *key_file,
					     const gchar          *group_name,
					     const gchar          *key,
					     GError              **error) G_GNUC_MALLOC;
void      lqg_key_file_set_value              (LqGKeyFile             *key_file,
					     const gchar          *group_name,
					     const gchar          *key,
					     const gchar          *value);
gchar    *lqg_key_file_get_string             (LqGKeyFile             *key_file,
					     const gchar          *group_name,
					     const gchar          *key,
					     GError              **error) G_GNUC_MALLOC;
void      lqg_key_file_set_string             (LqGKeyFile             *key_file,
					     const gchar          *group_name,
					     const gchar          *key,
					     const gchar          *string);
gboolean  lqg_key_file_get_boolean            (LqGKeyFile             *key_file,
					     const gchar          *group_name,
					     const gchar          *key,
					     GError              **error);
void      lqg_key_file_set_boolean            (LqGKeyFile             *key_file,
					     const gchar          *group_name,
					     const gchar          *key,
					     gboolean              value);
gint      lqg_key_file_get_integer            (LqGKeyFile             *key_file,
					     const gchar          *group_name,
					     const gchar          *key,
					     GError              **error);
void      lqg_key_file_set_integer            (LqGKeyFile             *key_file,
					     const gchar          *group_name,
					     const gchar          *key,
					     gint                  value);
gchar   **lqg_key_file_get_string_list        (LqGKeyFile             *key_file,
					     const gchar          *group_name,
					     const gchar          *key,
					     gsize                *length,
					     GError              **error) G_GNUC_MALLOC;
void      lqg_key_file_set_string_list        (LqGKeyFile             *key_file,
					     const gchar          *group_name,
					     const gchar          *key,
					     const gchar * const   list[],
					     gsize                 length);
gboolean *lqg_key_file_get_boolean_list       (LqGKeyFile             *key_file,
					     const gchar          *group_name,
					     const gchar          *key,
					     gsize                *length,
					     GError              **error) G_GNUC_MALLOC;
void      lqg_key_file_set_boolean_list       (LqGKeyFile             *key_file,
					     const gchar          *group_name,
					     const gchar          *key,
					     gboolean              list[],
					     gsize                 length);
gint     *lqg_key_file_get_integer_list       (LqGKeyFile             *key_file,
					     const gchar          *group_name,
					     const gchar          *key,
					     gsize                *length,
					     GError              **error) G_GNUC_MALLOC;
void      lqg_key_file_set_integer_list       (LqGKeyFile             *key_file,
					     const gchar          *group_name,
					     const gchar          *key,
					     gint                  list[],
					     gsize                 length);
void      lqg_key_file_set_comment            (LqGKeyFile             *key_file,
                                             const gchar          *group_name,
                                             const gchar          *key,
                                             const gchar          *comment,
                                             GError              **error);
gchar    *lqg_key_file_get_comment            (LqGKeyFile             *key_file,
                                             const gchar          *group_name,
                                             const gchar          *key,
                                             GError              **error) G_GNUC_MALLOC;

void      lqg_key_file_remove_comment         (LqGKeyFile             *key_file,
                                             const gchar          *group_name,
                                             const gchar          *key,
					     GError              **error);
void      lqg_key_file_remove_key             (LqGKeyFile             *key_file,
					     const gchar          *group_name,
					     const gchar          *key,
					     GError              **error);
void      lqg_key_file_remove_group           (LqGKeyFile             *key_file,
					     const gchar          *group_name,
					     GError              **error);

G_END_DECLS

#endif /* __LQG_KEY_FILE_H__ */
