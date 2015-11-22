/******PROJECT:Sand Castle Engine******/
/**************************************/
/*********AUTHOR:Gwenn AUBERT**********/
/*****FILE:SCEFrustrumCulling.hpp******/
/**************************************/
#ifndef SCE_FRUSTRUMCULLING_HPP
#define SCE_FRUSTRUMCULLING_HPP

#include "SCEDefines.hpp"

namespace SCE
{

    namespace FrustrumCulling
    {
        void UpdateCulling(const glm::mat4& projectionMatrix);

        bool IsSphereInFrustrum(const glm::vec4& pos_cameraspace,
                                float radius_cameraspace);

        bool IsBoxInFrustrum(const glm::vec4& pos_cameraspace,
                             const glm::vec4& R_cameraspace,
                             const glm::vec4& S_cameraspace,
                             const glm::vec4& T_cameraspace);
    }

}

#endif
