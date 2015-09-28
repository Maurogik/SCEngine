/******PROJECT:Sand Castle Engine******/
/**************************************/
/*********AUTHOR:Gwenn AUBERT**********/
/********FILE:SCEDebugText.hpp*********/
/**************************************/
#ifndef SCE_DEBUGTEXT_HPP
#define SCE_DEBUGTEXT_HPP

#include "SCEDefines.hpp"

namespace SCE
{

namespace DebugText
{
    void Print(const std::string& message);
    void Print(const std::string& message, const vec3 &color);
    void RenderMessages(const mat4 &viewMatrix, const mat4 &projectionMatrix);
}

}

#endif
