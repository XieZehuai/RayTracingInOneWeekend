#ifndef MESH_H
#define MEHS_H

#include "hittable.h"
#include "hittable_list.h"
#include "triangle.h"
#include "bvh.h"

class mesh : public hittable
{
private:
    hittable_list triangles;
    bvh_node bvhNode;

public:
    mesh() {}
    mesh(shared_ptr<material> mat)
    {
        const float width = 1.0f / 20.0f;

        for (int y = -20; y < 20; y++)
        {
            for (int x = -20; x < 20; x++)
            {
                auto v0 = point3(x * width, y * width, -1.0f);
                auto v1 = point3((x + 1) * width, y * width, -1.0f);
                auto v2 = point3(x * width, (y + 1) * width, -1.0f);
                auto v3 = point3((x + 1) * width, (y + 1) * width, -1.0f);

                triangles.add(make_shared<triangle>(v0, v1, v2, mat));
                triangles.add(make_shared<triangle>(v1, v3, v2, mat));
            }
        }

        bvhNode = bvh_node(triangles, 0.0, 1.0);
    }

    virtual bool hit(const ray &ray, double t_min, double t_max, hit_record &rec) const override
    {
        return bvhNode.hit(ray, t_min, t_max, rec);
        // return triangles.hit(ray, t_min, t_max, rec);
    }

    virtual bool bounding_box(double time0, double time1, aabb &output_box) const override
    {
        return bvhNode.bounding_box(time0, time1, output_box);
        // return triangles.bounding_box(time0, time1, output_box);
    }
};

#endif