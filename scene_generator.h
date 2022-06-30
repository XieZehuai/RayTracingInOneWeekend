#ifndef SCENE_GENERATOR_H
#define SCENE_GENERATOR_H

#include "rtweekend.h"
#include "sphere.h"
#include "moving_sphere.h"
#include "aa_rect.h"
#include "box.h"
#include "hittable_list.h"
#include "texture.h"
#include "material.h"

class scene_generator
{
public:
    virtual hittable_list generate(double *aspect_ratio,
                                   int *image_width,
                                   int *image_height,
                                   int *samples_per_pixel,
                                   color *background_color,
                                   point3 *lookfrom,
                                   point3 *lookat,
                                   double *vfov,
                                   double *aperture) const = 0;
};

class random_scene : public scene_generator
{
public:
    virtual hittable_list generate(double *aspect_ratio,
                                   int *image_width,
                                   int *image_height,
                                   int *samples_per_pixel,
                                   color *background_color,
                                   point3 *lookfrom,
                                   point3 *lookat,
                                   double *vfov,
                                   double *aperture) const override
    {
        *lookfrom = point3(13, 2, 3);
        *lookat = point3(0, 0, 0);
        *vfov = 20.0;
        *aperture = 0.1;
        *background_color = color(0.70, 0.80, 1.00);

        hittable_list world;

        auto checker = make_shared<checker_texture>(color(0.2, 0.3, 0.1), color(0.9, 0.9, 0.9));
        world.add(make_shared<sphere>(point3(0, -1000, 0), 1000, make_shared<lambertian>(checker)));

        for (int a = -11; a < 11; a++)
        {
            for (int b = -11; b < 11; b++)
            {
                auto choose_mat = random_double();
                point3 center(a + 0.9 * random_double(), 0.2, b + 0.9 * random_double());

                if ((center - point3(4, 0.2, 0)).length() > 0.9)
                {
                    shared_ptr<material> sphere_material;

                    if (choose_mat < 0.8)
                    {
                        // diffuse
                        auto albedo = color::random() * color::random();
                        sphere_material = make_shared<lambertian>(albedo);

                        auto center2 = center + vec3(0, random_double(0, 0.5), 0);
                        world.add(make_shared<moving_sphere>(center, center2, 0.0, 1.0, 0.2, sphere_material));
                    }
                    else if (choose_mat < 0.95)
                    {
                        // metal
                        auto albedo = color::random(0.5, 1);
                        auto fuzz = random_double(0, 0.5);
                        sphere_material = make_shared<metal>(albedo, fuzz);
                        world.add(make_shared<sphere>(center, 0.2, sphere_material));
                    }
                    else
                    {
                        // glass
                        sphere_material = make_shared<dielectric>(1.5);
                        world.add(make_shared<sphere>(center, 0.2, sphere_material));
                    }
                }
            }
        }

        auto material1 = make_shared<dielectric>(1.5);
        world.add(make_shared<sphere>(point3(0, 1, 0), 1.0, material1));

        auto material2 = make_shared<lambertian>(color(0.4, 0.2, 0.1));
        world.add(make_shared<sphere>(point3(-4, 1, 0), 1.0, material2));

        auto material3 = make_shared<metal>(color(0.7, 0.6, 0.5), 0.0);
        world.add(make_shared<sphere>(point3(4, 1, 0), 1.0, material3));

        return world;
    }
};

class two_spheres : public scene_generator
{
public:
    virtual hittable_list generate(double *aspect_ratio,
                                   int *image_width,
                                   int *image_height,
                                   int *samples_per_pixel,
                                   color *background_color,
                                   point3 *lookfrom,
                                   point3 *lookat,
                                   double *vfov,
                                   double *aperture) const override
    {
        *lookfrom = point3(13, 2, 3);
        *lookat = point3(0, 0, 0);
        *vfov = 20.0;
        *background_color = color(0.70, 0.80, 1.00);

        hittable_list objects;

        auto checker = make_shared<checker_texture>(color(0.2, 0.3, 0.1), color(0.9, 0.9, 0.9));

        objects.add(make_shared<sphere>(point3(0, -10, 0), 10, make_shared<lambertian>(checker)));
        objects.add(make_shared<sphere>(point3(0, 10, 0), 10, make_shared<lambertian>(checker)));

        return objects;
    }
};

class two_perlin_spheres : public scene_generator
{
public:
    virtual hittable_list generate(double *aspect_ratio,
                                   int *image_width,
                                   int *image_height,
                                   int *samples_per_pixel,
                                   color *background_color,
                                   point3 *lookfrom,
                                   point3 *lookat,
                                   double *vfov,
                                   double *aperture) const override
    {
        *lookfrom = point3(13, 2, 3);
        *lookat = point3(0, 0, 0);
        *vfov = 20.0;
        *background_color = color(0.70, 0.80, 1.00);

        auto earth_texture = make_shared<image_texture>("D:/Workspace/RayTracingInOneWeekend/res/earthmap.jpg");
        auto earth_material = make_shared<lambertian>(earth_texture);
        auto globe = make_shared<sphere>(point3(0, 0, 0), 2, earth_material);

        return hittable_list(globe);
    };
};

class earth : public scene_generator
{
public:
    virtual hittable_list generate(double *aspect_ratio,
                                   int *image_width,
                                   int *image_height,
                                   int *samples_per_pixel,
                                   color *background_color,
                                   point3 *lookfrom,
                                   point3 *lookat,
                                   double *vfov,
                                   double *aperture) const override
    {
        *lookfrom = point3(13, 2, 3);
        *lookat = point3(0, 0, 0);
        *vfov = 20.0;
        *background_color = color(0.70, 0.80, 1.00);

        hittable_list objects;

        auto perlin_tex = make_shared<noise_texture>(4);
        objects.add(make_shared<sphere>(point3(0, -1000, 0), 1000, make_shared<lambertian>(perlin_tex)));
        objects.add(make_shared<sphere>(point3(0, 2, 0), 2, make_shared<lambertian>(perlin_tex)));

        auto light_mat = make_shared<diffuse_light>(color(4, 4, 4));
        objects.add(make_shared<xy_rect>(3, 5, 1, 3, -2, light_mat));
        objects.add(make_shared<sphere>(point3(0, 8, 0), 2, light_mat));

        return objects;
    };
};

class simple_light : public scene_generator
{
public:
    virtual hittable_list generate(double *aspect_ratio,
                                   int *image_width,
                                   int *image_height,
                                   int *samples_per_pixel,
                                   color *background_color,
                                   point3 *lookfrom,
                                   point3 *lookat,
                                   double *vfov,
                                   double *aperture) const override
    {
        *samples_per_pixel = 400;
        *background_color = color(0, 0, 0);
        *lookfrom = point3(26, 3, 6);
        *lookat = point3(0, 2, 0);
        *vfov = 20.0;

        hittable_list objects;

        auto perlin_tex = make_shared<noise_texture>(4);
        objects.add(make_shared<sphere>(point3(0, -1000, 0), 1000, make_shared<lambertian>(perlin_tex)));
        objects.add(make_shared<sphere>(point3(0, 2, 0), 2, make_shared<lambertian>(perlin_tex)));

        auto light_mat = make_shared<diffuse_light>(color(4, 4, 4));
        objects.add(make_shared<xy_rect>(3, 5, 1, 3, -2, light_mat));
        objects.add(make_shared<sphere>(point3(0, 8, 0), 2, light_mat));

        return objects;
    };
};

class cornell_box : public scene_generator
{
public:
    virtual hittable_list generate(double *aspect_ratio,
                                   int *image_width,
                                   int *image_height,
                                   int *samples_per_pixel,
                                   color *background_color,
                                   point3 *lookfrom,
                                   point3 *lookat,
                                   double *vfov,
                                   double *aperture) const override
    {
        *aspect_ratio = 1.0;
        *image_width = 400;
        *image_height = 400;
        *samples_per_pixel = 200;
        *background_color = color(0, 0, 0);
        *lookfrom = point3(278, 278, -800);
        *lookat = point3(278, 278, 0);
        *vfov = 40.0;

        hittable_list objects;

        auto red = make_shared<lambertian>(color(.65, .05, .05));
        auto white = make_shared<lambertian>(color(.73, .73, .73));
        auto green = make_shared<lambertian>(color(.12, .45, .15));
        auto light = make_shared<diffuse_light>(color(15, 15, 15));

        objects.add(make_shared<yz_rect>(0, 555, 0, 555, 555, green));
        objects.add(make_shared<yz_rect>(0, 555, 0, 555, 0, red));
        objects.add(make_shared<xz_rect>(213, 343, 227, 332, 554, light));
        objects.add(make_shared<xz_rect>(0, 555, 0, 555, 0, white));
        objects.add(make_shared<xz_rect>(0, 555, 0, 555, 555, white));
        objects.add(make_shared<xy_rect>(0, 555, 0, 555, 555, white));

        objects.add(make_shared<box>(point3(130, 0, 65), point3(295, 165, 230), white));
        objects.add(make_shared<box>(point3(265, 0, 295), point3(430, 330, 460), white));

        return objects;
    };
};

#endif
