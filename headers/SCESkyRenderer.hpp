/******PROJECT:Sand Castle Engine******/
/**************************************/
/*********AUTHOR:Gwenn AUBERT**********/
/*******FILE:SCESkyRenderer.hpp********/
/**************************************/
#ifndef SCE_SKYRENDERER_HPP
#define SCE_SKYRENDERER_HPP

#include "SCEDefines.hpp"

//TODO make sky colors and fade factor customisable
//TODO make sun color customisalbe

namespace SCE
{

    struct CameraRenderData;
    class SCE_GBuffer;

    namespace SkyRenderer
    {
        void Init(uint windowWidth, uint windowHeight);
        void Render(const SCE::CameraRenderData& renderData, SCE::SCE_GBuffer& gBuffer,
                    const glm::vec3& sunPosition, const glm::vec3& sunColor);
        void SetSkyColors(const glm::vec3& skyBottomColor, const glm::vec3& skyTopColor,
                          const glm::vec3& fogColor);
        void Cleanup();
    }

}

#endif
