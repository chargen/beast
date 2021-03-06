// Licensed GNU LGPL v2.1 or later: http://www.gnu.org/licenses/lgpl.html
#ifndef __BST_WAVE_VIEW_H__
#define __BST_WAVE_VIEW_H__

#include	"bstitemview.hh"


/* --- Gtk+ type macros --- */
#define	BST_TYPE_WAVE_VIEW	      (bst_wave_view_get_type ())
#define	BST_WAVE_VIEW(object)	      (GTK_CHECK_CAST ((object), BST_TYPE_WAVE_VIEW, BstWaveView))
#define	BST_WAVE_VIEW_CLASS(klass)    (GTK_CHECK_CLASS_CAST ((klass), BST_TYPE_WAVE_VIEW, BstWaveViewClass))
#define	BST_IS_WAVE_VIEW(object)      (GTK_CHECK_TYPE ((object), BST_TYPE_WAVE_VIEW))
#define	BST_IS_WAVE_VIEW_CLASS(klass) (GTK_CHECK_CLASS_TYPE ((klass), BST_TYPE_WAVE_VIEW))
#define BST_WAVE_VIEW_GET_CLASS(obj)  (GTK_CHECK_GET_CLASS ((obj), BST_TYPE_WAVE_VIEW, BstWaveViewClass))


/* --- structures & typedefs --- */
typedef	struct	_BstWaveView		BstWaveView;
typedef	struct	_BstWaveViewClass	BstWaveViewClass;
struct _BstWaveView
{
  BstItemView	 parent_object;
  guint          editable : 1;
};
struct _BstWaveViewClass
{
  BstItemViewClass parent_class;
};


/* --- prototypes --- */
GType		bst_wave_view_get_type          (void);
GtkWidget*	bst_wave_view_new               (SfiProxy     wrepo);
void            bst_wave_view_set_editable      (BstWaveView *self,
                                                 gboolean     enabled);

#endif /* __BST_WAVE_VIEW_H__ */
