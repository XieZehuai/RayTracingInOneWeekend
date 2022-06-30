#ifndef RENDERER_H
#define RENDERER_H

#include <thread>
#include <mutex>
#include <vector>

#include "rtweekend.h"
#include "hittable.h"
#include "material.h"
#include "scene_generator.h"

class renderer
{
public:
    virtual void render(const shared_ptr<scene_generator> &scene) = 0;

    std::vector<color> get_frame_buffer() const
    {
        return frame_buffer;
    }

protected:
    std::vector<color> frame_buffer;

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

    void update_progress(double progress)
    {
        std::cerr << "\rRendering: " << progress * 100.0 << " %" << std::flush;
    }
};

class multi_thread_renderer : public renderer
{
public:
    multi_thread_renderer(int batch_x, int batch_y)
        : batch_x(batch_x), batch_y(batch_y)
    {
    }

    virtual void render(const shared_ptr<scene_generator> &scene) override
    {
        int image_width = scene->image_width;
        int image_height = scene->image_height;
        int samples_per_pixel = scene->samples_per_pixel;
        int max_depth = scene->max_depth;
        color background_color = scene->background_color;

        bvh_node world = scene->generate_bvh_scene();
        camera cam = scene->get_camera();

        frame_buffer = std::vector<color>(image_height * image_width);

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

                        pixel_color += ray_color(r, background_color, world, max_depth);
                    }

                    frame_buffer[m++] = pixel_color;
                    progress++;
                }

                std::lock_guard<std::mutex> g1(mutex_ins);
                update_progress(1.0 * progress / image_width / image_height);
            }
        };

        int stride_x = int(ceil((double)image_width / batch_x));
        int stride_y = int(ceil((double)image_height / batch_y));
        std::vector<std::thread> render_threads;

        for (int j = 0, index = 0; j < image_height; j += stride_y)
        {
            for (int i = 0; i < image_width; i += stride_x, index++)
            {
                uint32_t rowStart = i;
                uint32_t rowEnd = std::min(i + stride_x, image_width);
                uint32_t colStart = j;
                uint32_t colEnd = std::min(j + stride_y, image_height);

                render_threads.push_back(std::thread(subRenderThread, rowStart, rowEnd, colStart, colEnd));
            }
        }

        for (int i = 0; i < batch_x * batch_y; i++)
        {
            render_threads[i].join();
        }

        update_progress(1.0);
    }

private:
    const int batch_x;
    const int batch_y;

    std::mutex mutex_ins;
};

class single_thread_renderer : public renderer
{
public:
    virtual void render(const shared_ptr<scene_generator> &scene) override
    {
        int image_width = scene->image_width;
        int image_height = scene->image_height;
        int samples_per_pixel = scene->samples_per_pixel;
        int max_depth = scene->max_depth;
        color background_color = scene->background_color;

        bvh_node world = scene->generate_bvh_scene();
        camera cam = scene->get_camera();

        frame_buffer = std::vector<color>(image_height * image_width);

        int progress = 0;
        for (int j = image_height - 1, m = 0; j >= 0; j--)
        {
            for (int i = 0; i < image_width; i++, m++)
            {
                color pixel_color(0, 0, 0);

                for (int s = 0; s < samples_per_pixel; s++)
                {
                    auto u = (i + random_double()) / (image_width - 1);
                    auto v = (j + random_double()) / (image_height - 1);
                    ray r = cam.get_ray(u, v);
                    pixel_color += ray_color(r, background_color, world, max_depth);
                }

                frame_buffer[m] = pixel_color;
                progress++;
            }

            update_progress(1.0 * progress / image_width / image_height);
        }

        update_progress(1.0);
    }
};

#endif
