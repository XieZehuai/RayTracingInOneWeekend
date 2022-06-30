#ifndef AA_RECT_H
#define AA_RECT_H

#include "rtweekend.h"
#include "hittable.h"

/**
 * @brief 垂直于 z 轴的 xy 平面
 */
class xy_rect : public hittable
{
private:
    shared_ptr<material> mat;
    double x0, x1, y0, y1, z;

public:
    xy_rect() {}

    xy_rect(double x0, double x1, double y0, double y1, double z, shared_ptr<material> mat)
        : x0(x0), x1(x1), y0(y0), y1(y1), z(z), mat(mat)
    {
    }

    virtual bool hit(const ray &r, double t_min, double t_max, hit_record &rec) const override;

    virtual bool bounding_box(double time0, double time1, aabb &output_box) const override
    {
        // The bounding box must have non-zero width in each dimension, so pad the Z
        // dimension a small amount.
        output_box = aabb(point3(x0, y0, z - 0.0001), point3(x1, y1, z + 0.0001));
        return true;
    }
};

class xz_rect : public hittable
{
public:
    xz_rect() {}

    xz_rect(double x0, double x1, double z0, double z1, double y, shared_ptr<material> mat)
        : x0(x0), x1(x1), z0(z0), z1(z1), y(y), mat(mat){};

    virtual bool hit(const ray &r, double t_min, double t_max, hit_record &rec) const override;

    virtual bool bounding_box(double time0, double time1, aabb &output_box) const override
    {
        // The bounding box must have non-zero width in each dimension, so pad the Y
        // dimension a small amount.
        output_box = aabb(point3(x0, y - 0.0001, z0), point3(x1, y + 0.0001, z1));
        return true;
    }

public:
    shared_ptr<material> mat;
    double x0, x1, z0, z1, y;
};

class yz_rect : public hittable
{
public:
    yz_rect() {}

    yz_rect(double _y0, double _y1, double _z0, double _z1, double _k,
            shared_ptr<material> mat)
        : y0(_y0), y1(_y1), z0(_z0), z1(_z1), k(_k), mp(mat){};

    virtual bool hit(const ray &r, double t_min, double t_max, hit_record &rec) const override;

    virtual bool bounding_box(double time0, double time1, aabb &output_box) const override
    {
        // The bounding box must have non-zero width in each dimension, so pad the X
        // dimension a small amount.
        output_box = aabb(point3(k - 0.0001, y0, z0), point3(k + 0.0001, y1, z1));
        return true;
    }

public:
    shared_ptr<material> mp;
    double y0, y1, z0, z1, k;
};

bool xy_rect::hit(const ray &r, double t_min, double t_max, hit_record &rec) const
{
    auto t = (z - r.origin().z()) / r.direction().z();
    if (t < t_min || t > t_max)
        return false;

    auto x = r.origin().x() + t * r.direction().x();
    auto y = r.origin().y() + t * r.direction().y();
    if (x < x0 || x > x1 || y < y0 || y > y1)
        return false;

    rec.u = (x - x0) / (x1 - x0);
    rec.v = (y - y0) / (y1 - y0);
    rec.t = t;
    auto outward_normal = vec3(0, 0, 1);
    rec.set_face_normal(r, outward_normal);
    rec.mat = mat;
    rec.p = r.at(t);

    return true;
}

bool xz_rect::hit(const ray &r, double t_min, double t_max, hit_record &rec) const
{
    auto t = (y - r.origin().y()) / r.direction().y();
    if (t < t_min || t > t_max)
        return false;
    auto x = r.origin().x() + t * r.direction().x();
    auto z = r.origin().z() + t * r.direction().z();
    if (x < x0 || x > x1 || z < z0 || z > z1)
        return false;
    rec.u = (x - x0) / (x1 - x0);
    rec.v = (z - z0) / (z1 - z0);
    rec.t = t;
    auto outward_normal = vec3(0, 1, 0);
    rec.set_face_normal(r, outward_normal);
    rec.mat = mat;
    rec.p = r.at(t);
    return true;
}

bool yz_rect::hit(const ray &r, double t_min, double t_max, hit_record &rec) const
{
    auto t = (k - r.origin().x()) / r.direction().x();
    if (t < t_min || t > t_max)
        return false;
    auto y = r.origin().y() + t * r.direction().y();
    auto z = r.origin().z() + t * r.direction().z();
    if (y < y0 || y > y1 || z < z0 || z > z1)
        return false;
    rec.u = (y - y0) / (y1 - y0);
    rec.v = (z - z0) / (z1 - z0);
    rec.t = t;
    auto outward_normal = vec3(1, 0, 0);
    rec.set_face_normal(r, outward_normal);
    rec.mat = mp;
    rec.p = r.at(t);
    return true;
}

#endif
