/*
 *
 * Copyright (C) 2012 Julien Colafrancesco & Pierre Guillot, Universite Paris 8
 * 
 * This library is free software; you can redistribute it and/or modify it 
 * under the terms of the GNU Library General Public License as published 
 * by the Free Software Foundation; either version 2 of the License.
 * 
 * This library is distributed in the hope that it will be useful, but WITHOUT 
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or 
 * FITNESS FOR A PARTICULAR PURPOSE.  See the GNU Library General Public 
 * License for more details.
 *
 * You should have received a copy of the GNU Library General Public License 
 * along with this library; if not, write to the Free Software Foundation, 
 * Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
 *
 */

extern "C"
{
	#include "ext.h"
	#include "ext_obex.h"
	#include "z_dsp.h"
}

#include "ambisonicDisretise.h"


typedef struct _HoaDiscretise
{
	t_pxobject					f_ob;			
	ambisonicDecode				*f_ambisonicDiscretiser;

	int							f_n;
	int							f_sr;
	
	long						f_order;
	long						f_inputNumber;
	long						f_outputNumber;
	double						*f_insig;
	double						*f_outsig;

} t_HoaDiscretise;

void *HoaDiscretise_new(t_symbol *s, long argc, t_atom *argv);
void HoaDiscretise_free(t_HoaDiscretise *x);
void HoaDiscretise_assist(t_HoaDiscretise *x, void *b, long m, long a, char *s);
void HoaDiscretise_optim(t_HoaDiscretise *x, t_symbol *s, long argc, t_atom *argv);

void HoaDiscretise_dsp(t_HoaDiscretise *x, t_signal **sp, short *count);
t_int *HoaDiscretise_perform(t_int *w);

void HoaDiscretise_dsp64(t_HoaDiscretise *x, t_object *dsp64, short *count, double samplerate, long maxvectorsize, long flags);
void HoaDiscretise_perform64(t_HoaDiscretise *x, t_object *dsp64, double **ins, long numins, double **outs, long numouts, long sampleframes, long flags, void *userparam);

void *HoaDiscretise_class;

int main(void)
{	

	t_class *c;
	
	c = class_new("hoa.discretizer~", (method)HoaDiscretise_new, (method)dsp_free, (long)sizeof(t_HoaDiscretise), 0L, A_GIMME, 0);
	
	class_addmethod(c, (method)HoaDiscretise_dsp,			"dsp",		A_CANT, 0);
	class_addmethod(c, (method)HoaDiscretise_dsp64,			"dsp64",	A_CANT, 0);
	class_addmethod(c, (method)HoaDiscretise_assist,		"assist",	A_CANT, 0);
	class_addmethod(c, (method)HoaDiscretise_optim,			"optim",	A_GIMME, 0);
	
	class_dspinit(c);				
	class_register(CLASS_BOX, c);	
	HoaDiscretise_class = c;
	
	post("hoa.discretizer~ by Julien Colafrancesco & Pierre Guillot",0);
	post("Copyright (C) 2012, CICM / Universite Paris 8");
	
	return 0;
}

void *HoaDiscretise_new(t_symbol *s, long argc, t_atom *argv)
{
	t_HoaDiscretise *x = NULL;
	
	if (x = (t_HoaDiscretise *)object_alloc((t_class*)HoaDiscretise_class)) 
	{
		x->f_order = 1;
		x->f_outputNumber = 4;
		
		if(atom_gettype(argv) == A_LONG)
			x->f_order			= atom_getlong(argv);
		if(atom_gettype(argv+1) == A_LONG)
			x->f_outputNumber	= atom_getlong(argv+1);
		
		x->f_inputNumber		= x->f_order * 2 + 1;
		if(x->f_outputNumber < 1)
			x->f_outputNumber = 1;
		
		x->f_ambisonicDiscretiser	= new ambisonicDiscretise(x->f_outputNumber, x->f_order);
		
		dsp_setup((t_pxobject *)x, x->f_inputNumber);
		for (int i = 0; i < x->f_outputNumber; i++)
			outlet_new(x, "signal");		
	}
	return (x);
}

void HoaDiscretise_dsp64(t_HoaDiscretise *x, t_object *dsp64, short *count, double samplerate, long maxvectorsize, long flags)
{
	x->f_n	= maxvectorsize;
	x->f_sr	= samplerate;
	
	x->f_insig = (double *)getbytes(x->f_inputNumber * sizeof(double));
	x->f_outsig = (double *)getbytes(x->f_outputNumber * sizeof(double));
	object_method(dsp64, gensym("dsp_add64"), x, HoaDiscretise_perform64, 0, NULL);
}

void HoaDiscretise_perform64(t_HoaDiscretise *x, t_object *dsp64, double **ins, long numins, double **outs, long numouts, long sampleframes, long flags, void *userparam)
{
	for (int i = 0; i < sampleframes; i++) 
	{
		for (int j = 0; j < numins; j++)
			x->f_insig[j] = ins[j][i];
		
		x->f_ambisonicDiscretiser->process(x->f_insig);
		
		for (int j = 0; j < x->f_outputNumber; j++) 
			outs[j][i] = x->f_outsig[j];
	}
}

void HoaDiscretise_dsp(t_HoaDiscretise *x, t_signal **sp, short *count)
{
	int i;
	int pointer_count;
	t_int **sigvec;
	
	x->f_n	= (int)sp[0]->s_n;
	x->f_sr	= (int)sp[0]->s_sr;
	x->f_insig = (double *)getbytes(x->f_inputNumber * sizeof(double));
	x->f_outsig = (double *)getbytes(x->f_outputNumber * sizeof(double));
	pointer_count = x->f_inputNumber + x->f_outputNumber + 2;
	
	sigvec  = (t_int **)calloc(pointer_count, sizeof(t_int *));
	for(i = 0; i < pointer_count; i++)
		sigvec[i] = (t_int *)calloc(1, sizeof(t_int));
	
	sigvec[0] = (t_int *)x;
	sigvec[1] = (t_int *)sp[0]->s_n;
	for(i = 2; i < pointer_count; i++)
		sigvec[i] = (t_int *)sp[i - 2]->s_vec;
	
	dsp_addv(HoaDiscretise_perform, pointer_count, (void **)sigvec);
	
	free(sigvec);
}

t_int *HoaDiscretise_perform(t_int *w)
{
	t_HoaDiscretise *x			= (t_HoaDiscretise *)(w[1]);	
	int			n			= (int)(w[2]);
	t_float		**signals	= (t_float **)w+3;
	
	for (int i = 0; i < n; i++) 
	{
		
		for (int j = 0; j < x->f_inputNumber; j++)
			x->f_insig[j] = signals[j][i];
		
		x->f_outsig = x->f_ambisonicDiscretiser->process(x->f_insig);
		
		for (int j = 0; j < x->f_outputNumber; j++) 
			signals[j+x->f_inputNumber][i] = x->f_outsig[j];
	}
	
	return (w + x->f_outputNumber + x->f_inputNumber + 3);
}

void HoaDiscretise_assist(t_HoaDiscretise *x, void *b, long m, long a, char *s)
{
	if (m == ASSIST_INLET)
	{
		long harmonicIndex = 0;
		if (a == 0)
			harmonicIndex = 0;
		else {
			harmonicIndex = (int)floor((a-1)/2) + 1;
			if (a%2 == 1) 
				harmonicIndex = - harmonicIndex;
		}
		sprintf(s,"(Signal) Spherical harmonic %i dependant signal", harmonicIndex);
	}
	else 
		sprintf(s,"(Signal) Signal for speaker %i", (int)a);			
}


void HoaDiscretise_optim(t_HoaDiscretise *x, t_symbol *s, long argc, t_atom *argv)
{
	if(atom_gettype(argv) == A_SYM)
	{
		std::string decodingId = atom_getsym(argv)->s_name;
		x->f_ambisonicDiscretiser->setOptimMethod(decodingId);
	}
}

void HoaDiscretise_free(t_HoaDiscretise *x)
{
	dsp_free((t_pxobject *)x);
}

