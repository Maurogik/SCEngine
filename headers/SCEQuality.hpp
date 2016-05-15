/******PROJECT:Sand Castle Engine******/
/**************************************/
/*********AUTHOR:Gwenn AUBERT**********/
/*********FILE:SCEQuality.hpp**********/
/**************************************/
#ifndef SCE_QUALITY_HPP
#define SCE_QUALITY_HPP

namespace SCE
{
    namespace Quality
    {
        #define MAX_TREE_LOD_COUNT 5

        extern float TerrainLodMultiplier;
        namespace Trees
        {
            extern float LodDistances[MAX_TREE_LOD_COUNT];
            extern float BaseSpacing;
            extern float NbGroupPerKm;
        }
    }
}

#endif
