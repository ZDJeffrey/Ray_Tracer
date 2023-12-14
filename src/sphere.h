#ifndef SPHERE_H
#define SPHERE_H

#include "hitable.h"
#include "vec3.h"
#include "onb.h"

class sphere : public hitable
{
public:
	sphere() = default;
	sphere(point3 cen, double r, shared_ptr<material> m) : center(cen), radius(r), mat_ptr(m) {};

	virtual bool hit(const ray& r, double t_min, double t_max, hit_record& rec) const override
	{
		vec3 oc = r.origin() - center;
		auto a = r.direction().length_squared();
		auto half_b = dot(oc, r.direction());
		auto c = oc.length_squared() - radius * radius;
		auto discriminant = half_b * half_b - a * c;
		if (discriminant < 0) return false;
		auto sqrt_d = sqrt(discriminant);

		// 找出[t_min,t_max]中的最小值
		auto root = (-half_b - sqrt_d) / a;
		if (root < t_min || t_max < root)
		{
			root = (-half_b + sqrt_d) / a;
			if (root < t_min || t_max < root)
				return false;
		}
		rec.t = root;
		rec.p = r.at(rec.t);
		vec3 outward_normal = (rec.p - center) / radius;
		rec.set_face_normal(r, outward_normal);
		get_sphere_uv(outward_normal, rec.u, rec.v);
		rec.mat_ptr = mat_ptr;

		return true;
	}

	virtual bool bounding_box(double time0, double time1, aabb& output_box) const override
	{
		output_box = aabb(
			center - vec3(radius, radius, radius),
			center + vec3(radius, radius, radius));
		return true;
	}

	virtual double pdf_value(const vec3& o, const vec3& v) const override
	{
		hit_record rec;
		if (this->hit(ray(o, v), 0.001, infinity, rec))
		{
			double cos_theta_max = sqrt(1 - radius * radius / (center - o).length_squared());
			double solid_angle = 2 * pi * (1 - cos_theta_max);
			return 1 / solid_angle;
		}
		else
			return 0;
	}

	virtual vec3 random(const vec3& o) const override
	{
		vec3 direction = center - o;
		double distance_squared = direction.length_squared();
		onb uvw(direction);
		return direction + uvw.local(random_to_sphere(radius, distance_squared));
	}

private:
	static void get_sphere_uv(const point3& p, double& u, double& v)
	{
		// p: a given point on the sphere of radius one, centered at theorigin.
		// u: returned value [0,1] of angle around the Y axis from X=-1.
		// v: returned value [0,1] of angle from Y=-1 to Y=+1.
		// <1 0 0> yields <0.50 0.50> <-1 0 0> yields <0.00 0.50 >
		// <0 1 0> yields <0.50 1.00> < 0 -1 0> yields <0.50 0.00 >
		// <0 0 1> yields <0.25 0.50> < 0 0 -1> yields <0.75 0.50 >
		auto theta = acos(-p.y());
		auto phi = atan2(-p.z(), p.x()) + pi;

		u = phi / (2 * pi);
		v = theta / pi;
	}

public:
	point3 center;
	double radius;
	shared_ptr<material> mat_ptr;
};


#endif