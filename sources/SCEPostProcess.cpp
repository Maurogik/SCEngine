/******PROJECT:Sand Castle Engine******/
/**************************************/
/*********AUTHOR:Gwenn AUBERT**********/
/*******FILE:SCEPostProcess.cpp********/
/**************************************/

#include "../headers/SCEPostProcess.hpp"

#include "../headers/SCEShaders.hpp"
#include "../headers/SCETextures.hpp"
#include "../headers/SCETools.hpp"

#define COMPUTE_BLOCK_SIZE 128
#define BLUR_H_SHADER_NAME "Blur_Box_H"
#define BLUR_V_SHADER_NAME "Blur_Box_V"
#define BLUR_SRC_TEX "TexSrc"
#define BLUR_DST_TEX "TexDst"

namespace SCE
{
namespace PostProcess
{

    void BlurTexture2D(GLuint tex, uint width, uint height, uint kernelHalfSize)
    {
        BlurTexture2D(tex, width, height, kernelHalfSize, GL_RGBA32F, GL_RGBA);
    }

    void BlurTexture2D(GLuint tex, uint width, uint height, uint kernelHalfSize,
                       GLint texInternalFormat, GLenum format)
    {
//        Debug::Assert(width%COMPUTE_BLOCK_SIZE != 0, std::to_string(width) +
//                      std::string(" is not a multiple of ") + std::to_string(COMPUTE_BLOCK_SIZE));
//        Debug::Assert(height%COMPUTE_BLOCK_SIZE != 0, std::to_string(height) +
//                      std::string(" is not a multiple of ") + std::to_string(COMPUTE_BLOCK_SIZE));

        GLuint blurProgram = ShaderUtils::CreateShaderProgram("Blur_Box");
        GLuint tmpTex = GL_INVALID_INDEX;
        glGenTextures(1, &tmpTex);
        glBindTexture(GL_TEXTURE_2D, tmpTex);
        glTexImage2D(GL_TEXTURE_2D, 0, texInternalFormat, width, height, 0, format, GL_FLOAT, nullptr);
        glTexParameteri (GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
        glTexParameteri (GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
        glTexParameteri (GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        glTexParameteri (GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);

        //Do horizontal pass
        ShaderUtils::UseShader(blurProgram);
//        SCE::TextureUtils::BindTexture(tex, 0, glGetUniformLocation(blurProgram, BLUR_SRC_TEX));
//        SCE::TextureUtils::BindTexture(tmpTex, 1, glGetUniformLocation(blurProgram, BLUR_DST_TEX));

        glActiveTexture(GL_TEXTURE0);
        glBindImageTexture(0, tex, 0, GL_FALSE, 0, GL_READ_ONLY, GL_RGBA32F);
        glUniform1i(glGetUniformLocation(blurProgram, BLUR_SRC_TEX), 0);
        glActiveTexture(GL_TEXTURE1);
        glBindImageTexture(1, tmpTex, 0, GL_FALSE, 0, GL_WRITE_ONLY, GL_RGBA32F);
        glUniform1i(glGetUniformLocation(blurProgram, BLUR_DST_TEX), 1);

        //dispatch over all y
        glDispatchCompute(1, height/COMPUTE_BLOCK_SIZE, 1);

        glMemoryBarrier (GL_SHADER_IMAGE_ACCESS_BARRIER_BIT);

        //swap and rebind textures
//        SCE::TextureUtils::BindTexture(tmpTex, 0, glGetUniformLocation(blurProgram, BLUR_SRC_TEX));
//        SCE::TextureUtils::BindTexture(tex, 1, glGetUniformLocation(blurProgram, BLUR_DST_TEX));
        glActiveTexture(GL_TEXTURE0);
        glBindImageTexture(0, tmpTex, 0, GL_FALSE, 0, GL_READ_ONLY, GL_RGBA32F);
        glUniform1i(glGetUniformLocation(blurProgram, BLUR_SRC_TEX), 0);
        glActiveTexture(GL_TEXTURE1);
        glBindImageTexture(1, tex, 0, GL_FALSE, 0, GL_WRITE_ONLY, GL_RGBA32F);
        glUniform1i(glGetUniformLocation(blurProgram, BLUR_DST_TEX), 1);


        glDispatchCompute(1, height/COMPUTE_BLOCK_SIZE, 1);

        //Do vertical pass

        //dispatch over all x
        //glDispatchCompute(height/COMPUTE_BLOCK_SIZE, 1, 1);

        glMemoryBarrier (GL_SHADER_IMAGE_ACCESS_BARRIER_BIT);

        glDeleteTextures(1, &tmpTex);
        //SCE::ShaderUtils::DeleteShaderProgram(blurProgram);
    }


}
}
