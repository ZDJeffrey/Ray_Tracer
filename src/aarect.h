#ifndef AARECT_H
#define AARECT_H

#include "rtweekend.h"
#include "hitable.h"

class xy_rect : public hitable
{
public:
	xy_rect() = default;
	xy_rect(double _x0, double _x1, double _y0, double _y1, double _k, shared_ptr<material> mat)
		: x0(_x0), x1(_x1), y0(_y0), y1(_y1), k(_k), mp(mat) {};

	virtual bool hit(const ray& r, double t_min, double t_max, hit_record& rec) const override
	{
		auto t = (k - r.origin().z()) / r.direction().z();
		if (t < t_min || t > t_max)
			return false;

		auto x = r.origin().x() + t * r.direction().x();
		auto y = r.origin().y() + t * r.direction().y();
		if (x < x0 || x > x1 || y < y0 || y > y1)
			return false;

		rec.u = (x - x0) / (x1 - x0);
		rec.v = (y - y0) / (y1 - y0);
		rec.t = t;
		auto outward_normal = vec3(0, 0, 1);
		rec.set_face_normal(r, outward_normal);
		rec.mat_ptr = mp;
		rec.p = r.at(t);
		return true;
	}

	virtual bool bounding_box(double time0, double time1, aabb& output_box) const override
	{
		// The bounding box must have non-zero width in each dimension, so pad the Z dimension a small amount.
		output_box = aabb(point3(x0, y0, k - 0.0001), point3(x1, y1, k + 0.0001));
		return true;
	}

public:
	shared_ptr<material> mp;
	double x0, x1, y0, y1, k;
};

class xz_rect : public hitable
{
public:
	xz_rect() {}
	xz_rect(double _x0, double _x1, double _z0, double _z1, double _k, shared_ptr<material> mat)
		: x0(_x0), x1(_x1), z0(_z0), z1(_z1), k(_k), mp(mat) {};

	virtual bool hit(const ray& r, double t_min, double t_max, hit_record& rec) const override
	{
		auto t = (k - r.origin().y()) / r.direction().y();
		if (t < t_min || t > t_max)
			return false;

		auto x = r.origin().x() + t * r.direction().x();
		auto z = r.origin().z() + t * r.direction().z();
		if (x < x0 || x > x1 || z < z0 || z > z1)
			return false;

		rec.u = (x - x0) / (x1 - x0);
		rec.v = (z - z0) / (z1 - z0);
		rec.t = t;
		auto outward_normal = vec3(0, 1, 0);
		rec.set_face_normal(r, outward_normal);
		rec.mat_ptr = mp;
		rec.p = r.at(t);
		return true;
	}

	virtual bool bounding_box(double time0, double time1, aabb& output_box) const override
	{
		// The bounding box must have non-zero width in each dimension, so pad the Y
		// dimension a small amount.
		output_box = aabb(point3(x0, k - 0.0001, z0), point3(x1, k + 0.0001, z1));
		return true;
	}

	virtual double pdf_value(const vec3& o, const vec3& v) const override
	{
		hit_record rec;
		if (this->hit(ray(o, v), 0.001, infinity, rec))
		{
			double area = fabs((x1 - x0) * (z1 - z0));
			double distance_squared = rec.t * rec.t * v.length_squared();
			double cosine = fabs(dot(v, rec.normal) / v.length());
			return distance_squared / (cosine * area);
		}
		else
			return 0;
	}

	virtual vec3 random(const vec3& o) const override
	{
		vec3 random_point = vec3(x0 + random_double() * (x1 - x0), k, z0 + random_double() * (z1 - z0));
		return random_point - o;
	}

public:
	shared_ptr<material> mp;
	double x0, x1, z0, z1, k;
};


class yz_rect : public hitable
{
public:
	yz_rect() = default;
	yz_rect(double _y0, double _y1, double _z0, double _z1, double _k, shared_ptr<material> mat)
		: y0(_y0), y1(_y1), z0(_z0), z1(_z1), k(_k), mp(mat) {};

	virtual bool hit(const ray& r, double t_min, double t_max, hit_record& rec) const override
	{
		auto t = (k - r.origin().x()) / r.direction().x();
		if (t < t_min || t > t_max)
			return false;

		auto y = r.origin().y() + t * r.direction().y();
		auto z = r.origin().z() + t * r.direction().z();
		if (y < y0 || y > y1 || z < z0 || z > z1)
			return false;

		rec.u = (y - y0) / (y1 - y0);
		rec.v = (z - z0) / (z1 - z0);
		rec.t = t;
		auto outward_normal = vec3(1, 0, 0);
		rec.set_face_normal(r, outward_normal);
		rec.mat_ptr = mp;
		rec.p = r.at(t);
		return true;
	}

	virtual bool bounding_box(double time0, double time1, aabb& output_box) const override
	{
		// The bounding box must have non-zero width in each dimension, so pad the X
		// dimension a small amount.
		output_box = aabb(point3(k - 0.0001, y0, z0), point3(k + 0.0001, y1, z1));
		return true;
	}

public:
	shared_ptr<material> mp;
	double y0, y1, z0, z1, k;
};

#endif