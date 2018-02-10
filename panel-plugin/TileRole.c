/*  $Id$
 *
 *  ;   
 *  
 */

#ifdef HAVE_CONFIG_H
#   include <config.h>
#endif

#include <string.h>
#include <locale.h>
#include <gtk/gtk.h>
#include <libxfce4util/libxfce4util.h>
#include <libxfce4ui/libxfce4ui.h>
#include <libxfce4panel/xfce-panel-plugin.h>
#include <libxfce4panel/xfce-hvbox.h>



// Default settings
#define DEFAULT_SIZE 20
#define DEFAULT_HIST_SIZE 25



typedef struct {
    XfcePanelPlugin *plugin;

    GtkWidget *ebox;
    GtkWidget *hvbox;
    GtkWidget *combo;
    GList *expr_hist;   // Expression history  
    // Settings  
    gint size;		  // Size of comboboxentry 
    gint hist_size;

} TileRolePlugin;


static void TileRole_construct(XfcePanelPlugin *plugin);

XFCE_PANEL_PLUGIN_REGISTER_EXTERNAL(TileRole_construct);


void TileRole_save_config(XfcePanelPlugin *plugin, TileRolePlugin *TileRole)
{
    XfceRc *rc;
    gchar *file;

    file = xfce_panel_plugin_save_location(plugin, TRUE);
    if (file == NULL) return;

    rc = xfce_rc_simple_open(file, FALSE);
    g_free(file);

    if (rc != NULL) {
        xfce_rc_write_int_entry(rc, "size", TileRole->size);
        xfce_rc_write_int_entry(rc, "hist_size", TileRole->hist_size);
        xfce_rc_close(rc);
    }
}


static void TileRole_read_config(TileRolePlugin *TileRole)
{
    XfceRc *rc;
    gchar *file;

    file = xfce_panel_plugin_lookup_rc_file(TileRole->plugin);

    if (file) {
        rc = xfce_rc_simple_open(file, TRUE);
        g_free(file);
    } else
        rc = NULL;

    if (rc) {
        TileRole->size = xfce_rc_read_int_entry(rc, "size", DEFAULT_SIZE);
        TileRole->hist_size = xfce_rc_read_int_entry(rc, "hist_size", DEFAULT_HIST_SIZE);
        xfce_rc_close(rc);
    } else {
        /* Something went wrong, apply default values. */
        TileRole->size = DEFAULT_SIZE;
        TileRole->hist_size = DEFAULT_HIST_SIZE;
    }
}


static GList *add_to_expr_hist(GList *ehist, gint hist_size, const gchar *str)
{
    GList *elem;

    // Remove duplicates
    if ((elem = g_list_find_custom(ehist, str, (GCompareFunc)g_strcmp0))) {
        g_free(elem->data);
        ehist = g_list_delete_link(ehist, elem);
    }

    // Add the new expression
    ehist = g_list_append(ehist, g_strdup(str));

    // Remove oldest, if list is growing too long.
    if (g_list_length(ehist) > hist_size) {
        elem = g_list_first(ehist);
        g_free(elem->data);
        ehist = g_list_delete_link(ehist, elem);
    }

    return ehist;
}


/* Called when user presses enter in the entry. */

static void entry_enter_cb(GtkEntry *entry, TileRolePlugin *TileRole)
{
    node_t *parsetree;
    const gchar *input;
    GError *err = NULL;

    input = gtk_entry_get_text(entry);
    TileRole->expr_hist = add_to_expr_hist(TileRole->expr_hist, TileRole->hist_size, input);
    gtk_combo_set_popdown_strings(GTK_COMBO(TileRole->combo), TileRole->expr_hist);
gboolean result;
//    result = g_spawn_command_line_async ("/home/wim/python/3.py 60 100 0 0", NULL);
char * np = (char *) input ;
char * ip = "/home/wim/python/3.py ";
char *str = malloc(strlen(ip)+strlen(np)+1);
strcpy(str,ip);
strcat(str,np);

result = g_spawn_command_line_async (str, NULL);
gtk_editable_set_position(GTK_EDITABLE(entry), 0);
free(str);
free(result);

    
}


static gboolean entry_buttonpress_cb(GtkWidget *entry, GdkEventButton *event,
                                     TileRolePlugin *TileRole)
{
    GtkWidget *toplevel;

    toplevel = gtk_widget_get_toplevel(entry);

    if (event->button != 3 && toplevel && toplevel->window)
        xfce_panel_plugin_focus_widget(TileRole->plugin, entry);

    return FALSE;
}


static TileRolePlugin *TileRole_new(XfcePanelPlugin *plugin)
{
    TileRolePlugin *TileRole;
    GtkOrientation orientation;
    GtkWidget *icon;
    GtkWidget *combo;

    TileRole = panel_slice_new0(TileRolePlugin);
    TileRole->plugin = plugin;
    TileRole_read_config(TileRole);

    orientation = xfce_panel_plugin_get_orientation(plugin);

    TileRole->ebox = gtk_event_box_new();
    gtk_widget_show(TileRole->ebox);

    TileRole->hvbox = xfce_hvbox_new(orientation, FALSE, 2);
    gtk_widget_show(TileRole->hvbox);
    gtk_container_add(GTK_CONTAINER(TileRole->ebox), TileRole->hvbox);

    icon = gtk_label_new(_(" TileRole:"));
    gtk_widget_show(icon);
    gtk_box_pack_start(GTK_BOX(TileRole->hvbox), icon, FALSE, FALSE, 0);

    combo = gtk_combo_new();
    gtk_entry_set_max_length(GTK_ENTRY(GTK_COMBO(combo)->entry), 50);
    gtk_combo_set_use_arrows_always(GTK_COMBO(combo), TRUE);
    g_signal_connect(G_OBJECT(GTK_COMBO(combo)->entry), "activate",
                     G_CALLBACK(entry_enter_cb), (gpointer)TileRole);
    g_signal_connect(G_OBJECT(GTK_COMBO(combo)->entry), "button-press-event",
                     G_CALLBACK(entry_buttonpress_cb), (gpointer)TileRole);
    gtk_widget_show(combo);
    gtk_box_pack_start(GTK_BOX(TileRole->hvbox), combo, FALSE, FALSE, 0);
    TileRole->combo = combo;

    TileRole->expr_hist = NULL;

    gtk_entry_set_width_chars(GTK_ENTRY(GTK_COMBO(combo)->entry), TileRole->size);

    return TileRole;
}


/* Used with g_list_foreach() to free data items in a list. */

static void free_stuff(gpointer data, gpointer unused)
{
    g_free(data);
}


static void TileRole_free(XfcePanelPlugin *plugin, TileRolePlugin *TileRole)
{
    GtkWidget *dialog;

    dialog = g_object_get_data(G_OBJECT(plugin), "dialog");
    if (dialog != NULL)
        gtk_widget_destroy(dialog);

    gtk_widget_destroy(TileRole->ebox);
    gtk_widget_destroy(TileRole->hvbox);
    gtk_widget_destroy(TileRole->combo);

    g_list_foreach(TileRole->expr_hist, (GFunc)free_stuff, NULL);
    g_list_free(TileRole->expr_hist);

    /* 
     * FIXME: Do we need to free the strings in the combo list, or is the
     * freeing of expr_hist enough?
     */

    panel_slice_free(TileRolePlugin, TileRole);
}


static void TileRole_orientation_changed(XfcePanelPlugin *plugin,
                                     GtkOrientation orientation,
                                     TileRolePlugin *TileRole)
{
    xfce_hvbox_set_orientation(XFCE_HVBOX(TileRole->hvbox), orientation);
}


static gboolean TileRole_size_changed(XfcePanelPlugin *plugin, gint size,
                                  TileRolePlugin *TileRole)
{
    GtkOrientation orientation;

    orientation = xfce_panel_plugin_get_orientation(plugin);

    if (orientation == GTK_ORIENTATION_HORIZONTAL)
        gtk_widget_set_size_request(GTK_WIDGET(plugin), -1, size);
    else
        gtk_widget_set_size_request(GTK_WIDGET(plugin), size, -1);

    return TRUE;
}


static gboolean TileRole_plugin_update_size(XfcePanelPlugin *plugin, gint size,
                                        TileRolePlugin *TileRole)
{
	g_assert(TileRole);
	g_assert(TileRole->combo);

	TileRole->size = size;
	gtk_entry_set_width_chars(GTK_ENTRY(GTK_COMBO(TileRole->combo)->entry), size);
	return TRUE;
}


static void TileRole_plugin_size_changed(GtkSpinButton *spin, TileRolePlugin *TileRole)
{
	g_assert(TileRole);
	TileRole_plugin_update_size(NULL, gtk_spin_button_get_value_as_int(spin), TileRole);
}


static void TileRole_hist_size_changed(GtkSpinButton *spin, TileRolePlugin *TileRole)
{
    g_assert(TileRole);
    TileRole->hist_size = gtk_spin_button_get_value_as_int(spin);
}

static void TileRole_dialog_response(GtkWidget *dialog, gint response,
                                 TileRolePlugin *TileRole)
{
    if (response == GTK_RESPONSE_OK) {
        g_object_set_data(G_OBJECT(TileRole->plugin), "dialog", NULL);
        xfce_panel_plugin_unblock_menu(TileRole->plugin);
        TileRole_save_config(TileRole->plugin, TileRole);
        gtk_widget_destroy(dialog);
    } else
        g_assert_not_reached();
}


static void TileRole_configure(XfcePanelPlugin *plugin, TileRolePlugin *TileRole)
{
    GtkWidget *dialog;
    GtkWidget *toplevel;

    GtkWidget *frame;
    GtkWidget *bin;
    GtkWidget *hbox;
    GtkWidget *size_label;
    GtkWidget *size_spin;
    GtkObject *adjustment;

    xfce_panel_plugin_block_menu(plugin);

    toplevel = gtk_widget_get_toplevel(GTK_WIDGET(plugin)); 
    dialog = xfce_titled_dialog_new_with_buttons(_("TileRole Plugin"),
                       GTK_WINDOW(toplevel),
                       GTK_DIALOG_DESTROY_WITH_PARENT | GTK_DIALOG_NO_SEPARATOR,
                       GTK_STOCK_CLOSE, GTK_RESPONSE_OK, NULL);

    gtk_window_set_position(GTK_WINDOW(dialog), GTK_WIN_POS_CENTER);
    gtk_window_set_icon_name(GTK_WINDOW(dialog), "xfce4-TileRole-plugin");

    /* Link the dialog to the plugin, so we can destroy it when the plugin
     * is closed, but the dialog is still open */
    g_object_set_data(G_OBJECT(plugin), "dialog", dialog);

    g_signal_connect(G_OBJECT(dialog), "response",
                     G_CALLBACK(TileRole_dialog_response), TileRole);


	frame = xfce_gtk_frame_box_new (_("Appearance"), &bin);

	gtk_container_set_border_width(GTK_CONTAINER(frame), 6);
	gtk_box_pack_start(GTK_BOX(GTK_DIALOG(dialog)->vbox), frame, TRUE, TRUE, 0);
	gtk_widget_show(frame);

	hbox = gtk_hbox_new(FALSE, 8);
	gtk_container_add(GTK_CONTAINER (bin), hbox);
	gtk_widget_show(hbox);

	size_label = gtk_label_new(_("Width (in chars):"));
	gtk_box_pack_start(GTK_BOX(hbox), size_label, FALSE, TRUE, 0);
	gtk_widget_show(size_label);
	adjustment = gtk_adjustment_new(TileRole->size, 5, 100, 1, 5, 10);
	size_spin = gtk_spin_button_new(GTK_ADJUSTMENT(adjustment), 1, 0);
	gtk_widget_add_mnemonic_label(size_spin, size_label);
	gtk_box_pack_start(GTK_BOX(hbox), size_spin, FALSE, TRUE, 0);
	gtk_widget_show(size_spin);
	gtk_spin_button_set_value(GTK_SPIN_BUTTON(size_spin), TileRole->size);
	g_signal_connect(size_spin, "value-changed",
                     G_CALLBACK(TileRole_plugin_size_changed), TileRole);


    frame = xfce_gtk_frame_box_new (_("History"), &bin);

    gtk_container_set_border_width(GTK_CONTAINER (frame), 6);
    gtk_box_pack_start(GTK_BOX(GTK_DIALOG(dialog)->vbox), frame, TRUE, TRUE, 0);
    gtk_widget_show (frame);

    hbox = gtk_hbox_new(FALSE, 8);
    gtk_container_add(GTK_CONTAINER(bin), hbox);
    gtk_widget_show(hbox);

    size_label = gtk_label_new (_("Size:"));
    gtk_box_pack_start(GTK_BOX(hbox), size_label, FALSE, TRUE, 0);
    gtk_widget_show(size_label);
    adjustment = gtk_adjustment_new(TileRole->hist_size, 0, 100, 1, 10, 20);
    size_spin = gtk_spin_button_new(GTK_ADJUSTMENT(adjustment), 1, 0);
    gtk_box_pack_start(GTK_BOX(hbox), size_spin, FALSE, TRUE, 0);
    gtk_widget_show (size_spin);
    g_signal_connect(size_spin, "value-changed",
                     G_CALLBACK(TileRole_hist_size_changed), TileRole);


    gtk_widget_show(dialog);

}

void TileRole_about (XfcePanelPlugin *plugin)
{
       GdkPixbuf *icon;
   const gchar *authors[] = {
      "Wim Stockman <wim.stockman@gmail.com>", NULL };
   icon = xfce_panel_pixbuf_from_source("xfce4-TileRole-plugin", NULL, 32);
   gtk_show_about_dialog(NULL,
      "logo", icon,
      "license", xfce_get_license_text (XFCE_LICENSE_TEXT_GPL),
      "version", PACKAGE_VERSION,
      "program-name", PACKAGE_NAME,
      "comments", _("TileRole for Xfce panel"),
      "website", "https://github.com/wimstockman/xfce4-TileRole-plugin.git",
      "copyright", _("Copyright (c) 2018-2100\n"),
      "authors", authors, NULL);

   if(icon)
      g_object_unref(G_OBJECT(icon));

}



static void TileRole_construct(XfcePanelPlugin *plugin)
{
    TileRolePlugin *TileRole;
    GtkWidget *degrees, *radians, *hexadecimal;

    /* Make sure the comma sign (",") isn't treated as a decimal separator. */
    setlocale(LC_NUMERIC, "C");

    xfce_textdomain(GETTEXT_PACKAGE, PACKAGE_LOCALE_DIR, "UTF-8");

    TileRole = TileRole_new(plugin);
    gtk_container_add(GTK_CONTAINER(plugin), TileRole->ebox);

    /* Show the panel's right-click menu on this ebox */
    xfce_panel_plugin_add_action_widget(plugin, TileRole->ebox);
    
    g_signal_connect(G_OBJECT(plugin), "free-data",
                     G_CALLBACK(TileRole_free), TileRole);
    g_signal_connect(G_OBJECT(plugin), "save",
                     G_CALLBACK(TileRole_save_config), TileRole);
    g_signal_connect(G_OBJECT(plugin), "size-changed",
                     G_CALLBACK(TileRole_size_changed), TileRole);
    g_signal_connect(G_OBJECT(plugin), "orientation-changed",
                     G_CALLBACK(TileRole_orientation_changed), TileRole);

    /* Show the configure menu item and connect signal */
    xfce_panel_plugin_menu_show_configure(plugin);
    g_signal_connect(G_OBJECT(plugin), "configure-plugin",
                     G_CALLBACK(TileRole_configure), TileRole);

	/* Show the about menu item and connect signal */
	xfce_panel_plugin_menu_show_about (plugin);
	g_signal_connect (G_OBJECT (plugin), "about",
		G_CALLBACK (TileRole_about), TileRole);

    
}
