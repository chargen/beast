// Licensed GNU LGPL v2.1 or later: http://www.gnu.org/licenses/lgpl.html
#ifndef __BSE_LADSPA_MODULE_H__
#define __BSE_LADSPA_MODULE_H__

#include <bse/bsesource.hh>
#include <bse/bseladspa.hh>

/* --- object type macros --- */
#define BSE_TYPE_LADSPA_MODULE              (BSE_TYPE_ID (BseLadspaModule))
#define BSE_LADSPA_MODULE(object)           (G_TYPE_CHECK_INSTANCE_CAST ((object), BSE_TYPE_LADSPA_MODULE, BseLadspaModule))
#define BSE_LADSPA_MODULE_CLASS(class)      (G_TYPE_CHECK_CLASS_CAST ((class), BSE_TYPE_LADSPA_MODULE, BseLadspaModuleClass))
#define BSE_IS_LADSPA_MODULE(object)        (G_TYPE_CHECK_INSTANCE_TYPE ((object), BSE_TYPE_LADSPA_MODULE))
#define BSE_IS_LADSPA_MODULE_CLASS(class)   (G_TYPE_CHECK_CLASS_TYPE ((class), BSE_TYPE_LADSPA_MODULE))
#define BSE_LADSPA_MODULE_GET_CLASS(object) (G_TYPE_INSTANCE_GET_CLASS ((object), BSE_TYPE_LADSPA_MODULE, BseLadspaModuleClass))


/* --- BseLadspaModule --- */
struct BseLadspaModule : BseSource {
  gfloat    *cvalues;
};
struct BseLadspaModuleClass : BseSourceClass {
  BseLadspaInfo *bli;
  BseModuleClass *gsl_class;
};

void	bse_ladspa_module_derived_type_info	(GType			type,
						 BseLadspaInfo	       *bli,
						 GTypeInfo	       *type_info);

#endif /* __BSE_LADSPA_MODULE_H__ */
