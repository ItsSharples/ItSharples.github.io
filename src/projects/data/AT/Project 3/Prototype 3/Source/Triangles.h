#pragma once
#include "pch.h"
#include "Vec3.h"
#include "MathsConcepts.h"
namespace MyMaths
{
    struct Triangle
    {
        Vec3f x, y, z;
        Triangle() :x(0), y(0), z(0) {};
        Triangle(Vec3f _x, Vec3f _y, Vec3f _z) :x(_x), y(_y), z(_z) {};

        Triangle operator+(Vec3f scalar) { return { this->x + scalar, this->y + scalar, this->z + scalar }; }

        bool intersect(Vec3f rayorig, Vec3f raydir, float& t0, float& t1)
        {
            // use Moller-Trumbore method
            float u, v;
            // t is distance travelled
            // u and v are x, y through the triangle, as if it was a unit triangle

            Vec3f YX = y - x;
            Vec3f ZX = z - x;
            Vec3f pvec = cross(raydir, ZX);
            float det = dot(YX, pvec);

            // ray and triangle are parallel if det is close to 0
            if (fabs(det) < F_Epsilon) return false;

            float invDet = 1 / det;

            Vec3f t_vec = rayorig - x;
            u = dot(t_vec, pvec) * invDet;
            if (u < 0 || u > 1) return false;

            Vec3f q_vec = cross(t_vec, YX);
            v = dot(raydir, q_vec) * invDet;
            if (v < 0 || u + v > 1) return false;

            t0 = t1 = dot(ZX, q_vec) * invDet;

            // Does Intersect
            return true;
        }
    };

    struct TriIndex
    {
        unsigned int x, y, z;
        TriIndex(unsigned int _x, unsigned int _y, unsigned int _z) :x(_x), y(_y), z(_z) {};
    };
}