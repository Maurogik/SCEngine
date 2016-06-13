/******PROJECT:Sand Castle Engine******/
/**************************************/
/*********AUTHOR:Gwenn AUBERT**********/
/*********FILE:SCEQuality.cpp**********/
/**************************************/

#include "../headers/SCEQuality.hpp"

#define RENDER_QUALITY 1

#if RENDER_QUALITY == 0

        float SCE::Quality::TerrainLodMultiplier = 2.0f;
        float SCE::Quality::MaxShadowDistance = 4500.0f;
        float SCE::Quality::CameraFarPlane = 10000.0f;

        float SCE::Quality::Trees::LodDistances[MAX_TREE_LOD_COUNT] =
        {
            -1.0f, -1.0f, -1.0f, 500.0f, 800.0f
        };
        float SCE::Quality::Trees::BaseSpacing = 75.0f;
        float SCE::Quality::Trees::NbGroupPerKm = 4.0f;
        float SCE::Quality::Trees::VisibilityUpdateDuration = 2.0f;
        float SCE::Quality::Trees::MaxDrawDistance = 4000.0f;
        float SCE::Quality::Trees::ImpostorShadowEnabled = false;

#elif RENDER_QUALITY == 1

        float SCE::Quality::TerrainLodMultiplier = 1.0f;
        float SCE::Quality::MaxShadowDistance = 16000.0f;
        float SCE::Quality::CameraFarPlane = 32000.0f;

        float SCE::Quality::Trees::LodDistances[MAX_TREE_LOD_COUNT] =
        {
//            100.0f, 300.0f, 600.0f, 600.0f, 1000.0f
            100.0f, -1.0f, -1.0f, 600.0f, 1000.0f
        };
        float SCE::Quality::Trees::BaseSpacing = 90.0f;
        float SCE::Quality::Trees::NbGroupPerKm = 8.0f;
        float SCE::Quality::Trees::VisibilityUpdateDuration = 0.3f;
        float SCE::Quality::Trees::MaxDrawDistance = 16000.0f;
        bool  SCE::Quality::Trees::ImpostorShadowEnabled = false;

#elif RENDER_QUALITY == 2

        float SCE::Quality::TerrainLodMultiplier = 0.5f;
        float SCE::Quality::MaxShadowDistance = 4500.0f;
        float SCE::Quality::CameraFarPlane = 50000.0f;

        float SCE::Quality::Trees::LodDistances[MAX_TREE_LOD_COUNT] =
        {
            200.0f, 300.0f, 800.0f, 1000.0f, 1400.0f
        };
        float SCE::Quality::Trees::BaseSpacing = 30.0f;
        float SCE::Quality::Trees::NbGroupPerKm = 16.0f;
        float SCE::Quality::Trees::VisibilityUpdateDuration = 0.1f;
        float SCE::Quality::Trees::MaxDrawDistance = 16000.0f;
        float SCE::Quality::Trees::ImpostorShadowEnabled = true;

#endif
