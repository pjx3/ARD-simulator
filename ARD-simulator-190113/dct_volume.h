#pragma once
#define CUFFTFW 0
#if CUFFTFW
#include <cufftw.h>
#else
#include <fftw3.h>
#endif
#include <memory>
#include <cmath>
#include "types.h"

class DctVolume
{
	int m_width;
	int m_height;
	int m_depth;

#if CUFFTFW
	float2* m_values;			
	float2* m_modes;
	cufftHandle m_dct;
	cufftHandle m_idct_plan;
#else
	real_t* m_values;
	real_t* m_modes;
	//fftwf_complex* m_modes;
	fftwf_plan m_dct;
	fftwf_plan m_idct;
#endif


public:
	DctVolume(int w, int h, int d);
	~DctVolume();
	void ExcuteDct();
	void ExcuteIdct();
	real_t get_value(int x, int y, int z);
	real_t get_mode(int x, int y, int z);
	void set_value(int x, int y, int z, real_t v);
	void set_mode(int x, int y, int z, real_t m);

	friend class Partition;
	friend class DctPartition;
};

