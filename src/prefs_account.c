/* -*- Mode: C; tab-width: 8; indent-tabs-mode: t; c-basic-offset: 8 -*- */
/*
 * Loqui -- IRC client for Gtk2 <http://loqui.good-day.net/>
 * Copyright (C) 2003 Yoichi Imai <yoichi@silver-forest.com>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.
 */
#include "config.h"
#include "prefs_account.h"
#include "utils.h"
#include "main.h"
#include "intl.h"

#include <stdio.h>
#include <errno.h>
#include <string.h>
#include <glib.h>

#define RC_FILENAME "accountrc.xml"

#define SET_ATTRIBUTE_STR(pref_name, dest) { \
   if(g_strcasecmp(attribute_names[i], pref_name) == 0) { \
       dest = g_strdup(attribute_values[i]); \
       continue; \
  } \
}
#define SET_ATTRIBUTE_INT(pref_name, dest) { \
   if(g_strcasecmp(attribute_names[i], pref_name) == 0) { \
       dest = (gint) g_ascii_strtoull(attribute_values[i], NULL, 10); \
       continue; \
  } \
}

Account *current_account = NULL;
GSList *tmp_list = NULL;

typedef enum {
	ELEMENT_NONE,
	ELEMENT_USERINFO,
	ELEMENT_AUTOJOIN,
	ELEMENT_REALNAME,
} ElementType;

ElementType current_element = ELEMENT_NONE;

static void set_current_account(const gchar **attribute_names,
				const gchar **attribute_values);

static void add_server(const gchar **attribute_names,
		       const gchar **attribute_values);

static void
start_element_handler  (GMarkupParseContext *context,
                        const gchar         *element_name,
                        const gchar        **attribute_names,
                        const gchar        **attribute_values,
                        gpointer             user_data,
                        GError             **error);
static void
end_element_handler    (GMarkupParseContext *context,
                        const gchar         *element_name,
                        gpointer             user_data,
                        GError             **error);
static void
text_handler           (GMarkupParseContext *context,
                        const gchar         *text,
                        gsize                text_len,
                        gpointer             user_data,
                        GError             **error);
static void
error_handler          (GMarkupParseContext *context,
                        GError              *error,
                        gpointer             user_data);

static GMarkupParser parser = {
	start_element_handler,
	end_element_handler,
	text_handler,
	NULL,
	error_handler
};

static void set_current_account(const gchar **attribute_names,
				const gchar **attribute_values)
{
	int i;
	gchar *name = NULL, *nick = NULL, *username = NULL;

	for(i = 0; attribute_names[i] != NULL; i++) {
		SET_ATTRIBUTE_STR("name", name);
		SET_ATTRIBUTE_STR("nick", nick);
		SET_ATTRIBUTE_STR("username", username);
		g_warning(_("prefs_account: Invalid attribute for account: %s"), attribute_names[i]);
	}

#define CHECK_NULL(name, str) { \
  if(!str) { \
   g_warning(_("%s is null"), name); \
   return; \
  } \
}
	CHECK_NULL("name", name);
	CHECK_NULL("nick", nick);
	CHECK_NULL("username", username);

	current_account = account_new();
	account_set(current_account, name, nick, username, NULL, NULL, NULL);

	debug_puts("account: name => \"%s\", nick => \"%s\", username => \"%s\"",
		   name, nick, username);
#undef CHECK_NULL
#undef CHECK_SPACE
}

static void add_server(const gchar **attribute_names,
		       const gchar **attribute_values)
{

	int i;
	gchar *host = NULL, *password = NULL;
	gint port = 0, use = 0;

	if(!current_account) {
		g_warning(_("Current account is not set."));
		return;
	}

	for(i = 0; attribute_names[i] != NULL; i++) {
		SET_ATTRIBUTE_STR("host", host);
		SET_ATTRIBUTE_STR("password", password);
		SET_ATTRIBUTE_INT("port", port);
		SET_ATTRIBUTE_INT("use", use);
		g_warning(_("prefs_account: Invalid attribute for account: %s"), attribute_names[i]);
	}
	if(!host) {
		g_warning(_("Invalid hostname: %s"), host);
		return;
	}

	account_add_server(current_account, host,
			   port, password, (use != 0));
	debug_puts("  add server: host => \"%s\", port => %d, password => \"%s\", use => %d",
		   host, port, password, use);
}

static void
start_element_handler  (GMarkupParseContext *context,
                        const gchar         *element_name,
                        const gchar        **attribute_names,
                        const gchar        **attribute_values,
                        gpointer             user_data,
                        GError             **error)
{
	if(g_ascii_strcasecmp(element_name, "account") == 0) {
		set_current_account(attribute_names, attribute_values);
		return;
	}

	if(current_account == NULL) {
		return;
	}

	if(g_strcasecmp(element_name, "autojoin") == 0) {
		current_element = ELEMENT_AUTOJOIN;
		return;
	}
	if(g_strcasecmp(element_name, "userinfo") == 0) {
		current_element = ELEMENT_USERINFO;
		return;
	}
	if(g_strcasecmp(element_name, "realname") == 0) {
		current_element = ELEMENT_REALNAME;
		return;
	}
	if(g_strcasecmp(element_name, "server") == 0) {
		add_server(attribute_names, attribute_values);
		return;
	}

	g_warning(_("Invalid element: %s"), element_name);
}

static void
end_element_handler    (GMarkupParseContext *context,
                        const gchar         *element_name,
                        gpointer             user_data,
                        GError             **error)
{
	if(g_ascii_strcasecmp(element_name, "account") == 0) {
		debug_puts("End.");
		if(current_account) {
			tmp_list = g_slist_append(tmp_list, current_account);
		}
		current_account = NULL;
	}
	current_element = ELEMENT_NONE;
}

static void
text_handler           (GMarkupParseContext *context,
                        const gchar         *text,
                        gsize                text_len,
                        gpointer             user_data,
                        GError             **error)
{
	if(!current_account)
		return;

	switch(current_element) {
	case ELEMENT_REALNAME:
		account_set_realname(current_account, text);
		debug_puts("  realname: \"%s\"", text);
		break;
	case ELEMENT_USERINFO:
		account_set_userinfo(current_account, text);
		debug_puts("  userinfo: \"%s\"", text);
		break;
	case ELEMENT_AUTOJOIN:
		account_set_autojoin(current_account, text);
		debug_puts("  autojoin: \"%s\"", text);
		break;
	default:
		break;
	}

}
static void
error_handler          (GMarkupParseContext *context,
                        GError              *error,
                        gpointer             user_data)
{
	g_warning(_("Accountrc parse error: %s"), error->message);
}

GSList *prefs_account_load(void)
{
	GSList *slist;
	gchar *contents;
	gchar *path;
	gsize len;
	GError *error = NULL;
	GMarkupParseContext *context;

	tmp_list = NULL;
	current_account = NULL;

	debug_puts("Loading prefs_account...");

	path = g_build_filename(g_get_home_dir(), PREFS_DIR, RC_FILENAME, NULL);

	if(!g_file_get_contents(path, &contents, &len, &error)) {
		if(error->code != G_FILE_ERROR_NOENT)
			g_warning("%s", error->message);
		g_error_free(error);
		return NULL;
	}
	g_free(path);
	error = NULL;

	context = g_markup_parse_context_new(&parser, 0, NULL, NULL);
	
	if(!g_markup_parse_context_parse(context, contents, len, &error)) {
		g_warning(_("Loquirc parse error: %s"), error->message);
		g_error_free(error);
		g_markup_parse_context_free(context);
		return NULL;
	}
	g_markup_parse_context_free(context);

	slist = tmp_list;
	tmp_list = NULL;
	debug_puts("Done.");

	return slist;
}

void prefs_account_save(GSList *account_list)
{
	gchar *path;
	gchar *tmp1, *tmp2, *tmp3;
	gchar *escaped;
	FILE *fp;
	GSList *cur, *cur2;
	Account *account;
	Server *server;

	debug_puts("Saving prefs_account...");

	path = g_build_filename(g_get_home_dir(), PREFS_DIR, RC_FILENAME, NULL);
	if((fp = fopen(path, "w")) == NULL) {
		g_warning(_("Can't open %s: %s"), RC_FILENAME, strerror(errno));
		return;
	}

	/* FIXME: need error checking? */
	fputs("<?xml version=\"1.0\" encoding=\"UTF-8\" ?>\n", fp);
	cur = account_list;
	for(cur = account_list; cur != NULL; cur = cur->next) {
		account = ACCOUNT(cur->data);
		
		tmp1 = g_markup_escape_text(account_get_name(account), -1);
		tmp2 = g_markup_escape_text(account_get_nick(account), -1);
		tmp3 = g_markup_escape_text(account_get_username(account), -1);

		fprintf(fp, "<account name=\"%s\" nick=\"%s\" username=\"%s\">\n", tmp1, tmp2, tmp3);

		g_free(tmp1);
		g_free(tmp2);
		g_free(tmp3);

		escaped = g_markup_escape_text(account_get_realname(account), -1);
		fprintf(fp, " <realname>%s</realname>\n", escaped);
		g_free(escaped);

		if(account_get_userinfo(account)) {
			escaped = g_markup_escape_text(account_get_userinfo(account), -1);
			fprintf(fp, " <userinfo>%s</userinfo>\n", escaped);
			g_free(escaped);
		}

		if(account_get_autojoin(account)) {
			escaped = g_markup_escape_text(account_get_autojoin(account), -1);
			fprintf(fp, " <autojoin>%s</autojoin>\n", escaped);
			g_free(escaped);
		}

		for(cur2 = account->server_list; cur2 != NULL; cur2 = cur2->next) {
			server = (Server *) cur2->data;

			escaped = g_markup_escape_text(server->hostname, -1);
			fprintf(fp, " <server host=\"%s\" port=\"%d\"",
				escaped, server->port);
			g_free(escaped);

			if(server->password) {
				escaped = g_markup_escape_text(server->password, -1);
				fprintf(fp, " password=\"%s\"", escaped);
				g_free(escaped);
			}
			if(server->use != 0) {
				fprintf(fp, " use=\"1\"");
			}
			fprintf(fp, "/>\n");
		}

		fputs("</account>\n\n", fp);
	}

	fclose(fp);

	debug_puts("Done.");
}
