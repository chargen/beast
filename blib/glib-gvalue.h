/* BLib - BSE/BSI helper library
 * Copyright (C) 1997, 1998, 1999, 2000 Olaf Hoehmann and Tim Janik
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
 * You should have received a copy of the GNU Lesser General
 * Public License along with this library; if not, write to the
 * Free Software Foundation, Inc., 59 Temple Place, Suite 330,
 * Boston, MA 02111-1307, USA.
 *
 * gvalue.h: generic GValue functions
 */
#ifndef __G_VALUE_H__
#define __G_VALUE_H__


#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */


#include	<blib/glib-gparam.h>


/* --- type macros --- */
#define G_TYPE_IS_VALUE(type)		(G_TYPE_FUNDAMENTAL (type) == G_TYPE_PARAM)
#define	G_IS_VALUE(value)		(G_TYPE_CHECK_CLASS_TYPE ((value), G_TYPE_PARAM))
#define	G_VALUE_TYPE(value)		(G_TYPE_FROM_CLASS (value))
#define	G_VALUE_TYPE_NAME(value)	(g_type_name (G_VALUE_TYPE (value)))


/* --- typedefs & structures --- */
/* typedef struct _GValue           GValue; */
struct _GValue
{
  GType           g_type;	/* param value type */

  union {
    gint	v_int;
    guint	v_uint;
    glong	v_long;
    gulong	v_ulong;
    gfloat	v_float;
    gdouble	v_double;
    gpointer	v_pointer;
  } data[4];
};


/* --- prototypes --- */
void            g_value_init	   	(GValue       *value,
					 GType         g_type);
void            g_value_init_default  	(GValue       *value,
					 GParamSpec   *pspec);
gboolean        g_value_validate      	(GValue       *value,
					 GParamSpec   *pspec);
gboolean        g_value_defaults      	(const GValue *value,
					 GParamSpec   *pspec);
void            g_value_set_default   	(GValue       *value,
					 GParamSpec   *pspec);
gint            g_values_cmp    	(const GValue *value1,
					 const GValue *value2,
					 GParamSpec   *pspec);
void            g_value_copy    	(const GValue *src_value,
					 GValue       *dest_value);
gboolean	g_value_convert		(const GValue *src_value,
					 GValue       *dest_value);
gboolean        g_values_exchange	(GValue       *value1,
					 GValue       *value2);
void            g_value_reset   	(GValue       *value);
void            g_value_unset   	(GValue       *value);


/* --- implementation bits --- */
gboolean g_value_types_exchangable	(GType         value_type1,
					 GType         value_type2);
void     g_value_register_exchange_func	(GType         value_type1,
					 GType         value_type2,
					 GValueExchange func);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* __G_VALUE_H__ */
