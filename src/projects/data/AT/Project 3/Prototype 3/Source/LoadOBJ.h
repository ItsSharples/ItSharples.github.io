#pragma once
#include "pch.h"


#include "Vec3.h"
#include "OBJ.h"
#include "Triangles.h"

namespace
{
    using Vertex = MyMaths::Vec3f;
    using Index = uint32_t;
    using OBJ = MyMaths::OBJ;
    using TriOBJ = MyMaths::TriOBJ;


}

//
//static bool loadOBJ(_In_ std::string filename, _Out_ std::vector<Vertex>& vertices, _Out_ std::vector<Index>& indices)
//{
//    auto search = OBJ_map.find(filename);
//    if(search != OBJ_map.end())
//    {
//        //Logging::TRACE("Already loaded OBJ: " + filename + ".obj");
//        vertices = search->second.vertices;
//        indices = search->second.indices;
//    }
//    else
//    {
//        //Logging::TRACE("Loading OBJ: " + filename + ".obj");
//        loadNewOBJ(filename, vertices, indices);
//
//        OBJ_map[filename] = { vertices, indices };
//    }
//
//    return true;
//}
namespace DXL
{
    OBJ loadOBJ(std::string filename);
    TriOBJ loadTriOBJ(std::string filename);
}