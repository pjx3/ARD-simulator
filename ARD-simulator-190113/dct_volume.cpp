#include "dct_volume.h"
#include <assert.h>

DctVolume::DctVolume(int w, int h, int d, VkGPU* vkGPU) 
	: m_width(w)
	, m_height(h)
	, m_depth(d)
	, m_gpu(true)
{
	int numCells = m_width * m_height * m_depth;

	m_values = (real_t*)calloc(numCells, sizeof(real_t));
	m_modes = (real_t*)calloc(numCells, sizeof(real_t));

	// FFTW plans
	// FFTW_REDFT10 == DCT-II (the DCT)
	m_dct = fftwf_plan_r2r_3d(m_depth, m_height, m_width, m_values, m_modes, FFTW_REDFT10, FFTW_REDFT10, FFTW_REDFT10, FFTW_MEASURE);
	// FFTW_REDFT01 == IDCT-III (the IDCT)
	m_idct = fftwf_plan_r2r_3d(m_depth, m_height, m_width, m_modes, m_values, FFTW_REDFT01, FFTW_REDFT01, FFTW_REDFT01, FFTW_MEASURE);

	// vkFFT applications
	m_vkFFTdct = new VkFFT_DCT(vkGPU, 2, m_width, m_height, m_depth, m_values, m_modes);
	assert(m_vkFFTdct != nullptr);
	m_vkFFTidct = new VkFFT_DCT(vkGPU, 3, m_width, m_height, m_depth, m_modes, m_values);
	assert(m_vkFFTidct != nullptr);
}

DctVolume::~DctVolume()
{
	delete m_vkFFTdct;
	delete m_vkFFTidct;
	fftwf_destroy_plan(m_dct);
	fftwf_destroy_plan(m_idct);
	free(m_values);
	free(m_modes);
}

void DctVolume::ExcuteDct()
{
	// pressure to modes
	if (m_gpu)	
		m_vkFFTdct->execute();
	else		
		fftwf_execute(m_dct);

	// FFTW3 does not normalize values, so we must perform this step, or values will be wacky.
	int const total = m_depth * m_height * m_width;
	float const scale = 1.0f / (2.0f * sqrtf(2.0f * m_depth * m_width * m_height));
	for (int i = 0; i < total; i++) {
		m_modes[i] *= scale;
	}
}

void DctVolume::ExcuteIdct()
{
	// modes to pressure
	if (m_gpu)	
		m_vkFFTidct->execute();
	else		
		fftwf_execute(m_idct);

	// Normalization
	int const total = m_depth * m_height * m_width;
	float const scale = 1.0f / sqrtf(2.0f * m_depth * m_width * m_height);
	for (int i = 0; i < total; i++)	{
		m_values[i] *= scale;
	}
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
