/*
 *      about.c - this file is part of Geany, a fast and lightweight IDE
 *
 *      Copyright 2005-2007 Enrico Tröger <enrico.troeger@uvena.de>
 *      Copyright 2006-2007 Nick Treleaven <nick.treleaven@btinternet.com>
 *
 *      This program is free software; you can redistribute it and/or modify
 *      it under the terms of the GNU General Public License as published by
 *      the Free Software Foundation; either version 2 of the License, or
 *      (at your option) any later version.
 *
 *      This program is distributed in the hope that it will be useful,
 *      but WITHOUT ANY WARRANTY; without even the implied warranty of
 *      MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *      GNU General Public License for more details.
 *
 *      You should have received a copy of the GNU General Public License
 *      along with this program; if not, write to the Free Software
 *      Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 *
 * $Id$
 */

/*
 * About dialog and credits.
 */

#include "about.h"
#include "geany.h"
#include "utils.h"
#include "ui_utils.h"
#include "support.h"

static GtkWidget *gb_window = NULL;
#include "gb.c"


#define HEADER "<span size=\"larger\" weight=\"bold\">Geany " VERSION "%s</span>"
#define INFO "<span size=\"larger\" weight=\"bold\">%s</span>"
#define CODENAME "<span weight=\"bold\">\"" GEANY_CODENAME "\"</span>"
#define BUILDDATE "<span size=\"smaller\">%s</span>"
#define COPYRIGHT "Copyright (c)  2005-2007\nEnrico Tröger\nNick Treleaven\nFrank Lanitz\nAll rights reserved."

const gchar *translators[][2] = {
	{ "be_BY", "Yura Semashko &lt;yurand2@gmail.com&gt;" },
	{ "bg", "Dilyan Rusev &lt;dilyanrusev@gmail.com&gt;" },
	{ "ca_ES", "Toni Garcia-Navarro &lt;topi@elpiset.net&gt;" },
	{ "cs_CZ", "Petr Messner &lt;messa@messa.cz&gt;" },
	{ "de_DE", "Enrico Tröger &lt;enrico.troeger@uvena.de&gt;\nFrank Lanitz &lt;frank@frank.uvena.de&gt;" },
	{ "es", "Damián Viano &lt;debian@damianv.com.ar&gt;\nNacho Cabanes &lt;ncabanes@gmail.com&gt;" },
	{ "fi_FI", "Harri Koskinen &lt;harri@fastmonkey.org&gt;" },
	{ "fr", "Jean-Philippe Moal &lt;skateinmars@skateinmars.net&gt;" },
	{ "hu", "Gabor Kmetyko &lt;kg_kilo@freemail.hu&gt;" },
	{ "it", "Max Baldinelli &lt;m.baldinelli@agora.it&gt;,\nDario Santomarco &lt;dariello@yahoo.it&gt;" },
	{ "nl", "Kurt De Bree &lt;kdebree@telenet.be&gt;" },
	{ "pl_PL", "Jarosław Foksa &lt;jfoksa@gmail.com&gt;" },
	{ "pt_BR", "Alexandra Moreire &lt;alexandream@gmail.com&gt;\nAdrovane Marques Kade &lt;adrovane@gmail.com&gt;" },
	{ "ru_RU", "brahmann_ &lt;brahmann@mthr.net.ru&gt;" },
	{ "sv", "Tony Mattsson &lt;superxorn@gmail.com&gt;" },
	{ "vi_VN", "Clytie Siddall &lt;clytie@riverland.net.au&gt;" },
	{ "zh_CN", "Dormouse Young &lt;dormouse.young@gmail.com&gt;" },
	{ "zh_TW", "KoViCH &lt;kovich.ian@gmail.com&gt;" }
};
static const gint translators_len = G_N_ELEMENTS(translators);

const gchar *prev_translators[][2] = {
	{ "pl_PL", "Jacek Wolszczak &lt;shutdownrunner@o2.pl&gt;" },
};
static const gint prev_translators_len = G_N_ELEMENTS(prev_translators);


static void header_eventbox_style_set(GtkWidget *widget);
static void header_label_style_set(GtkWidget *widget);
static void homepage_clicked(GtkButton *button, gpointer data);


#define ROW(text, row, col, x_align, y_padding, col_span) \
	label = gtk_label_new((text)); \
	gtk_table_attach(GTK_TABLE(table), label, (col), (col) + (col_span) + 1, (row), (row) + 1, \
			(GtkAttachOptions) (GTK_FILL), (GtkAttachOptions) (GTK_FILL), 0, (y_padding)); \
	gtk_label_set_use_markup(GTK_LABEL(label), TRUE); \
	gtk_misc_set_alignment(GTK_MISC(label), (x_align), 0);


static GtkWidget *create_dialog(void)
{
	GtkWidget *dialog;
	GtkWidget *header_image;
	GtkWidget *header_label;
	GtkWidget *label_info;
	GtkWidget *codename_label;
	GtkWidget *builddate_label;
	GtkWidget *url_button;
	GtkWidget *cop_label;
	GtkWidget *label;
	GtkWidget *license_textview;
	GtkWidget *notebook;
	GtkWidget *box;
	GtkWidget *credits_scrollwin;
	GtkWidget *table;
	GtkWidget *license_scrollwin;
	GtkWidget *info_box;
	GtkWidget *button;
	GtkWidget *header_hbox;
	GtkWidget *header_eventbox;
	GdkPixbuf *icon;
	GtkTextBuffer* tb;
	gchar *license_text = NULL;
	gchar buffer[512];
	gchar buffer2[128];
	gint i, row = 0;

	dialog = gtk_dialog_new();

	// configure dialog
	gtk_window_set_transient_for(GTK_WINDOW(dialog), GTK_WINDOW(app->window));
	gtk_window_set_position(GTK_WINDOW(dialog), GTK_WIN_POS_CENTER_ON_PARENT);
	gtk_window_set_title(GTK_WINDOW(dialog), _("About Geany"));
	gtk_widget_set_name(dialog, "GeanyDialog");
	button = gtk_dialog_add_button(GTK_DIALOG(dialog), GTK_STOCK_CLOSE, GTK_RESPONSE_CLOSE);
	gtk_dialog_set_default_response(GTK_DIALOG(dialog), GTK_RESPONSE_CLOSE);
	gtk_dialog_set_has_separator(GTK_DIALOG(dialog), FALSE);
	g_signal_connect(G_OBJECT(dialog), "key-press-event", G_CALLBACK(gb_on_key_pressed), NULL);

	// create header
	header_eventbox = gtk_event_box_new();
	gtk_widget_show(header_eventbox);
	header_hbox = gtk_hbox_new(FALSE, 12);
	gtk_container_set_border_width(GTK_CONTAINER(header_hbox), 4);
	gtk_widget_show(header_hbox);
	gtk_container_add(GTK_CONTAINER(header_eventbox), header_hbox);
	header_image = gtk_image_new();
	gtk_box_pack_start(GTK_BOX(header_hbox), header_image, FALSE,FALSE,0);
	header_label = gtk_label_new(NULL);
	gtk_label_set_use_markup(GTK_LABEL(header_label), TRUE);
	// print the subversion revision generated by ./configure if it is available
	g_snprintf(buffer, sizeof(buffer), HEADER,
		(utils_str_equal(REVISION, "-1")) ? "" : "svn >= " REVISION);
	gtk_label_set_markup(GTK_LABEL(header_label), buffer);
	gtk_widget_show(header_label);
	gtk_box_pack_start(GTK_BOX(header_hbox), header_label, FALSE,FALSE,0);
	header_eventbox_style_set(header_eventbox);
	header_label_style_set(header_label);
	g_signal_connect_after(G_OBJECT(header_eventbox), "style_set", G_CALLBACK(header_eventbox_style_set), NULL);
	g_signal_connect_after(G_OBJECT(header_label), "style_set", G_CALLBACK(header_label_style_set), NULL);
	gtk_box_pack_start(GTK_BOX(GTK_DIALOG(dialog)->vbox), header_eventbox, FALSE, FALSE, 0);

	// set image
	icon = ui_new_pixbuf_from_inline(GEANY_IMAGE_LOGO, FALSE);
	gtk_image_set_from_pixbuf(GTK_IMAGE(header_image), icon);
	gtk_window_set_icon(GTK_WINDOW(dialog), icon);
	g_object_unref(icon);	// free our reference

	// create notebook
	notebook = gtk_notebook_new();
	gtk_widget_show(notebook);
	gtk_container_set_border_width(GTK_CONTAINER(notebook), 2);
	gtk_box_pack_start(GTK_BOX(GTK_DIALOG(dialog)->vbox), notebook, TRUE, TRUE, 0);

	// create "Info" tab
	info_box = gtk_vbox_new(FALSE, 0);
	gtk_container_set_border_width(GTK_CONTAINER(info_box), 6);
	gtk_widget_show(info_box);

	label_info = gtk_label_new(NULL);
	gtk_label_set_justify(GTK_LABEL(label_info), GTK_JUSTIFY_CENTER);
	gtk_label_set_selectable(GTK_LABEL(label_info), TRUE);
	gtk_label_set_use_markup(GTK_LABEL(label_info), TRUE);
	g_snprintf(buffer, sizeof(buffer), INFO, _("A fast and lightweight IDE"));
	gtk_label_set_markup(GTK_LABEL(label_info), buffer);
	gtk_misc_set_padding(GTK_MISC(label_info), 2, 11);
	gtk_widget_show(label_info);
	gtk_box_pack_start(GTK_BOX(info_box), label_info, FALSE, FALSE, 0);

	// Codename label
	codename_label = gtk_label_new(NULL);
	gtk_label_set_justify(GTK_LABEL(codename_label), GTK_JUSTIFY_CENTER);
	gtk_label_set_selectable(GTK_LABEL(codename_label), TRUE);
	gtk_label_set_use_markup(GTK_LABEL(codename_label), TRUE);
	gtk_label_set_markup(GTK_LABEL(codename_label), CODENAME);
	gtk_misc_set_padding(GTK_MISC(codename_label), 2, 8);
	gtk_widget_show(codename_label);
	gtk_box_pack_start(GTK_BOX(info_box), codename_label, FALSE, FALSE, 0);

	// build date label
	builddate_label = gtk_label_new(NULL);
	gtk_label_set_justify(GTK_LABEL(builddate_label), GTK_JUSTIFY_CENTER);
	gtk_label_set_selectable(GTK_LABEL(builddate_label), TRUE);
	gtk_label_set_use_markup(GTK_LABEL(builddate_label), TRUE);
	g_snprintf(buffer2, sizeof(buffer2), _("(built on or after %s)"), __DATE__);
	g_snprintf(buffer, sizeof(buffer), BUILDDATE, buffer2);
	gtk_label_set_markup(GTK_LABEL(builddate_label), buffer);
	gtk_misc_set_padding(GTK_MISC(builddate_label), 2, 2);
	gtk_widget_show(builddate_label);
	gtk_box_pack_start(GTK_BOX(info_box), builddate_label, FALSE, FALSE, 0);

	box = gtk_hbutton_box_new();
	url_button = gtk_button_new();
	gtk_button_set_relief(GTK_BUTTON(url_button), GTK_RELIEF_NONE);
	g_signal_connect(G_OBJECT(url_button), "clicked", G_CALLBACK(homepage_clicked), GEANY_HOMEPAGE);
	label = gtk_label_new(NULL);
	gtk_label_set_text(GTK_LABEL(label), GEANY_HOMEPAGE);
	gtk_widget_show(label);
	gtk_container_add(GTK_CONTAINER(url_button), label);
	gtk_widget_show(url_button);
	gtk_box_pack_start(GTK_BOX(box), url_button, FALSE, FALSE, 0);
	gtk_widget_show(box);
	gtk_box_pack_start(GTK_BOX(info_box), box, FALSE, FALSE, 10);

	// copyright label
	cop_label = gtk_label_new(NULL);
	gtk_label_set_justify(GTK_LABEL(cop_label), GTK_JUSTIFY_CENTER);
	gtk_label_set_selectable(GTK_LABEL(cop_label), FALSE);
	gtk_label_set_use_markup(GTK_LABEL(cop_label), TRUE);
	gtk_label_set_markup(GTK_LABEL(cop_label), COPYRIGHT);
	gtk_misc_set_padding(GTK_MISC(cop_label), 2, 10);
	gtk_widget_show(cop_label);
	gtk_box_pack_start(GTK_BOX(info_box), cop_label, FALSE, FALSE, 0);
	//gtk_container_add(GTK_CONTAINER(info_box), cop_label);

	label = gtk_label_new(_("Info"));
	gtk_widget_show(label);
	gtk_widget_show_all(info_box);
	gtk_notebook_append_page(GTK_NOTEBOOK(notebook), info_box, label);

	// create "Credits" tab
	credits_scrollwin = gtk_scrolled_window_new(NULL, NULL);
	gtk_container_set_border_width(GTK_CONTAINER(credits_scrollwin), 10);
	gtk_scrolled_window_set_policy(GTK_SCROLLED_WINDOW(credits_scrollwin),
		GTK_POLICY_NEVER, GTK_POLICY_AUTOMATIC);

	table = gtk_table_new(14 + translators_len + prev_translators_len, 3, FALSE);
	gtk_table_set_col_spacings(GTK_TABLE(table), 10);

	row = 0;
	g_snprintf(buffer, sizeof(buffer),
		"<span size=\"larger\" weight=\"bold\">%s</span>", _("Developers"));
	label = gtk_label_new(buffer);
	gtk_table_attach(GTK_TABLE(table), label, 0, 2, row, row + 1,
					(GtkAttachOptions) (GTK_FILL),
					(GtkAttachOptions) (0), 0, 5);
	gtk_label_set_use_markup(GTK_LABEL(label), TRUE);
	gtk_misc_set_alignment(GTK_MISC(label), 0, 0);
	row++;

	g_snprintf(buffer, sizeof(buffer), "Enrico Tröger - %s", _("maintainer"));
	ROW(buffer, row, 0, 0, 0, 1);
	row++;
	ROW("&lt;enrico.troeger@uvena.de&gt;", row, 0, 0, 0, 1);
	row++;
	ROW("", row, 0, 0, 0, 0);
	row++;

	g_snprintf(buffer, sizeof(buffer), "Nick Treleaven - %s", _("developer"));
	ROW(buffer, row, 0, 0, 0, 1);
	row++;
	ROW("&lt;nick.treleaven@btinternet.com&gt;", row, 0, 0, 0, 1);
	row++;
	ROW("", row, 0, 0, 0, 0);
	row++;

	g_snprintf(buffer, sizeof(buffer), "Frank Lanitz - %s", _("translation maintainer"));
	ROW(buffer, 7, 0, 0, 0, 1);
	row++;
	ROW("&lt;frank@frank.uvena.de&gt;", row, 0, 0, 0, 1);
	row++;
	ROW("", row, 0, 0, 0, 0);
	row++;

	g_snprintf(buffer, sizeof(buffer),
		"<span size=\"larger\" weight=\"bold\">%s</span>", _("Translators"));
	label = gtk_label_new(buffer);
	gtk_table_attach(GTK_TABLE(table), label, 0, 2, row, row + 1,
					(GtkAttachOptions) (GTK_FILL),
					(GtkAttachOptions) (0), 0, 5);
	gtk_label_set_use_markup(GTK_LABEL(label), TRUE);
	gtk_misc_set_alignment(GTK_MISC(label), 0, 0.5);
	row++;

	for (i = 0; i < translators_len; i++)
	{
		ROW(translators[i][0], row, 0, 1, 4, 0);
		ROW(translators[i][1], row, 1, 0, 4, 0);
		row++;
	}

	ROW("", row, 0, 0, 0, 0);
	row++;

	g_snprintf(buffer, sizeof(buffer),
		"<span size=\"larger\" weight=\"bold\">%s</span>", _("Previous Translators"));
	label = gtk_label_new(buffer);
	gtk_table_attach(GTK_TABLE(table), label, 0, 2, row, row + 1,
					(GtkAttachOptions) (GTK_FILL),
					(GtkAttachOptions) (0), 0, 5);
	gtk_label_set_use_markup(GTK_LABEL(label), TRUE);
	gtk_misc_set_alignment(GTK_MISC(label), 0, 0.5);
	row++;

	for (i = 0; i < prev_translators_len; i++)
	{
		ROW(prev_translators[i][0], row, 0, 1, 4, 0);
		ROW(prev_translators[i][1], row, 1, 0, 4, 0);
		row++;
	}

	gtk_scrolled_window_add_with_viewport(GTK_SCROLLED_WINDOW(credits_scrollwin), table);
	gtk_viewport_set_shadow_type(GTK_VIEWPORT(gtk_widget_get_parent(table)), GTK_SHADOW_NONE);
	gtk_widget_show_all(table);
	label = gtk_label_new(_("Credits"));
	gtk_widget_show(label);
	gtk_notebook_append_page(GTK_NOTEBOOK(notebook), credits_scrollwin, label);

	// create "License" tab
	license_scrollwin = gtk_scrolled_window_new(NULL, NULL);
	gtk_container_set_border_width(GTK_CONTAINER(license_scrollwin), 6);
	gtk_scrolled_window_set_policy(GTK_SCROLLED_WINDOW(license_scrollwin),
		GTK_POLICY_AUTOMATIC, GTK_POLICY_AUTOMATIC);
	license_textview = gtk_text_view_new();
	gtk_text_view_set_editable(GTK_TEXT_VIEW(license_textview), FALSE);
	gtk_text_view_set_cursor_visible(GTK_TEXT_VIEW(license_textview), FALSE);
	gtk_widget_show(license_textview);
	gtk_container_add(GTK_CONTAINER(license_scrollwin), license_textview);
	label = gtk_label_new(_("License"));
	gtk_widget_show(label);

	g_snprintf(buffer, sizeof(buffer), "%s" G_DIR_SEPARATOR_S "GPL-2", app->datadir);

	g_file_get_contents(buffer, &license_text, NULL, NULL);
	if (license_text == NULL)
	{
		license_text = g_strdup(
			_("License text could not be found, please visit http://www.gnu.org/licenses/gpl-2.0.txt to view it online."));
	}
	tb = gtk_text_view_get_buffer(GTK_TEXT_VIEW(license_textview));
	gtk_text_buffer_set_text(tb, license_text, strlen(license_text));

	g_free(license_text);

	gtk_notebook_append_page(GTK_NOTEBOOK(notebook), license_scrollwin, label);

	gtk_widget_show_all(dialog);
	return dialog;
}


void about_dialog_show(void)
{
	GtkWidget *dialog;

	dialog = create_dialog();

	gtk_dialog_run(GTK_DIALOG(dialog));
	gtk_widget_destroy(dialog);
}


static void header_eventbox_style_set(GtkWidget *widget)
{
	static gint recursive = 0;
	GtkStyle *style;

	if (recursive > 0)
		return;

	++recursive;
	style = gtk_widget_get_style(widget);
	gtk_widget_modify_bg(widget, GTK_STATE_NORMAL, &style->bg[GTK_STATE_SELECTED]);
	--recursive;
}


static void header_label_style_set(GtkWidget *widget)
{
	static gint recursive = 0;
	GtkStyle *style;

	if (recursive > 0)
		return;

	++recursive;
	style = gtk_widget_get_style(widget);
	gtk_widget_modify_fg(widget, GTK_STATE_NORMAL, &style->fg[GTK_STATE_SELECTED]);
	--recursive;
}


static void homepage_clicked(GtkButton *button, gpointer data)
{
	utils_start_browser(data);
}

