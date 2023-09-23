#pragma once

#include "partition.h"
#include "dct_volume.h"

class DctPartition : public Partition
{
	real_t lx2_, ly2_, lz2_;	// actual length ^2
	real_t *cwt_{ nullptr };	// cos(wt)
	real_t *w2_{ nullptr };		// w^2

	DctVolume pressure_;
	DctVolume force_;

	real_t *prev_modes_{ nullptr };
	real_t *next_modes_{ nullptr };

public:
	DctPartition(int xs, int ys, int zs, int w, int h, int d, VkGPU* vkGPU);
	~DctPartition();

	virtual void Update();

	virtual real_t* get_pressure_field();
	virtual real_t get_pressure(int x, int y, int z);
	virtual void set_force(int x, int y, int z, real_t f);
	virtual std::vector<real_t> get_xy_forcing_plane(int z);

	real_t get_force(int x, int y, int z);
	std::vector<real_t> get_xy_force_plane(int z);
	friend class Boundary;
};
