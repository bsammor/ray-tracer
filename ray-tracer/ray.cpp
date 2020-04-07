#include "ray.h"

Ray::Ray()
{
	origin = Vec3();
	direction = Vec3();
	tnear = MINIMUM;
	tmax = INFINITY;
}

Ray::Ray(Vec3 origin, Vec3 direction, double tnear, double tmax)
{
	this->origin = origin;
	this->direction = direction;
	this->tnear = tnear;
	this->tmax = tmax;
}

Vec3 Ray::get_origin()
{
	return origin;
}

void Ray::set_tmax(double z)
{
	tmax = z;
}

double Ray::get_tmax()
{
	return tmax;
}

Vec3 Ray::get_direction()
{
	return direction;
}

Vec3 Ray::get_intersection_point()
{
	return origin + (direction * tmax);
}

double Ray::get_tnear()
{
	return tnear;
}