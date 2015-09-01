/******PROJECT:Sand Castle Engine******/
/**************************************/
/*********AUTHOR:Gwenn AUBERT**********/
/*******FILE:SCEAssimpLoader.hpp*******/
/**************************************/
#ifndef SCE_ASSIMPLOADER_HPP
#define SCE_ASSIMPLOADER_HPP


#include "SCEDefines.hpp"
#include "SCERenderStructs.hpp"
#include <vector>

namespace SCE
{

    namespace AssimpLoader
    {
        void LoadModel(const std::string& path,
                       std::vector<ushort>&   out_indices,
                       std::vector<vec3>&     out_vertices,
                       std::vector<vec3>&     out_normals,
                       std::vector<vec2>&     out_uvs,
                       std::vector<vec3>&     out_tangents,
                       std::vector<vec3>&     out_bitangents,
                       bool winCW = false);
    }

}

#endif
