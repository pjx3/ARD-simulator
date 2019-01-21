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
	should_render_ = false;
	info_.type = "PML";

	if (type_ == P_LEFT || type_ == P_RIGHT) is_x_pml_ = true;
	if (type_ == P_TOP || type_ == P_BOTTOM) is_y_pml_ = true;
	if (type_ == P_FRONT || type_ == P_BACK) is_z_pml_ = true;

	thickness_ = Simulation::n_pml_layers_ * dh_;
	zeta_ = Simulation::c0_ / thickness_ * log10(1 / R_);

	int size = width_ * height_*depth_ + 1;
	p_old_ = (double *)malloc(size * sizeof(double));
	p_ = (double *)malloc(size * sizeof(double));
	p_new_ = (double *)malloc(size * sizeof(double));

	phi_x_ = (double *)malloc(size * sizeof(double));
	phi_x_new_ = (double *)malloc(size * sizeof(double));
	phi_y_ = (double *)malloc(size * sizeof(double));
	phi_y_new_ = (double *)malloc(size * sizeof(double));
	phi_z_ = (double *)malloc(size * sizeof(double));
	phi_z_new_ = (double *)malloc(size * sizeof(double));

	force_ = (double *)malloc(size * sizeof(double));
	memset((void *)p_old_, 0, size * sizeof(double));
	memset((void *)p_, 0, size * sizeof(double));
	memset((void *)p_new_, 0, size * sizeof(double));
	memset((void *)phi_x_, 0, size * sizeof(double));
	memset((void *)phi_x_new_, 0, size * sizeof(double));
	memset((void *)phi_y_, 0, size * sizeof(double));
	memset((void *)phi_y_new_, 0, size * sizeof(double));
	memset((void *)phi_z_, 0, size * sizeof(double));
	memset((void *)phi_z_new_, 0, size * sizeof(double));
	memset((void *)force_, 0, size * sizeof(double));
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
}

void PmlPartition::Update()
{
	//memset((void *)p_, 0, width_ * height_ * depth_ * sizeof(double));
	//return;
	//if (type_ != P_LEFT && type_ != P_RIGHT)
	//{
	//	memset((void *)p_, 0, width_ * height_ * depth_ * sizeof(double));
	//	return;
	//}
	int width = width_;
	int height = height_;
	int depth = depth_;
	auto type = type_;
	auto thickness = thickness_;
	auto dh = Simulation::dh_;
	auto dt = Simulation::dt_;
	auto c0 = Simulation::c0_;
	auto zeta = zeta_;

#pragma omp parallel for
	for (int k = 0; k < depth; k++)
	{
		double zetax = 0.0;
		double zetay = 0.0;
		double zetaz = 0.0;
		switch (type)
		{
		case PmlPartition::P_BACK:
			zetaz = zeta * ((k + 1) / thickness * dh - sin(2 * M_PI * (k + 1) * dh / thickness) / 2 / M_PI);
			break;
		case PmlPartition::P_FRONT:
			zetaz = zeta * ((depth - k) / thickness * dh - sin(2 * M_PI * (depth - k) * dh / thickness) / 2 / M_PI);
			break;
		default:
			break;
		}
#pragma omp parallel for
		for (int j = 0; j < height; j++)
		{
			switch (type)
			{
			case PmlPartition::P_BOTTOM:
				zetay = zeta * ((j + 1) / thickness * dh - sin(2 * M_PI * (j + 1) * dh / thickness) / 2 / M_PI);
				break;
			case PmlPartition::P_TOP:
				zetay = zeta * ((height - j) / thickness * dh - sin(2 * M_PI * (height - j) * dh / thickness) / 2 / M_PI);
				break;
			default:
				break;
			}
#pragma omp parallel for
			for (int i = 0; i < width; i++)
			{
				switch (type)
				{
				case PmlPartition::P_RIGHT:
					zetax = zeta * ((i + 1) / thickness * dh - sin(2 * M_PI * (i + 1) * dh / thickness) / 2 / M_PI);
					break;
				case PmlPartition::P_LEFT:
					zetax = zeta * ((width - i) / thickness * dh - sin(2 * M_PI * (width - i) * dh / thickness) / 2 / M_PI);
					break;
				default:
					break;
				}

				double coefs[] = { 2.0, -27.0, 270.0, -490.0, 270.0, -27.0, 2.0 };
				double d2udx2 = 0.0;
				double d2udy2 = 0.0;
				double d2udz2 = 0.0;
				for (int m = 0; m < 7; m++)
				{
					d2udx2 += coefs[m] * p_[GetIndex(i + m - 3, j, k)];
					d2udy2 += coefs[m] * p_[GetIndex(i, j + m - 3, k)];
					d2udz2 += coefs[m] * p_[GetIndex(i, j, k + m - 3)];

					//if (i + m - 3 < 0 && type==P_RIGHT) {
					//	d2udx2 += coefs[m] * neighbor_part_->get_pressure(neighbor_part_->width_ + i + m - 3, j, k);
					//}
					//else if (i + m - 3 >= width && type == P_LEFT) {
					//	d2udx2 += coefs[m] * neighbor_part_->get_pressure(i + m - 3 - width, j, k);
					//}
					//else {
					//	d2udx2 += coefs[m] * p_[GetIndex(i + m - 3, j, k)];
					//}

					//if (j + m - 3 < 0 && type == P_BOTTOM) {
					//	d2udy2 += coefs[m] * neighbor_part_->get_pressure(i, neighbor_part_->height_ + j + m - 3, k);
					//}
					//else if (j + m - 3 >= height && type == P_TOP) {
					//	d2udy2 += coefs[m] * neighbor_part_->get_pressure(i, j + m - 3 - height, k);
					//}
					//else {
					//	d2udy2 += coefs[m] * p_[GetIndex(i, j + m - 3, k)];
					//}

					//if (k + m - 3 < 0 && type == P_BACK) {
					//	d2udz2 += coefs[m] * neighbor_part_->get_pressure(i, j, neighbor_part_->depth_ + k + m - 3);
					//}
					//else if (k + m - 3 >= depth && type == P_FRONT) {
					//	d2udz2 += coefs[m] * neighbor_part_->get_pressure(i, j, k + m - 3 - depth);
					//}
					//else {
					//	d2udz2 += coefs[m] * p_[GetIndex(i, j, k + m - 3)];
					//}
				}
				d2udx2 /= (180.0 * dh * dh);
				d2udy2 /= (180.0 * dh * dh);
				d2udz2 /= (180.0 * dh * dh);

				double term1 = 2 * p_[GetIndex(i, j, k)];
				double term2 = -p_old_[GetIndex(i, j, k)];
				double term3 = c0 * c0 * (d2udx2 + d2udy2 + d2udz2);	// c^2*(d2udx2+d2udy2+d2udz2)
				double term4 = -(zetax + zetay + zetaz) * (p_[GetIndex(i, j, k)] - p_old_[GetIndex(i, j, k)]) / dt;
				double term5 = -(zetax * zetay + zetay * zetaz + zetax * zetaz) * p_[GetIndex(i, j, k)];

				double fourthCoefs[] = { 1.0, -8.0, 0.0, 8.0, -1.0 };
				double dphidx = 0.0;
				double dphidy = 0.0;
				double dphidz = 0.0;
				for (int m = 0; m < 5; m++)
				{
					dphidx += fourthCoefs[m] * phi_x_[GetIndex(i + m - 2, j, k)];
					dphidy += fourthCoefs[m] * phi_y_[GetIndex(i, j + m - 2, k)];
					dphidz += fourthCoefs[m] * phi_z_[GetIndex(i, j, k + m - 2)];
				}
				dphidx /= (12.0 * dh);
				dphidy /= (12.0 * dh);
				dphidz /= (12.0 * dh);
				double term6 = dphidx + dphidy + dphidz;

				p_new_[GetIndex(i, j, k)] = term1 + term2 + dt * dt * (term3 + term4 + term5 + term6 + force_[GetIndex(i, j, k)]);

				double dudx = 0.0;
				double dudy = 0.0;
				double dudz = 0.0;

				for (int m = 0; m < 5; m++)
				{
					dudx += fourthCoefs[m] * p_[GetIndex(i + m - 2, j, k)];
					dudy += fourthCoefs[m] * p_[GetIndex(i, j + m - 2, k)];
					dudz += fourthCoefs[m] * p_[GetIndex(i, j, k + m - 2)];

					/*if (i + m - 2 < 0 && type == P_RIGHT)
					{
						dudx += fourthCoefs[m] * neighbor_part_->get_pressure(neighbor_part_->width_ + i + m - 2, j, k);
					}
					else if (i + m - 2 >= width && type == P_LEFT)
					{
						dudx += fourthCoefs[m] * neighbor_part_->get_pressure(i + m - 2 - width, j, k);
					}
					else
					{
						dudx += fourthCoefs[m] * p_[GetIndex(i + m - 2, j, k)];
					}

					if (j + m - 2 < 0 && type == P_BOTTOM)
					{
						dudy += fourthCoefs[m] * neighbor_part_->get_pressure(i, neighbor_part_->height_ + j + m - 2, k);
					}
					else if (j + m - 2 >= width && type == P_TOP)
					{
						dudy += fourthCoefs[m] * neighbor_part_->get_pressure(i, j + m - 2 - height, k);
					}
					else
					{
						dudy += fourthCoefs[m] * p_[GetIndex(i, j + m - 2, k)];
					}

					if (k + m - 2 < 0 && type == P_BACK)
					{
						dudz += fourthCoefs[m] * neighbor_part_->get_pressure(i, j, neighbor_part_->depth_ + k + m - 2);
					}
					else if (k + m - 2 >= depth && type == P_FRONT)
					{
						dudz += fourthCoefs[m] * neighbor_part_->get_pressure(i, j, k + m - 2 - depth);
					}
					else
					{
						dudz += fourthCoefs[m] * p_[GetIndex(i, j, k + m - 2)];
					}*/
				}
				dudx /= (12.0 * dh);
				dudy /= (12.0 * dh);
				dudz /= (12.0 * dh);
				phi_x_new_[GetIndex(i, j, k)] = phi_x_[GetIndex(i, j, k)] - dt * zetax * phi_x_[GetIndex(i, j, k)] + dt * (zetay + zetaz - zetax) * dudx;
				phi_y_new_[GetIndex(i, j, k)] = phi_y_[GetIndex(i, j, k)] - dt * zetay * phi_y_[GetIndex(i, j, k)] + dt * (zetax + zetaz - zetay) * dudy;
				phi_z_new_[GetIndex(i, j, k)] = phi_z_[GetIndex(i, j, k)] - dt * zetaz * phi_z_[GetIndex(i, j, k)] + dt * (zetax + zetay - zetaz) * dudz;
			}
		}
	}
	std::swap(phi_x_new_, phi_x_);
	std::swap(phi_y_new_, phi_y_);
	std::swap(phi_z_new_, phi_z_);

	double *temp = p_old_;
	p_old_ = p_;
	p_ = p_new_;
	p_new_ = temp;

	memset((void *)force_, 0, width * height * depth * sizeof(double));
}

double* PmlPartition::get_pressure_field()
{
	return p_;
}

double PmlPartition::get_pressure(int x, int y, int z)
{
	return p_[GetIndex(x, y, z)];
}

void PmlPartition::set_force(int x, int y, int z, double f)
{
	force_[GetIndex(x, y, z)] = f;
}
