// Licensed GNU LGPL v2.1 or later: http://www.gnu.org/licenses/lgpl.html

/* --- string parameters --- */
static void
param_entry_changed (GtkWidget *entry,
                     GxkParam  *param)
{
  const gchar *estring = gtk_entry_get_text (GTK_ENTRY (entry));
  const gchar *vstring = g_value_get_string (&param->value);
  if (!estring)
    estring = "";
  if (!vstring)
    vstring = "";
  if (strcmp (estring, vstring) != 0)
    {
      g_value_set_string (&param->value, estring);
      gxk_param_apply_value (param);
    }
}

static GtkWidget*
param_entry_create (GxkParam    *param,
                    const gchar *tooltip,
                    guint        variant)
{
  GtkWidget *widget = (GtkWidget*) g_object_new (GTK_TYPE_ENTRY,
                                                 "visible", TRUE,
                                                 "activates_default", TRUE,
                                                 "width_chars", 0,
                                                 NULL);
  const GxkParamEditorSizes *esizes = gxk_param_get_editor_sizes (param);
  gxk_widget_add_font_requisition (widget, esizes->string_chars, esizes->string_digits);
  gxk_param_entry_connect_handlers (param, widget, param_entry_changed);
  gxk_widget_set_tooltip (widget, tooltip);
  gxk_widget_add_option (widget, "hexpand", "+");
  return widget;
}

static void
param_entry_update (GxkParam  *param,
		    GtkWidget *widget)
{
  gxk_param_entry_set_text (param, widget, g_value_get_string (&param->value));
}

static GxkParamEditor param_entry = {
  { "entry",            N_("Text Entry"), },
  { G_TYPE_STRING, },
  { NULL,       +5,     TRUE, },        /* options, rating, editing */
  param_entry_create,   param_entry_update,
};
