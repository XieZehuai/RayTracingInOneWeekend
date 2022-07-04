#ifndef SCENE_GENERATOR_H
#define SCENE_GENERATOR_H

#include <string>

#include "rtweekend.h"
#include "sphere.h"
#include "moving_sphere.h"
#include "aa_rect.h"
#include "box.h"
#include "hittable_list.h"
#include "constant_medium.h"
#include "texture.h"
#include "material.h"
#include "camera.h"

class scene_generator
{
public:
    double aspect_ratio = 16.0 / 9.0;
    int image_width = 600;
    int image_height = static_cast<int>(image_width / aspect_ratio);
    int max_depth = 16;
    int samples_per_pixel = 200;

    point3 lookfrom = point3(0, 0, 3);
    point3 lookat = point3(0, 0, 0);
    double vfov = 40.0;
    double aperture = 0.0;
    vec3 vup = vec3(0, 1, 0);
    double dist_to_focus = 3.0;
    color background_color = vec3(0);

    camera get_camera() const
    {
        return camera(lookfrom, lookat, vup, vfov, aspect_ratio, aperture, dist_to_focus, 0.0, 1.0);
    }

    bvh_node generate_bvh_scene() const
    {
        return bvh_node(generate(), 0.0, 1.0);
    }

    virtual std::string output_filename() const = 0;

    virtual hittable_list generate() const = 0;

    virtual shared_ptr<hittable_list> lights() const
    {
        return make_shared<hittable_list>();
    }
};

class test_scene : public scene_generator
{
public:
    test_scene()
    {
        lookfrom = point3(-2, 2, 3);
        lookat = point3(-1, 0, 0);
        dist_to_focus = 5;
        // background_color = color(0.9, 0.95, 1.0);
    }

    virtual std::string output_filename() const override
    {
        return "test_scene.ppm";
    }

    virtual hittable_list generate() const override
    {
        hittable_list world;

        auto material_ground = make_shared<lambertian>(color(0.2, 0.7, 0.2));
        auto material_center = make_shared<lambertian>(color(0.3, 0.5, 0.8));
        auto material_left = make_shared<dielectric>(1.5);
        auto material_right = make_shared<metal>(color(0.6, 0.5, 0.4), 0.0);

        world.add(make_shared<sphere>(point3(0.0, -100.5, -1.0), 100.0, material_ground));
        world.add(make_shared<sphere>(point3(0.0, 0.0, -1.0), 0.5, material_center));
        world.add(make_shared<sphere>(point3(-1.0, 0.0, -1.0), 0.5, material_left));
        world.add(make_shared<sphere>(point3(1.0, 0.0, -1.0), 0.5, material_right));

        auto material_light = make_shared<diffuse_light>(color(10, 10, 10));
        auto light_rect = make_shared<xy_rect>(-1, 1, 0.5, 1.5, 4, material_light);
        world.add(make_shared<flip_face>(light_rect));

        return world;
    }
};

class random_scene : public scene_generator
{
public:
    random_scene()
    {
        lookfrom = point3(13, 2, 3);
        lookat = point3(0, 0, 0);
        vfov = 20.0;
        aperture = 0.1;
        background_color = color(0.70, 0.80, 1.00);
    }

    virtual std::string output_filename() const override
    {
        return "random_scene.ppm";
    }

    virtual hittable_list generate() const override
    {
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
    two_spheres()
    {
        lookfrom = point3(13, 2, 3);
        lookat = point3(0, 0, 0);
        vfov = 20.0;
        background_color = color(0.70, 0.80, 1.00);
    }

    virtual std::string output_filename() const override
    {
        return "two_spheres.ppm";
    }

    virtual hittable_list generate() const override
    {
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
    two_perlin_spheres()
    {
        lookfrom = point3(13, 2, 3);
        lookat = point3(0, 0, 0);
        vfov = 20.0;
        background_color = color(0.70, 0.80, 1.00);
    }

    virtual std::string output_filename() const override
    {
        return "two_perlin_spheres.ppm";
    }

    virtual hittable_list generate() const override
    {
        hittable_list objects;

        auto perlin_tex = make_shared<noise_texture>(4);
        objects.add(make_shared<sphere>(point3(0, -1000, 0), 1000, make_shared<lambertian>(perlin_tex)));
        objects.add(make_shared<sphere>(point3(0, 2, 0), 2, make_shared<lambertian>(perlin_tex)));

        return objects;
    };
};

class earth : public scene_generator
{
public:
    earth()
    {
        lookfrom = point3(13, 2, 3);
        lookat = point3(0, 0, 0);
        vfov = 20.0;
        background_color = color(0.70, 0.80, 1.00);
    }

    virtual std::string output_filename() const override
    {
        return "earth.ppm";
    }

    virtual hittable_list generate() const override
    {
        auto earth_texture = make_shared<image_texture>("../../res/earthmap.jpg");
        auto earth_material = make_shared<lambertian>(earth_texture);
        auto globe = make_shared<sphere>(point3(0, 0, 0), 2, earth_material);

        return hittable_list(globe);
    };
};

class simple_light : public scene_generator
{
public:
    simple_light()
    {
        samples_per_pixel = 400;
        background_color = color(0, 0, 0);
        lookfrom = point3(26, 3, 6);
        lookat = point3(0, 2, 0);
        vfov = 20.0;
    }

    virtual std::string output_filename() const override
    {
        return "simple_light.ppm";
    }

    virtual hittable_list generate() const override
    {
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
    cornell_box()
    {
        aspect_ratio = 1.0;
        image_width = 400;
        image_height = 400;
        samples_per_pixel = 100;
        max_depth = 20;
        background_color = color(0, 0, 0);
        lookfrom = point3(278, 278, -800);
        lookat = point3(278, 278, 0);
        vfov = 40.0;
    }

    virtual std::string output_filename() const override
    {
        return "cornell_box.ppm";
    }

    virtual shared_ptr<hittable_list> lights() const override
    {
        auto lights = make_shared<hittable_list>();
        lights->add(make_shared<xz_rect>(213, 343, 227, 332, 554, make_shared<material>()));
        lights->add(make_shared<sphere>(point3(190, 90, 190), 90, make_shared<material>()));

        return lights;
    }

    virtual hittable_list generate() const override
    {
        hittable_list objects;

        auto red = make_shared<lambertian>(color(.65, .05, .05));
        auto white = make_shared<lambertian>(color(.73, .73, .73));
        auto green = make_shared<lambertian>(color(.12, .45, .15));

        // 灯光
        auto light = make_shared<diffuse_light>(color(15, 15, 15));
        auto rect_light = make_shared<xz_rect>(213, 343, 227, 332, 554, light);
        objects.add(make_shared<flip_face>(rect_light));

        // 墙壁
        objects.add(make_shared<yz_rect>(0, 555, 0, 555, 555, green));
        objects.add(make_shared<yz_rect>(0, 555, 0, 555, 0, red));
        objects.add(make_shared<xz_rect>(0, 555, 0, 555, 555, white));
        objects.add(make_shared<xz_rect>(0, 555, 0, 555, 0, white));
        objects.add(make_shared<xy_rect>(0, 555, 0, 555, 555, white));

        // 后面的盒子
        shared_ptr<material> aluminum = make_shared<metal>(color(0.8, 0.85, 0.88), 0.0);
        shared_ptr<hittable> box1 = make_shared<box>(point3(0, 0, 0), point3(165, 330, 165), white);
        box1 = make_shared<rotate_y>(box1, 15);
        box1 = make_shared<translate>(box1, vec3(265, 0, 295));
        objects.add(box1);

        // 前面的盒子
        auto glass = make_shared<dielectric>(1.5);
        objects.add(make_shared<sphere>(point3(190, 90, 190), 90, glass));

        return objects;
    };
};

class cornell_smoke : public scene_generator
{
public:
    cornell_smoke()
    {
        aspect_ratio = 1.0;
        image_width = 600;
        image_height = 600;
        samples_per_pixel = 200;
        lookfrom = point3(278, 278, -800);
        lookat = point3(278, 278, 0);
        vfov = 40.0;
    }

    virtual std::string output_filename() const override
    {
        return "cornell_smoke.ppm";
    }

    virtual hittable_list generate() const override
    {
        hittable_list objects;

        auto red = make_shared<lambertian>(color(.65, .05, .05));
        auto white = make_shared<lambertian>(color(.73, .73, .73));
        auto green = make_shared<lambertian>(color(.12, .45, .15));
        auto light = make_shared<diffuse_light>(color(7, 7, 7));

        objects.add(make_shared<yz_rect>(0, 555, 0, 555, 555, green));
        objects.add(make_shared<yz_rect>(0, 555, 0, 555, 0, red));
        objects.add(make_shared<xz_rect>(113, 443, 127, 432, 554, light));
        objects.add(make_shared<xz_rect>(0, 555, 0, 555, 555, white));
        objects.add(make_shared<xz_rect>(0, 555, 0, 555, 0, white));
        objects.add(make_shared<xy_rect>(0, 555, 0, 555, 555, white));

        shared_ptr<hittable> box1 = make_shared<box>(point3(0, 0, 0), point3(165, 330, 165), white);
        box1 = make_shared<rotate_y>(box1, 15);
        box1 = make_shared<translate>(box1, vec3(265, 0, 295));

        shared_ptr<hittable> box2 = make_shared<box>(point3(0, 0, 0), point3(165, 165, 165), white);
        box2 = make_shared<rotate_y>(box2, -18);
        box2 = make_shared<translate>(box2, vec3(130, 0, 65));

        objects.add(make_shared<constant_medium>(box1, 0.01, color(0, 0, 0)));
        objects.add(make_shared<constant_medium>(box2, 0.01, color(1, 1, 1)));

        return objects;
    }
};

class the_next_week_final_scene : public scene_generator
{
public:
    the_next_week_final_scene()
    {
        aspect_ratio = 1.0;
        image_width = 400;
        image_height = 400;
        samples_per_pixel = 1000;
        max_depth = 16;
        background_color = color(0, 0, 0);
        lookfrom = point3(478, 278, -600);
        lookat = point3(278, 278, 0);
        vfov = 40.0;
    }

    virtual std::string output_filename() const override
    {
        return "the_next_week_final_scene.ppm";
    }

    virtual shared_ptr<hittable_list> lights() const override
    {
        auto lights = make_shared<hittable_list>();
        lights->add(make_shared<xz_rect>(123, 423, 147, 412, 554, make_shared<material>()));
        lights->add(make_shared<sphere>(point3(260, 150, 45), 50, make_shared<material>()));

        return lights;
    }

    virtual hittable_list generate() const override
    {
        hittable_list boxes1;
        auto ground = make_shared<lambertian>(color(0.48, 0.83, 0.53));

        const int boxes_per_side = 20;
        for (int i = 0; i < boxes_per_side; i++)
        {
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
        }

        hittable_list objects;

        objects.add(make_shared<bvh_node>(boxes1, 0, 1));

        auto light_mat = make_shared<diffuse_light>(color(7, 7, 7));
        auto rect_light = make_shared<xz_rect>(123, 423, 147, 412, 554, light_mat);
        objects.add(make_shared<flip_face>(rect_light));

        auto center1 = point3(400, 400, 200);
        auto center2 = center1 + vec3(30, 0, 0);
        auto moving_sphere_material = make_shared<lambertian>(color(0.7, 0.3, 0.1));
        objects.add(make_shared<moving_sphere>(center1, center2, 0, 1, 50, moving_sphere_material));

        objects.add(make_shared<sphere>(point3(260, 150, 45), 50, make_shared<dielectric>(1.5)));
        objects.add(make_shared<sphere>(
            point3(0, 150, 145), 50, make_shared<metal>(color(0.8, 0.8, 0.9), 1.0)));

        auto boundary = make_shared<sphere>(point3(360, 150, 145), 70, make_shared<dielectric>(1.5));
        objects.add(boundary);
        objects.add(make_shared<constant_medium>(boundary, 0.2, color(0.2, 0.4, 0.9)));
        boundary = make_shared<sphere>(point3(0, 0, 0), 5000, make_shared<dielectric>(1.5));
        objects.add(make_shared<constant_medium>(boundary, .0001, color(1, 1, 1)));

        auto emat = make_shared<lambertian>(make_shared<image_texture>("../../res/earthmap.jpg"));
        objects.add(make_shared<sphere>(point3(400, 200, 400), 100, emat));
        auto pertext = make_shared<noise_texture>(0.1);
        objects.add(make_shared<sphere>(point3(220, 280, 300), 80, make_shared<lambertian>(pertext)));

        hittable_list boxes2;
        auto white = make_shared<lambertian>(color(.73, .73, .73));
        int ns = 1000;
        for (int j = 0; j < ns; j++)
        {
            boxes2.add(make_shared<sphere>(point3::random(0, 165), 10, white));
        }

        objects.add(make_shared<translate>(
            make_shared<rotate_y>(
                make_shared<bvh_node>(boxes2, 0.0, 1.0), 15),
            vec3(-100, 270, 395)));

        return objects;
    };
};

#endif
