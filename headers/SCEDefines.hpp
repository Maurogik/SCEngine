#ifndef SCE_DEFINES_HPP
#define SCE_DEFINES_HPP

// Include GLEW
#include <GL/glew.h>

#include <glm/glm.hpp>
using namespace glm;

//define here because it is used in SCETools ans SCEInternal
#define SCE_DEBUG
#define SCE_DEBUG_ENGINE

#include <string>
#include <iostream>
#include <vector>
#include <algorithm>
#include <memory>


#define DEFAULT_LAYER "DefaultLayer"
#define DEFAULT_TAG "DefaultTag"

#define ENGINE_RESSOURCE_PATH "SCE_Assets/"
#define RESSOURCE_PATH "ressources/"
#define SHADER_SUFIX ".shader"
#define MATERIAL_SUFIX ".material"
#define TEXTURE_METADATA_SUFIX ".texData"


#endif
