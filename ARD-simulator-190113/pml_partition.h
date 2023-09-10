#pragma once
#include "partition.h"

class PmlPartition : public Partition
{
	std::shared_ptr<Partition> neighbor_part_;

	real_t R_{0.00001f};
	real_t zeta_;
	real_t thickness_;

	real_t* p_old_{ nullptr };
	real_t* p_{ nullptr };
	real_t* p_new_{ nullptr };

	real_t* phi_x_;
	real_t* phi_x_new_;
	real_t* phi_y_;
	real_t* phi_y_new_;
	real_t* phi_z_;
	real_t* phi_z_new_;
	real_t* force_;

	real_t* zetax_;
	real_t* zetay_;
	real_t* zetaz_;
	
	// PML damping values
	real_t kxMin_{ 0.1f };
	real_t kxMax_{ 0.1f };
	real_t kyMin_{ 0.1f };
	real_t kyMax_{ 0.1f };
	real_t kzMin_{ 0.1f };
	real_t kzMax_{ 0.1f };

	int GetIndex(int x, int y, int z);

public:
	enum PmlType {
		P_LEFT,
		P_RIGHT,
		P_TOP,
		P_BOTTOM,
		P_FRONT,
		P_BACK
	}type_;

	PmlPartition(std::shared_ptr<Partition> neighbor_part, PmlType type, int xs, int ys, int zs, int w, int h, int d);
	~PmlPartition();

	virtual void Update();

	virtual real_t* get_pressure_field();
	virtual real_t get_pressure(int x, int y, int z);
	virtual void set_force(int x, int y, int z, real_t f);
};

