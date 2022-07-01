#ifndef ONB_H
#define ONB_H

#include "rtweekend.h"

/**
 * @brief Orthonormal Bases，标准正交基，表示空间中的一个坐标系
 */
class onb
{
public:
    onb() {}

    vec3 u() const { return axis[0]; }
    vec3 v() const { return axis[1]; }
    vec3 w() const { return axis[2]; }
    vec3 operator[](int n) const { return axis[n]; }

    vec3 local(double x, double y, double z) const
    {
        return x * u() + y * v() + z * w();
    }

    vec3 local(const vec3 &a) const
    {
        return a.x() * u() + a.y() * v() + a.z() * w();
    }

    void build_from_w(const vec3 &);

private:
    vec3 axis[3];
};

void onb::build_from_w(const vec3 &n)
{
    axis[2] = unit_vector(n);

    vec3 a = (fabs(w().x()) > 0.9 ? vec3(0, 1, 0) : vec3(1, 0, 0));
    axis[1] = unit_vector(cross(w(), a));
    axis[0] = cross(w(), v());
}

#endif
