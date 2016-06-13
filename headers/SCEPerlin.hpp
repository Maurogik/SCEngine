/******PROJECT:Sand Castle Engine******/
/**************************************/
/*********AUTHOR:Gwenn AUBERT**********/
/**********FILE:SCEPerlin.hpp**********/
/**************************************/
#ifndef SCE_SCEPERLIN_HPP
#define SCE_SCEPERLIN_HPP

#include "SCEDefines.hpp"

namespace SCE
{
    namespace Perlin
    {

        void    MakePerlin(ui16 gridSize);
        float   GetPerlinAt(float x, float y, float period);
        float   GetPerlinAt(float x, float y);
        float   GetLayeredPerlinAt(float x, float y, int layers, float persistence);
        void    DestroyPerlin();

    }
}

#endif
