#include <iostream>
#include <thread>
#include <mutex>
#include <vector>
#include <iomanip>
#include <ctime>

#include "rtweekend.h"

#include "color.h"
#include "hittable_list.h"
#include "sphere.h"
#include "moving_sphere.h"
#include "camera.h"
#include "material.h"
#include "bvh.h"
#include "aa_rect.h"

std::mutex mutex_ins;

void update_progress(double progress)
{
    std::cerr << "\rRendering: " << progress * 100.0 << " %" << std::flush;
}

color ray_color(const ray &r, const color &background_color, const hittable &world, int depth)
{
    if (depth <= 0)
        return color(0, 0, 0);

    // 如果射线没击中任何物体，则返回背景色
    hit_record rec;
    if (!world.hit(r, 0.001, infinity, rec))
        return background_color;

    ray scattered;
    color attenuation;
    color emitted = rec.mat->emitted(rec.u, rec.v, rec.p); // 物体表面的自发光

    if (!rec.mat->scatter(r, rec, attenuation, scattered))
        return emitted;

    return emitted + attenuation * ray_color(scattered, background_color, world, depth - 1);
}

hittable_list random_scene()
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

hittable_list two_spheres()
{
    hittable_list objects;

    auto checker = make_shared<checker_texture>(color(0.2, 0.3, 0.1), color(0.9, 0.9, 0.9));

    objects.add(make_shared<sphere>(point3(0, -10, 0), 10, make_shared<lambertian>(checker)));
    objects.add(make_shared<sphere>(point3(0, 10, 0), 10, make_shared<lambertian>(checker)));

    return objects;
}

hittable_list two_perlin_spheres()
{
    hittable_list objects;

    auto pertext = make_shared<noise_texture>(4);
    objects.add(make_shared<sphere>(point3(0, -1000, 0), 1000, make_shared<lambertian>(pertext)));
    objects.add(make_shared<sphere>(point3(0, 2, 0), 2, make_shared<lambertian>(pertext)));

    return objects;
}

hittable_list earth()
{
    auto earth_texture = make_shared<image_texture>("D:/Workspace/RayTracingInOneWeekend/res/earthmap.jpg");
    auto earth_material = make_shared<lambertian>(earth_texture);
    auto globe = make_shared<sphere>(point3(0, 0, 0), 2, earth_material);

    return hittable_list(globe);
}

hittable_list simple_light()
{
    hittable_list objects;

    auto perlin_tex = make_shared<noise_texture>(4);
    objects.add(make_shared<sphere>(point3(0, -1000, 0), 1000, make_shared<lambertian>(perlin_tex)));
    objects.add(make_shared<sphere>(point3(0, 2, 0), 2, make_shared<lambertian>(perlin_tex)));

    auto light_mat = make_shared<diffuse_light>(color(4, 4, 4));
    objects.add(make_shared<xy_rect>(3, 5, 1, 3, -2, light_mat));
    objects.add(make_shared<sphere>(point3(0, 8, 0), 2, light_mat));

    return objects;
}

hittable_list cornell_box()
{
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

    return objects;
}

int main()
{
    // Image =========================================================================================
    auto aspect_ratio = 16.0 / 9.0;
    int image_width = 600;
    int image_height = static_cast<int>(image_width / aspect_ratio);
    int max_depth = 10;
    int samples_per_pixel = 200;

    // World =========================================================================================
    point3 lookfrom;
    point3 lookat;
    auto vfov = 40.0;
    auto aperture = 0.0;
    vec3 vup(0, 1, 0);
    auto dist_to_focus = 10.0;
    color background_color(0);

    hittable_list world;
    switch (6)
    {
    case 1:
        world = random_scene();
        lookfrom = point3(13, 2, 3);
        lookat = point3(0, 0, 0);
        vfov = 20.0;
        aperture = 0.1;
        background_color = color(0.70, 0.80, 1.00);
        break;

    case 2:
        world = two_spheres();
        lookfrom = point3(13, 2, 3);
        lookat = point3(0, 0, 0);
        vfov = 20.0;
        background_color = color(0.70, 0.80, 1.00);
        break;

    case 3:
        world = two_perlin_spheres();
        lookfrom = point3(13, 2, 3);
        lookat = point3(0, 0, 0);
        vfov = 20.0;
        background_color = color(0.70, 0.80, 1.00);
        break;

    case 4:
        world = earth();
        lookfrom = point3(13, 2, 3);
        lookat = point3(0, 0, 0);
        vfov = 20.0;
        background_color = color(0.70, 0.80, 1.00);
        break;

    case 5:
        world = simple_light();
        samples_per_pixel = 400;
        background_color = color(0, 0, 0);
        lookfrom = point3(26, 3, 6);
        lookat = point3(0, 2, 0);
        vfov = 20.0;
        break;

    case 6:
        world = cornell_box();
        aspect_ratio = 1.0;
        image_width = 600;
        image_height = 600;
        samples_per_pixel = 200;
        background_color = color(0, 0, 0);
        lookfrom = point3(278, 278, -800);
        lookat = point3(278, 278, 0);
        vfov = 40.0;
        break;

    default:
        background_color = color(0);
        break;
    }

    camera cam(lookfrom, lookat, vup, vfov, aspect_ratio, aperture, dist_to_focus, 0.0, 1.0);
    bvh_node bvh(world, 0.0, 1.0);

    // Render =========================================================================================
    bool multi_thread = true;
    clock_t start = 0;
    clock_t end = 0;

    // 设置控制台打印浮点数时保留两位数字
    std::cerr << std::setiosflags(std::ios::fixed) << std::setprecision(2);

    std::cout << "P3\n"
              << image_width << ' ' << image_height << "\n255\n";

    if (multi_thread)
    {
        std::vector<color> frame_buffer(image_height * image_width);

        int progress = 0;
        auto subRenderThread = [&](int rowStart, int rowEnd, int colStart, int colEnd)
        {
            for (int j = colStart; j < colEnd; j++)
            {
                // 计算当前像素在 frame buffer 中的索引
                int m = (image_height - 1 - j) * image_width + rowStart;

                for (int i = rowStart; i < rowEnd; i++)
                {
                    color pixel_color(0, 0, 0);

                    for (int s = 0; s < samples_per_pixel; s++)
                    {
                        auto u = (i + random_double()) / (image_width - 1);
                        auto v = (j + random_double()) / (image_height - 1);
                        ray r = cam.get_ray(u, v);

                        pixel_color += ray_color(r, background_color, bvh, max_depth);
                    }

                    frame_buffer[m++] = pixel_color;
                    progress++;
                }

                std::lock_guard<std::mutex> g1(mutex_ins);
                update_progress(1.0 * progress / image_width / image_height);
            }
        };

        const int batch_x = 4;
        const int batch_y = 4;
        std::thread render_threads[batch_x * batch_y];

        int stride_x = int(ceil((double)image_width / batch_x));
        int stride_y = int(ceil((double)image_height / batch_y));

        for (int j = 0, index = 0; j < image_height; j += stride_y)
        {
            for (int i = 0; i < image_width; i += stride_x, index++)
            {
                uint32_t rowStart = i;
                uint32_t rowEnd = std::min(i + stride_x, image_width);
                uint32_t colStart = j;
                uint32_t colEnd = std::min(j + stride_y, image_height);

                render_threads[index] = std::thread(subRenderThread, rowStart, rowEnd, colStart, colEnd);
            }
        }

        start = clock();
        for (int i = 0; i < batch_x * batch_y; i++)
        {
            render_threads[i].join();
        }
        end = clock();

        update_progress(1.0);

        for (int i = 0; i < frame_buffer.size(); i++)
        {
            write_color(std::cout, frame_buffer[i], samples_per_pixel);
        }
    }
    else
    {
        start = clock();

        for (int j = image_height - 1; j >= 0; --j)
        {
            std::cerr << "\rScanlines remaining: " << j << ' ' << std::flush;

            for (int i = 0; i < image_width; ++i)
            {
                color pixel_color(0, 0, 0);

                for (int s = 0; s < samples_per_pixel; s++)
                {
                    auto u = (i + random_double()) / (image_width - 1);
                    auto v = (j + random_double()) / (image_height - 1);
                    ray r = cam.get_ray(u, v);

                    pixel_color += ray_color(r, background_color, world, max_depth);
                }

                write_color(std::cout, pixel_color, samples_per_pixel);
            }
        }

        end = clock();
    }

    std::cerr << "\nDone, cost time: " << ((end - start) / CLOCKS_PER_SEC) << " seconds";

    return 0;
}
