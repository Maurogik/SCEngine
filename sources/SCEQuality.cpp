/******PROJECT:Sand Castle Engine******/
/**************************************/
/*********AUTHOR:Gwenn AUBERT**********/
/*********FILE:SCEQuality.cpp**********/
/**************************************/

#include "../headers/SCEQuality.hpp"

#define RENDER_QUALITY 1

#if RENDER_QUALITY == 0
        float SCE::Quality::TreeLodDistances[MAX_TREE_LOD_COUNT] = {
            50.0f, 100.0f, 250.0f, 500.0f, 800.0f
        };
        float SCE::Quality::TerrainLodMultiplier = 2.0f;
#elif RENDER_QUALITY == 1
        float SCE::Quality::TreeLodDistances[MAX_TREE_LOD_COUNT] = {
            100.0f, 200.0f, 350.0f, 600.0f, 1000.0f
        };
        float SCE::Quality::TerrainLodMultiplier = 1.0f;
#elif RENDER_QUALITY == 2
        float SCE::Quality::TreeLodDistances[MAX_TREE_LOD_COUNT] = {
            150.0f, 350.0f, 750.0f, 1000.0f, 1400.0f
        };
        float SCE::Quality::TerrainLodMultiplier = 0.5f;
#endif
