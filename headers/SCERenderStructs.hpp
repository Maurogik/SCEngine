/******PROJECT:Sand Castle Engine******/
/**************************************/
/*********AUTHOR:Gwenn AUBERT**********/
/******FILE:SCERenderStructs.hpp*******/
/**************************************/
#ifndef SCE_RENDER_STRUCTS_HPP
#define SCE_RENDER_STRUCTS_HPP

#include "SCEDefines.hpp"
#include <vector>

namespace SCE
{
    struct CameraRenderData
    {
        CameraRenderData() : viewMatrix(1.0), projectionMatrix(1.0) {}
        glm::mat4 viewMatrix;
        glm::mat4 projectionMatrix;
    };

    enum
    {
        PERSPECTIVE = 0,
        ORTHOGRAPHIC
    } typedef ProjectionType;

    struct FrustrumData
    {
        //Default values
        FrustrumData()
            : type(PERSPECTIVE),
              near(0.0f),
              far(0.0f),
              right(0.0f),
              left(0.0f),
              bottom(0.0f),
              top(0.0f),
              fov(0.0f),
              aspectRatio(0.0f) {}

        ProjectionType                        type;
        float                       near;
        float                       far;
        float                       right;
        float                       left;
        float                       bottom;
        float                       top;
        float                       fov;
        float                       aspectRatio;
    };

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

}

#endif
