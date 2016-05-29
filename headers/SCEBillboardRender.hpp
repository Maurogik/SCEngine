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

        void GenerateTexturesFromMesh(ui16 nbAngles, ui16 texWidth, const vec3 &center, const vec3 &dimensions,
                                      GLuint* diffuseTex, GLuint* normalTex,
                                      RenderCallback renderCallback);
        ui16 GetIdFromAngle(float angleInRad, ui16 nbAngles);
    }
}

#endif
