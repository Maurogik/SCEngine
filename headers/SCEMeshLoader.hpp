/******PROJECT:Sand Castle Engine******/
/**************************************/
/*********AUTHOR:Gwenn AUBERT**********/
/********FILE:SCEMeshLoader.hpp********/
/**************************************/
#ifndef SCE_MESH_LOADER_HPP
#define SCE_MESH_LOADER_HPP

#include "SCEDefines.hpp"
#include <map>

namespace SCE
{    
    struct MeshData
    {
        MeshData() : indices(), vertices(), normals(), uvs(), tangents(0), bitangents(0) {}
        std::vector<ushort>     indices;
        std::vector<vec3>       vertices;
        std::vector<vec3>       normals;
        std::vector<vec2>       uvs;
        std::vector<vec3>       tangents;
        std::vector<vec3>       bitangents;
    };

    namespace MeshLoader
    {

        void         Init();
        void         CleanUp();

        ui16         CreateMeshFromFile  (const std::string &meshFileName);

        ui16         CreateSphereMesh    ( float tesselation);

        ui16         CreateConeMesh      ( float angle, float tesselation);

        ui16         CreateQuadMesh      ();

        ui16         CreateCubeMesh      ();

        ui16         CreateCustomMesh    ( const std::vector<ushort> &indices,
                                                  const std::vector<vec3>   &vertices,
                                                  const std::vector<vec3>   &normals,
                                                  const std::vector<vec2>   &uvs,
                                                  const std::vector<vec3>   &tangents,
                                                  const std::vector<vec3>   &bitangents
                                                 );

        void                DeleteMesh(ui16 meshId);
        const MeshData&     GetMeshData(ui16 meshId);
    }
}


#endif
