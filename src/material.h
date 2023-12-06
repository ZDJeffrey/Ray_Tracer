#ifndef MATERIAL_H
#define MATERIAL_H

#include "rtweekend.h"
#include "vec3.h"
#include "ray.h"
#include "hitable.h"
#include "texture.h"
#include "onb.h"
#include "pdf.h"

struct hit_record;

struct scatter_record
{
	ray specular_ray;
	bool is_specular;
	vec3 attenuation;
	shared_ptr<pdf> pdf_ptr;
};

class material
{
public:
	virtual color emitted(const ray& r_in, const hit_record& rec, double u, double v, const point3& p) const
	{
		return color(0, 0, 0);
	}
	virtual bool scatter(const ray& r_in, const hit_record& hrec, scatter_record& srec) const
	{
		return false;
	}
	virtual float scatter_pdf(const ray& r_in, const hit_record& rec, ray& scattered) const
	{
		return 1.0;
	}
};

class lambertian :public material
{
public:
	lambertian(const color& a) : albedo(make_shared<solid_color>(a)) {}
	lambertian(shared_ptr<texture> a) : albedo(a) {}

	virtual bool scatter(const ray& r_in, const hit_record& hrec, scatter_record& srec) const override
	{
		srec.is_specular = false;
		srec.attenuation = albedo->value(hrec.u, hrec.v, hrec.p);
		srec.pdf_ptr = make_shared<cosine_pdf>(hrec.normal);
		return true;
	}

	float scatter_pdf(const ray& r_in, const hit_record& rec, ray& scattered) const override
	{
		float cosine = dot(rec.normal, unit_vector(scattered.direction()));
		return cosine < 0 ? 0 : cosine / pi;
	}

public:
	shared_ptr<texture> albedo;
};

class metal : public material
{
public:
	metal(const color& a, double f) :albedo(a), fuzz(fabs(f) < 1 ? f : 1) {}

	virtual bool scatter(const ray& r_in, const hit_record& hrec, scatter_record& srec) const override
	{
		vec3 reflected = reflect(unit_vector(r_in.direction()), hrec.normal);
		srec.specular_ray = ray(hrec.p, reflected + fuzz * random_in_unit_sphere(), r_in.time());
		srec.is_specular = true;
		srec.attenuation = albedo;
		srec.pdf_ptr = nullptr;
		return (dot(srec.specular_ray.direction(), hrec.normal) > 0);
	}

public:
	color albedo;
	double fuzz;
};

class dielectric :public material
{
public:
	dielectric(double index_of_refraction) :ir(index_of_refraction) {};

	virtual bool scatter(const ray& r_in, const hit_record& hrec, scatter_record& srec) const override
	{
		srec.attenuation = color(1.0, 1.0, 1.0);
		double refraction_ratio = hrec.front_face ? (1.0 / ir) : ir;

		vec3 unit_direction = unit_vector(r_in.direction());
		double cos_theta = fmin(dot(-unit_direction, hrec.normal), 1.0);
		double sin_theta = sqrt(1.0 - cos_theta * cos_theta);

		bool cannot_refract = (refraction_ratio * sin_theta) > 1.0;
		vec3 direction;

		if (cannot_refract || reflectance(cos_theta, refraction_ratio) > random_double())
			direction = reflect(unit_direction, hrec.normal);
		else
			direction = refract(unit_direction, hrec.normal, refraction_ratio);

		srec.specular_ray = ray(hrec.p, direction, r_in.time());
		srec.is_specular = true;
		return true;
	}

private:
	static double reflectance(double cosine, double ref_idx)
	{
		// Schlick's approximation
		auto r0 = (1 - ref_idx) / (1 + ref_idx);
		r0 = r0 * r0;
		return r0 + (1 - r0) * pow((1 - cosine), 5);
	}

public:
	double ir;
};

class diffuse_light : public material
{
public:
	diffuse_light(shared_ptr<texture> a) : emit(a) {}
	diffuse_light(color c) : emit(make_shared<solid_color>(c)) {}

	virtual color emitted(const ray& r_in, const hit_record& rec, double u, double v, const point3& p) const override
	{
		return emit->value(u, v, p);
	}

public:
	shared_ptr<texture> emit;
};

class isotropic : public material
{
public:
	isotropic(color c) : albedo(make_shared<solid_color>(c)) {}
	isotropic(shared_ptr<texture> a) : albedo(a) {}

	virtual bool scatter(const ray& r_in, const hit_record& hrec, scatter_record& srec) const override
	{
		srec.is_specular = true;
		srec.specular_ray = ray(hrec.p, random_in_unit_sphere(), r_in.time());
		srec.attenuation = albedo->value(hrec.u, hrec.v, hrec.p);
		return true;
	}

public:
	shared_ptr<texture> albedo;
};

#endif // !MATERIAL_H
