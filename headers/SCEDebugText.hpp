/******PROJECT:Sand Castle Engine******/
/**************************************/
/*********AUTHOR:Gwenn AUBERT**********/
/********FILE:SCEDebugText.hpp*********/
/**************************************/
#ifndef SCE_DEBUGTEXT_HPP
#define SCE_DEBUGTEXT_HPP

#include "SCEDefines.hpp"

#ifdef SCE_DEBUG_ENGINE

namespace SCE
{
    namespace DebugText
    {
        void LogMessage(const std::string& message);
        void LogMessage(const std::string& message, const vec3 &color);
        void RenderMessages(const mat4 &viewMatrix, const mat4 &projectionMatrix);
        void SetDefaultPrintColor(const vec3 &color);
        void RenderString(glm::vec2 const& position, std::string const& text);
        void RenderString(glm::vec2 const& position, std::string const& text, const vec3 &color);
    }
}

#endif

#endif
