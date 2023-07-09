#include "pch.h"
#include "LoadOBJ.h"

#undef ERROR

#define TINYOBJLOADER_IMPLEMENTATION // define this in only *one* .cc
#include "tiny_obj_loader.h"
namespace
{
    static bool loadNewOBJ(std::string filename, std::vector<Vertex>& vertices, std::vector<Index>& indices)
    {
        std::string inputfile = "Resources/Objects/" + filename + ".obj";
        std::string basedir = "Resources/Objects";
        tinyobj::attrib_t attrib;
        std::vector<tinyobj::shape_t> shapes;
        std::vector<tinyobj::material_t> materials;

        std::string warn;
        std::string err;

        //Logging::INFO("File location: " + inputfile);

        bool ret = tinyobj::LoadObj(&attrib, &shapes, &materials, &warn, &err, inputfile.c_str(), basedir.c_str());

        if (!warn.empty())
        {
            //Logging::WARN(warn);
        }

        if (!err.empty())
        {
            //Logging::ERROR(err);
            return false;
        }

        if (!ret)
        {
            throw std::exception();
        }

        Vertex tmp_vertex;
        //tmp_vertex.colour = { 1, 1, 1, 1 };

        std::vector<Vertex> int_vertices;
        std::vector<Index> int_indices;

        constexpr auto texcoords_per_vertex = 2;
        //DirectX::XMVECTOR colour;
        // Loop over shapes
        for (const auto& shape : shapes)
        {
            // Loop over faces(polygon)
            size_t index_offset = 0;
            for (size_t f = 0; f < shape.mesh.num_face_vertices.size(); f++)
            {
                constexpr int vertices_per_face = 3;// Because Triangulate is true, else use shape.mesh.num_face_vertices[face]; (also remove constexpr)

                // If there's no materials
                //if(!shape.mesh.material_ids.empty())
                //{
                //    auto material = shape.mesh.material_ids[face];
                //    auto tmp_colour = materials[material].diffuse;
                //    Logging::DEBUG(std::to_string(material));
                //    colour = DirectX::XMVectorSet(tmp_colour[0], tmp_colour[1], tmp_colour[2], 1);
                //}
                //else
                //{
                //colour = DirectX::XMVectorReplicate(1);
                //}


                //auto mat = shape.mesh.material_ids[f];
                //auto colour = materials[mat].diffuse;

                // Loop over vertices in the face.
                for (size_t v = 0; v < vertices_per_face; v++)
                {
                    const auto& index = shape.mesh.indices[index_offset + v];

                    Vertex vertex{};

                    vertex = Vertex(
                        attrib.vertices[3 * index.vertex_index + 0],
                        attrib.vertices[3 * index.vertex_index + 1],
                        attrib.vertices[3 * index.vertex_index + 2]);

                    struct TexCoord
                    {
                        float u, v;
                    };

                    TexCoord texcoord = {
                        attrib.texcoords[2 * index.texcoord_index + 0],
                        attrib.texcoords[2 * index.texcoord_index + 1] };

                    int_vertices.push_back(vertex);
                    int_indices.push_back((Index)int_indices.size());
                }
                index_offset += vertices_per_face;
            };
        }

        vertices = int_vertices;
        indices = int_indices;

        return true;
    }
    std::unordered_map <std::string, OBJ> OBJ_map;
}
OBJ DXL::loadOBJ(std::string filename)
{
    OBJ output = OBJ();
    auto search = OBJ_map.find(filename);
    if (search != OBJ_map.end())
    {
        //Logging::TRACE("Already loaded OBJ: " + filename + ".obj");
        output.vertices = search->second.vertices;
        output.indices = search->second.indices;
    }
    else
    {
        //Logging::TRACE("Loading OBJ: " + filename + ".obj");
        loadNewOBJ(filename, output.vertices, output.indices);

        OBJ_map[filename] = { output.vertices, output.indices };
    }

    return output;
}
TriOBJ DXL::loadTriOBJ(std::string filename)
{
    TriOBJ output{};
    auto obj = loadOBJ(filename);

    for (size_t index = 0; index < obj.indices.size(); index += 3)
    {
        output.indices.emplace_back(MyMaths::TriIndex(index, index + 1, index + 2));
        output.vertices.emplace_back(MyMaths::Triangle(obj.vertices[index], obj.vertices[index + 1], obj.vertices[index + 2]));
    }

    return output;
}