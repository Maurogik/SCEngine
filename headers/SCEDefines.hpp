#ifndef SCE_DEFINES_HPP
#define SCE_DEFINES_HPP

// Include GLEW
#include <GL/glew.h>

#include <glm/glm.hpp>
using namespace glm;

//defined in Cmake with -DSCE_DEBUG compile option
//#define SCE_DEBUG
//#define SCE_DEBUG_ENGINE

#include <string>
#include <iostream>
#include <vector>
#include <algorithm>
#include <memory>


#define DEFAULT_LAYER "DefaultLayer"
#define LIGHTS_LAYER "Lighting"
#define DEFAULT_TAG "DefaultTag"

#define ENGINE_RESSOURCE_PATH "SCE_Assets/"
#define RESSOURCE_PATH "ressources/"
#define SHADER_SUFIX ".shader"
#define MATERIAL_SUFIX ".material"
#define TEXTURE_METADATA_SUFIX ".texData"


typedef int16_t i16;
typedef int32_t i32;
typedef int64_t i64;

typedef uint16_t ui16;
typedef uint32_t ui32;
typedef uint64_t ui64;

#endif
