/*
// Copyright (c) 2012-2014 Eliott Paris, Julien Colafrancesco & Pierre Guillot, CICM, Universite Paris 8.
// For information on usage and redistribution, and for a DISCLAIMER OF ALL
// WARRANTIES, see the file, "LICENSE.txt," in this distribution.
*/

#include "HoaCommon.pd.h"

typedef struct _hoa_in
{
    t_eobj  f_obj;
    int    f_extra;
} t_hoa_in;

t_eclass *hoa_in_class;

void *hoa_in_new(t_symbol *s, long argc, t_atom *argv);
void hoa_in_free(t_hoa_in *x);

void hoa_in_bang(t_hoa_in *x);
void hoa_in_float(t_hoa_in *x, float f);
void hoa_in_symbol(t_hoa_in *x, t_symbol* s);
void hoa_in_list(t_hoa_in *x, t_symbol* s, int argc, t_atom* argv);
void hoa_in_anything(t_hoa_in *x, t_symbol* s, int argc, t_atom* argv);

t_hoa_err hoa_getinfos(t_hoa_in* x, t_hoa_boxinfos* boxinfos);

extern "C" void setup_hoa0x2ein(void)
{
    t_eclass* c;
    c = eclass_new("hoa.in", (method)hoa_in_new, (method)hoa_in_free, (short)sizeof(t_hoa_in), CLASS_NOINLET, A_GIMME, 0);
    
    hoa_initclass(c, (method)hoa_getinfos);
    class_sethelpsymbol((t_class *)c, gensym("help/hoa.io"));
    eclass_addmethod(c, (method)hoa_in_bang,       "bang",     A_CANT,  0);
    eclass_addmethod(c, (method)hoa_in_float,      "float",    A_FLOAT, 0);
    eclass_addmethod(c, (method)hoa_in_symbol,     "symbol",   A_SYMBOL,0);
    eclass_addmethod(c, (method)hoa_in_list,       "list",     A_GIMME, 0);
    eclass_addmethod(c, (method)hoa_in_anything,   "anything", A_GIMME, 0);
    
    eclass_register(CLASS_OBJ, c);
    hoa_in_class = c;
    
}

void *hoa_in_new(t_symbol *s, long argc, t_atom *argv)
{
    t_hoa_in *x = NULL;
    
    x = (t_hoa_in *)eobj_new(hoa_in_class);
	if(x)
	{
        outlet_new(&x->f_obj.o_obj, 0);
        x->f_extra = 0;
        if(argc > 1 && argv && atom_gettype(argv) == A_SYM && atom_gettype(argv+1) == A_FLOAT && atom_getsym(argv) == gensym("extra") && atom_getfloat(argv+1) > 0)
        {
            x->f_extra = atom_getfloat(argv+1);
        }
        else if(argc && argv)
        {
            pd_error(x, "wrong argument.");
            eobj_free(x);
            return NULL;
        }
	}
    
	return x;
}

void hoa_in_free(t_hoa_in *x)
{
    eobj_free(x);
}

t_hoa_err hoa_getinfos(t_hoa_in* x, t_hoa_boxinfos* boxinfos)
{
    boxinfos->object_type = HOA_OBJECT_2D;
	boxinfos->autoconnect_inputs = 0;
	boxinfos->autoconnect_outputs = 0;
	boxinfos->autoconnect_inputs_type = HOA_CONNECT_TYPE_STANDARD;
	boxinfos->autoconnect_outputs_type = HOA_CONNECT_TYPE_STANDARD;
	return HOA_ERR_NONE;
}

void hoa_in_bang(t_hoa_in *x)
{
     outlet_bang(x->f_obj.o_obj.ob_outlet);
}

void hoa_in_float(t_hoa_in *x, float f)
{
    outlet_float(x->f_obj.o_obj.ob_outlet, f);
}

void hoa_in_symbol(t_hoa_in *x, t_symbol* s)
{
    outlet_symbol(x->f_obj.o_obj.ob_outlet, s);
}

void hoa_in_list(t_hoa_in *x, t_symbol* s, int argc, t_atom* argv)
{
    outlet_list(x->f_obj.o_obj.ob_outlet, s, argc, argv);
}

void hoa_in_anything(t_hoa_in *x, t_symbol* s, int argc, t_atom* argv)
{
    outlet_anything(x->f_obj.o_obj.ob_outlet, s, argc, argv);
}

