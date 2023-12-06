#ifndef HITABLE_LIST_H
#define HITABLE_LIST_H

#include "hitable.h"
#include "ray.h"
#include <vector>

class hitable_list : public hitable
{
public:
	hitable_list() = default;
	hitable_list(shared_ptr<hitable> object) { add(object); }

	void clear() { objects.clear(); }
	void add(shared_ptr<hitable> object) { objects.push_back(object); }

	virtual bool hit(const ray& r, double t_min, double t_max, hit_record& rec) const override
	{
		hit_record temp_rec;
		bool hit_anything = false;
		auto closest_so_far = t_max;

		for (const auto& object : objects)
			if (object->hit(r, t_min, closest_so_far, temp_rec))
			{
				hit_anything = true;
				closest_so_far = temp_rec.t;
				rec = temp_rec;
			}

		return hit_anything;
	}

	virtual bool bounding_box(double time0, double time1, aabb& output_box) const override
	{
		if (objects.empty()) 
			return false;
		aabb temp_box;
		bool first_box = true;
		for (const auto& object : objects)
		{
			if (!object->bounding_box(time0, time1, temp_box))
				return false;
			output_box = first_box ? temp_box : surrounding_box(output_box, temp_box);
			first_box = false;
		}
		return true;
	}

	virtual double pdf_value(const vec3& o, const vec3& v) const override
	{
		double weight = 1.0 / objects.size();
		double sum = 0;
		for (auto& object : objects)
			sum += weight * object->pdf_value(o, v);
		return sum;
	}

	virtual vec3 random(const vec3& o) const override
	{
		int index = random_double(0, objects.size() - 0.001);
		return objects[index]->random(o);
	}

public:
	std::vector<shared_ptr<hitable>> objects;
};



#endif // !HITABLE_LIST_H
