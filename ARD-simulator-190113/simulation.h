#pragma once
#include <vector>
#include <memory>
#include <string>
#include <SDL.h>
#include "types.h"

class Partition;
class Boundary;
class SoundSource;

class Simulation
{
	struct Info
	{
		size_t num_partitions{ 0 };
		size_t num_dct_partitions{ 0 };
		size_t num_pml_partitions{ 0 };
		size_t num_sources{ 0 };
		size_t num_boundaries{ 0 };
		std::vector<std::vector<char>> model_map;
	};

	std::vector<std::shared_ptr<Partition>>		m_partitions;
	std::vector<std::shared_ptr<Boundary>>		m_boundaries;
	std::vector<std::shared_ptr<SoundSource>>	m_sources;

	int x_start_, x_end_;
	int y_start_, y_end_;
	int z_start_, z_end_;

	int size_x_, size_y_, size_z_;

	bool ready_;
	std::vector<Uint32> pixels_;
	
	Info info_;

public:

	static real_t m_duration;

	static real_t m_dh;
	static real_t m_dt;
	static real_t m_c0;
	static int m_pml_layers;

	int time_step_{ 0 };

	int look_from_{ 0 };	// 0: visualize xy plane
							// 1: visualize yz plane
							// 2: 

	Simulation(std::vector<std::shared_ptr<Partition>> &partitions, std::vector<std::shared_ptr<SoundSource>> &sources);
	~Simulation();

	int Update();

	void Info();
	//FindBoundaries();

	int size_x()
	{
		return size_x_;
	}
	int size_y()
	{
		return size_y_;
	}
	int size_z()
	{
		return size_z_;
	}
	bool ready()
	{
		return ready_;
	}
	decltype(pixels_) pixels()
	{
		return pixels_;
	}
};

