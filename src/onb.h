#ifndef ONB_H
#define ONB_H

#include "vec3.h"

class onb
{
public:
	onb() = default;
	onb(const vec3& n) { build_from_w(n); }
	inline vec3 operator[](int i) const { return axis[i]; }
	vec3 u()const { return axis[0]; }
	vec3 v()const { return axis[1]; }
	vec3 w() const { return axis[2]; }
	vec3 local(float a, float b, float c) const { return a * axis[0] + b * axis[1] + c * axis[2]; }
	vec3 local(const vec3& a) const { return a.x() * axis[0] + a.y() * axis[1] + a.z() * axis[2]; }
	void build_from_w(const vec3& n)
	{
		axis[2] = unit_vector(n);
		vec3 a = fabs(axis[2].x()) > 0.9 ? vec3(0, 1, 0) : vec3(1, 0, 0);
		axis[1] = unit_vector(cross(axis[2], a));
		axis[0] = cross(axis[2], axis[1]);
	}

public:
	vec3 axis[3];
};

#endif // !ONB_H