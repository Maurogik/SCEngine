/******PROJECT:Sand Castle Engine******/
/**************************************/
/*********AUTHOR:Gwenn AUBERT**********/
/*******FILE:SCEToneMapping.hpp********/
/**************************************/
#ifndef SCE_TONEMAPPING_HPP
#define SCE_TONEMAPPING_HPP

#include "SCEDefines.hpp"

namespace SCE
{

    class SCEToneMapping
    {
    public :

        SCEToneMapping();

    private :

        GLuint          mTonemapShader;
        GLint           mExposureUniform;
        GLint           mMaxBrightnessUniform;

    };

}

#endif
