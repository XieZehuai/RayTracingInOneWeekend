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
#include "box.h"

#include "scene_generator.h"

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

int main()
{
    std::vector<shared_ptr<scene_generator>> scenes;
    scenes.push_back(make_shared<random_scene>());
    scenes.push_back(make_shared<two_spheres>());
    scenes.push_back(make_shared<two_perlin_spheres>());
    scenes.push_back(make_shared<earth>());
    scenes.push_back(make_shared<simple_light>());
    scenes.push_back(make_shared<cornell_box>());

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

    hittable_list world = scenes[0]->generate(&aspect_ratio,
                                              &image_width,
                                              &image_height,
                                              &samples_per_pixel,
                                              &background_color,
                                              &lookfrom,
                                              &lookat,
                                              &vfov,
                                              &aperture);
    bvh_node bvh(world, 0.0, 1.0);

    camera cam(lookfrom, lookat, vup, vfov, aspect_ratio, aperture, dist_to_focus, 0.0, 1.0);

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
