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

#endif
