#ifndef TEXTURE_H
#define TEXTURE_H

#include "rtweekend.h"
#include "perlin.h"
#include "rtw_stb_image.h"

class texture
{
public:
    virtual color sample(double u, double v, const point3 &p) const = 0;
};

class solid_color : public texture
{
public:
    solid_color() {}
    solid_color(double r, double g, double b) : solid_color(color(r, g, b)) {}
    solid_color(const color &c) : c(c) {}

    virtual color sample(double u, double v, const point3 &p) const override
    {
        return c;
    }

private:
    color c;
};

class checker_texture : public texture
{
public:
    checker_texture() {}

    checker_texture(shared_ptr<texture> _even, shared_ptr<texture> _odd)
        : even(_even), odd(_odd) {}

    checker_texture(color c1, color c2)
        : even(make_shared<solid_color>(c1)), odd(make_shared<solid_color>(c2)) {}

    virtual color sample(double u, double v, const point3 &p) const override
    {
        auto sines = sin(10 * p.x()) * sin(10 * p.y()) * sin(10 * p.z());

        if (sines < 0)
            return odd->sample(u, v, p);
        else
            return even->sample(u, v, p);
    }

public:
    shared_ptr<texture> odd;
    shared_ptr<texture> even;
};

class noise_texture : public texture
{
public:
    noise_texture() {}
    noise_texture(double s) : scale(s) {}

    virtual color sample(double u, double v, const point3 &p) const override
    {
        return color(1, 1, 1) * 0.5 * (1 + sin(scale * p.z() + 10 * noise.turb(p)));
    }

public:
    perlin noise;
    double scale;
};

class image_texture : public texture
{
public:
    const static int bytes_per_pixel = 3;

    image_texture() : data(nullptr), width(0), height(0), bytes_per_scanline(0)
    {
    }

    /**
     * @brief Construct a new image texture object
     * 
     * @param filename absolute path of image file
     */
    image_texture(const char *filename)
    {
        auto components_per_pixel = bytes_per_pixel;
        data = stbi_load(filename, &width, &height, &components_per_pixel, components_per_pixel);

        if (!data)
        {
            std::cerr << "ERROR: Could not load texture image file '" << filename << "'.\n";
            std::cerr << stbi_failure_reason() << "\n";
            width = height = 0;
        }

        bytes_per_scanline = bytes_per_pixel * width;
    }

    ~image_texture()
    {
        delete data;
    }

    virtual color sample(double u, double v, const vec3 &p) const override
    {
        // 当没有纹理数据时，返回紫色作为错误色
        if (data == nullptr)
        {
            return color(1, 0, 1);
        }

        u = clamp(u, 0.0, 1.0);
        v = 1.0 - clamp(v, 0.0, 1.0); // 反转 y 轴

        auto i = static_cast<int>(u * width);
        auto j = static_cast<int>(v * height);

        if (i >= width)
            i = width - 1;
        if (j >= height)
            j = height - 1;

        const auto color_scale = 1.0 / 255.0;
        auto pixel = data + j * bytes_per_scanline + i * bytes_per_pixel;

        return color(color_scale * pixel[0], color_scale * pixel[1], color_scale * pixel[2]);
    }

private:
    unsigned char *data;
    int width;
    int height;
    int bytes_per_scanline;
};

#endif
