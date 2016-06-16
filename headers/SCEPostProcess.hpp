/******PROJECT:Sand Castle Engine******/
/**************************************/
/*********AUTHOR:Gwenn AUBERT**********/
/*******FILE:SCEPostProcess.hpp********/
/**************************************/
#ifndef SCE_POSTPROCESS_HPP
#define SCE_POSTPROCESS_HPP

#include "SCECore.hpp"

namespace SCE
{
    namespace PostProcess
    {
        void BlurTexture2D(GLuint targetTex, ivec4 rectArea, uint kernelHalfSize, uint nbIter,
                           GLint texInternalFormat, GLenum format);
    }
}

#endif
