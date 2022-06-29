#ifndef MOVING_SPHERE_H
#define MOVING_SPHERE_H

#include "rtweekend.h"
#include "hittable.h"

class moving_sphere : public hittable
{
public:
    moving_sphere() {}

    moving_sphere(point3 center0, point3 center1, double time0, double time1, double radius, shared_ptr<material> m)
        : center0(center0), center1(center1), time0(time0), time1(time1), radius(radius), mat(m) {}

    virtual bool hit(const ray &r, double t_min, double t_max, hit_record &rec) const override;

    virtual bool bounding_box(double center0, double center1, aabb &output_box) const override;

    point3 center(double time) const;

public:
    point3 center0, center1;
    double time0, time1;
    double radius;
    shared_ptr<material> mat;
};

point3 moving_sphere::center(double time) const
{
    float t = (time - time0) / (time1 - time0);
    return center0 + (center1 - center0) * t;
}

bool moving_sphere::hit(const ray &r, double t_min, double t_max, hit_record &rec) const
{
    vec3 oc = r.origin() - center(r.time());
    auto a = r.direction().length_squared();
    auto half_b = dot(oc, r.direction());
    auto c = oc.length_squared() - radius * radius;

    auto discriminant = half_b * half_b - a * c;
    if (discriminant < 0)
        return false;
    auto sqrtd = sqrt(discriminant);

    // Find the nearest root that lies in the acceptable range.
    auto root = (-half_b - sqrtd) / a;
    if (root < t_min || t_max < root)
    {
        root = (-half_b + sqrtd) / a;
        if (root < t_min || t_max < root)
            return false;
    }

    rec.t = root;
    rec.p = r.at(rec.t);
    auto outward_normal = (rec.p - center(r.time())) / radius;
    rec.set_face_normal(r, outward_normal);
    rec.mat = mat;

    return true;
}

bool moving_sphere::bounding_box(double time0, double time1, aabb &output_box) const
{
    aabb box0(center(time0) - vec3(radius, radius, radius),
              center(time0) + vec3(radius, radius, radius));

    aabb box1(center(time1) - vec3(radius, radius, radius),
              center(time1) + vec3(radius, radius, radius));

    output_box = surrounding_box(box0, box1);
    return true;
}

#endif
