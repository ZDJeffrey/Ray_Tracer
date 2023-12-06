#ifndef PDF_H
#define PDF_H

#include "rtweekend.h"
#include "vec3.h"
#include "onb.h"
#include "hitable.h"

class pdf
{
public:
	virtual double value(const vec3& direction) const = 0;
	virtual vec3 generate() const = 0;
};

class cosine_pdf : public pdf
{
public:
	cosine_pdf(const vec3& w) : uvw(w) {}

	virtual double value(const vec3& direction) const override
	{
		double cosine = dot(unit_vector(direction), uvw.w());
		return cosine > 0 ? cosine / pi : 0;
	}

	virtual vec3 generate() const override
	{
		return uvw.local(random_cosine_direction());
	}

public:
	onb uvw;
};

class hitable_pdf : public pdf
{
public:
	hitable_pdf(shared_ptr<hitable> p, const vec3& origin) :ptr(p), o(origin) {}

	virtual double value(const vec3& direction) const override
	{
		return ptr->pdf_value(o, direction);
	}

	virtual vec3 generate() const override
	{
		return ptr->random(o);
	}

public:
	vec3 o;
	shared_ptr<hitable> ptr;
};

class mixture_pdf : public pdf
{
public:
	mixture_pdf(shared_ptr<pdf> p0, shared_ptr<pdf> p1) : ptr0(p0), ptr1(p1) {}

	virtual double value(const vec3& direction) const override
	{
		return 0.5 * ptr0->value(direction) + 0.5 * ptr1->value(direction);
	}

	virtual vec3 generate() const override
	{
		return random_double() < 0.5 ? ptr0->generate() : ptr1->generate();
	}

public:
	shared_ptr<pdf> ptr0, ptr1;
};

#endif // !PDF_H
