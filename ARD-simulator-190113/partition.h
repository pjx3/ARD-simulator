#pragma once
#include <string>
#include <vector>
#include <memory>
//#include "sound_source.h"
#include "types.h"

class Boundary;
class SoundSource;

class Partition
{
protected:
	real_t dh_;
	real_t dt_;
	real_t c0_;
	int x_start_, x_end_;
	int y_start_, y_end_;
	int z_start_, z_end_;
	int width_, height_, depth_;
	struct Info
	{
		int id;
		std::string type;
		int num_sources{ 0 };
		int num_boundaries{ 0 };
	} info_;

	std::vector<std::shared_ptr<SoundSource>> sources_;
	std::vector<std::vector<int>> right_free_borders_;
	std::vector<std::vector<int>> left_free_borders_;
	std::vector<std::vector<int>> top_free_borders_;
	std::vector<std::vector<int>> bottom_free_borders_;
	std::vector<std::vector<int>> front_free_borders_;
	std::vector<std::vector<int>> back_free_borders_;

	bool include_self_terms_{ true };
	bool should_render_{ true };
	bool is_x_pml_{ false };
	bool is_y_pml_{ false };
	bool is_z_pml_{ false };
	
public:
	static real_t absorption_;

	Partition(int xs, int ys, int zs, int w, int h, int d);
	~Partition();

	virtual void Update() = 0;

	virtual real_t* get_pressure_field() = 0;
	std::vector<real_t> get_xy_plane(int z);
	std::vector<real_t> get_yz_plane(int x);
	std::vector<real_t> get_xz_plane(int y);
	virtual real_t get_pressure(int x, int y, int z) = 0;
	virtual void set_force(int x, int y, int z, real_t f) = 0;
	virtual std::vector<real_t> get_xy_forcing_plane(int z);

	void AddBoundary(std::shared_ptr<Boundary> boundary);
	void AddSource(std::shared_ptr<SoundSource> source);
	static std::vector<std::shared_ptr<Partition>> ImportPartitions(std::string path);
	void Info();

	void ComputeSourceForcingTerms(real_t t);

	friend class Boundary;
	//friend class SoundSource;
	friend class Simulation;
	friend class Tools;
	friend class PmlPartition;
	friend class Recorder;
};

