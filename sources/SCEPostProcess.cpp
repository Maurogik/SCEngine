/******PROJECT:Sand Castle Engine******/
/**************************************/
/*********AUTHOR:Gwenn AUBERT**********/
/*******FILE:SCEPostProcess.cpp********/
/**************************************/

#include "../headers/SCEPostProcess.hpp"

#include "../headers/SCEShaders.hpp"
#include "../headers/SCETextures.hpp"
#include "../headers/SCETools.hpp"

//-- !! WARNING !! -- Changing this means changing the compute layout in the shaders too !
#define COMPUTE_BLOCK_SIZE 128

#define BLUR_H_SHADER_NAME "Blur_Box_H"
#define BLUR_V_SHADER_NAME "Blur_Box_V"
#define BLUR_SRC_TEX "TexSrc"
#define BLUR_DST_TEX "TexDst"
#define KERNEL_HALF_SIZE "KernelHalfSize"
#define SRC_RECT_AREA "SrcRectArea"
#define DST_RECT_AREA "DstRectArea"

namespace SCE
{
namespace PostProcess
{
    void bindBlurUniforms(GLuint blurProgram,
                          glm::ivec4 srcRectArea, glm::ivec4 dstRectArea,
                          uint kernelHalfSize,
                          GLuint srcTex, GLuint dstTex, GLuint dstInternalFormat)
    {
        SCE::TextureUtils::BindTexture(srcTex, 0, glGetUniformLocation(blurProgram, BLUR_SRC_TEX));

//        glBindImageTexture(0, srcTex, 0, GL_FALSE, 0, GL_READ_ONLY, dstInternalFormat);
//        glUniform1i(glGetUniformLocation(blurProgram, BLUR_SRC_TEX), 0);

        glBindImageTexture(1, dstTex, 0, GL_FALSE, 0, GL_WRITE_ONLY, dstInternalFormat);
        glUniform1i(glGetUniformLocation(blurProgram, BLUR_DST_TEX), 1);

        glUniform1i(glGetUniformLocation(blurProgram, KERNEL_HALF_SIZE), kernelHalfSize);

        glUniform4i(glGetUniformLocation(blurProgram, SRC_RECT_AREA),
                    srcRectArea.x, srcRectArea.y, srcRectArea.z, srcRectArea.w);
        glUniform4i(glGetUniformLocation(blurProgram, DST_RECT_AREA),
                    dstRectArea.x, dstRectArea.y, dstRectArea.z, dstRectArea.w);
    }

    bool IsValidComputeFormat(GLint texInternalFormat)
    {
        return
        texInternalFormat == GL_RGBA32F ||
        texInternalFormat == GL_RGBA16F ||
        texInternalFormat == GL_RG32F ||
        texInternalFormat == GL_RG16F ||
        texInternalFormat == GL_R11F_G11F_B10F ||
        texInternalFormat == GL_R32F ||
        texInternalFormat == GL_R16F ||
        texInternalFormat == GL_RGBA32UI ||
        texInternalFormat == GL_RGBA16UI ||
        texInternalFormat == GL_RGB10_A2UI ||
        texInternalFormat == GL_RGBA8UI ||
        texInternalFormat == GL_RG32UI ||
        texInternalFormat == GL_RG16UI ||
        texInternalFormat == GL_RG8UI ||
        texInternalFormat == GL_R32UI ||
        texInternalFormat == GL_R16UI ||
        texInternalFormat == GL_R8UI ||
        texInternalFormat == GL_RGBA32I ||
        texInternalFormat == GL_RGBA16I ||
        texInternalFormat == GL_RGBA8I ||
        texInternalFormat == GL_RG32I ||
        texInternalFormat == GL_RG16I ||
        texInternalFormat == GL_RG8I ||
        texInternalFormat == GL_R32I ||
        texInternalFormat == GL_R16I ||
        texInternalFormat == GL_R8I ||
        texInternalFormat == GL_RGBA16 ||
        texInternalFormat == GL_RGB10_A2 ||
        texInternalFormat == GL_RGBA8 ||
        texInternalFormat == GL_RG16 ||
        texInternalFormat == GL_RG8 ||
        texInternalFormat == GL_R16 ||
        texInternalFormat == GL_R8 ||
        texInternalFormat == GL_RGBA16_SNORM ||
        texInternalFormat == GL_RGBA8_SNORM ||
        texInternalFormat == GL_RG16_SNORM ||
        texInternalFormat == GL_RG8_SNORM ||
        texInternalFormat == GL_R16_SNORM ||
        texInternalFormat == GL_R8_SNORM;
    }

    void BlurTexture2D(GLuint targetTex, glm::ivec4 rectArea, uint kernelHalfSize, uint nbIter,
                       GLint texInternalFormat, GLenum format)
    {
        Debug::Assert(IsValidComputeFormat(texInternalFormat), "Texture format not supported by compute shader");
//        Debug::Assert(width%COMPUTE_BLOCK_SIZE != 0, std::to_string(width) +
//                      std::string(" is not a multiple of ") + std::to_string(COMPUTE_BLOCK_SIZE));
//        Debug::Assert(height%COMPUTE_BLOCK_SIZE != 0, std::to_string(height) +
//                      std::string(" is not a multiple of ") + std::to_string(COMPUTE_BLOCK_SIZE));

        glBindTexture(GL_TEXTURE_2D, targetTex);
        GLint originalMagFilter, originalMinFilder;
        glGetTexParameteriv(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, &originalMagFilter);
        glGetTexParameteriv(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, &originalMinFilder);
        ///filters need to be nearest
        glTexParameteri (GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
        glTexParameteri (GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);


        glm::ivec4 tmpRectArea = glm::ivec4(0, 0, rectArea.z, rectArea.w);
        GLuint tmpTex = GL_INVALID_INDEX;
        glGenTextures(1, &tmpTex);
        glBindTexture(GL_TEXTURE_2D, tmpTex);
        glTexImage2D(GL_TEXTURE_2D, 0, texInternalFormat, tmpRectArea.z, tmpRectArea.w, 0, format, GL_FLOAT, nullptr);
        glTexParameteri (GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
        glTexParameteri (GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
        glTexParameteri (GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
        glTexParameteri (GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);


        uint yWorkGroupCount = (rectArea.w + COMPUTE_BLOCK_SIZE-1)/COMPUTE_BLOCK_SIZE;
        uint xWorkGroupCount = (rectArea.z + COMPUTE_BLOCK_SIZE-1)/COMPUTE_BLOCK_SIZE;

        GLuint blurHorizontalProgram = ShaderUtils::CreateShaderProgram(BLUR_H_SHADER_NAME);
        GLuint blurVerticalProgram = ShaderUtils::CreateShaderProgram(BLUR_V_SHADER_NAME);

        while(nbIter > 0)
        {
            --nbIter;

            //Do horizontal pass
            ShaderUtils::UseShader(blurHorizontalProgram);
            bindBlurUniforms(blurHorizontalProgram, rectArea, tmpRectArea, kernelHalfSize,
                             targetTex, tmpTex, texInternalFormat);

            //dispatch over all y (because each compute then iterate over all x)
            glDispatchCompute(1, yWorkGroupCount, 1);

            glMemoryBarrier(GL_SHADER_IMAGE_ACCESS_BARRIER_BIT);

            //Do vertical pass
            ShaderUtils::UseShader(blurVerticalProgram);
            bindBlurUniforms(blurVerticalProgram, tmpRectArea, rectArea, kernelHalfSize,
                             tmpTex, targetTex, texInternalFormat);

            //dispatch over all y (because each compute then terate over all x)
            glDispatchCompute(xWorkGroupCount, 1, 1);

            glMemoryBarrier(GL_SHADER_IMAGE_ACCESS_BARRIER_BIT);
        }

        glDeleteTextures(1, &tmpTex);
        //reset filters to the original ones
        glBindTexture(GL_TEXTURE_2D, targetTex);
        glTexParameteri (GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, originalMagFilter);
        glTexParameteri (GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, originalMinFilder);
        //SCE::ShaderUtils::DeleteShaderProgram(blurProgram);
    }


}
}
