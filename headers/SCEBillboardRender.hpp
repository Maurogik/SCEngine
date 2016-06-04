/******PROJECT:Sand Castle Engine******/
/**************************************/
/*********AUTHOR:Gwenn AUBERT**********/
/*****FILE:SCEBillboardRender.hpp******/
/**************************************/
#ifndef SCE_BILLBOARDRENDER_HPP
#define SCE_BILLBOARDRENDER_HPP

#include "SCEDefines.hpp"
#include <vector>
#include <functional>

namespace SCE
{
    namespace BillboardRender
    {
        typedef std::function<void(const mat4 & modelMatrix,
                                   const mat4 & viewMatrix,
                                   const mat4 & projectionMatrix)> RenderCallback;

        glm::vec3 GenerateTexturesFromMesh(ui16 nbAngles, ui16 texWidth, float billboardBorder,
                                           const vec3 &center, const vec3 &dimensions,
                                      GLuint* diffuseTex, GLuint* normalTex,
                                      RenderCallback renderCallback);
        glm::vec4 GetMappingForAgnle(float angleRad, ui16 nbAngles, bool flipX = false);
    }
}

#endif
