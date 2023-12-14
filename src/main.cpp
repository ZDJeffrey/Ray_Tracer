/*The MIT License (MIT)

Copyright (c) 2021-Present, Wencong Yang (yangwc3@mail2.sysu.edu.cn).

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in
all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
THE SOFTWARE.*/

#include <array>
#include <vector>
#include <thread>
#include <iostream>
#include <omp.h>

#include "rtweekend.h"
#include "vec3.h"
#include "ray.h"
#include "hitable_list.h"
#include "sphere.h"
#include "moving_sphere.h"
#include "aarect.h"
#include "box.h"
#include "constant_medium.h"
#include "camera.h"
#include "material.h"
#include "bvh.h"
#include "pdf.h"
#include "WindowsApp.h"

static std::vector<std::vector<color>> gCanvas;		//Canvas

// The width and height of the screen
const auto aspect_ratio = 16.0 / 9.0;
const int gWidth = 800;
const int gHeight = static_cast<int>(gWidth / aspect_ratio);

void rendering();

int main(int argc, char* args[])
{
	// Create window app handle
	WindowsApp::ptr winApp = WindowsApp::getInstance(gWidth, gHeight, "CGAssignment4: Ray Tracing");
	if (winApp == nullptr)
	{
		std::cerr << "Error: failed to create a window handler" << std::endl;
		return -1;
	}

	// Memory allocation for canvas
	gCanvas.resize(gHeight, std::vector<color>(gWidth));

	// Launch the rendering thread
	// Note: we run the rendering task in another thread to avoid GUI blocking
	std::thread renderingThread(rendering);

	// Window app loop
	while (!winApp->shouldWindowClose())
	{
		// Process event
		winApp->processEvent();

		// Display to the screen
		winApp->updateScreenSurface(gCanvas);

	}

	renderingThread.join();

	return 0;
}

void random_scene(hitable_list& objects, shared_ptr<hitable_list> hlist)
{
	auto checker = make_shared<checker_texture>(color(0.2, 0.3, 0.1), color(0.9, 0.9, 0.9));
	objects.add(make_shared<sphere>(point3(0, -1000, 0), 1000, make_shared<lambertian>(checker)));
	for (int a = -11; a < 11; a++) {
		for (int b = -11; b < 11; b++)
		{
			auto choose_mat = random_double();
			point3 center(a + 0.9 * random_double(), 0.2, b +
				0.9 * random_double());
			if ((center - vec3(4, 0.2, 0)).length() > 0.9)
			{
				shared_ptr<material> sphere_material;
				if (choose_mat < 0.8)
				{
					// diffuse
					auto albedo = color::random() * color::random();
					sphere_material = make_shared<lambertian>(albedo);
					auto center2 = center + vec3(0, random_double(0, .5), 0);
					objects.add(make_shared<moving_sphere>(center, center2, 0.0, 1.0, 0.2, sphere_material));
				}
				else if (choose_mat < 0.95)
				{
					// metal
					auto albedo = color::random(0.5, 1);
					auto fuzz = random_double(0, 0.5);
					sphere_material = make_shared<metal>(albedo, fuzz);
					objects.add(make_shared<sphere>(center, 0.2, sphere_material));
				}
				else
				{
					// glass
					sphere_material = make_shared<dielectric>(1.5);
					objects.add(make_shared<sphere>(center, 0.2, sphere_material));
				}
			}
		}
	}
	auto material1 = make_shared<dielectric>(1.5);
	auto glass_sphere = make_shared<sphere>(point3(0, 1, 0), 1.0, material1);
	objects.add(glass_sphere);
	hlist->add(glass_sphere);
	auto material2 = make_shared<lambertian>(color(0.4, 0.2, 0.1));
	objects.add(make_shared<sphere>(point3(-4, 1, 0), 1.0, material2));
	auto material3 = make_shared<metal>(color(0.7, 0.6, 0.5), 0.0);
	objects.add(make_shared<sphere>(point3(4, 1, 0), 1.0, material3));
}

void two_spheres(hitable_list& objects, shared_ptr<hitable_list> hlist)
{
	auto checker = make_shared<checker_texture>(color(0.2, 0.3, 0.1), color(0.9, 0.9, 0.9));
	objects.add(make_shared<sphere>(point3(0, -10, 0), 10, make_shared<lambertian>(checker)));
	objects.add(make_shared<sphere>(point3(0, 10, 0), 10, make_shared<lambertian>(checker)));
}

void two_perlin_spheres(hitable_list& objects, shared_ptr<hitable_list> hlist)
{
	auto pertext = make_shared<noise_texture>(4);
	objects.add(make_shared<sphere>(point3(0, -1000, 0), 1000, make_shared<lambertian>(pertext)));
	objects.add(make_shared<sphere>(point3(0, 2, 0), 2, make_shared<lambertian>(pertext)));
}

void earth(hitable_list& objects, shared_ptr<hitable_list> hlist)
{
	auto earth_texture = make_shared<image_texture>("earthmap.jpg");
	auto earth_surface = make_shared<lambertian>(earth_texture);
	auto globe = make_shared<sphere>(point3(0, 0, 0), 2, earth_surface);
	objects.add(globe);
}

void simple_light(hitable_list& objects, shared_ptr<hitable_list> hlist)
{
	auto pertext = make_shared<noise_texture>(4);
	objects.add(make_shared<sphere>(point3(0, -1000, 0), 1000, make_shared<lambertian>(pertext)));
	objects.add(make_shared<sphere>(point3(0, 2, 0), 2, make_shared<lambertian>(pertext)));
	auto difflight = make_shared<diffuse_light>(color(4, 4, 4));
	objects.add(make_shared<xy_rect>(3, 5, 1, 3, -2, difflight));
	objects.add(make_shared<sphere>(point3(0, 7, 0), 2, difflight));
}

void cornell_box(hitable_list& objects, shared_ptr<hitable_list> hlist)
{
	auto red = make_shared<lambertian>(color(.65, .05, .05));
	auto white = make_shared<lambertian>(color(.73, .73, .73));
	auto green = make_shared<lambertian>(color(.12, .45, .15));
	auto light = make_shared<diffuse_light>(color(15, 15, 15));
	objects.add(make_shared<yz_rect>(0, 555, 0, 555, 555, green));
	objects.add(make_shared<yz_rect>(0, 555, 0, 555, 0, red));
	auto light_src = make_shared<xz_rect>(213, 343, 227, 332, 554, light);
	objects.add(light_src);
	hlist->add(light_src);
	objects.add(make_shared<xz_rect>(0, 555, 0, 555, 0, white));
	objects.add(make_shared<xz_rect>(0, 555, 0, 555, 555, white));
	objects.add(make_shared<xy_rect>(0, 555, 0, 555, 555, white));
	shared_ptr<hitable> box1 = make_shared<box>(point3(0, 0, 0), point3(165, 330, 165), white);
	box1 = make_shared<rotate_y>(box1, 15);
	box1 = make_shared<translate>(box1, vec3(265, 0, 295));
	objects.add(box1);
	shared_ptr<hitable> box2 = make_shared<box>(point3(0, 0, 0), point3(165, 165, 165), white);
	box2 = make_shared<rotate_y>(box2, -18);
	box2 = make_shared<translate>(box2, vec3(130, 0, 65));
	objects.add(box2);
	auto glass_sphere = make_shared<sphere>(point3(190, 255, 190), 90, make_shared<dielectric>(1.5));
	objects.add(glass_sphere);
	hlist->add(glass_sphere);
}

void cornell_smoke(hitable_list& objects, shared_ptr<hitable_list> hlist)
{
	auto red = make_shared<lambertian>(color(.65, .05, .05));
	auto white = make_shared<lambertian>(color(.73, .73, .73));
	auto green = make_shared<lambertian>(color(.12, .45, .15));
	auto light = make_shared<diffuse_light>(color(7, 7, 7));
	objects.add(make_shared<yz_rect>(0, 555, 0, 555, 555, green));
	objects.add(make_shared<yz_rect>(0, 555, 0, 555, 0, red));
	auto light_src = make_shared<xz_rect>(113, 443, 127, 432, 554, light);
	objects.add(light_src);
	objects.add(make_shared<xz_rect>(0, 555, 0, 555, 555, white));
	objects.add(make_shared<xz_rect>(0, 555, 0, 555, 0, white));
	objects.add(make_shared<xy_rect>(0, 555, 0, 555, 555, white));
	shared_ptr<hitable> box1 = make_shared<box>(point3(0, 0, 0), point3(165, 330, 165), white);
	box1 = make_shared<rotate_y>(box1, 15);
	box1 = make_shared<translate>(box1, vec3(265, 0, 295));
	shared_ptr<hitable> box2 = make_shared<box>(point3(0, 0, 0), point3(165, 165, 165), white);
	box2 = make_shared<rotate_y>(box2, -18);
	box2 = make_shared<translate>(box2, vec3(130, 0, 65));
	objects.add(make_shared<constant_medium>(box1, 0.01, color(0, 0, 0)));
	objects.add(make_shared<constant_medium>(box2, 0.01, color(1, 1, 1)));
}

void final_scene(hitable_list& objects, shared_ptr<hitable_list> hlist)
{
	hitable_list boxes1;
	auto ground = make_shared<lambertian>(color(0.48, 0.83, 0.53));
	const int boxes_per_side = 20;
	for (int i = 0; i < boxes_per_side; i++)
		for (int j = 0; j < boxes_per_side; j++)
		{
			auto w = 100.0;
			auto x0 = -1000.0 + i * w;
			auto z0 = -1000.0 + j * w;
			auto y0 = 0.0;
			auto x1 = x0 + w;
			auto y1 = random_double(1, 101);
			auto z1 = z0 + w;
			boxes1.add(make_shared<box>(point3(x0, y0, z0), point3(x1, y1, z1), ground));
		}
	objects.add(make_shared<bvh_node>(boxes1, 0, 1));
	auto light = make_shared<diffuse_light>(color(7, 7, 7));
	auto light_src = make_shared<xz_rect>(123, 423, 147, 412, 554, light);
	objects.add(light_src);
	hlist->add(light_src);
	auto center1 = point3(400, 400, 200);
	auto center2 = center1 + vec3(30, 0, 0);
	auto moving_sphere_material = make_shared<lambertian>(color(0.7, 0.3, 0.1));
	objects.add(make_shared<moving_sphere>(center1, center2, 0, 1, 50, moving_sphere_material));
	auto glass_sphere = make_shared<sphere>(point3(260, 150, 45), 50, make_shared<dielectric>(1.5));
	objects.add(glass_sphere);
	hlist->add(glass_sphere);
	objects.add(make_shared<sphere>(point3(0, 150, 145), 50, make_shared<metal>(color(0.8, 0.8, 0.9), 1.0)));
	auto boundary = make_shared<sphere>(point3(360, 150, 145), 70, make_shared<dielectric>(1.5));
	objects.add(boundary);
	hlist->add(boundary);
	objects.add(make_shared<constant_medium>(boundary, 0.2, color(0.2, 0.4, 0.9)));
	boundary = make_shared<sphere>(point3(0, 0, 0), 5000, make_shared<dielectric>(1.5));
	objects.add(make_shared<constant_medium>(boundary, .0001, color(1, 1, 1)));
	auto emat = make_shared<lambertian>(make_shared<image_texture>("earthmap.jpg"));
	objects.add(make_shared<sphere>(point3(400, 200, 400), 100, emat));
	auto pertext = make_shared<noise_texture>(0.1);
	objects.add(make_shared<sphere>(point3(220, 280, 300), 80, make_shared<lambertian>(pertext)));
	hitable_list boxes2;
	auto white = make_shared<lambertian>(color(.73, .73, .73));
	int ns = 1000;
	for (int j = 0; j < ns; j++)
		boxes2.add(make_shared<sphere>(point3::random(0, 165), 10, white));
	objects.add(make_shared<translate>(make_shared<rotate_y>(make_shared<bvh_node>(boxes2, 0.0, 1.0), 15), vec3(-100, 270, 395)));
}

void cornell_box_spot(hitable_list& objects, shared_ptr<hitable_list> hlist)
{
	auto red = make_shared<lambertian>(color(.65, .05, .05));
	auto white = make_shared<lambertian>(color(.73, .73, .73));
	auto green = make_shared<lambertian>(color(.12, .45, .15));
	auto spot = make_shared<spot_light>(color(20, 20, 20), vec3(0.0, -1.0, 0.0), 22.5);
	objects.add(make_shared<yz_rect>(0, 555, 0, 555, 555, green));
	objects.add(make_shared<yz_rect>(0, 555, 0, 555, 0, red));
	auto light_src = make_shared<xz_rect>(213, 343, 227, 332, 554.99, spot);
	objects.add(light_src);
	hlist->add(light_src);
	objects.add(make_shared<xz_rect>(0, 555, 0, 555, 0, white));
	objects.add(make_shared<xz_rect>(0, 555, 0, 555, 555, white));
	objects.add(make_shared<xy_rect>(0, 555, 0, 555, 555, white));
	shared_ptr<hitable> box1 = make_shared<box>(point3(0, 0, 0), point3(165, 330, 165), white);
	box1 = make_shared<rotate_y>(box1, 15);
	box1 = make_shared<translate>(box1, vec3(265, 0, 295));
	objects.add(box1);
	shared_ptr<hitable> box2 = make_shared<box>(point3(0, 0, 0), point3(165, 165, 165), white);
	box2 = make_shared<rotate_y>(box2, -18);
	box2 = make_shared<translate>(box2, vec3(130, 0, 65));
	objects.add(box2);
	auto glass_sphere = make_shared<sphere>(point3(190, 255, 190), 90, make_shared<dielectric>(1.5));
	objects.add(glass_sphere);
	hlist->add(glass_sphere);
}

void cornell_box_light(hitable_list& objects, shared_ptr<hitable_list> hlist)
{
	auto red = make_shared<lambertian>(color(.65, .05, .05));
	auto white = make_shared<lambertian>(color(.73, .73, .73));
	auto green = make_shared<lambertian>(color(.12, .45, .15));
	auto light = make_shared<diffuse_light>(color(3, 1.4, 0.4));
	auto aluminum = make_shared<metal>(color(0.8, 0.85, 0.88), 0.0);
	objects.add(make_shared<yz_rect>(0, 555, 0, 555, 555, green));
	objects.add(make_shared<yz_rect>(0, 555, 0, 555, 0, red));
	objects.add(make_shared<xz_rect>(0, 555, 0, 555, 0, white));
	objects.add(make_shared<xz_rect>(0, 555, 0, 555, 555, white));
	objects.add(make_shared<xy_rect>(0, 555, 0, 555, 555, white));
	shared_ptr<hitable> box1 = make_shared<box>(point3(0, 0, 0), point3(165, 330, 165), aluminum);
	box1 = make_shared<rotate_y>(box1, 15);
	box1 = make_shared<translate>(box1, vec3(265, 0, 295));
	objects.add(box1);
	shared_ptr<hitable> box2 = make_shared<box>(point3(0, 0, 0), point3(165, 165, 165), white);
	box2 = make_shared<rotate_y>(box2, -18);
	box2 = make_shared<translate>(box2, vec3(130, 0, 65));
	objects.add(box2);
	auto light_sphere = make_shared<sphere>(point3(190, 195, 190), 30, light);
	objects.add(light_sphere);
	hlist->add(light_sphere);
}

void universe(hitable_list& objects, shared_ptr<hitable_list> hlist)
{
	auto stars = make_shared<sphere>(point3(0, 0, 0), 1000, make_shared<diffuse_light>(make_shared<image_texture>("stars.jpg")));
	objects.add(stars);
	auto sun = make_shared<sphere>(point3(-50, 0, 0), 100, make_shared<diffuse_light>(make_shared<image_texture>("sun.jpg")));
	objects.add(sun);
	hlist->add(sun);
	auto mercury = make_shared<sphere>(75 * unit_vector(point3(1, 0, 1)), 2, make_shared<lambertian>(make_shared<image_texture>("mercury.jpg")));
	objects.add(mercury);
	auto venus = make_shared<sphere>(91 * unit_vector(point3(1, 0, -0.6)), 6, make_shared<lambertian>(make_shared<image_texture>("venus.jpg")));
	objects.add(venus);
	auto earth = make_shared<sphere>((point3(0, 0, 0)), 7, make_shared<lambertian>(make_shared<image_texture>("earth.jpg")));
	objects.add(make_shared<translate>(make_shared<rotate_y>(earth, 180), 115 * unit_vector(point3(1, 0, 1))));
	auto mars = make_shared<sphere>(133 * unit_vector(point3(1, 0, -0.1)), 3, make_shared<lambertian>(make_shared<image_texture>("mars.jpg")));
	objects.add(mars);
	auto jupiter = make_shared<sphere>(279 * unit_vector(point3(1, 0, -2)), 30, make_shared<lambertian>(make_shared<image_texture>("jupiter.jpg")));
	objects.add(jupiter);
	for (int i = 0; i < 5000; i++)
		objects.add(make_shared<sphere>(vec3(0,random_double(-2.0,2.0), 0) + (150 + random_double() * 50) * unit_vector(point3(1, 0, random_double(-5.5, 1))), random_double(0.1, 0.5), make_shared<lambertian>(color(0.5, 0.5, 0.5))));
}

void write_color(int x, int y, color pixel_color, int samples_per_pixel)
{
	// Out-of-range detection
	if (x < 0 || x >= gWidth)
	{
		std::cerr << "Warnning: try to write the pixel out of range: (x,y) -> (" << x << "," << y << ")" << std::endl;
		return;
	}

	if (y < 0 || y >= gHeight)
	{
		std::cerr << "Warnning: try to write the pixel out of range: (x,y) -> (" << x << "," << y << ")" << std::endl;
		return;
	}

	auto scale = 1.0 / samples_per_pixel;
	// gamma矫正
	auto r = clamp(sqrt(pixel_color.x() * scale), 0.0, 1.0);
	auto g = clamp(sqrt(pixel_color.y() * scale), 0.0, 1.0);
	auto b = clamp(sqrt(pixel_color.z() * scale), 0.0, 1.0);

	// Note: x -> the column number, y -> the row number
	gCanvas[y][x] = color(r, g, b);
}

double hit_sphere(const point3& center, double radius, const ray& r)
{
	vec3 oc = r.origin() - center;
	auto a = dot(r.direction(), r.direction());
	auto half_b = dot(oc, r.direction());
	auto c = dot(oc, oc) - radius * radius;
	auto discriminant = half_b * half_b - a * c;
	if (discriminant < 0)
		return -1.0;
	else
		return (-half_b - sqrt(discriminant)) / a;
}

color ray_color(const ray& r, const color& background, const hitable& world, shared_ptr<hitable_list> hlist, int depth)
{
	if (depth <= 0) // 有限递归
		return color(0, 0, 0);

	hit_record hrec;
	if (!world.hit(r, 0.001, infinity, hrec))
		return background;

	scatter_record srec;
	color emitted = hrec.mat_ptr->emitted(r, hrec, hrec.u, hrec.v, hrec.p);
	if (hrec.mat_ptr->scatter(r, hrec, srec))
	{
		if (srec.is_specular)
			return srec.attenuation * ray_color(srec.specular_ray, background, world, hlist, depth - 1);

		ray scattered;
		double pdf_val;
		if (hlist->objects.empty())
		{
			scattered = ray(hrec.p, srec.pdf_ptr->generate(), r.time());
			pdf_val = srec.pdf_ptr->value(scattered.direction());
		}
		else
		{
			mixture_pdf p(make_shared<hitable_pdf>(hlist, hrec.p), srec.pdf_ptr);
			scattered = ray(hrec.p, p.generate(), r.time());
			pdf_val = p.value(scattered.direction());
		}

		return emitted
			+ srec.attenuation
			* hrec.mat_ptr->scatter_pdf(r, hrec, scattered)
			* ray_color(scattered, background, world, hlist, depth - 1)
			/ pdf_val;
	}
	return emitted;
}


void rendering()
{
	double startFrame = clock();

	printf("CGAssignment4 (built %s at %s) \n", __DATE__, __TIME__);
	std::cout << "Ray-tracing based rendering launched..." << std::endl;

	// Image
	const int image_width = gWidth;
	const int image_height = gHeight;
	int samples_per_pixel = 100;
	int max_depth = 50;

	// Camera
	point3 lookfrom, lookat;
	vec3 vup(0, 1, 0);
	auto vfov = 40.0;
	auto dist_to_focus = 10.0;
	auto aperture = 0.0;
	auto time0 = 0.0, time1 = 1.0;

	hitable_list objects;
	shared_ptr<hitable_list> hlist = make_shared<hitable_list>();
	color background(0, 0, 0);
	switch (9)
	{
	case 1:
		random_scene(objects, hlist);
		background = color(0.7, 0.8, 1.0);
		lookfrom = point3(13, 2, 3);
		lookat = point3(0, 0, 0);
		vfov = 20.0;
		aperture = 0.1;
		break;
	case 2:
		two_spheres(objects, hlist);
		background = color(0.7, 0.8, 1.0);
		lookfrom = point3(13, 2, 3);
		lookat = point3(0, 0, 0);
		vfov = 20.0;
		break;
	case 3:
		two_perlin_spheres(objects, hlist);
		background = color(0.7, 0.8, 1.0);
		lookfrom = point3(13, 2, 3);
		lookat = point3(0, 0, 0);
		vfov = 20.0;
		break;
	case 4:
		earth(objects, hlist);
		background = color(0.7, 0.8, 1.0);
		lookfrom = point3(13, 2, 3);
		lookat = point3(0, 0, 0);
		vfov = 20.0;
		break;
	case 5:
		simple_light(objects, hlist);
		background = color(0, 0, 0);
		samples_per_pixel = 400;
		lookfrom = point3(26, 3, 6);
		lookat = point3(0, 2, 0);
		vfov = 20.0;
		break;
	case 6:
		cornell_box(objects, hlist);
		samples_per_pixel = 500;
		background = color(0, 0, 0);
		lookfrom = point3(278, 278, -800);
		lookat = point3(278, 278, 0);
		vfov = 40.0;
		break;
	case 7:
		cornell_smoke(objects, hlist);
		samples_per_pixel = 200;
		lookfrom = point3(278, 278, -800);
		lookat = point3(278, 278, 0);
		vfov = 40.0;
		break;
	case 8:
		final_scene(objects, hlist);
		samples_per_pixel = 10000;
		background = color(0, 0, 0);
		lookfrom = point3(478, 278, -600);
		lookat = point3(278, 278, 0);
		vfov = 40.0;
		break;
	case 9:
		cornell_box_spot(objects, hlist);
		samples_per_pixel = 1000;
		background = color(0, 0, 0);
		lookfrom = point3(278, 278, -800);
		lookat = point3(278, 278, 0);
		vfov = 40.0;
		break;
	case 10:
		cornell_box_light(objects, hlist);
		samples_per_pixel = 1000;
		background = color(0, 0, 0);
		lookfrom = point3(278, 278, -800);
		lookat = point3(278, 278, 0);
		vfov = 40.0;
		break;
	case 11:
	default:
		universe(objects, hlist);
		samples_per_pixel = 1000;
		background = color(1.0, 1.0, 1.0);
		lookfrom = point3(50, 50, 200);
		lookat = point3(100, 0, 0);
		vfov = 30.0;
		break;
	}

	camera cam(lookfrom, lookat, vup, vfov, aspect_ratio, aperture, dist_to_focus, time0, time1);

	// World
	hitable_list world(make_shared<bvh_node>(objects, time0, time1));

	// Render
	// The main ray-tracing based rendering loop
	// TODO: finish your own ray-tracing renderer according to the given tutorials
#pragma omp parallel for schedule(dynamic) collapse(2)
	for (int j = image_height - 1; j >= 0; j--)
	{
		for (int i = 0; i < image_width; i++)
		{
			color pixel_color(0, 0, 0);
			for (int s = 0; s < samples_per_pixel; s++)
			{
				auto u = (i + random_double()) / (image_width - 1);
				auto v = (j + random_double()) / (image_height - 1);
				ray r = cam.get_ray(u, v);
				pixel_color += de_nan(ray_color(r, background, world, hlist, max_depth));
			}
			write_color(i, j, pixel_color, samples_per_pixel);
		}
	}

	double endFrame = clock();
	double timeConsuming = static_cast<double>(endFrame - startFrame) / CLOCKS_PER_SEC;
	std::cout << "Ray-tracing based rendering over..." << std::endl;
	std::cout << "The rendering task took " << timeConsuming << " seconds" << std::endl;
}