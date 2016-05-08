/******PROJECT:Sand Castle Engine******/
/**************************************/
/*********AUTHOR:Gwenn AUBERT**********/
/*********FILE:SCEQuality.cpp**********/
/**************************************/

#include "../headers/SCEQuality.hpp"

#define RENDER_QUALITY 1

#if RENDER_QUALITY == 0

        float SCE::Quality::TerrainLodMultiplier = 2.0f;

        float SCE::Quality::Trees::LodDistances[MAX_TREE_LOD_COUNT] = {
            50.0f, 100.0f, 250.0f, 500.0f, 800.0f
        };
        float SCE::Quality::Trees::VisibilityUpdateDelay = 5.0f;
        float SCE::Quality::Trees::BaseSpacing = 200.0f;
        float SCE::Quality::Trees::PlacementAccuracy = 0.3f;

#elif RENDER_QUALITY == 1

        float SCE::Quality::TerrainLodMultiplier = 1.0f;

        float SCE::Quality::Trees::LodDistances[MAX_TREE_LOD_COUNT] = {
            -1.0f, -1.0f, 400.0f, 600.0f, 1000.0f
        };
        float SCE::Quality::Trees::VisibilityUpdateDelay = 0.001f;
        float SCE::Quality::Trees::BaseSpacing = 100.0f;
        float SCE::Quality::Trees::PlacementAccuracy = 0.5f;

#elif RENDER_QUALITY == 2

        float SCE::Quality::TerrainLodMultiplier = 0.5f;

        float SCE::Quality::Trees::LodDistances[MAX_TREE_LOD_COUNT] = {
            150.0f, 350.0f, 750.0f, 1000.0f, 1400.0f
        };
        float SCE::Quality::Trees::VisibilityUpdateDelay = 0.1f;
        float SCE::Quality::Trees::BaseSpacing = 50.0f;
        float SCE::Quality::Trees::PlacementAccuracy = 1.0f;

#endif
