/******PROJECT:Sand Castle Engine******/
/**************************************/
/*********AUTHOR:Gwenn AUBERT**********/
/**********FILE:SCERender.hpp**********/
/**************************************/

#ifndef SCE_SCERENDER_HPP
#define SCE_SCERENDER_HPP

#include "SCE_GBuffer.hpp"
#include "SCEHandle.hpp"
#include "Container.hpp"
#include "Light.hpp"
#include <vector>

namespace SCE
{
    class SCERender
    {
    public :

        SCERender();

        static void         Init();
        static void         CleanUp();
        static void         Render(const SCEHandle<Camera> &camera,
                                   std::vector<Container*>* objectsToRender);
        static void         ResetClearColorToDefault();

    private :

        static SCERender*   s_instance;

        SCE_GBuffer         mGBuffer;
        glm::vec4           mDefaultClearColor;

        void                renderGeometryPass(const SCEHandle<Camera> &camera,
                                               std::vector<Container*>* objectsToRender);

    };
}

#endif
