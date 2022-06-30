#ifndef AABB_H
#define AABB_H

#include "rtweekend.h"

/**
 * @brief 轴对齐包围盒
 */
class aabb
{
public:
    aabb() {}

    /**
     * @brief 生成一个新的包围盒
     *
     * @param a 包围盒最小端
     * @param b 包围盒最大端
     */
    aabb(const point3 &a, const point3 &b) : minimum(a), maximum(b)
    {
    }

    point3 min() const { return minimum; }
    point3 max() const { return maximum; }

    /**
     * @brief 判断射线在指定时间范围内是否与包围盒相交
     *
     * @param r 射线
     * @param t_min 时间范围下限
     * @param t_max 时间范围上限
     */
    inline bool hit(const ray &r, double t_min, double t_max) const
    {
        for (int a = 0; a < 3; a++)
        {
            auto invD = 1.0f / r.direction()[a];
            auto t0 = (min()[a] - r.origin()[a]) * invD;
            auto t1 = (max()[a] - r.origin()[a]) * invD;

            if (invD < 0.0f)
                std::swap(t0, t1);

            t_min = t0 > t_min ? t0 : t_min;
            t_max = t1 < t_max ? t1 : t_max;

            if (t_max <= t_min)
                return false;
        }

        return true;
    }

public:
    point3 minimum;
    point3 maximum;
};

/**
 * @brief 将两个包围盒合并成一个更大的包围盒
 */
aabb surrounding_box(aabb box0, aabb box1)
{
    point3 small(fmin(box0.min().x(), box1.min().x()),
                 fmin(box0.min().y(), box1.min().y()),
                 fmin(box0.min().z(), box1.min().z()));

    point3 big(fmax(box0.max().x(), box1.max().x()),
               fmax(box0.max().y(), box1.max().y()),
               fmax(box0.max().z(), box1.max().z()));

    return aabb(small, big);
}

#endif
