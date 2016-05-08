/******PROJECT:Sand Castle Engine******/
/**************************************/
/*********AUTHOR:Gwenn AUBERT**********/
/**********FILE:SCERender.cpp**********/
/**************************************/

#include "../headers/SCERender.hpp"
#include "../headers/SCELighting.hpp"
#include "../headers/SCECore.hpp"
#include "../headers/SCETools.hpp"
#include "../headers/SCEMeshLoader.hpp"
#include "../headers/SCEMeshRender.hpp"
#include "../headers/SCEShaders.hpp"
#include "../headers/SCETerrain.hpp"
#include "../headers/SCEDebugText.hpp"
#include "../headers/SCEFrustrumCulling.hpp"


using namespace std;

#define TONEMAP_EXPOSURE_NAME "SCE_Exposure"
#define TONEMAP_MAX_BRIGHTNESS_NAME "SCE_MaxBrightness"
#define TONEMAP_STRENGTH_NAME "SCE_TonemapStrength"

namespace SCE
{
namespace Render
{
    struct ToneMappingData
    {
        ToneMappingData()
            : toneMapShader(GL_INVALID_INDEX),
              luminanceShader(GL_INVALID_INDEX),
              exposureUniform(GL_INVALID_INDEX),
              maxBrightnessUniform(GL_INVALID_INDEX),
              exposure(1.0f), maxBrightness(1.5f) {}

        GLuint  toneMapShader;
        GLuint  luminanceShader;
        GLint   exposureUniform;
        GLint   maxBrightnessUniform;
        GLint   tonemapStrengthUniform;
        float   exposure;
        float   maxBrightness;
    };

    static ToneMappingData      mToneMapData;
    static SCE_GBuffer          mGBuffer;
    static glm::vec4            mDefaultClearColor;
    static ui16                 mQuadMeshId;
    static bool                 mDebugTonemapOff = false;

    namespace //anonymous namespace to avoid name conflict in case of cpp inclusion (ie Unity build)
    {
        void renderGeometryPass(const CameraRenderData& renderData,
                                           std::vector<Container*> objectsToRender)
        {
            mGBuffer.BindForGeometryPass();
            // The geometry pass updates the depth buffer
            glDepthMask(GL_TRUE);
            glEnable(GL_DEPTH_TEST);
            glEnable(GL_CULL_FACE);
            glCullFace(GL_BACK);

            SCE::Terrain::RenderTerrain(renderData.projectionMatrix, renderData.viewMatrix);
            SCE::Terrain::RenderTrees(renderData.projectionMatrix, renderData.viewMatrix);

            for(Container* container : objectsToRender)
            {
                SCEHandle<Material> mat = container->GetComponent<Material>();
                // Use the shader
                mat->BindMaterialData();

                SCEHandle<MeshRenderer> renderer = container->GetComponent<MeshRenderer>();
                renderer->Render(renderData);
            }
        }
    }


    void Init()
    {
        SCELighting::Init();

        mQuadMeshId = -1;
        mDefaultClearColor = glm::vec4(0.0, 0.0, 0.0, 0.0);

        glClearColor(mDefaultClearColor.r,
                     mDefaultClearColor.g,
                     mDefaultClearColor.b,
                     mDefaultClearColor.a);

        // Enable depth test
        glEnable(GL_DEPTH_TEST);
        // Accept fragment if it closer to the camera than the former one
        glDepthFunc(GL_LEQUAL);
        // Cull triangles which normal is not towards the camera
        glFrontFace(GL_CCW); //this is the default open gl winding order
        glEnable(GL_CULL_FACE);

        //initialize the Gbuffer used to deferred lighting
        mGBuffer.Init(SCECore::GetWindowWidth(), SCECore::GetWindowHeight());

        mQuadMeshId = SCE::MeshLoader::CreateQuadMesh();

        mToneMapData.toneMapShader = SCE::ShaderUtils::CreateShaderProgram("ToneMapping");
        mToneMapData.luminanceShader = SCE::ShaderUtils::CreateShaderProgram("LuminanceShader");
        mToneMapData.exposureUniform = glGetUniformLocation(mToneMapData.toneMapShader,
                                                            TONEMAP_EXPOSURE_NAME);
        mToneMapData.maxBrightnessUniform = glGetUniformLocation(mToneMapData.toneMapShader,
                                                                 TONEMAP_MAX_BRIGHTNESS_NAME);
        mToneMapData.tonemapStrengthUniform = glGetUniformLocation(mToneMapData.toneMapShader,
                                                                 TONEMAP_STRENGTH_NAME);
    }

    void CleanUp()
    {
        SCE::ShaderUtils::DeleteShaderProgram(mToneMapData.toneMapShader);
        SCE::ShaderUtils::DeleteShaderProgram(mToneMapData.luminanceShader);
        SCELighting::CleanUp();     
    }

    void Render(const SCEHandle<Camera>& camera,
                           vector<Container*> objectsToRender)
    {
        vector<MeshRenderer*> shadowCasters;

        for(Container* obj : objectsToRender)
        {
            SCEHandle<MeshRenderer> meshRenderer = obj->GetComponent<MeshRenderer>();
            if(meshRenderer->IsCastingShadow())
            {
                shadowCasters.push_back(meshRenderer.getRaw());
            }
        }

        //extract data used for rendering
        CameraRenderData renderData = camera->GetRenderData();

        // Clear the screen (default framebuffer)
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        SCE::FrustrumCulling::UpdateCulling(renderData.projectionMatrix);
        SCE::Terrain::UpdateTerrain(renderData.projectionMatrix, renderData.viewMatrix);

        //render shadows to shadowmap
        SCEHandle<Transform> camTransform = camera->GetContainer()->GetComponent<Transform>();
        glm::mat4 camToWorld = camTransform->GetWorldTransform();
        SCELighting::RenderCascadedShadowMap(renderData, camera->GetFrustrumData(),
                                             camToWorld, shadowCasters);

        //render objects without lighting
        renderGeometryPass(renderData, objectsToRender);

        //lighting & sky
        mGBuffer.ClearFinalBuffer();
        SCELighting::RenderLightsToGBuffer(renderData, mGBuffer);               

        SCELighting::RenderSkyToGBuffer(renderData, mGBuffer);

        //luminance
        ToneMappingData& tonemap = mToneMapData;
        glDisable(GL_DEPTH_TEST);
        glUseProgram(tonemap.luminanceShader);
        mGBuffer.BindForLuminancePass();
        RenderFullScreenPass(tonemap.luminanceShader, renderData.projectionMatrix, renderData.viewMatrix);
        mGBuffer.GenerateLuminanceMimap();

        //Tonemapping & render to back buffer
        glUseProgram(tonemap.toneMapShader);
        mGBuffer.BindForToneMapPass();

        glUniform1f(tonemap.exposureUniform, tonemap.exposure);
        glUniform1f(tonemap.maxBrightnessUniform, tonemap.maxBrightness);

        glUniform1f(tonemap.tonemapStrengthUniform, mDebugTonemapOff ? 0.0f : 1.0f);

        RenderFullScreenPass(tonemap.toneMapShader, renderData.projectionMatrix, renderData.viewMatrix);

        //reset to default framebufffer
        glBindFramebuffer(GL_FRAMEBUFFER, 0);

        //render debug text
        SCE::DebugText::RenderMessages(renderData.viewMatrix, renderData.projectionMatrix);
        glEnable(GL_DEPTH_TEST);
    }

    void ResetClearColorToDefault()
    {
        glClearColor(mDefaultClearColor.r,
                     mDefaultClearColor.g,
                     mDefaultClearColor.b,
                     mDefaultClearColor.a);
    }

    glm::mat4 FixOpenGLProjectionMatrix(const mat4& projMat)
    {
        //Needed because opengl/glm camera renders along the negative Z axis and
        //I want it to render along the positive axis
        return glm::scale(projMat, glm::vec3(1, 1, -1));
    }

    void RenderFullScreenPass(GLuint shaderId, const mat4& projectionMatrix, const mat4& viewMatrix)
    {
        glCullFace(GL_FRONT);
        glm::mat4 modelMatrix = inverse(projectionMatrix * viewMatrix);
        SCE::MeshRender::RenderMesh(mQuadMeshId, projectionMatrix, viewMatrix, modelMatrix);
        glCullFace(GL_BACK);
    }

#ifdef SCE_DEBUG_ENGINE
    bool ToggleTonemapOff()
    {
        mDebugTonemapOff = !mDebugTonemapOff;
        return mDebugTonemapOff;
    }
#endif

}
}


