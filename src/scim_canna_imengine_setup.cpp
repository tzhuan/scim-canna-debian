/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 *  Copyright (C) 2004 Hiroyuki Ikezoe
 *  Copyright (C) 2004 - 2005 Takuro Ashie
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2, or (at your option)
 *  any later version.
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

/*
 * Based on scim-hangul.
 * Copyright (c) 2004 James Su <suzhe@turbolinux.com.cn>
 */

#define Uses_SCIM_CONFIG_BASE
#define Uses_SCIM_EVENT

#include <string.h>
#include <gtk/gtk.h>
#include <gdk/gdkkeysyms.h>

#ifdef HAVE_CONFIG_H
  #include <config.h>
#endif

#include <scim.h>
#include <gtk/scimkeyselection.h>
#include "scim_canna_prefs.h"
#include "intl.h"

using namespace scim;

#define scim_module_init canna_imengine_setup_LTX_scim_module_init
#define scim_module_exit canna_imengine_setup_LTX_scim_module_exit

#define scim_setup_module_create_ui       canna_imengine_setup_LTX_scim_setup_module_create_ui
#define scim_setup_module_get_category    canna_imengine_setup_LTX_scim_setup_module_get_category
#define scim_setup_module_get_name        canna_imengine_setup_LTX_scim_setup_module_get_name
#define scim_setup_module_get_description canna_imengine_setup_LTX_scim_setup_module_get_description
#define scim_setup_module_load_config     canna_imengine_setup_LTX_scim_setup_module_load_config
#define scim_setup_module_save_config     canna_imengine_setup_LTX_scim_setup_module_save_config
#define scim_setup_module_query_changed   canna_imengine_setup_LTX_scim_setup_module_query_changed

#define DATA_POINTER_KEY "scim-canna::ConfigPointer"

static GtkWidget * create_setup_window ();
static void        load_config (const ConfigPointer &config);
static void        save_config (const ConfigPointer &config);
static bool        query_changed ();

// Module Interface.
extern "C" {
    void scim_module_init (void)
    {
        bindtextdomain (GETTEXT_PACKAGE, SCIM_CANNA_LOCALEDIR);
        bind_textdomain_codeset (GETTEXT_PACKAGE, "UTF-8");
    }

    void scim_module_exit (void)
    {
    }

    GtkWidget * scim_setup_module_create_ui (void)
    {
        return create_setup_window ();
    }

    String scim_setup_module_get_category (void)
    {
        return String ("IMEngine");
    }

    String scim_setup_module_get_name (void)
    {
        return String (_("Canna"));
    }

    String scim_setup_module_get_description (void)
    {
        return String (_("A Canna IMEngine Module."));
    }

    void scim_setup_module_load_config (const ConfigPointer &config)
    {
        load_config (config);
    }

    void scim_setup_module_save_config (const ConfigPointer &config)
    {
        save_config (config);
    }

    bool scim_setup_module_query_changed ()
    {
        return query_changed ();
    }
} // extern "C"


// Internal data structure
struct BoolConfigData
{
    const char *key;
    bool        value;
    const char *label;
    const char *title;
    const char *tooltip;
    GtkWidget  *widget;
    bool        changed;
};

struct StringConfigData
{
    const char *key;
    String      value;
    const char *label;
    const char *title;
    const char *tooltip;
    GtkWidget  *widget;
    bool        changed;
};

struct KeyboardConfigPage
{
    const char       *label;
    StringConfigData *data;
};

struct ComboConfigData
{
    const char *label;
    const char *data;
};

struct ComboConfigCandidate
{
    const char *label;
    const char *data;
};

// Internal data declaration.
static bool __have_changed    = true;

static GtkTooltips * __widget_tooltips = 0;

static BoolConfigData __config_bool_common [] =
{
    {
        SCIM_CANNA_CONFIG_SPECIFY_INIT_FILE_NAME,
        SCIM_CANNA_CONFIG_SPECIFY_INIT_FILE_NAME_DEFAULT,
        N_("Specify Canna initialize file"),
        NULL,
        NULL,
        NULL,
        false,
    },
    {
        SCIM_CANNA_CONFIG_SPECIFY_SERVER_NAME,
        SCIM_CANNA_CONFIG_SPECIFY_SERVER_NAME_DEFAULT,
        N_("Specify Canna server name"),
        NULL,
        NULL,
        NULL,
        false,
    },
};
static unsigned int __config_bool_common_num = sizeof (__config_bool_common) / sizeof (BoolConfigData);

static StringConfigData __config_string_common [] =
{
    {
        SCIM_CANNA_CONFIG_INIT_FILE_NAME,
        SCIM_CANNA_CONFIG_INIT_FILE_NAME_DEFAULT,
        NULL,
        NULL,
        N_("The Canna initialize file name to use."),
        NULL,
        false,
    },
    {
        SCIM_CANNA_CONFIG_SERVER_NAME,
        SCIM_CANNA_CONFIG_SERVER_NAME_DEFAULT,
        NULL,
        NULL,
        N_("The Canna host name to connect."),
        NULL,
        false,
    },
    {
        SCIM_CANNA_CONFIG_ON_OFF,
        SCIM_CANNA_CONFIG_ON_OFF_DEFAULT,
        N_("Default mode:"),
        NULL,
        NULL,
        NULL,
        false,
    },
    {
        SCIM_CANNA_CONFIG_ON_OFF_KEY,
        SCIM_CANNA_CONFIG_ON_OFF_KEY_DEFAULT,
        N_("On/Off key:"),
        NULL,
        NULL,
        NULL,
        false,
    },
};
static unsigned int __config_string_common_num = sizeof (__config_string_common) / sizeof (StringConfigData);

static ComboConfigCandidate on_off_modes[] =
{
    {N_("Japanese On"),  "On"},
    {N_("Japanese Off"), "Off"},
    {NULL, NULL},
};


static void on_default_editable_changed       (GtkEditable     *editable,
                                               gpointer         user_data);
static void on_default_toggle_button_toggled  (GtkToggleButton *togglebutton,
                                               gpointer         user_data);
static void on_toggle_button_toggled_set_sensitive
                                              (GtkToggleButton *togglebutton,
                                               gpointer         user_data);
static void on_default_combo_changed          (GtkEditable     *editable,
                                               gpointer         user_data);
static void on_default_key_selection_clicked  (GtkButton       *button,
                                               gpointer         user_data);
static void setup_widget_value ();


static BoolConfigData *
find_bool_config_entry (const char *config_key)
{
    if (!config_key)
        return NULL;

    for (unsigned int i = 0; i < __config_bool_common_num; i++) {
        BoolConfigData *entry = &__config_bool_common[i];
        if (entry->key && !strcmp (entry->key, config_key))
            return entry;
    }

    return NULL;
}

static StringConfigData *
find_string_config_entry (const char *config_key)
{
    if (!config_key)
        return NULL;

    for (unsigned int i = 0; i < __config_string_common_num; i++) {
        StringConfigData *entry = &__config_string_common[i];
        if (entry->key && !strcmp (entry->key, config_key))
            return entry;
    }

    return NULL;
}

static GtkWidget *
create_check_button (const char *config_key)
{
    BoolConfigData *entry = find_bool_config_entry (config_key);
    if (!entry)
        return NULL;

    entry->widget = gtk_check_button_new_with_mnemonic (_(entry->label));
    gtk_container_set_border_width (GTK_CONTAINER (entry->widget), 4);
    g_signal_connect (G_OBJECT (entry->widget), "toggled",
                      G_CALLBACK (on_default_toggle_button_toggled),
                      entry);
    gtk_widget_show (entry->widget);

    if (!__widget_tooltips)
        __widget_tooltips = gtk_tooltips_new();
    if (entry->tooltip)
        gtk_tooltips_set_tip (__widget_tooltips, entry->widget,
                              _(entry->tooltip), NULL);

    return entry->widget;
}

GtkWidget *
create_entry (const char *config_key, GtkTable *table, int idx)
{
    StringConfigData *entry = find_string_config_entry (config_key);
    if (!entry)
        return NULL;

    (entry)->widget = gtk_entry_new ();
    GtkWidget *label = NULL;
    if (_(entry->label) && *_(entry->label)) {
        label = gtk_label_new (NULL);
        gtk_label_set_text_with_mnemonic (GTK_LABEL (label), _(entry->label));
        gtk_widget_show (label);
        gtk_misc_set_alignment (GTK_MISC (label), 0.0, 0.5);
        gtk_misc_set_padding (GTK_MISC (label), 4, 0);
        gtk_table_attach (GTK_TABLE (table), label, 0, 1, idx, idx + 1,
                          (GtkAttachOptions) (GTK_FILL),
                          (GtkAttachOptions) (GTK_FILL), 4, 4);
        gtk_label_set_mnemonic_widget (GTK_LABEL (label),
                                       GTK_WIDGET (entry->widget));
    }

    g_signal_connect ((gpointer) (entry)->widget, "changed",
                      G_CALLBACK (on_default_editable_changed),
                      entry);
    gtk_widget_show (GTK_WIDGET (entry->widget));
    gtk_table_attach (GTK_TABLE (table), GTK_WIDGET (entry->widget),
                      1, 2, idx, idx + 1,
                      (GtkAttachOptions) (GTK_FILL|GTK_EXPAND),
                      (GtkAttachOptions) (GTK_FILL), 4, 4);

    if (!__widget_tooltips)
        __widget_tooltips = gtk_tooltips_new();
    if (entry->tooltip)
        gtk_tooltips_set_tip (__widget_tooltips, GTK_WIDGET (entry->widget),
                              _(entry->tooltip), NULL);

    return GTK_WIDGET (entry->widget);
}

GtkWidget *
create_combo (const char *config_key, gpointer candidates_p,
              GtkTable *table, gint idx)
{
    StringConfigData *entry = find_string_config_entry (config_key);
    if (!entry)
        return NULL;

    GtkWidget *label;

    label = gtk_label_new_with_mnemonic (_(entry->label));
    gtk_misc_set_alignment (GTK_MISC (label), 0.0, 0.5);
    gtk_misc_set_padding (GTK_MISC (label), 4, 0);
    gtk_table_attach (GTK_TABLE (table), label, 0, 1, idx, idx + 1,
                      (GtkAttachOptions) (GTK_FILL),
                      (GtkAttachOptions) (GTK_FILL), 4, 4);
    gtk_widget_show (label);

    entry->widget = gtk_combo_new ();
    gtk_label_set_mnemonic_widget (GTK_LABEL (label),
                                   GTK_COMBO (entry->widget)->entry);
    gtk_combo_set_value_in_list (GTK_COMBO (entry->widget), TRUE, FALSE);
    gtk_combo_set_case_sensitive (GTK_COMBO (entry->widget), TRUE);
    gtk_entry_set_editable (GTK_ENTRY (GTK_COMBO (entry->widget)->entry),
                            FALSE);
    gtk_widget_show (GTK_WIDGET (entry->widget));
    gtk_table_attach (GTK_TABLE (table), GTK_WIDGET (entry->widget),
                      1, 2, idx, idx + 1,
                      (GtkAttachOptions) (GTK_FILL|GTK_EXPAND),
                      (GtkAttachOptions) (GTK_FILL), 4, 4);
    g_object_set_data (G_OBJECT (GTK_COMBO (entry->widget)->entry),
                       DATA_POINTER_KEY,
                       (gpointer) candidates_p);

    g_signal_connect ((gpointer) GTK_COMBO (entry->widget)->entry, "changed",
                      G_CALLBACK (on_default_combo_changed),
                      entry);

    if (!__widget_tooltips)
        __widget_tooltips = gtk_tooltips_new();
    if (entry->tooltip)
        gtk_tooltips_set_tip (__widget_tooltips, GTK_WIDGET (entry->widget),
                              _(entry->tooltip), NULL);

    return GTK_WIDGET (entry->widget);
}

GtkWidget *
create_key_select_button (const char *config_key, GtkTable *table, int idx)
{
    StringConfigData *entry = find_string_config_entry (config_key);
    if (!entry)
        return NULL;

    GtkWidget *button = gtk_button_new_with_label ("...");
    gtk_widget_show (button);
    gtk_table_attach (GTK_TABLE (table), button, 2, 3, idx, idx + 1,
                      GTK_FILL, GTK_FILL, 4, 4);
    g_signal_connect ((gpointer) button, "clicked",
                      G_CALLBACK (on_default_key_selection_clicked),
                      entry);

    return button;
}

static GtkWidget *
create_common_page ()
{
    GtkWidget *vbox, *frame, *table, *check, *widget;

    vbox = gtk_vbox_new (FALSE, 0);
    gtk_widget_show (vbox);

    if (!__widget_tooltips)
        __widget_tooltips = gtk_tooltips_new();

    /* specify initialize file name */
    frame = gtk_frame_new ("");
    gtk_container_set_border_width (GTK_CONTAINER (frame), 4);
    gtk_box_pack_start (GTK_BOX (vbox), frame, FALSE, FALSE, 4);
    gtk_widget_show (frame);

    check = create_check_button (SCIM_CANNA_CONFIG_SPECIFY_INIT_FILE_NAME);
     gtk_frame_set_label_widget (GTK_FRAME (frame), check);

    table = gtk_table_new (2, 2, FALSE);
    gtk_container_add (GTK_CONTAINER (frame), table);
    gtk_widget_show (table);
    widget = create_entry (SCIM_CANNA_CONFIG_INIT_FILE_NAME, GTK_TABLE (table), 0);

    g_signal_connect (G_OBJECT (check), "toggled",
                      G_CALLBACK (on_toggle_button_toggled_set_sensitive),
                      widget);
    gtk_widget_set_sensitive (widget, FALSE);

    /* specify server name */
    frame = gtk_frame_new ("");
    gtk_container_set_border_width (GTK_CONTAINER (frame), 4);
    gtk_box_pack_start (GTK_BOX (vbox), frame, FALSE, FALSE, 4);
    gtk_widget_show (frame);

    check = create_check_button (SCIM_CANNA_CONFIG_SPECIFY_SERVER_NAME);
    gtk_frame_set_label_widget (GTK_FRAME (frame), check);

    table = gtk_table_new (2, 2, FALSE);
    gtk_container_add (GTK_CONTAINER (frame), table);
    gtk_widget_show (table);
    widget = create_entry (SCIM_CANNA_CONFIG_SERVER_NAME, GTK_TABLE (table), 0);

    g_signal_connect (G_OBJECT (check), "toggled",
                      G_CALLBACK (on_toggle_button_toggled_set_sensitive),
                      widget);
    gtk_widget_set_sensitive (widget, FALSE);

    return vbox;
}

static GtkWidget *
create_mode_page ()
{
    GtkWidget *vbox, *table;

    vbox = gtk_vbox_new (FALSE, 0);
    gtk_widget_show (vbox);

    table = gtk_table_new (2, 2, FALSE);
    gtk_box_pack_start (GTK_BOX (vbox), table, FALSE, FALSE, 0);
    gtk_widget_show (table);

    // Default mode
    create_combo (SCIM_CANNA_CONFIG_ON_OFF, on_off_modes,
                  GTK_TABLE (table), 0);

    //ON/OFF key
    create_entry (SCIM_CANNA_CONFIG_ON_OFF_KEY, GTK_TABLE (table), 1);
    create_key_select_button (SCIM_CANNA_CONFIG_ON_OFF_KEY,
                              GTK_TABLE (table), 1);

    return vbox;
}

static GtkWidget *
create_about_page ()
{
    GtkWidget *vbox, *label;
    gchar str[256];

    vbox = gtk_vbox_new (FALSE, 0);
    gtk_widget_show (vbox);

    g_snprintf (
        str, 256,
        _("<span size=\"20000\">"
          "%s-%s"
          "</span>\n\n"

          "<span size=\"16000\" style=\"italic\">"
          "A Japanese input method module\nfor SCIM using Canna"
          "</span>\n\n\n\n"

          "<span size=\"12000\">"
          "Copyright 2005-2006, Takuro Ashie &lt;ashie@homa.ne.jp&gt;"
          "</span>"),
        PACKAGE, PACKAGE_VERSION);

    label = gtk_label_new (NULL);
    gtk_label_set_justify (GTK_LABEL (label), GTK_JUSTIFY_CENTER);
    gtk_label_set_markup (GTK_LABEL (label), str);
    gtk_box_pack_start (GTK_BOX (vbox), label, TRUE, TRUE, 0);
    gtk_widget_show (label);

    return vbox;
}

static GtkWidget *
create_setup_window ()
{
    static GtkWidget *window = NULL;

    if (window) return NULL;

    GtkWidget *notebook = gtk_notebook_new();
    gtk_widget_show (notebook);
    window = notebook;
    gtk_notebook_set_scrollable (GTK_NOTEBOOK (notebook), TRUE);

    // Create the first page.
    GtkWidget *page = create_common_page ();
    GtkWidget *label = gtk_label_new (_("Common"));
    gtk_widget_show (label);
    gtk_notebook_append_page (GTK_NOTEBOOK (notebook), page, label);

    // Create the second page.
    page = create_mode_page ();
    label = gtk_label_new (_("Mode"));
    gtk_widget_show (label);
    gtk_notebook_append_page (GTK_NOTEBOOK (notebook), page, label);

    // Create the third page.
    page = create_about_page ();
    label = gtk_label_new (_("About"));
    gtk_widget_show (label);
    gtk_notebook_append_page (GTK_NOTEBOOK (notebook), page, label);

    // for preventing enabling left arrow.
    gtk_notebook_set_current_page (GTK_NOTEBOOK (notebook), 1);
    gtk_notebook_set_current_page (GTK_NOTEBOOK (notebook), 0);

    setup_widget_value ();

    return window;
}

static void
setup_combo_value (GtkCombo *combo, const String & str)
{
    GList *list = NULL;
    const char *defval = NULL;

    ComboConfigCandidate *data
        = static_cast<ComboConfigCandidate*>
        (g_object_get_data (G_OBJECT (GTK_COMBO(combo)->entry),
                            DATA_POINTER_KEY));

    for (unsigned int i = 0; data[i].label; i++) {
        list = g_list_append (list, (gpointer) _(data[i].label));
        if (!strcmp (data[i].data, str.c_str ()))
            defval = _(data[i].label);
    }

    gtk_combo_set_popdown_strings (combo, list);
    g_list_free (list);

    if (defval)
        gtk_entry_set_text (GTK_ENTRY (combo->entry), defval);
}

static void
setup_widget_value ()
{
    for (unsigned int i = 0; i < __config_bool_common_num; i++) {
        BoolConfigData &entry = __config_bool_common[i];
        if (entry.widget)
            gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (entry.widget),
                                          entry.value);
    }

    for (unsigned int i = 0; i < __config_string_common_num; i++) {
        StringConfigData &entry = __config_string_common[i];
        if (entry.widget && GTK_IS_COMBO (entry.widget))
            setup_combo_value (GTK_COMBO (entry.widget), entry.value);
        else if (entry.widget && GTK_IS_ENTRY (entry.widget))
            gtk_entry_set_text (GTK_ENTRY (entry.widget),
                                entry.value.c_str ());
    }
}

static void
load_config (const ConfigPointer &config)
{
    if (config.null ())
        return;

    for (unsigned int i = 0; i < __config_bool_common_num; i++) {
        BoolConfigData &entry = __config_bool_common[i];
        entry.value = config->read (String (entry.key), entry.value);
    }

    for (unsigned int i = 0; i < __config_string_common_num; i++) {
        StringConfigData &entry = __config_string_common[i];
        entry.value = config->read (String (entry.key), entry.value);
    }

    setup_widget_value ();

    for (unsigned int i = 0; i < __config_bool_common_num; i++)
        __config_bool_common[i].changed = false;

    for (unsigned int i = 0; i < __config_string_common_num; i++)
        __config_string_common[i].changed = false;

    __have_changed = false;
}

static void
save_config (const ConfigPointer &config)
{
    if (config.null ())
        return;

    for (unsigned int i = 0; i < __config_bool_common_num; i++) {
        BoolConfigData &entry = __config_bool_common[i];
        if (entry.changed)
            entry.value = config->write (String (entry.key), entry.value);
        entry.changed = false;
    }

    for (unsigned int i = 0; i < __config_string_common_num; i++) {
        StringConfigData &entry = __config_string_common[i];
        if (entry.changed)
            entry.value = config->write (String (entry.key), entry.value);
        entry.changed = false;
    }

    __have_changed = false;
}

static bool
query_changed ()
{
    return __have_changed;
}


static void
on_default_toggle_button_toggled (GtkToggleButton *togglebutton,
                                  gpointer         user_data)
{
    BoolConfigData *entry = static_cast<BoolConfigData*> (user_data);

    if (entry) {
        entry->value = gtk_toggle_button_get_active (togglebutton);
        entry->changed = true;
        __have_changed = true;
    }
}

static void
on_default_editable_changed (GtkEditable *editable,
                             gpointer     user_data)
{
    StringConfigData *entry = static_cast <StringConfigData*> (user_data);

    if (entry) {
        entry->value = String (gtk_entry_get_text (GTK_ENTRY (editable)));
        entry->changed = true;
        __have_changed = true;
    }
}

static void
on_toggle_button_toggled_set_sensitive (GtkToggleButton *togglebutton,
                                        gpointer         user_data)
{
    GtkWidget *widget = GTK_WIDGET (user_data);

    if (widget) {
        bool active = gtk_toggle_button_get_active (togglebutton);
        gtk_widget_set_sensitive (widget, active);
    }
}

static void
on_default_combo_changed (GtkEditable *editable,
                          gpointer user_data)
{
    StringConfigData *entry = static_cast<StringConfigData*> (user_data);
    ComboConfigCandidate *data = static_cast<ComboConfigCandidate*>
        (g_object_get_data (G_OBJECT (editable),
                            DATA_POINTER_KEY));

    if (!entry) return;
    if (!data) return;

    const char *label =  gtk_entry_get_text (GTK_ENTRY (editable));

    for (unsigned int i = 0; data[i].label; i++) {
        if (label && !strcmp (_(data[i].label), label)) {
            entry->value   = data[i].data;
            entry->changed = true;
            __have_changed = true;
            break;
        }
    }
}

static void
on_default_key_selection_clicked (GtkButton *button,
                                  gpointer   user_data)
{
    StringConfigData *data = static_cast <StringConfigData*> (user_data);

    if (data) {
        GtkWidget *dialog = scim_key_selection_dialog_new (_(data->title));
        gint result;

        scim_key_selection_dialog_set_keys (
            SCIM_KEY_SELECTION_DIALOG (dialog),
            gtk_entry_get_text (GTK_ENTRY (data->widget)));

        result = gtk_dialog_run (GTK_DIALOG (dialog));

        if (result == GTK_RESPONSE_OK) {
            const gchar *keys = scim_key_selection_dialog_get_keys (
                            SCIM_KEY_SELECTION_DIALOG (dialog));

            if (!keys) keys = "";

            if (strcmp (keys, gtk_entry_get_text (GTK_ENTRY (data->widget))))
                gtk_entry_set_text (GTK_ENTRY (data->widget), keys);
        }

        gtk_widget_destroy (dialog);
    }
}
/*
vi:ts=4:nowrap:ai:expandtab
*/
