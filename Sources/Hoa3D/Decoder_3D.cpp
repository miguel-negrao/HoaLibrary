/*
// Copyright (c) 2012-2014 Eliott Paris, Julien Colafrancesco & Pierre Guillot, CICM, Universite Paris 8.
// For information on usage and redistribution, and for a DISCLAIMER OF ALL
// WARRANTIES, see the file, "LICENSE.txt," in this distribution.
*/

#include "Decoder_3D.h"

namespace Hoa3D
{
	Decoder::Decoder(unsigned int order, unsigned int numberOfChannels) : Ambisonic(order), Planewaves(numberOfChannels)
	{
        m_harmonics_vector          = new double[m_number_of_harmonics];
        m_decoder_matrix            = new double[m_number_of_channels * m_number_of_harmonics];
        m_decoder_matrix_float      = new float[m_number_of_channels * m_number_of_harmonics];
        m_encoder                   = new Encoder(m_order);
        setChannelPosition(0, m_channels_azimuth[0], m_channels_elevation[0]);
	}
	
	void Decoder::setChannelPosition(unsigned int index, double anAzimuth, double anElevation)
	{
        Planewaves::setChannelPosition(index, anAzimuth, anElevation);
        
        m_encoder->setAzimuth(m_channels_azimuth[index]);
        m_encoder->setElevation(m_channels_elevation[index]);
        m_encoder->process(12.5 / (double)((m_order+1.)*(m_order+1.)), m_harmonics_vector);
        
        for(unsigned int j = 0; j < m_number_of_harmonics; j++)
        {
            m_decoder_matrix_float[index * m_number_of_harmonics + j] = m_decoder_matrix[index * m_number_of_harmonics + j] = m_harmonics_vector[j] * m_encoder->getNormalization(j) * m_encoder->getNormalization(j);
        }
	}
	
	void Decoder::process(const float* input, float* output)
	{
		cblas_sgemv(CblasRowMajor, CblasNoTrans, m_number_of_channels, m_number_of_harmonics, 1.f, m_decoder_matrix_float, m_number_of_harmonics, input, 1, 0.f, output, 1);
	}
	
	void Decoder::process(const double* input, double* output)
	{
		cblas_dgemv(CblasRowMajor, CblasNoTrans, m_number_of_channels, m_number_of_harmonics, 1.f, m_decoder_matrix, m_number_of_harmonics, input, 1, 0.f, output, 1);
	}
	
	Decoder::~Decoder()
	{
		delete [] m_decoder_matrix;
        delete [] m_decoder_matrix_float;
        delete [] m_harmonics_vector;
        delete m_encoder;
	}
	
}

