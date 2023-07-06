#ifndef MESH_H
#define MEHS_H

#include "hittable.h"
#include "hittable_list.h"
#include "triangle.h"
#include "bvh.h"

#include <iostream>
#include <fstream>
#include <sstream>
#include <string>

std::vector<shared_ptr<triangle>> load_model_from_obj_file(const char *filename, shared_ptr<material> mat, float scale = 1.0f)
{
    std::ifstream file;
    file.open(filename, std::ios::in);

    if (!file || !file.good())
    {
        std::cout << filename << std::endl;
        return {};
    }

    std::vector<point3> positions;
    std::vector<vec3> uvs;
    std::vector<vec3> normals;
    std::vector<uint32_t> indices;

    // 读取顶点，UV，三角形索引信息
    std::string line;
    while (std::getline(file, line))
    {
        std::stringstream ss(line);

        std::string type;
        ss >> type;

        if (strcmp(type.c_str(), "v") == 0)
        {
            float x, y, z;
            ss >> x >> y >> z;
            positions.push_back(point3(x * scale, y * scale, z * scale));
        }
        else if (strcmp(type.c_str(), "vt") == 0)
        {
            float u, v;
            ss >> u >> v;
            uvs.push_back(vec3(u, v, 0));
        }
        else if (strcmp(type.c_str(), "f") == 0)
        {
            int i0, i1, i2;
            ss >> i0 >> i1 >> i2;
            indices.push_back(i0 - 1);
            indices.push_back(i1 - 1);
            indices.push_back(i2 - 1);
        }
    }

    // 计算平滑法线
    for (int i = 0; i < positions.size(); i++)
    {
        normals.push_back(vec3(0.0f));
    }
    for (int i = 0; i < indices.size(); i += 3)
    {
        auto i0 = indices[i], i1 = indices[i + 1], i2 = indices[i + 2];

        auto p0 = positions[i0];
        auto p1 = positions[i1];
        auto p2 = positions[i2];

        auto normal = cross(p1 - p0, p2 - p0);
        normals[i0] += normal;
        normals[i1] += normal;
        normals[i2] += normal;
    }
    for (int i = 0; i < normals.size(); i++)
    {
        normals[i] = unit_vector(normals[i]);
    }

    // 构建三角形
    auto defaultUV = vec3(0);
    std::vector<shared_ptr<triangle>> triangles;

    for (int i = 0; i < indices.size(); i += 3)
    {
        auto i0 = indices[i], i1 = indices[i + 1], i2 = indices[i + 2];

        auto p0 = positions[i0];
        auto p1 = positions[i1];
        auto p2 = positions[i2];

        auto n0 = normals[i0];
        auto n1 = normals[i1];
        auto n2 = normals[i2];

        auto uv0 = i0 >= uvs.size() ? defaultUV : uvs[i0];
        auto uv1 = i1 >= uvs.size() ? defaultUV : uvs[i1];
        auto uv2 = i2 >= uvs.size() ? defaultUV : uvs[i2];

        auto v0 = vertex(p0, n0, uv0);
        auto v1 = vertex(p1, n1, uv1);
        auto v2 = vertex(p2, n2, uv2);

        triangles.push_back(make_shared<triangle>(v0, v1, v2, mat));
    }

    return triangles;
}

class mesh : public hittable
{
private:
    hittable_list triangles;
    bvh_node bvhNode;

public:
    mesh() {}

    // mesh(shared_ptr<material> mat)
    // {
    //     const int width = 1;
    //     const float size = 1.0f / width;
    //     const float uv_stride = 0.5f / width;

    //     for (int y = -width; y < width; y++)
    //     {
    //         for (int x = -width; x < width; x++)
    //         {
    //             auto p0 = point3(x * size, y * size, -1.0f);
    //             auto p1 = point3((x + 1) * size, y * size, -1.0f);
    //             auto p2 = point3(x * size, (y + 1) * size, -1.0f);
    //             auto p3 = point3((x + 1) * size, (y + 1) * size, -1.0f);

    //             auto uv0 = vec3((x + width) * uv_stride, (y + width) * uv_stride, 0);
    //             auto uv1 = vec3((x + 1 + width) * uv_stride, (y + width) * uv_stride, 0);
    //             auto uv2 = vec3((x + width) * uv_stride, (y + 1 + width) * uv_stride, 0);
    //             auto uv3 = vec3((x + 1 + width) * uv_stride, (y + 1 + width) * uv_stride, 0);

    //             vertex v0 = {p0, uv0};
    //             vertex v1 = {p1, uv1};
    //             vertex v2 = {p2, uv2};
    //             vertex v3 = {p3, uv3};

    //             triangles.add(make_shared<triangle>(v0, v1, v2, mat));
    //             triangles.add(make_shared<triangle>(v1, v3, v2, mat));
    //         }
    //     }

    //     bvhNode = bvh_node(triangles, 0.0, 1.0);
    // }

    mesh(const char *obj_filename, shared_ptr<material> mat, float scale = 1.0f)
    {
        auto tris = load_model_from_obj_file(obj_filename, mat, scale);

        for (int i = 0; i < tris.size(); i++)
        {
            triangles.add(tris[i]);
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
