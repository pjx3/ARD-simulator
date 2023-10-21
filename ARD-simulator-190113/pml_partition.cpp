#include "pml_partition.h"
#include "simulation.h"
#include <omp.h>


int PmlPartition::GetIndex(int x, int y, int z)
{
	if (x < 0 || x >= width_)
		return width_ * height_ * depth_;
	if (y < 0 || y >= height_)
		return width_ * height_ * depth_;
	if (z < 0 || z >= depth_)
		return width_ * height_ * depth_;
	return z * height_ * width_ + y * width_ + x;
}

PmlPartition::PmlPartition(std::shared_ptr<Partition> neighbor_part, PmlType type, int xs, int ys, int zs, int w, int h, int d)
	: Partition(xs, ys, zs, w, h, d), type_(type), neighbor_part_(neighbor_part)
{
	include_self_terms_ = false;
	should_render_ = true;
	info_.type = "PML";

	if (type_ == P_LEFT || type_ == P_RIGHT) is_x_pml_ = true;
	if (type_ == P_TOP || type_ == P_BOTTOM) is_y_pml_ = true;
	if (type_ == P_FRONT || type_ == P_BACK) is_z_pml_ = true;

	thickness_ = Simulation::m_pml_layers * dh_;
	zeta_ = Simulation::m_c0 / thickness_ * log10f(1.0f / R_);

	int size = width_ * height_*depth_ + 1;
	p_old_ = (real_t *)malloc(size * sizeof(real_t));
	p_ = (real_t *)malloc(size * sizeof(real_t));
	p_new_ = (real_t *)malloc(size * sizeof(real_t));

	phi_x_ = (real_t *)malloc(size * sizeof(real_t));
	phi_x_new_ = (real_t *)malloc(size * sizeof(real_t));
	phi_y_ = (real_t *)malloc(size * sizeof(real_t));
	phi_y_new_ = (real_t *)malloc(size * sizeof(real_t));
	phi_z_ = (real_t *)malloc(size * sizeof(real_t));
	phi_z_new_ = (real_t *)malloc(size * sizeof(real_t));

	force_ = (real_t *)malloc(size * sizeof(real_t));
	memset((void *)p_old_, 0, size * sizeof(real_t));
	memset((void *)p_, 0, size * sizeof(real_t));
	memset((void *)p_new_, 0, size * sizeof(real_t));
	memset((void *)phi_x_, 0, size * sizeof(real_t));
	memset((void *)phi_x_new_, 0, size * sizeof(real_t));
	memset((void *)phi_y_, 0, size * sizeof(real_t));
	memset((void *)phi_y_new_, 0, size * sizeof(real_t));
	memset((void *)phi_z_, 0, size * sizeof(real_t));
	memset((void *)phi_z_new_, 0, size * sizeof(real_t));
	memset((void *)force_, 0, size * sizeof(real_t));

	zetax_ = (real_t*)calloc(size, sizeof(real_t));
	zetay_ = (real_t*)calloc(size, sizeof(real_t));
	zetaz_ = (real_t*)calloc(size, sizeof(real_t));

	float const pi = (float)M_PI;

	for (int k = 0; k < depth_; k++)
	{
		for (int j = 0; j < height_; j++)
		{
			for (int i = 0; i < width_; i++)
			{
				switch (type)
				{
				case PmlPartition::P_BACK:
					zetaz_[GetIndex(i,j,k)] = zeta_ * ((k + 1) / thickness_ * dh_ - sinf(2.0f * pi * (k + 1) * dh_ / thickness_) / 2.0f / pi);
					break;
				case PmlPartition::P_FRONT:
					zetaz_[GetIndex(i, j, k)] = zeta_ * ((depth_ - k) / thickness_ * dh_ - sinf(2.0f * pi * (depth_ - k) * dh_ / thickness_) / 2.0f / pi);
					break;
				case PmlPartition::P_BOTTOM:
					zetay_[GetIndex(i, j, k)] = zeta_ * ((j + 1) / thickness_ * dh_ - sinf(2.0f * pi * (j + 1) * dh_ / thickness_) / 2.0f / pi);
					break;
				case PmlPartition::P_TOP:
					zetay_[GetIndex(i, j, k)] = zeta_ * ((height_ - j) / thickness_ * dh_ - sinf(2.0f * pi * (height_ - j) * dh_ / thickness_) / 2.0f / pi);
					break;
				case PmlPartition::P_RIGHT:
					zetax_[GetIndex(i, j, k)] = zeta_ * ((i + 1) / thickness_ * dh_ - sinf(2.0f * pi * (i + 1) * dh_ / thickness_) / 2.0f / pi);
					break;
				case PmlPartition::P_LEFT:
					zetax_[GetIndex(i, j, k)] = zeta_ * ((width_ - i) / thickness_ * dh_ - sinf(2.0f * pi * (width_ - i) * dh_ / thickness_) / 2.0f / pi);
					break;
				default:
					break;
				}
			}
		}
	}
}


PmlPartition::~PmlPartition()
{
	free(p_old_);
	free(p_);
	free(p_new_);
	free(phi_x_);
	free(phi_x_new_);
	free(phi_y_);
	free(phi_y_new_);
	free(phi_z_);
	free(phi_z_new_);
	free(force_);
	free(zetax_);
	free(zetay_);
	free(zetaz_);
}

void PmlPartition::Update()
{
	int width = width_;
	int height = height_;
	int depth = depth_;
	auto type = type_;
	auto thickness = thickness_;
	auto dh = Simulation::m_dh;
	auto dt = Simulation::m_dt;
	auto c0 = Simulation::m_c0;
	auto zeta = zeta_;

#pragma omp parallel for
	for (int k = 0; k < depth; k++)
	{
//#pragma omp parallel for
		for (int j = 0; j < height; j++)
		{
//#pragma omp parallel for
			for (int i = 0; i < width; i++)
			{
				real_t coefs[] = { 2.0, -27.0, 270.0, -490.0, 270.0, -27.0, 2.0 };
				real_t d2udx2 = 0.0;
				real_t d2udy2 = 0.0;
				real_t d2udz2 = 0.0;
				for (int m = 0; m < 7; m++)
				{
					d2udx2 += coefs[m] * p_[GetIndex(i + m - 3, j, k)];
					d2udy2 += coefs[m] * p_[GetIndex(i, j + m - 3, k)];
					d2udz2 += coefs[m] * p_[GetIndex(i, j, k + m - 3)];
				}
				d2udx2 /= (180.0f * dh * dh);
				d2udy2 /= (180.0f * dh * dh);
				d2udz2 /= (180.0f * dh * dh);

				real_t term1 = 2 * p_[GetIndex(i, j, k)];
				real_t term2 = -p_old_[GetIndex(i, j, k)];
				real_t term3 = c0 * c0 * (d2udx2 + d2udy2 + d2udz2);	// c^2*(d2udx2+d2udy2+d2udz2)
				real_t term4 = -(zetax_[GetIndex(i, j, k)] + zetay_[GetIndex(i, j, k)] + zetaz_[GetIndex(i, j, k)]) * (p_[GetIndex(i, j, k)] - p_old_[GetIndex(i, j, k)]) / dt;
				real_t term5 = -(zetax_[GetIndex(i, j, k)] * zetay_[GetIndex(i, j, k)] + zetay_[GetIndex(i, j, k)] * zetaz_[GetIndex(i, j, k)] + zetax_[GetIndex(i, j, k)] * zetaz_[GetIndex(i, j, k)]) * p_[GetIndex(i, j, k)];

				real_t fourthCoefs[] = { 1.0, -8.0, 0.0, 8.0, -1.0 };
				real_t dphidx = 0.0;
				real_t dphidy = 0.0;
				real_t dphidz = 0.0;
				for (int m = 0; m < 5; m++)
				{
					dphidx += fourthCoefs[m] * phi_x_[GetIndex(i + m - 2, j, k)];
					dphidy += fourthCoefs[m] * phi_y_[GetIndex(i, j + m - 2, k)];
					dphidz += fourthCoefs[m] * phi_z_[GetIndex(i, j, k + m - 2)];
				}
				dphidx /= (12.0f * dh);
				dphidy /= (12.0f * dh);
				dphidz /= (12.0f * dh);
				real_t term6 = dphidx + dphidy + dphidz;

				p_new_[GetIndex(i, j, k)] = term1 + term2 + dt * dt * (term3 + term4 + term5 + term6 + force_[GetIndex(i, j, k)]);

				real_t dudx = 0.0f;
				real_t dudy = 0.0f;
				real_t dudz = 0.0f;

				for (int m = 0; m < 5; m++)
				{
					dudx += fourthCoefs[m] * p_[GetIndex(i + m - 2, j, k)];
					dudy += fourthCoefs[m] * p_[GetIndex(i, j + m - 2, k)];
					dudz += fourthCoefs[m] * p_[GetIndex(i, j, k + m - 2)];
				}
				dudx /= (12.0f * dh);
				dudy /= (12.0f * dh);
				dudz /= (12.0f * dh);
				phi_x_new_[GetIndex(i, j, k)] = phi_x_[GetIndex(i, j, k)] - dt * zetax_[GetIndex(i, j, k)] * phi_x_[GetIndex(i, j, k)] + dt * (zetay_[GetIndex(i, j, k)] + zetaz_[GetIndex(i, j, k)] - zetax_[GetIndex(i, j, k)]) * dudx;
				phi_y_new_[GetIndex(i, j, k)] = phi_y_[GetIndex(i, j, k)] - dt * zetay_[GetIndex(i, j, k)] * phi_y_[GetIndex(i, j, k)] + dt * (zetax_[GetIndex(i, j, k)] + zetaz_[GetIndex(i, j, k)] - zetay_[GetIndex(i, j, k)]) * dudy;
				phi_z_new_[GetIndex(i, j, k)] = phi_z_[GetIndex(i, j, k)] - dt * zetaz_[GetIndex(i, j, k)] * phi_z_[GetIndex(i, j, k)] + dt * (zetax_[GetIndex(i, j, k)] + zetay_[GetIndex(i, j, k)] - zetaz_[GetIndex(i, j, k)]) * dudz;
			}
		}
	}
	std::swap(phi_x_new_, phi_x_);
	std::swap(phi_y_new_, phi_y_);
	std::swap(phi_z_new_, phi_z_);

	real_t *temp = p_old_;
	p_old_ = p_;
	p_ = p_new_;
	p_new_ = temp;

	memset((void *)force_, 0, width * height * depth * sizeof(real_t));
}

real_t* PmlPartition::get_pressure_field()
{
	return p_;
}

real_t PmlPartition::get_pressure(int x, int y, int z)
{
	return p_[GetIndex(x, y, z)];
}

void PmlPartition::set_force(int x, int y, int z, real_t f)
{
	force_[GetIndex(x, y, z)] = f;
}
