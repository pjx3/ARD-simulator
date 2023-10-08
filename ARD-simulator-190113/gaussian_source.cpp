#define _USE_MATH_DEFINES
#include <cmath>
#include "gaussian_source.h"
#include "simulation.h"


GaussianSource::GaussianSource(int x, int y, int z) :SoundSource(x, y, z)
{
}


GaussianSource::~GaussianSource()
{
}

real_t GaussianSource::SampleValue(real_t t)
{
	real_t arg = powf((float)M_PI * ((2 * (Simulation::m_c0*Simulation::m_dt / Simulation::m_dh) * t) / 6 - 2.0f), 2);
	return 1e9f * expf(-arg);
}
