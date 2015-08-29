/******PROJECT:Sand Castle Engine******/
/**************************************/
/*********AUTHOR:Gwenn AUBERT**********/
/********FILE:SCEMeshLoader.hpp********/
/**************************************/
#ifndef SCE_MESH_LOADER_HPP
#define SCE_MESH_LOADER_HPP

#include "SCEDefines.hpp"
#include "SCERenderStructs.hpp"
#include <map>

namespace SCE
{
    class SCEMeshLoader
    {
    public :

        SCEMeshLoader();

        static void         Init();
        static void         CleanUp();

        static uint         CreateMeshFromFile  ( const std::string &meshFileName);

        static uint         CreateSphereMesh    ( float tesselation);

        static uint         CreateConeMesh      ( float angle, float tesselation);

        static uint         CreateQuadMesh      ();

        static uint         CreateCubeMesh      ();

        static uint         CreateCustomMesh    ( const std::vector<ushort> &indices,
                                                  const std::vector<vec3>   &vertices,
                                                  const std::vector<vec3>   &normals,
                                                  const std::vector<vec2>   &uvs,
                                                  const std::vector<vec3>   &tangents,
                                                  const std::vector<vec3>   &bitangents
                                                 );

        static void         DeleteMesh(uint meshId);

        static const MeshData& GetMeshData(uint meshId);

    private :

        static SCEMeshLoader*           s_instance;

        std::map<std::string, uint>     mMeshIds;
        std::map<uint, MeshData>        mMeshData;
        uint                            mNextId;

        uint                addMeshData(const std::string &meshName,
                                        const std::vector<ushort> &indices,
                                        const std::vector<vec3>   &vertices,
                                        const std::vector<vec3>   &normals,
                                        const std::vector<vec2>   &uvs,
                                        const std::vector<vec3>   &tangents,
                                        const std::vector<vec3>   &bitangents);
    };
}


#endif
