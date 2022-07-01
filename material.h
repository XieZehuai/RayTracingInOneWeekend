#ifndef MATERIAL_H
#define MATERIAL_H

#include "rtweekend.h"
#include "hittable.h"
#include "texture.h"
#include "onb.h"

/**
 * @brief 所有材质的基类
 */
class material
{
public:
    /**
     * @brief 着色
     *
     * @param r 光线
     * @param rec 物体表面属性
     * @param attenuation 返回的颜色
     * @param scattered 散射后的射线
     * @return true
     * @return false
     */
    virtual bool scatter(const ray &r, const hit_record &rec, color &albedo, ray &scattered, double &pdf) const
    {
        return false;
    }

    virtual double scattering_pdf(const ray &r, const hit_record &rec, const ray &scattered) const
    {
        return 0;
    }

    virtual color emitted(const ray &r, const hit_record &rec, double u, double v, const point3 &p) const
    {
        return color(0);
    }
};

class lambertian : public material
{
public:
    lambertian(const color &a) : albedo(make_shared<solid_color>(a)) {}
    lambertian(shared_ptr<texture> a) : albedo(a) {}

    virtual bool scatter(const ray &r, const hit_record &rec, color &alb, ray &scattered, double &pdf) const override
    {
        onb uvw;
        uvw.build_from_w(rec.normal);

        auto direction = uvw.local(random_in_hemisphere());
        scattered = ray(rec.p, unit_vector(direction), r.time());
        alb = albedo->sample(rec.u, rec.v, rec.p);
        pdf = dot(uvw.w(), scattered.direction()) / pi;

        return true;
    }

    virtual double scattering_pdf(const ray &r, const hit_record &rec, const ray &scattered) const override
    {
        auto cosine = dot(rec.normal, unit_vector(scattered.direction()));
        return cosine < 0 ? 0 : cosine / pi;
    }

public:
    shared_ptr<texture> albedo;
};

class metal : public material
{
public:
    metal(const color &a, const float &f) : albedo(a), fuzz(f) {}

    virtual bool scatter(const ray &r, const hit_record &rec, color &alb, ray &scattered, double &pdf) const override
    {
        vec3 reflected = reflect(unit_vector(r.direction()), rec.normal);
        scattered = ray(rec.p, reflected + fuzz * random_in_unit_sphere(), r.time());
        alb = albedo;

        return dot(scattered.direction(), rec.normal) > 0;
    }

public:
    color albedo;
    double fuzz;
};

class dielectric : public material
{
public:
    dielectric(double ir) : ir(ir) {}

    virtual bool scatter(const ray &r, const hit_record &rec, color &alb, ray &scattered, double &pdf) const override
    {
        alb = color(1.0, 1.0, 1.0);
        double refraction_ratio = rec.front_face ? (1.0 / ir) : ir;

        vec3 unit_direction = r.direction();
        double cos_theta = fmin(dot(-unit_direction, rec.normal), 1.0);
        double sin_theta = sqrt(1.0 - cos_theta * cos_theta);

        bool cannot_refract = refraction_ratio * sin_theta > 1.0;
        vec3 direction;

        if (cannot_refract || reflectance(cos_theta, refraction_ratio) > random_double())
        {
            direction = reflect(unit_direction, rec.normal);
        }
        else
        {
            direction = refract(unit_direction, rec.normal, refraction_ratio);
        }

        scattered = ray(rec.p, direction, r.time());

        return true;
    }

public:
    double ir; // 折射系数

private:
    static double reflectance(double cosine, double ref_idx)
    {
        // use Schlick's approximation for reflectance.
        auto r0 = (1 - ref_idx) / (1 + ref_idx);
        r0 = r0 * r0;
        return r0 + (1 - r0) * pow((1 - cosine), 5);
    }
};

class diffuse_light : public material
{
public:
    diffuse_light(shared_ptr<texture> a) : emit(a) {}

    diffuse_light(color c) : emit(make_shared<solid_color>(c)) {}

    virtual bool scatter(const ray &r_in, const hit_record &rec, color &alb, ray &scattered, double &pdf) const override
    {
        return false;
    }

    virtual color emitted(const ray &r, const hit_record &rec, double u, double v, const point3 &p) const override
    {
        if (rec.front_face)
        {
            return emit->sample(u, v, p);
        }
        else
        {
            return color(0);
        }
    }

private:
    shared_ptr<texture> emit;
};

class isotropic : public material
{
public:
    isotropic(shared_ptr<texture> a) : albedo(a)
    {
    }

    isotropic(color c) : albedo(make_shared<solid_color>(c))
    {
    }

    virtual bool scatter(const ray &r, const hit_record &rec, color &alb, ray &scattered, double &pdf) const override
    {
        scattered = ray(rec.p, random_in_unit_sphere(), r.time());
        alb = albedo->sample(rec.u, rec.v, rec.p);
        return true;
    }

private:
    shared_ptr<texture> albedo;
};

#endif
