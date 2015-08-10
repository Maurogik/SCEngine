/******PROJECT:Sand Castle Engine******/
/**************************************/
/*********AUTHOR:Gwenn AUBERT**********/
/*********FILE:SCELighting.hpp*********/
/**************************************/


#ifndef SCE_LIGHTING_HPP
#define SCE_LIGHTING_HPP

#include "SCEDefines.hpp"

namespace SCE
{
    class SCELighting
    {
    public :

        static void         Init();
        static void         CleanUp();
        static void         StartLightPass();
        static GLuint       GetLightShader();

    private :

        static GLuint       s_DefaultLightShader;

        static void         initLightShader();

    };
}

#endif
