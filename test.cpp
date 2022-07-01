#include <iostream>
#include <math.h>
#include <iomanip>

#include "rtweekend.h"

inline vec3 random_cosine_direction()
{
    auto r1 = random_double();
    auto r2 = random_double();
    auto z = sqrt(1 - r2);

    auto phi = 2 * pi * r1;
    auto x = cos(phi) * sqrt(r2);
    auto y = sin(phi) * sqrt(r2);

    return vec3(x, y, z);
}

int main()
{
    int N = 10;
    for (int i = 0; i < N; i++)
    {
        auto v = random_cosine_direction();
        std::cout << v.x() << ' ' << v.y() << ' ' << v.z() << ' ' << v.length() << '\n';
    }

    return 0;
}
