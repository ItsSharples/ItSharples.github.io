#pragma once
#include "Vec3.h"
#include "Triangles.h"
namespace MyMaths
{
    struct OBJ
    {
        std::vector<Vec3f> vertices;
        std::vector<uint32_t> indices;
    };

    struct TriOBJ
    {
        std::vector<Triangle> vertices;
        std::vector<TriIndex> indices;
    };
}