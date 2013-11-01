/**
 * HoaLibrary : A High Order Ambisonics Library
 * Copyright (c) 2012-2013 Julien Colafrancesco, Pierre Guillot, Eliott Paris, CICM, Universite Paris-8.
 * All rights reserved.re Guillot, CICM - Université Paris 8
 * All rights reserved.
 *
 * Website  : http://www.mshparisnord.fr/HoaLibrary/
 * Contacts : cicm.mshparisnord@gmail.com
 *
 * This file is part of HOA LIBRARY.
 *
 * HOA LIBRARY is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 */

#include "../hoaLibrary/hoa.library_pd.h"

typedef struct _hoa_rotate
{
    t_jbox           f_ob;
    AmbisonicRotate	*f_ambi_rotate;
} t_hoa_rotate;

void *hoa_rotate_new(t_symbol *s, long argc, t_atom *argv);
void hoa_rotate_free(t_hoa_rotate *x);

void hoa_rotate_dsp(t_hoa_rotate *x, t_object *dsp, short *count, double samplerate, long maxvectorsize, long flags);
void hoa_rotate_perform(t_hoa_rotate *x, t_object *dsp, float **ins, long ni, float **outs, long no, long sf, long f,void *up);

t_eclass *hoa_rotate_class;

extern "C" void setup_hoa0x2erotate_tilde(void)
{
    t_eclass* c;
    
    c = class_new("hoa.rotate~",(method)hoa_rotate_new,(method)hoa_rotate_free, (short)sizeof(t_hoa_rotate), 0L, A_GIMME, 0);
    
	class_dspinit(c);

	class_addmethod(c, (method)hoa_rotate_dsp, "dsp", A_CANT, 0);
    
    class_register(CLASS_BOX, c);
    hoa_rotate_class = c;
}

void *hoa_rotate_new(t_symbol *s, long argc, t_atom *argv)
{
    t_hoa_rotate *x = NULL;
	int	order = 4;
    
    x = (t_hoa_rotate *)object_alloc(hoa_rotate_class);
    
    order = atom_getint(argv);
    x->f_ambi_rotate = new AmbisonicRotate(order, sys_getblksize());
    dsp_setupjbox((t_jbox *)x, x->f_ambi_rotate->getNumberOfInputs(), x->f_ambi_rotate->getNumberOfOutputs());
    
	x->f_ob.z_misc = Z_NO_INPLACE;
    
	return (x);
}

void hoa_rotate_dsp(t_hoa_rotate *x, t_object *dsp, short *count, double samplerate, long maxvectorsize, long flags)
{
	x->f_ambi_rotate->setVectorSize(maxvectorsize);
    object_method(dsp, gensym("dsp_add"), x, (method)hoa_rotate_perform, 0, NULL);
}

void hoa_rotate_perform(t_hoa_rotate *x, t_object *dsp, float **ins, long ni, float **outs, long no, long sf, long f,void *up)
{
	x->f_ambi_rotate->process(ins, outs, ins[x->f_ambi_rotate->getNumberOfInputs()-1]);
}


void hoa_rotate_free(t_hoa_rotate *x)
{
	dsp_freejbox((t_jbox *)x);
	delete(x->f_ambi_rotate);
}
