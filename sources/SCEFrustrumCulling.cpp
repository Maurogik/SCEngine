/******PROJECT:Sand Castle Engine******/
/**************************************/
/*********AUTHOR:Gwenn AUBERT**********/
/*****FILE:SCEFrustrumCulling.cpp******/
/**************************************/


#include "../headers/SCEFrustrumCulling.hpp"


namespace SCE
{

namespace FrustrumCulling
{

    namespace
    {
        struct FrustrumCullingData
        {
            glm::vec4 leftPlane;
            glm::vec4 rightPlane;
            glm::vec4 bottomPlane;
            glm::vec4 topPlane;
            glm::vec4 nearPlane;
            glm::vec4 farPlane;
        };

        FrustrumCullingData cullingData;
    }

    void UpdateCulling(const mat4 &projectionMatrix)
    {
        glm::mat4 invP = glm::inverse(projectionMatrix);
        cullingData.leftPlane = invP*vec4(1.0, 0.0, 0.0, 1.0);
        cullingData.rightPlane = invP*vec4(-1.0, 0.0, 0.0, 1.0);
        cullingData.bottomPlane = invP*vec4(0.0, 1.0, 0.0, 1.0);
        cullingData.topPlane = invP*vec4(0.0, -1.0, 0.0, 1.0);
        cullingData.nearPlane = invP*vec4(0.0, 0.0, 1.0, 1.0);
        cullingData.farPlane = invP*vec4(0.0, 0.0, -1.0, 1.0);
    }

    bool IsSphereInFrustrum(const vec4 &pos_cameraspace, float radius_cameraspace)
    {
        if( glm::dot(cullingData.leftPlane, pos_cameraspace)   < -radius_cameraspace ||
            glm::dot(cullingData.rightPlane, pos_cameraspace)  < -radius_cameraspace ||
            glm::dot(cullingData.topPlane, pos_cameraspace)    < -radius_cameraspace ||
            glm::dot(cullingData.bottomPlane, pos_cameraspace) < -radius_cameraspace ||
            glm::dot(cullingData.nearPlane, pos_cameraspace)   < -radius_cameraspace ||
            glm::dot(cullingData.farPlane, pos_cameraspace)    < -radius_cameraspace )
        {
            return false;
        }

        return true;
    }

    bool IsBoxInFrustrum(const vec4 &pos_cameraspace,
                         const vec4 &R_cameraspace,
                         const vec4 &S_cameraspace,
                         const vec4 &T_cameraspace)
    {
        glm::vec4 xVec = R_cameraspace;
        glm::vec4 yVec = S_cameraspace;
        glm::vec4 zVec = T_cameraspace;

        //compute effective radius in each frustrum direction
        float radiusProjLeft =
                (glm::abs(glm::dot(cullingData.leftPlane, xVec)) +
                glm::abs(glm::dot(cullingData.leftPlane, yVec)) +
                glm::abs(glm::dot(cullingData.leftPlane, zVec)))*0.5;

        float radiusProjRight =
                (glm::abs(glm::dot(cullingData.rightPlane, xVec)) +
                glm::abs(glm::dot(cullingData.rightPlane, yVec)) +
                glm::abs(glm::dot(cullingData.rightPlane, zVec)))*0.5;

        float radiusProjTop =
                (glm::abs(glm::dot(cullingData.topPlane, xVec)) +
                glm::abs(glm::dot(cullingData.topPlane, yVec)) +
                glm::abs(glm::dot(cullingData.topPlane, zVec)))*0.5;

        float radiusProjBottom =
                (glm::abs(glm::dot(cullingData.bottomPlane, xVec)) +
                glm::abs(glm::dot(cullingData.bottomPlane, yVec)) +
                glm::abs(glm::dot(cullingData.bottomPlane, zVec)))*0.5;

        float radiusProjNear =
                (glm::abs(glm::dot(cullingData.nearPlane, xVec)) +
                glm::abs(glm::dot(cullingData.nearPlane, yVec)) +
                glm::abs(glm::dot(cullingData.nearPlane, zVec)))*0.5;

        float radiusProjFar =
                (glm::abs(glm::dot(cullingData.farPlane, xVec)) +
                glm::abs(glm::dot(cullingData.farPlane, yVec)) +
                glm::abs(glm::dot(cullingData.farPlane, zVec)))*0.5;

        //for each plane, compare the distance from the (center, plane) to the effective radius
        //in this frustrum direction
        if(     glm::dot(cullingData.leftPlane, pos_cameraspace)   < -radiusProjLeft   ||
                glm::dot(cullingData.rightPlane, pos_cameraspace)  < -radiusProjRight  ||
                glm::dot(cullingData.topPlane, pos_cameraspace)    < -radiusProjTop    ||
                glm::dot(cullingData.bottomPlane, pos_cameraspace) < -radiusProjBottom ||
                glm::dot(cullingData.nearPlane, pos_cameraspace)   < -radiusProjNear   ||
                glm::dot(cullingData.farPlane, pos_cameraspace)    < -radiusProjFar    )
        {
            return false;
        }

        return true;
    }

}

}
