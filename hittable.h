#ifndef HITTABLE_H
#define HITTABLE_H

#include "rtweekend.h"
#include "ray.h"
#include "aabb.h"

class material;

/**
 * @brief 物体表面与射线相交点的属性
 */
struct hit_record
{
    point3 p;                 // 坐标
    vec3 normal;              // 法线
    double t;                 // 时间；射线从发射点出发，沿着指定方向经过 t 时间到达物体表面
    double u;                 // 纹理坐标
    double v;                 // 纹理坐标
    bool front_face;          // 是正面还是背面
    shared_ptr<material> mat; // 材质

    /**
     * @brief 一个点可以有两条法线，一条垂直于物体正面，一条垂直于物体背面；
     * 当射线击中物体正面时，法线方向与物体表面法线方向相同；当射线击中物体
     * 背面时，法线法线与物体表面法线方向相反
     *
     * @param r 射线
     * @param outward_normal 物体表面法线
     */
    inline void set_face_normal(const ray &r, const vec3 &outward_normal)
    {
        front_face = dot(r.direction(), outward_normal) < 0;
        normal = front_face ? outward_normal : -outward_normal;
    }
};

/**
 * @brief 所有可与射线交互物体的基类
 */
class hittable
{
public:
    /**
     * @brief 判断在指定时间范围内射线是否与物体相交
     *
     * @param r 射线
     * @param t_min 时间范围下限（包括该值）
     * @param t_max 时间范围上限（包括该值）
     * @param rec 相交时记录相关信息
     */
    virtual bool hit(const ray &r, double t_min, double t_max, hit_record &rec) const = 0;

    /**
     * @brief 为物体生成包围盒
     *
     * @param time0 快门时间下限
     * @param time1 快门时间上限
     * @param output_box 生成成功时用于保存包围盒
     * @return true 生成成功
     * @return false 生成失败
     */
    virtual bool bounding_box(double time0, double time1, aabb &output_box) const = 0;
};

/**
 * @brief 表示一个经过位移的物体
 */
class translate : public hittable
{
private:
    shared_ptr<hittable> object;
    vec3 offset; // 位移的距离

public:
    translate(shared_ptr<hittable> object, const vec3 &displacement)
        : object(object), offset(displacement)
    {
    }

    virtual bool hit(const ray &r, double t_min, double t_max, hit_record &rec) const override;

    virtual bool bounding_box(double time0, double time1, aabb &output_box) const override;
};

bool translate::hit(const ray &r, double t_min, double t_max, hit_record &rec) const
{
    // 物体位移 offset 的距离，相当于射线向反方向位移 offset 距离
    ray moved_r(r.origin() - offset, r.direction(), r.time());

    if (!object->hit(moved_r, t_min, t_max, rec))
    {
        return false;
    }

    rec.p += offset;
    rec.set_face_normal(moved_r, rec.normal);

    return true;
}

bool translate::bounding_box(double time0, double time1, aabb &output_box) const
{
    if (!object->bounding_box(time0, time1, output_box))
    {
        return false;
    }

    output_box = aabb(output_box.min() + offset, output_box.max() + offset);

    return true;
}

class rotate_y : public hittable
{
public:
    rotate_y(shared_ptr<hittable> object, double angle)
        : object(object)
    {
        auto radians = degrees_to_radians(angle);
        sin_theta = sin(radians);
        cos_theta = cos(radians);

        hasBox = object->bounding_box(0, 1, bounds);

        point3 min(infinity, infinity, infinity);
        point3 max(-infinity, -infinity, -infinity);

        for (int i = 0; i < 2; i++)
        {
            for (int j = 0; j < 2; j++)
            {
                for (int k = 0; k < 2; k++)
                {
                    auto x = i * bounds.max().x() + (1 - i) * bounds.min().x();
                    auto y = j * bounds.max().y() + (1 - j) * bounds.min().y();
                    auto z = k * bounds.max().z() + (1 - k) * bounds.min().z();

                    auto newx = cos_theta * x + sin_theta * z;
                    auto newz = -sin_theta * x + cos_theta * z;

                    vec3 tester(newx, y, newz);

                    for (int c = 0; c < 3; c++)
                    {
                        min[c] = fmin(min[c], tester[c]);
                        max[c] = fmax(max[c], tester[c]);
                    }
                }
            }
        }

        bounds = aabb(min, max);
    }

    virtual bool hit(const ray &r, double t_min, double t_max, hit_record &rec) const override
    {
        auto origin = r.origin();
        auto direction = r.direction();

        origin[0] = cos_theta * r.origin()[0] - sin_theta * r.origin()[2];
        origin[2] = sin_theta * r.origin()[0] + cos_theta * r.origin()[2];

        direction[0] = cos_theta * r.direction()[0] - sin_theta * r.direction()[2];
        direction[2] = sin_theta * r.direction()[0] + cos_theta * r.direction()[2];

        ray rotated_r(origin, direction, r.time());

        if (!object->hit(rotated_r, t_min, t_max, rec))
            return false;

        auto p = rec.p;
        auto normal = rec.normal;

        p[0] = cos_theta * rec.p[0] + sin_theta * rec.p[2];
        p[2] = -sin_theta * rec.p[0] + cos_theta * rec.p[2];

        normal[0] = cos_theta * rec.normal[0] + sin_theta * rec.normal[2];
        normal[2] = -sin_theta * rec.normal[0] + cos_theta * rec.normal[2];

        rec.p = p;
        rec.set_face_normal(rotated_r, normal);

        return true;
    }

    virtual bool bounding_box(double time0, double time1, aabb &output_box) const override
    {
        output_box = bounds;
        return hasBox;
    }

private:
    shared_ptr<hittable> object;
    double sin_theta;
    double cos_theta;
    bool hasBox;
    aabb bounds;
};

class flip_face : public hittable
{
private:
    shared_ptr<hittable> object;

public:
    flip_face(shared_ptr<hittable> object) : object(object)
    {
    }

    virtual bool hit(const ray &r, double t_min, double t_max, hit_record &rec) const override
    {
        if (!object->hit(r, t_min, t_max, rec))
        {
            return false;
        }

        rec.front_face = !rec.front_face;
        return true;
    }

    virtual bool bounding_box(double time0, double time1, aabb &output_box) const override
    {
        return object->bounding_box(time0, time1, output_box);
    }
};

#endif
