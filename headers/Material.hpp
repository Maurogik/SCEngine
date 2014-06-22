/**********Sand Castle Engine**********/
/**************************************/
/******** AUTHOR : Gwenn AUBERT *******/
/********* FILE : Material.hpp ********/
/**************************************/
#ifndef SCE_MATERIAL_HPP
#define SCE_MATERIAL_HPP

#include "SCEDefines.hpp"
#include <map>


namespace SCE {

    enum LightingType {
        LIGHTING_NONE,
        LIGHTING_CUSTOM
    };

    enum UniformType {
        UNIFORM_F,
    };

    struct attrib_data{
        void* data;
        size_t size;
        GLuint dataID;
    };

    struct uniform_data{
        void* data;
        UniformType type;
        GLuint dataID;
    };

    class Material {


    public :

                        Material();

                        ~Material();

        /**
         * @brief Load, parse and compile the shader,
         * initialize the appropriate uniforms and attribute.
         */
        void            LoadMaterial(std::string filename);

        /**
         * @brief Binds this material shader and uniforms
         */
        void            BindRenderData();

        void            ReloadMaterial();

        void            CleanMaterial();


    private :

        //Lod for shader ? later ?

        int                                     mProgramShaderId;
        std::map<std::string, attrib_data>      mAttributes;
        std::map<std::string, uniform_data>     mUniforms;


    };

}


#endif
