#define _USE_MATH_DEFINES
#include <cmath>
#include "dct_partition.h"


DctPartition::DctPartition(int xs, int ys, int zs, int w, int h, int d)
	: Partition(xs, ys, zs, w, h, d), pressure_(w, h, d), force_(w, h, d)
{
	should_render_ = true;
	info_.type = "DCT";

	prev_modes_ = (real_t*)calloc(width_*height_*depth_, sizeof(real_t));
	next_modes_ = (real_t*)calloc(width_*height_*depth_, sizeof(real_t));
	cwt_ = (real_t*)calloc(width_*height_*depth_, sizeof(real_t));
	w2_ = (real_t*)calloc(width_*height_*depth_, sizeof(real_t));

	lx2_ = width_ * width_*dh_*dh_;
	ly2_ = height_ * height_*dh_*dh_;
	lz2_ = depth_ * depth_*dh_*dh_;

	for (int i = 1; i <= depth_; i++)
	{
		for (int j = 1; j <= height_; j++)
		{
			for (int k = 1; k <= width_; k++)
			{
				int idx = (i - 1) * height_ * width_ + (j - 1) * width_ + (k - 1);
				real_t w = c0_ * (float)M_PI * sqrtf(i * i / lz2_ + j * j / ly2_ + k * k / lx2_);
				w2_[idx] = w * w;
				cwt_[idx] = cosf(w * dt_);
			}
		}
	}
}


DctPartition::~DctPartition()
{
	free(prev_modes_);
	free(next_modes_);
	free(cwt_);
	free(w2_);
}

void DctPartition::Update()
{
	force_.ExcuteDct();
#if 0
	for (int i = 0; i < depth_; i++)
	{
		for (int j = 0; j < height_; j++)
		{
			for (int k = 0; k < width_; k++)
			{
				int idx = i * height_ * width_ + j * width_ + k;
				next_modes_[idx] = 0.999 * (2.0 * pressure_.modes_[idx] * cwt_[idx] - prev_modes_[idx] + (2.0 * force_.modes_[idx] / w2_[idx]) * (1.0 - cwt_[idx]));
			}
		}
	}
	memcpy((void*)prev_modes_, (void*)pressure_.modes_, depth_ * width_ * height_ * sizeof(real_t));
	memcpy((void*)pressure_.modes_, (void*)next_modes_, depth_ * width_ * height_ * sizeof(real_t));
#else
	int total = depth_ * height_ * width_;
	for (int idx = 0; idx < total; idx++)
		next_modes_[idx] = 0.999f * (2.0f * pressure_.m_modes[idx] * cwt_[idx] - prev_modes_[idx] + (2.0f * force_.m_modes[idx] / w2_[idx]) * (1.0f - cwt_[idx]));
	memcpy((void*)prev_modes_, (void*)pressure_.m_modes, depth_ * width_ * height_ * sizeof(real_t));
	memcpy((void*)pressure_.m_modes, (void*)next_modes_, depth_ * width_ * height_ * sizeof(real_t));
#endif
	pressure_.ExcuteIdct();
}

real_t* DctPartition::get_pressure_field()
{
	return pressure_.m_values;
}

real_t DctPartition::get_pressure(int x, int y, int z)
{
	return pressure_.get_value(x, y, z);
}

void DctPartition::set_force(int x, int y, int z, real_t f)
{
	force_.set_value(x, y, z, f);
}

std::vector<real_t> DctPartition::get_xy_forcing_plane(int z)
{
	std::vector<real_t> xy_plane;
	for (int i = 0; i < height_; i++) {
		for (int j = 0; j < width_; j++) {
			xy_plane.push_back(get_force(j, i, z));
		}
	}
	return xy_plane;
}

real_t DctPartition::get_force(int x, int y, int z)
{
	return force_.get_value(x, y, z);
}

std::vector<real_t> DctPartition::get_xy_force_plane(int z)
{
	std::vector<real_t> xy_plane;
	for (int i = 0; i < height_; i++) {
		for (int j = 0; j < width_; j++) {
			xy_plane.push_back(get_force(j, i, z));
		}
	}
	return xy_plane;
}
