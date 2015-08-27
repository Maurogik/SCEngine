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

    class SCERender
    {
    public :

        SCERender();

        static void         Init();
        static void         CleanUp();
        static void         Render(const SCEHandle<Camera> &camera,
                                   std::vector<Container*> objectsToRender);
        static void         ResetClearColorToDefault();
        static glm::mat4    FixOpenGLProjectionMatrix(const glm::mat4& projMat);

    private :

        static SCERender*   s_instance;

        SCE_GBuffer         mGBuffer;
        glm::vec4           mDefaultClearColor;

        void                renderGeometryPass(const CameraRenderData& renderData,
                                               std::vector<Container*> objectsToRender);

    };
}

#endif
