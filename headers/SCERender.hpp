/******PROJECT:Sand Castle Engine******/
/**************************************/
/*********AUTHOR:Gwenn AUBERT**********/
/**********FILE:SCERender.hpp**********/
/**************************************/

#ifndef SCE_SCERENDER_HPP
#define SCE_SCERENDER_HPP

#include "SCE_GBuffer.hpp"
#include "SCEHandle.hpp"
#include "SCERenderStructs.hpp"
#include <vector>

namespace SCE
{
    class Camera;
    class Container;

    namespace Render
    {       

        void         Init();
        void         CleanUp();
        void         Render(const SCEHandle<Camera> &camera,
                                   std::vector<Container*> objectsToRender);
        void         ResetClearColorToDefault();
        glm::mat4    FixOpenGLProjectionMatrix(const glm::mat4& projMat);
        void         RenderFullScreenPass(GLuint shaderId,
                                                 const glm::mat4 &projectionMatrix,
                                                 const glm::mat4 &viewMatrix);
        void BindGBufferTexture(SCE_GBuffer::GBUFFER_TEXTURE_TYPE type, GLuint texUnit, GLuint uniform);

#ifdef SCE_DEBUG_ENGINE
        bool         ToggleTonemapOff();
#endif
    }
}

#endif
