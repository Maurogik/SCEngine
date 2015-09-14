/******PROJECT:Sand Castle Engine******/
/**************************************/
/*********AUTHOR:Gwenn AUBERT**********/
/*******FILE:SCESkyRenderer.hpp********/
/**************************************/
#ifndef SCE_SKYRENDERER_HPP
#define SCE_SKYRENDERER_HPP

#include "SCEDefines.hpp"
namespace SCE
{

struct CameraRenderData;
class SCE_GBuffer;

namespace SkyRenderer
{
    void Init(uint windowWidth, uint windowHeight);
    void Render(const SCE::CameraRenderData& renderData, SCE::SCE_GBuffer& gBuffer,
                const glm::vec3& sunPosition);
    void Cleanup();
}

}

#endif
