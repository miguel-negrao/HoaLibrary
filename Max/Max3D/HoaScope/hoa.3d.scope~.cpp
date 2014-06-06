/*
 // Copyright (c) 2012-2013 Eliott Paris & Pierre Guillot, CICM, Universite Paris 8.
 // For information on usage and redistribution, and for a DISCLAIMER OF ALL
 // WARRANTIES, see the file, "LICENSE.txt," in this distribution.
 */

/**
 @file      hoa.3d.scope~.cpp
 @name      hoa.3d.scope~
 @realname  hoa.3d.scope~
 @type      object
 @module    hoa
 @author    Julien Colafrancesco, Pierre Guillot, Eliott Paris.
 
 @digest
 A spherical harmonic visualizer
 
 @description
 <o>hoa.3d.scope~</o> displays spherical harmonics of an ambisonic sound field
 
 @discussion
 <o>hoa.3d.scope~</o> displays spherical harmonics of an ambisonic sound field
 
 @category ambisonics, hoa objects, audio, GUI, MSP
 
 @seealso hoa.2d.scope~, hoa.3d.decoder~, hoa.3d.encoder~, hoa.3d.map~, hoa.3d.optim~, hoa.3d.vector~, hoa.3d.wider~, hoa.dac~
 */

#include "../Hoa3D.max.h"
#include "../../MaxJuceBox/jucebox_wrapper.h"

typedef struct _hoa_scope
{
	t_jucebox	j_box;
    
    Hoa3D::Scope*   f_scope;
    long            f_order;
    double*         f_signals;
    double          f_gain;
    
    int             f_index;
    void*           f_clock;
	int             f_startclock;
    long            f_interval;
    
    long            f_mode;
	long			f_color_interp_mode;
    long            f_vectors;
	long            f_light;
    long            f_sphere;
    t_jrgba         f_color_bg;
	t_jrgba         f_color_bd;
    t_jrgba         f_color_ph;
    t_jrgba         f_color_nh;
	t_jrgba         f_color_sp;
    
	double          f_camera[2];
    double          f_camera_ref[2];
    t_pt            f_mouse;
} t_hoa_scope;

t_class *hoa_scope_class;

void *hoa_scope_new(t_symbol *s, long argc, t_atom *argv);
void hoa_scope_free(t_hoa_scope *x);
void hoa_scope_assist(t_hoa_scope *x, void *b, long m, long a, char *s);
void hoa_scope_paint(t_hoa_scope *x, double w, double h);
void hoa_scope_tick(t_hoa_scope *x);

void hoa_scope_getdrawparams(t_hoa_scope *x, t_object *patcherview, t_jboxdrawparams *params);
t_max_err hoa_scope_notify(t_hoa_scope *x, t_symbol *s, t_symbol *m, void *sender, void *data);

void hoa_scope_dsp64(t_hoa_scope *x, t_object *dsp64, short *count, double samplerate, long maxvectorsize, long flags);
void hoa_scope_perform64(t_hoa_scope *x, t_object *dsp64, double **ins, long numins, double **outs, long numouts, long sampleframes, long flags, void *userparam);

void hoa_scope_mousedown(t_hoa_scope *x, t_object *patcherview, t_pt pt, long modifiers);
void hoa_scope_mousedrag(t_hoa_scope *x, t_object *patcherview, t_pt pt, long modifiers);
void hoa_scope_dblclick(t_hoa_scope *x, t_object *patcherview, t_pt pt, long modifiers);

t_max_err hoa_scope_attr_set_order(t_hoa_scope *x, t_object *attr, long argc, t_atom *argv);
t_max_err hoa_scope_attr_set_camera(t_hoa_scope *x, t_object *attr, long argc, t_atom *argv);
t_hoa_err hoa_getinfos(t_hoa_scope* x, t_hoa_boxinfos* boxinfos);

void hoa_draw_lighting(t_hoa_scope *x);
void hoa_draw_camera(t_hoa_scope *x);
void hoa_draw_harmonics_shape(t_hoa_scope *x);
void hoa_draw_harmonics_mapping(t_hoa_scope *x);

int C74_EXPORT main(void)
{
	t_class *c;
    
	c = class_new("hoa.3d.scope~", (method)hoa_scope_new, (method)hoa_scope_free, sizeof(t_hoa_scope), 0L, A_GIMME, 0);
    
	c->c_flags |= CLASS_FLAG_NEWDICTIONARY;

    class_dspinitjbox(c);
    jucebox_initclass(c, (method)hoa_scope_paint,  JBOX_COLOR | JBOX_FIXWIDTH);
    hoa_initclass(c, (method)hoa_getinfos);
    
	// @method signal @digest Array of spherical harmonic signals that represent a sound field
	// @description Array of spherical harmonic signals that represent a sound field
	class_addmethod(c, (method)hoa_scope_dsp64,				"dsp64",            A_CANT, 0);
	
	// @method (mouse) @digest Change point of view by dragging the scene.
	// @description Change point of view by dragging the scene.
	class_addmethod(c, (method)hoa_scope_mousedown,			"mousedown",        A_CANT, 0);
    class_addmethod(c, (method)hoa_scope_mousedrag,			"mousedrag",        A_CANT, 0);
	class_addmethod(c, (method)hoa_scope_dblclick,			"mousedoubleclick",	A_CANT, 0);
	class_addmethod(c, (method)hoa_scope_assist,			"assist",           A_CANT, 0);
	class_addmethod(c, (method)hoa_scope_getdrawparams,		"getdrawparams",    A_CANT, 0);
	class_addmethod(c, (method)hoa_scope_notify,			"notify",           A_CANT, 0);
	
    CLASS_ATTR_DEFAULT              (c, "patching_rect", 0, "0 0 200 200");
    CLASS_ATTR_INVISIBLE            (c, "color", 0);
    
    CLASS_ATTR_LONG                 (c, "order", 0, t_hoa_scope, f_order);
    CLASS_ATTR_ACCESSORS            (c, "order", NULL,  hoa_scope_attr_set_order);
	CLASS_ATTR_CATEGORY             (c, "order", 0, "Ambisonic");
	CLASS_ATTR_ORDER                (c, "order", 0, "1");
	CLASS_ATTR_LABEL                (c, "order", 0, "Order");
	CLASS_ATTR_FILTER_MIN           (c, "order", 1);
	CLASS_ATTR_DEFAULT              (c, "order", 0, "1");
	CLASS_ATTR_SAVE                 (c, "order", 1);
    CLASS_ATTR_PAINT                (c, "order", 1);
	// @description The ambisonic order of decomposition. Will adapt the number of input accordingly.
    
    CLASS_ATTR_RGBA                 (c, "bgcolor", 0, t_hoa_scope, f_color_bg);
	CLASS_ATTR_CATEGORY             (c, "bgcolor", 0, "Color");
	CLASS_ATTR_STYLE                (c, "bgcolor", 0, "rgba");
	CLASS_ATTR_LABEL                (c, "bgcolor", 0, "Background Color");
	CLASS_ATTR_ORDER                (c, "bgcolor", 0, "1");
	CLASS_ATTR_DEFAULT_SAVE_PAINT   (c, "bgcolor", 0, "0.9 0.9 0.9 1.");
	// @description Sets the RGBA values for the background color of the <o>hoa.3d.scope~</o> object
	
	CLASS_ATTR_RGBA                 (c, "bdcolor", 0, t_hoa_scope, f_color_bd);
	CLASS_ATTR_CATEGORY             (c, "bdcolor", 0, "Color");
	CLASS_ATTR_STYLE                (c, "bdcolor", 0, "rgba");
	CLASS_ATTR_LABEL                (c, "bdcolor", 0, "Border Color");
	CLASS_ATTR_ORDER                (c, "bdcolor", 0, "1");
	CLASS_ATTR_DEFAULT_SAVE_PAINT   (c, "bdcolor", 0, "0.1 0.1 0.1 1.");
	// @description Sets the RGBA values for the border color of the <o>hoa.3d.scope~</o> object
	
    CLASS_ATTR_RGBA                 (c, "phcolor", 0, t_hoa_scope, f_color_ph);
	CLASS_ATTR_CATEGORY             (c, "phcolor", 0, "Color");
	CLASS_ATTR_STYLE                (c, "phcolor", 0, "rgba");
	CLASS_ATTR_LABEL                (c, "phcolor", 0, "Positive Harmonics Color");
	CLASS_ATTR_ORDER                (c, "phcolor", 0, "2");
	CLASS_ATTR_DEFAULT_SAVE_PAINT   (c, "phcolor", 0, "1. 0. 0. 1.");
	// @description Sets the RGBA values for the positive harmonics color of the <o>hoa.3d.scope~</o> object
	
	CLASS_ATTR_RGBA                 (c, "nhcolor", 0, t_hoa_scope, f_color_nh);
	CLASS_ATTR_CATEGORY             (c, "nhcolor", 0, "Color");
	CLASS_ATTR_STYLE                (c, "nhcolor", 0, "rgba");
	CLASS_ATTR_LABEL                (c, "nhcolor", 0, "Negative Harmonics Color");
	CLASS_ATTR_ORDER                (c, "nhcolor", 0, "3");
	CLASS_ATTR_DEFAULT_SAVE_PAINT   (c, "nhcolor", 0, "0. 0. 1. 1.");
	// @description Sets the RGBA values for the negative harmonics color of the <o>hoa.2d.scope~</o> object
    
	CLASS_ATTR_RGBA                 (c, "spcolor", 0, t_hoa_scope, f_color_sp);
	CLASS_ATTR_CATEGORY             (c, "spcolor", 0, "Color");
	CLASS_ATTR_STYLE                (c, "spcolor", 0, "rgba");
	CLASS_ATTR_LABEL                (c, "spcolor", 0, "Sphere Color");
	CLASS_ATTR_ORDER                (c, "spcolor", 0, "4");
	CLASS_ATTR_DEFAULT_SAVE_PAINT   (c, "spcolor", 0, "0. 0. 0. 1.");
	// @description Sets the RGBA values for the sphere color of the <o>hoa.3d.scope~</o> object
	
    CLASS_ATTR_LONG                 (c, "interval", 0, t_hoa_scope, f_interval);
	CLASS_ATTR_CATEGORY             (c, "interval", 0, "Rendering");
	CLASS_ATTR_ORDER                (c, "interval", 0, "1");
	CLASS_ATTR_LABEL                (c, "interval", 0, "Refresh Interval in Milliseconds");
	CLASS_ATTR_FILTER_MIN           (c, "interval", 20);
	CLASS_ATTR_DEFAULT              (c, "interval", 0, "100");
	CLASS_ATTR_SAVE                 (c, "interval", 1);
	// @description The refresh interval time in milliseconds.
    
	CLASS_ATTR_DOUBLE_ARRAY         (c, "camera", 0, t_hoa_scope, f_camera, 2);
	CLASS_ATTR_CATEGORY             (c, "camera", 0, "Rendering");
    CLASS_ATTR_ORDER                (c, "camera", 0, "2");
	CLASS_ATTR_LABEL                (c, "camera", 0, "Camera");
	CLASS_ATTR_DEFAULT_SAVE         (c, "camera", 0, "0. 0.");
	CLASS_ATTR_ACCESSORS            (c, "camera", NULL, hoa_scope_attr_set_camera);
    CLASS_ATTR_PAINT                (c, "camera", 1);
	// @description The scene point of view.
    
    CLASS_ATTR_ATOM_LONG            (c, "vectors", 0, t_hoa_scope, f_vectors);
	CLASS_ATTR_CATEGORY             (c, "vectors", 0, "Rendering");
	CLASS_ATTR_ORDER                (c, "vectors", 0, "3");
	CLASS_ATTR_STYLE_LABEL          (c, "vectors", 0, "onoff", "Draw Vectors");
	CLASS_ATTR_DEFAULT              (c, "vectors", 0, "1");
	CLASS_ATTR_SAVE                 (c, "vectors", 1);
    CLASS_ATTR_PAINT                (c, "vectors", 1);
	// @description Display or not the cartesian vectors.
    
    CLASS_ATTR_ATOM_LONG            (c, "sphere", 0, t_hoa_scope, f_sphere);
	CLASS_ATTR_CATEGORY             (c, "sphere", 0, "Rendering");
	CLASS_ATTR_ORDER                (c, "sphere", 0, "4");
    CLASS_ATTR_STYLE_LABEL          (c, "sphere", 0, "onoff", "Draw Sphere");
	CLASS_ATTR_DEFAULT              (c, "sphere", 0, "1");
	CLASS_ATTR_SAVE                 (c, "sphere", 1);
    CLASS_ATTR_PAINT                (c, "sphere", 1);
	// @description Display or not the sphere.
    
    CLASS_ATTR_ATOM_LONG            (c, "style", 0, t_hoa_scope, f_mode);
	CLASS_ATTR_CATEGORY             (c, "style", 0, "Rendering");
	CLASS_ATTR_ORDER                (c, "style", 0, "5");
    CLASS_ATTR_ENUMINDEX2           (c, "style", 0, "Shape", "Mapping");
    CLASS_ATTR_LABEL                (c, "style", 0, "Style");
	CLASS_ATTR_DEFAULT              (c, "style", 0, "0");
    CLASS_ATTR_FILTER_CLIP          (c, "style", 0, 1);
	CLASS_ATTR_SAVE                 (c, "style", 1);
    CLASS_ATTR_PAINT                (c, "style", 1);
	// @description Display mode of the spherical harmonics.
	
	CLASS_ATTR_ATOM_LONG            (c, "colorinterp", 0, t_hoa_scope, f_color_interp_mode);
	CLASS_ATTR_CATEGORY             (c, "colorinterp", 0, "Rendering");
	CLASS_ATTR_ORDER                (c, "colorinterp", 0, "5");
    CLASS_ATTR_ENUMINDEX3           (c, "colorinterp", 0, "Linear", "Cosine", "Absolute");
    CLASS_ATTR_LABEL                (c, "colorinterp", 0, "Color interpolation type");
	CLASS_ATTR_DEFAULT              (c, "colorinterp", 0, "0");
    CLASS_ATTR_FILTER_CLIP          (c, "colorinterp", 0, 2);
	CLASS_ATTR_SAVE                 (c, "colorinterp", 1);
    CLASS_ATTR_PAINT                (c, "colorinterp", 1);
	// @description Color interpolation mode (only works in <m>mapping</m> drawing mode)
    
    CLASS_ATTR_DOUBLE               (c, "gain", 0, t_hoa_scope, f_gain);
	CLASS_ATTR_CATEGORY             (c, "gain", 0, "Rendering");
	CLASS_ATTR_ORDER                (c, "gain", 0, "6");
	CLASS_ATTR_LABEL                (c, "gain", 0, "Gain factor");
	CLASS_ATTR_FILTER_MIN           (c, "gain", 1.);
	CLASS_ATTR_DEFAULT              (c, "gain", 0, "1.");
	CLASS_ATTR_SAVE                 (c, "gain", 1);
	// @description The <b>gain</b> factor can be used to offer a better visualisation of low amplitude sound fields.
	
	CLASS_ATTR_ATOM_LONG            (c, "light", 0, t_hoa_scope, f_light);
	CLASS_ATTR_CATEGORY             (c, "light", 0, "Rendering");
	CLASS_ATTR_ORDER                (c, "light", 0, "3");
	CLASS_ATTR_STYLE_LABEL          (c, "light", 0, "onoff", "Enable light");
	CLASS_ATTR_DEFAULT              (c, "light", 0, "1");
	CLASS_ATTR_SAVE                 (c, "light", 1);
    CLASS_ATTR_PAINT                (c, "light", 1);
	// @description Activate or deactivate lighting.
    
	class_register(CLASS_BOX, c);
	hoa_scope_class = c;
    
	return 0;
}

void *hoa_scope_new(t_symbol *s, long argc, t_atom *argv)
{
	t_hoa_scope *x    = NULL;
 	t_dictionary *d = NULL;
	long flags;
    
	if (!(d = object_dictionaryarg(argc,argv)))
		return NULL;
    
	x = (t_hoa_scope *)object_alloc(hoa_scope_class);
	flags = 0
    | JBOX_DRAWFIRSTIN
    | JBOX_NODRAWBOX
    | JBOX_DRAWINLAST
    | JBOX_TRANSPARENT
    | JBOX_GROWY
    | JBOX_DRAWBACKGROUND
    ;
    
	jbox_new((t_jbox *)x, flags, argc, argv);
    x->j_box.j_box.z_box.b_firstin = (t_object *)x;
    
    dictionary_getlong(d, gensym("order"), (t_atom_long *)&x->f_order);
    x->f_scope      = new Hoa3D::Scope(x->f_order, 100, 199);
    x->f_order      = x->f_scope->getDecompositionOrder();
    x->f_signals    = new double[x->f_scope->getNumberOfHarmonics() * SYS_MAXBLKSIZE];
    x->f_clock      = clock_new((void *)x, (method)hoa_scope_tick);
    x->f_startclock = 0;
    x->f_index      = 0;
    
    dsp_setupjbox((t_pxjbox *)x, x->f_scope->getNumberOfHarmonics());
	jucebox_new((t_jucebox *) x);
    
	attr_dictionary_process(x,d);
	jbox_ready((t_jbox *)x);
	return x;
}

t_max_err hoa_scope_attr_set_order(t_hoa_scope *x, t_object *attr, long ac, t_atom *av)
{
    long d;
    t_object *b = NULL;
	if (ac && av)
    {
        if(atom_gettype(av) == A_LONG)
        {
            d = clip_min(long(atom_getlong(av)), long(1));
            if (d != x->f_order)
            {
                int dspState = sys_getdspobjdspstate((t_object*)x);
                if(dspState)
                    object_method(gensym("dsp")->s_thing, gensym("stop"));
                
                delete x->f_scope;
                delete [] x->f_signals;
                x->f_scope      = new Hoa3D::Scope(d, 100, 199);
                x->f_order      = x->f_scope->getDecompositionOrder();
                x->f_signals    = new double[x->f_scope->getNumberOfHarmonics() * SYS_MAXBLKSIZE];
                
                object_obex_lookup(x, gensym("#B"), (t_object **)&b);
                object_method(b, gensym("dynlet_begin"));
                dsp_resize((t_pxobject*)x, x->f_scope->getNumberOfHarmonics());
                object_method(b, gensym("dynlet_end"));
                
                if(dspState)
                    object_method(gensym("dsp")->s_thing, gensym("start"));
            }
        }
	}
    
	return MAX_ERR_NONE;
}

void hoa_scope_dsp64(t_hoa_scope *x, t_object *dsp64, short *count, double samplerate, long maxvectorsize, long flags)
{
	x->f_index = 0;
    object_method(dsp64, gensym("dsp_add64"), x, hoa_scope_perform64, 0, NULL);
    x->f_startclock = 1;
}

void hoa_scope_perform64(t_hoa_scope *x, t_object *dsp64, double **ins, long numins, double **outs, long numouts, long sampleframes, long flags, void *userparam)
{
	for(int i = 0; i < numins; i++)
    {
        cblas_dcopy(sampleframes, ins[i], 1, x->f_signals+i, numins);
    }
    cblas_dscal(numins * sampleframes, x->f_gain, x->f_signals, 1);
    x->f_index = 0;
    while(--sampleframes)
    {
        x->f_index++;
    }
    if(x->f_startclock)
	{
		x->f_startclock = 0;
		clock_delay(x->f_clock,0);
	}
}

void hoa_scope_tick(t_hoa_scope *x)
{
    x->f_scope->process(x->f_signals + x->f_index * x->f_scope->getNumberOfHarmonics());
    jbox_redraw((t_jbox *)x);
	if (sys_getdspstate())
		clock_fdelay(x->f_clock, x->f_interval);
}

t_max_err hoa_scope_attr_set_camera(t_hoa_scope *x, t_object *attr, long argc, t_atom *argv)
{
	if(argc && argv)
    {
		for(int i = 0; i < 2 && i < argc; i++)
		{
            if(atom_gettype(argv+i) == A_FLOAT)
                x->f_camera[i] = wrap(atom_getfloat(argv+i), -HOA_PI, HOA_PI);
		}
    }
    return MAX_ERR_NONE;
}

void hoa_scope_getdrawparams(t_hoa_scope *x, t_object *patcherview, t_jboxdrawparams *params)
{
	params->d_bordercolor = x->f_color_bd;
    params->d_boxfillcolor = x->f_color_bg;
	params->d_borderthickness = 1;
	params->d_cornersize = 4;
}

t_max_err hoa_scope_notify(t_hoa_scope *x, t_symbol *s, t_symbol *m, void *sender, void *data)
{
	return jbox_notify((t_jbox *)x, s, m, sender, data);
}

t_hoa_err hoa_getinfos(t_hoa_scope* x, t_hoa_boxinfos* boxinfos)
{
	boxinfos->object_type = HOA_OBJECT_3D;
	boxinfos->autoconnect_inputs = x->f_scope->getNumberOfHarmonics();
	boxinfos->autoconnect_outputs = 0;
	boxinfos->autoconnect_inputs_type = HOA_CONNECT_TYPE_AMBISONICS;
	boxinfos->autoconnect_outputs_type = HOA_CONNECT_TYPE_STANDARD;
	return HOA_ERR_NONE;
}

void hoa_scope_assist(t_hoa_scope *x, void *b, long m, long a, char *s)
{
    sprintf(s,"(Signal) %s", x->f_scope->getHarmonicName(a).c_str());
}

void hoa_draw_vectors(t_hoa_scope *x)
{
	glLineWidth(4);
	glBegin(GL_LINE_STRIP);
	glColor4d(0., 1., 0., 1.);
	glVertex3d(0, 0, 1);
    glVertex3d(0, 0, 0);
    glVertex3d(0, 1, 0);
    glVertex3d(0, 0, 0);
    glVertex3d(1, 0, 0);
    glEnd();
}

void hoa_draw_sphere(t_hoa_scope *x, t_jrgba color)
{
    double one, cos_one, sin_one, two ,cos_two, sin_two;
    
	glLineWidth(1);
    glColor4d(color.red, color.green, color.blue, color.alpha);
    for(int i = 1; i < 10; i++)
    {
        one  =   0;
        two = (double)i / 10. * HOA_PI;
        cos_two = cos(two);
        sin_two = sin(two);
        glBegin(GL_LINE_LOOP);
        for(int j = 0; j < 50; j++)
        {
            one  =   (double)j / 50. * HOA_2PI;
            cos_one = cos(one);
            sin_one = sin(one);
            glVertex3d(sin_two * cos_one, cos_two, sin_two * sin_one);
        }
        glEnd();
    }
    
    for(int j = 0; j < 20; j++)
    {
        one  =   (double)j / 20. * HOA_2PI;
        two  = 0.;
        cos_one = cos(one);
        sin_one = sin(one);
        glBegin(GL_LINE_STRIP);
        for(int i = 0; i < 25; i++)
        {
            two = (double)i / 20. * HOA_PI;
            cos_two = cos(two);
            sin_two = sin(two);
            glVertex3d(sin_two * cos_one, cos_two, sin_two * sin_one);
        }
        glEnd();
    }
}

void hoa_draw_harmonics_shape(t_hoa_scope *x)
{
	int number_of_rows = x->f_scope->getNumberOfRows();
	int number_of_columns = x->f_scope->getNumberOfColumns();
	float value;
    float azimuth, elevation;
	t_jrgba color_positive = x->f_color_ph;
	t_jrgba color_negative = x->f_color_nh;
	
	glBegin(GL_TRIANGLE_STRIP);
    for(int i = 1; i < number_of_rows; i++)
	{
		for(int j = 0; j < number_of_columns; j++)
		{
			azimuth     =  x->f_scope->getAzimuth(j);
			elevation   = x->f_scope->getElevation(i-1);
			value       = x->f_scope->getValue(i-1, j);
			if(value < 0)
			{
				glColor4d(color_negative.red, color_negative.green, color_negative.blue, color_negative.alpha);
				value= -value;
			}
			else
				glColor4d(color_positive.red, color_positive.green, color_positive.blue, color_positive.alpha);
			
			glVertex3d(abscissa(value, azimuth, elevation), height(value, azimuth, elevation), ordinate(value, azimuth, elevation));
			
			elevation   = x->f_scope->getElevation(i);
			value       = x->f_scope->getValue(i, j);
			if(value < 0)
			{
				glColor4d(color_negative.red, color_negative.green, color_negative.blue, color_negative.alpha);
				value= -value;
			}
			else
				glColor4d(color_positive.red, color_positive.green, color_positive.blue, color_positive.alpha);
			
			glVertex3d(abscissa(value, azimuth, elevation), height(value, azimuth, elevation), ordinate(value, azimuth, elevation));
			
		}
	}
	glEnd();
}

double cosine_interpolation(double mu, double y1, float y2)
{
    double mu2;
    mu2 = (1-cos(mu*HOA_PI))/2;
    return(y1*(1-mu2)+y2*mu2);;
}

double color_interp(t_hoa_scope *x, double val, double y1, float y2)
{
	if (x->f_color_interp_mode == 0)
	{
		return y2 * val + y1 * (1. - val);
	}
    else if (x->f_color_interp_mode == 1)
	{
		double val2;
		val2 = (1-cos(val*HOA_PI))/2;
		return (y1*(1-val2)+y2*val2);
	}
	else
	{
		return (val < 0.5) ? y1 : y2;
	}
}

void hoa_draw_harmonics_mapping(t_hoa_scope *x)
{
	int number_of_rows = x->f_scope->getNumberOfRows();
	int number_of_columns = x->f_scope->getNumberOfColumns();
    
	float value;
    float azimuth, elevation;
	t_jrgba color_positive = x->f_color_ph;
	t_jrgba color_negative = x->f_color_nh;
	
	glBegin(GL_TRIANGLE_STRIP);
	for(int i = 1; i < number_of_rows; i++)
	{
		for(int j = 0; j < number_of_columns; j++)
		{
			azimuth     =  x->f_scope->getAzimuth(j);
			elevation   = x->f_scope->getElevation(i-1);
			value       = (x->f_scope->getValue(i-1, j) + 1.) * 0.5;
			
			glColor4d(color_interp(x, value, color_negative.red, color_positive.red), color_interp(x, value, color_negative.green, color_positive.green), color_interp(x, value, color_negative.blue, color_positive.blue), color_interp(x, value, color_negative.alpha, color_positive.alpha));
			
			glVertex3d(abscissa(1., azimuth, elevation), height(1., azimuth, elevation), ordinate(1., azimuth, elevation));
			
			elevation   = x->f_scope->getElevation(i);
			value       = (x->f_scope->getValue(i, j) + 1.) * 0.5;
			
			glColor4d(color_interp(x, value, color_negative.red, color_positive.red), color_interp(x, value, color_negative.green, color_positive.green), color_interp(x, value, color_negative.blue, color_positive.blue), color_interp(x, value, color_negative.alpha, color_positive.alpha));
			
			glVertex3d(abscissa(1., azimuth, elevation), height(1., azimuth, elevation), ordinate(1., azimuth, elevation));
		}
	}
	glEnd();
}

void hoa_draw_lighting(t_hoa_scope *x)
{
	if (x->f_light && x->f_mode == 0)
	{
		// enable lighting
		glEnable(GL_LIGHTING);
		glEnable(GL_LIGHT0);
		glEnable(GL_COLOR_MATERIAL);
		glColorMaterial ( GL_FRONT_AND_BACK, GL_AMBIENT ) ;
		
		//Add ambient light
		GLfloat ambientColor[] = {0.6f, 0.6f, 0.6f, 1.0f};
		glLightModelfv(GL_LIGHT_MODEL_AMBIENT, ambientColor);
		
		//Add positioned light
		GLfloat lightColor0[] = {0.5f, 0.5f, 0.5f, 1.0f};
		glLightfv(GL_LIGHT0, GL_DIFFUSE, lightColor0);
		//GLfloat lightPos0[] = {0, 0, 0, 1.};
		GLfloat lightPos0[] = {0, 0, 0, 1.};
		glLightfv(GL_LIGHT0, GL_POSITION, lightPos0);
	}
	else
	{
		glDisable(GL_LIGHTING);
	}
}

void CrossProd(float x1, float y1, float z1, float x2, float y2, float z2, float res[3])
{
	res[0] = y1*z2 - y2*z1;
	res[1] = x2*z1 - x1*z2;
	res[2] = x1*y2 - x2*y1;
}

// my own implementation
void CicmGluLookAt(float eyeX, float eyeY, float eyeZ, float lookAtX, float lookAtY, float lookAtZ, float upX, float upY, float upZ)
{
	// i am not using here proper implementation for vectors.
	// if you want, you can replace the arrays with your own
	// vector types
	float f[3];
	
	// calculating the viewing vector
	f[0] = lookAtX - eyeX;
	f[1] = lookAtY - eyeY;
	f[2] = lookAtZ - eyeZ;
	
	float fMag, upMag;
	fMag = sqrt(f[0]*f[0] + f[1]*f[1] + f[2]*f[2]);
	upMag = sqrt(upX*upX + upY*upY + upZ*upZ);
	
	// normalizing the viewing vector
	if( fMag != 0)
	{
		f[0] = f[0]/fMag;
		f[1] = f[1]/fMag;
		f[2] = f[2]/fMag;
	}
	
	// normalising the up vector. no need for this here if you have your
	// up vector already normalised, which is mostly the case.
	if( upMag != 0 )
	{
		upX = upX/upMag;
		upY = upY/upMag;
		upZ = upZ/upMag;
	}
	
	float s[3], u[3];
	
	CrossProd(f[0], f[1], f[2], upX, upY, upZ, s);
	CrossProd(s[0], s[1], s[2], f[0], f[1], f[2], u);
	
	float M[]=
	{
		s[0], u[0], -f[0], 0,
		s[1], u[1], -f[1], 0,
		s[2], u[2], -f[2], 0,
		0, 0, 0, 1
	};
	
	glMultMatrixf(M);
	glTranslatef (-eyeX, -eyeY, -eyeZ); 
}

void hoa_draw_camera(t_hoa_scope *x)
{
	/*
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	glOrtho(1.0, 1.0, 1.0, 1.0, 1.0, 1.0);
	glFrustum(-1, 1, -1, 1, 0.0, 40.0);
	//glOrtho(-0.8, 0.8, -0.8, 0.8, 0, 4.0);
	//CicmGluLookAt(0, 1, -1, 0, 0, 0, 0, 1, 0);
	*/
	
	glRotated(x->f_camera[0] / HOA_2PI * 360., 0., 1., 0.);
	glRotated(-x->f_camera[1] / HOA_2PI * 360., 1., 0., 0.);
}

void hoa_scope_paint(t_hoa_scope *x, double w, double h)
{
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glEnable(GL_DEPTH_TEST); // enable depth buffer
	
	//glMatrixMode(GL_PROJECTION);
	//glLoadIdentity();
	
	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();
		
	hoa_draw_lighting(x);
	hoa_draw_camera(x);

	if (x->f_mode == 0)
	{
		// draw sphere
		if(x->f_sphere)
			hoa_draw_sphere(x, x->f_color_sp);
		
		// draw vectors
		if(x->f_vectors)
			hoa_draw_vectors(x);
		
		hoa_draw_harmonics_shape(x);
	}
	else
	{
		hoa_draw_harmonics_mapping(x);
	}
}

void hoa_scope_mousedown(t_hoa_scope *x, t_object *patcherview, t_pt pt, long modifiers)
{
    t_rect rect;
    jbox_get_rect_for_view((t_object *)x, patcherview, &rect);
    x->f_mouse = pt;
    x->f_camera_ref[0] = x->f_camera[0];
    x->f_camera_ref[1] = x->f_camera[1];
}

void hoa_scope_mousedrag(t_hoa_scope *x, t_object *patcherview, t_pt pt, long modifiers)
{
    t_atom av[2];
    t_rect rect;
    jbox_get_rect_for_view((t_object *)x, patcherview, &rect);
    atom_setfloat(av, x->f_camera_ref[0] + (pt.x - x->f_mouse.x) / (rect.width * 0.5) * HOA_PI2);
    atom_setfloat(av+1, x->f_camera_ref[1] + (pt.y - x->f_mouse.y) / (-rect.height * 0.5) * HOA_PI2);
    object_method(x, gensym("camera"), 2, av);
}

void hoa_scope_dblclick(t_hoa_scope *x, t_object *patcherview, t_pt pt, long modifiers)
{
	t_atom av[2];
    atom_setfloat(av, 0);
    atom_setfloat(av+1, 0);
    object_method(x, gensym("camera"), 2, av);
}

void hoa_scope_free(t_hoa_scope *x)
{
    clock_free((t_object *)x->f_clock);
    dsp_freejbox((t_pxjbox *)x);
    jucebox_free((t_jucebox*)x);
	delete x->f_scope;
    delete [] x->f_signals;
}
