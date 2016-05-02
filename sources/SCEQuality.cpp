/******PROJECT:Sand Castle Engine******/
/**************************************/
/*********AUTHOR:Gwenn AUBERT**********/
/*********FILE:SCEQuality.cpp**********/
/**************************************/

#include "../headers/SCEQuality.hpp"

#define RENDER_QUALITY 1

#if RENDER_QUALITY == 0
        float SCE::Quality::TreeLodMultiplier = 8.0f;
        float SCE::Quality::TerrainLodMultiplier = 2.0f;
#elif RENDER_QUALITY == 1
        float SCE::Quality::TreeLodMultiplier = 4.0f;
        float SCE::Quality::TerrainLodMultiplier = 1.0f;
#elif RENDER_QUALITY == 2
        float SCE::Quality::TreeLodMultiplier = 1.0f;
        float SCE::Quality::TerrainLodMultiplier = 0.5f;
#endif
