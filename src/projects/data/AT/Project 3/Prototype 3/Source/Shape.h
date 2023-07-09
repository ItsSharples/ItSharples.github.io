#pragma once
#include "pch.h"

#include "Vec3.h"
#include "Triangles.h"
#include "OBJ.h"
#include "Bounds.h"
namespace MyMaths
{


    class Shape
    {
    public:
        Vec3f position;                           /// position of the sphere 
        Vec3f surfaceColour, emissionColour;      /// surface color and emission (light) 
        float transparency, reflection;         /// surface transparency and reflectivity
        Bounds localBounds, worldBounds; bool dirty;
        Shape(
            const Vec3f& position,
            const Vec3f& surfaceColour,
            const float& reflectivity = 0,
            const float& transparancy = 0,
            const Vec3f& emissionColour = 0) :
            position(position), surfaceColour(surfaceColour), emissionColour(emissionColour),
            transparency(transparancy), reflection(reflectivity), dirty(false)
        {
            updateBounds();
        }
        virtual bool intersect(const Vec3f& rayorig, const Vec3f& raydir, float& t0, float& t1) const { printf("This Shouldn't happen!"); return false; };
        void updateBounds(){worldBounds = localBounds + position; }
        
        const Bounds getBoundsWorldSpace() const { return worldBounds; }
        const Bounds getBoundsLocalSpace() const { return localBounds; }
    };

    class Sphere : public Shape
    {
    public:
        float radius, radius2;                  /// sphere radius and radius^2 
        Sphere(
            const Vec3f& position,
            const float& radius,
            const Vec3f& surfaceColour,
            const float& reflectivity = 0,
            const float& transparancy = 0,
            const Vec3f& emissionColour = 0) : Shape(position, surfaceColour, reflectivity, transparancy, emissionColour)
            , radius(radius), radius2(radius* radius) 
        {
            localBounds = Bounds(Vec3f(-radius), Vec3f(radius)); updateBounds();
        }

        bool intersect(const Vec3f& rayorig, const Vec3f& raydir, float& t0, float& t1) const
        {
            Vec3f l = position - rayorig;
            float tca = l.dot(raydir);
            if (tca < 0) return false;
            float d2 = l.dot(l) - tca * tca;
            if (d2 > radius2) return false;
            float thc = sqrt(radius2 - d2);
            t0 = tca - thc;
            t1 = tca + thc;

            return true;
        }
    };

    class Mesh : public Shape
    {
    public:
        std::vector<Triangle> triangles;

        Mesh(
            const Vec3f& position,
            const Vec3f& surfaceColour,
            const float& reflectivity = 0.f,
            const float& transparancy = 0.f,
            const Vec3f& emissionColour = 0) : Shape(position, surfaceColour, reflectivity, transparancy, emissionColour)
        {}


        void addOBJ(const OBJ& obj)
        {
            addTriangleList(toTriangleList(obj));
            updateBounds();
        }
        static std::vector<Triangle> toTriangleList(const OBJ& obj)
        {
            std::vector<Triangle> out;

            for (size_t index = 0; index < obj.indices.size(); index += 3)
            {
                auto tri = Triangle();
                // Get Indices
                tri.x = obj.indices[(size_t)(index + 0)];
                tri.y = obj.indices[(size_t)(index + 1)];
                tri.z = obj.indices[(size_t)(index + 2)];

                // Convert to Vertices
                tri.x = obj.vertices[(size_t)tri.x.x];
                tri.y = obj.vertices[(size_t)tri.y.x];
                tri.z = obj.vertices[(size_t)tri.z.x];

                out.emplace_back(tri);
            }

            return out;
        }

        void addTriangleList(std::vector<Triangle> triangleList)
        {
            for (auto& triangle : triangleList)
            {
                triangles.emplace_back(triangle);
                // Expand the Bounds to include this triangle
                this->localBounds.Union(triangle);
            }
        };

        bool intersect(const Vec3f& rayorig, const Vec3f& raydir, float& t0, float& t1) const
        {
            //if(worldBounds.intersect(rayorig, raydir, t0, t1));
            {
                for (auto triangle : triangles)
                {
                    auto worldTriangle = triangle + position;
                    if (worldTriangle.intersect(rayorig, raydir, t0, t1))
                    {
                        return true;
                    }
                }
            }
            return false;
        }


    };

}