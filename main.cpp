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

std::mutex mutex_ins;

void update_progress(double progress)
{
    std::cerr << "\rRendering: " << progress * 100.0 << " %" << std::flush;
}

color ray_color(const ray &r, const hittable &world, int depth)
{
    if (depth <= 0)
        return color(0, 0, 0);

    hit_record rec;
    if (world.hit(r, 0.001, infinity, rec))
    {
        ray scattered;
        color attenuation;

        if (rec.mat->scatter(r, rec, attenuation, scattered))
        {
            return attenuation * ray_color(scattered, world, depth - 1);
        }
        else
        {
            return color(0, 0, 0);
        }
    }

    vec3 unit_direction = unit_vector(r.direction());
    auto t = 0.5 * (unit_direction.y() + 1.0);
    return (1.0 - t) * color(1.0, 1.0, 1.0) + t * color(0.5, 0.7, 1.0);
}

hittable_list random_scene()
{
    hittable_list world;

    auto ground_material = make_shared<lambertian>(color(0.5, 0.5, 0.5));
    world.add(make_shared<sphere>(point3(0, -1000, 0), 1000, ground_material));

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

int main()
{
    // Image =========================================================================================
    const auto aspect_ratio = 16.0 / 9.0;
    const int image_width = 400;
    const int image_height = static_cast<int>(image_width / aspect_ratio);
    const int samples_per_pixel = 100;
    const int max_depth = 20;

    // World =========================================================================================
    hittable_list world = random_scene();

    auto material_ground = make_shared<lambertian>(color(0.8, 0.8, 0.0));
    auto material_center = make_shared<lambertian>(color(0.1, 0.2, 0.5));
    auto material_left = make_shared<dielectric>(1.5);
    auto material_right = make_shared<metal>(color(0.8, 0.6, 0.2), 0.0);

    world.add(make_shared<sphere>(point3(0.0, -100.5, -1.0), 100.0, material_ground));
    world.add(make_shared<sphere>(point3(0.0, 0.0, -1.0), 0.5, material_center));
    world.add(make_shared<sphere>(point3(-1.0, 0.0, -1.0), 0.5, material_left));
    world.add(make_shared<sphere>(point3(-1.0, 0.0, -1.0), -0.45, material_left));
    world.add(make_shared<sphere>(point3(1.0, 0.0, -1.0), 0.5, material_right));

    // Camera =========================================================================================
    point3 lookfrom(13, 2, 3);
    point3 lookat(0, 0, 0);
    vec3 vup(0, 1, 0);
    auto dist_to_focus = 10.0;
    auto aperture = 0.1;

    camera cam(lookfrom, lookat, vup, 20, aspect_ratio, aperture, dist_to_focus, 0.0, 1.0);

    // Render =========================================================================================

    bool multi_thread = true;
    clock_t start = 0;
    clock_t end = 0;

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
                        pixel_color += ray_color(r, world, max_depth);
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
                    pixel_color += ray_color(r, world, max_depth);
                }

                write_color(std::cout, pixel_color, samples_per_pixel);
            }
        }
    }

    std::cerr << "\nDone, cost time: " << ((end - start) / CLOCKS_PER_SEC) << " seconds";

    return 0;
}
