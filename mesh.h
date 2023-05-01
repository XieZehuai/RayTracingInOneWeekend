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
        const int width = 1;
        const float size = 1.0f / width;
        const float uv_stride = 0.5f / width;

        for (int y = -width; y < width; y++)
        {
            for (int x = -width; x < width; x++)
            {
                auto p0 = point3(x * size, y * size, -1.0f);
                auto p1 = point3((x + 1) * size, y * size, -1.0f);
                auto p2 = point3(x * size, (y + 1) * size, -1.0f);
                auto p3 = point3((x + 1) * size, (y + 1) * size, -1.0f);

                auto uv0 = vec3((x + width) * uv_stride, (y + width) * uv_stride, 0);
                auto uv1 = vec3((x + 1 + width) * uv_stride, (y + width) * uv_stride, 0);
                auto uv2 = vec3((x + width) * uv_stride, (y + 1 + width) * uv_stride, 0);
                auto uv3 = vec3((x + 1 + width) * uv_stride, (y + 1 + width) * uv_stride, 0);

                vertex v0 = {p0, uv0};
                vertex v1 = {p1, uv1};
                vertex v2 = {p2, uv2};
                vertex v3 = {p3, uv3};

                triangles.add(make_shared<triangle>(v0, v1, v2, mat));
                triangles.add(make_shared<triangle>(v1, v3, v2, mat));
            }
        }

        bvhNode = bvh_node(triangles, 0.0, 1.0);
    }

    virtual bool hit(const ray &ray, double t_min, double t_max, hit_record &rec) const override
    {
        return bvhNode.hit(ray, t_min, t_max, rec);
    }

    virtual bool bounding_box(double time0, double time1, aabb &output_box) const override
    {
        return bvhNode.bounding_box(time0, time1, output_box);
    }
};

#endif