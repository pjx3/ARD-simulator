#include "dct_volume.h"

DctVolume::DctVolume(int w, int h, int d) :m_width(w), m_height(h), m_depth(d)
{
#if CUFFTFW
	m_values = (fftwf_complex*)calloc(m_width * m_height * m_depth, sizeof(fftwf_complex));
	m_modes = (real_t*)calloc(m_width * m_height * m_depth, sizeof(real_t));
#else

	int n = m_width * m_height * m_depth;
	m_values = (real_t*)calloc(n, sizeof(real_t));
	m_modes = (real_t*)calloc(n, sizeof(real_t));
	//m_modes = (fftwf_complex*)calloc(n, sizeof(fftwf_complex));
	
	// FFTW_REDFT10 == DCT-II (the DCT)
	m_dct = fftwf_plan_r2r_3d(m_depth, m_height, m_width, m_values, m_modes, FFTW_REDFT10, FFTW_REDFT10, FFTW_REDFT10, FFTW_MEASURE);
	//m_dct = fftwf_plan_dft_r2c_3d(m_depth, m_height, m_width, m_values, m_modes, FFTW_MEASURE);

	// FFTW_REDFT01 == IDCT-III (the IDCT)
	m_idct = fftwf_plan_r2r_3d(m_depth, m_height, m_width, m_modes, m_values, FFTW_REDFT01, FFTW_REDFT01, FFTW_REDFT01, FFTW_MEASURE);
	//m_idct = fftwf_plan_dft_c2r_3d(m_depth, m_height, m_width, m_modes, m_values, FFTW_MEASURE);
#endif
}

DctVolume::~DctVolume()
{
	fftwf_destroy_plan(m_dct);
	fftwf_destroy_plan(m_idct);
	free(m_values);
	free(m_modes);
}

void DctVolume::ExcuteDct()
{
	fftwf_execute(m_dct);
	// FFTW3 does not normalize values, so we must perform this
	// step, or values will be wacky.
#if 0
	for (int i = 0; i < depth_; i++)
	{
		for (int j = 0; j < height_; j++)
		{
			for (int k = 0; k < width_; k++)
			{
				modes_[i * height_ * width_ + j * width_ + k] /= 2.0 * sqrt(2.0 * depth_ * width_ * height_);
			}
		}
	}
#else
	int const total = m_depth * m_height * m_width;
	float const scale = 1.0f / (2.0f * sqrtf(2.0f * m_depth * m_width * m_height));
	for (int i = 0; i < total; i++)
	{
		m_modes[i] *= scale;
	}
#endif
}

void DctVolume::ExcuteIdct()
{
	fftwf_execute(m_idct); 
	// Normalization
#if 0
	for (int i = 0; i < depth_; i++)
	{
		for (int j = 0; j < height_; j++)
		{
			for (int k = 0; k < width_; k++)
			{
				values_[i * height_ * width_ + j * width_ + k] /= sqrt(2.0 * depth_ * width_ * height_);
			}
		}
	}
#else
	int const total = m_depth * m_height * m_width;
	float const scale = 1.0f / sqrtf(2.0f * m_depth * m_width * m_height);
	for (int i = 0; i < total; i++)
	{
		m_values[i] *= scale;
	}
#endif
}

real_t DctVolume::get_value(int x, int y, int z)
{
	return m_values[z * m_height * m_width + y * m_width + x];
}

real_t DctVolume::get_mode(int x, int y, int z)
{
	return m_modes[z * m_height * m_width + y * m_width + x];
}

void DctVolume::set_value(int x, int y, int z, real_t v)
{
	m_values[z * m_height * m_width + y * m_width + x] = v;
}

void DctVolume::set_mode(int x, int y, int z, real_t m)
{
	m_modes[z * m_height * m_width + y * m_width + x] = m;
}
