#pragma once
#include "sound_source.h"

class GaussianSource :public SoundSource
{
public:
	GaussianSource(int x, int y,int z);
	~GaussianSource();

	virtual real_t SampleValue(real_t t);
};

