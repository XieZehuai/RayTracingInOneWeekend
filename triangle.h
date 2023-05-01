#ifndef TRIANGLE_H
#define TRIANGLE_H

#include "hittable.h"
#include "vec3.h"

struct vertex
{
    point3 position;
    vec3 uv; // 没有 vec2 类，用 vec3 代替
};

class triangle : public hittable
{
public:
    vertex v0, v1, v2;
    shared_ptr<material> mat;

private:
    vec3 e1, e2;
    vec3 uv1, uv2;
    vec3 normal;

public:
    triangle(vertex v0, vertex v1, vertex v2, shared_ptr<material> mat)
        : v0(v0), v1(v1), v2(v2), mat(mat)
    {
        e1 = v1.position - v0.position;
        e2 = v2.position - v0.position;
        normal = unit_vector(cross(e1, e2));

        uv1 = v1.uv - v0.uv;
        uv2 = v2.uv - v0.uv;
    }

    virtual bool hit(const ray &ray, double t_min, double t_max, hit_record &rec) const override;

    virtual bool bounding_box(double time0, double time1, aabb &output_box) const override;

    virtual double pdf_value(const point3 &origin, const vec3 &direction) const override;

    virtual vec3 random(const vec3 &origin) const override;
};

bool triangle::hit(const ray &ray, double t_min, double t_max, hit_record &rec) const
{
    auto s = ray.origin() - v0.position;
    auto s1 = cross(ray.direction(), e2);
    auto s2 = cross(s, e1);

    auto invS1dE1 = 1.0f / dot(s1, e1);

    float t = dot(s2, e2) * invS1dE1;
    if (t < t_min || t > t_max)
        return false;

    float u = dot(s1, s) * invS1dE1;
    if (u < 0.0f || u > 1.0f)
        return false;

    float v = dot(s2, ray.direction()) * invS1dE1;
    if (v < 0.0f || u + v > 1.0f)
        return false;

    auto uv = uv1 * u + uv2 * v + v0.uv;

    rec.t = t;
    rec.u = uv.x();
    rec.v = uv.y();
    rec.set_face_normal(ray, normal);
    rec.mat = mat;
    rec.p = ray.at(t);

    return true;
}

bool triangle::bounding_box(double time0, double time1, aabb &output_box) const
{
    float x_min = std::min(v0.position.x(), std::min(v1.position.x(), v2.position.x()));
    float x_max = std::max(v0.position.x(), std::max(v1.position.x(), v2.position.x()));
    float y_min = std::min(v0.position.y(), std::min(v1.position.y(), v2.position.y()));
    float y_max = std::max(v0.position.y(), std::max(v1.position.y(), v2.position.y()));
    float z_min = std::min(v0.position.z(), std::min(v1.position.z(), v2.position.z()));
    float z_max = std::max(v0.position.z(), std::max(v1.position.z(), v2.position.z()));

    if (x_max - x_min < 0.0001f)
    {
        x_min -= 0.0001f;
        x_max += 0.0001f;
    }
    if (y_max - y_min < 0.0001f)
    {
        y_min -= 0.0001f;
        y_max += 0.0001f;
    }
    if (z_max - z_min < 0.0001f)
    {
        z_min -= 0.0001f;
        z_max += 0.0001f;
    }

    output_box = aabb(point3(x_min, y_min, z_min), point3(x_max, y_max, z_max));
    return true;
}

double triangle::pdf_value(const point3 &origin, const vec3 &direction) const
{
    hit_record rec;
    if (!hit(ray(origin, direction), 0.001, infinity, rec))
    {
        return 0;
    }

    auto area = 0.5f * cross(e1, e2).length();
    auto distance_squared = rec.t * rec.t * direction.length_squared();
    auto cosine = fabs(dot(direction, rec.normal) / direction.length());

    return distance_squared / (cosine * area);
}

vec3 triangle::random(const vec3 &origin) const
{
    float random_u = random_double(0.0f, 1.0f);
    float random_v = random_double(0.0f, 1.0f - random_u);
    return e1 * random_u + e2 * random_v;
}

#endif