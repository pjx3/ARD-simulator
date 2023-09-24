#pragma once

#include <fftw3.h>
#include <memory>
#include <cmath>
#include "types.h"
#include "utils_VkFFT.h"

class DctVolume
{
public:
	DctVolume(int w, int h, int d, VkGPU* vkGPU);
	~DctVolume();

	void ExcuteDct();
	void ExcuteIdct();

	real_t get_value(int x, int y, int z);
	real_t get_mode(int x, int y, int z);
	void set_value(int x, int y, int z, real_t v);
	void set_mode(int x, int y, int z, real_t m);

private:
	int			m_width;
	int			m_height;
	int			m_depth;
	real_t*		m_values;
	real_t*		m_modes;
	fftwf_plan	m_dct;
	fftwf_plan	m_idct;
	VkFFT_DCT*	m_vkFFTdct;		// forward DCT (DCT-II)
	VkFFT_DCT*	m_vkFFTidct;	// inverse DCT (DCT-III)
	bool		m_gpu{ false };	// use GPU or CPU

	friend class Partition;
	friend class DctPartition;
};
