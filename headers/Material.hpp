/**********Sand Castle Engine**********/
/**************************************/
/******** AUTHOR : Gwenn AUBERT *******/
/********* FILE : Material.hpp ********/
/**************************************/
#ifndef SCE_MATERIAL_HPP
#define SCE_MATERIAL_HPP

#include "SCEDefines.hpp"
#include "Component.hpp"
#include <map>


namespace SCE {

    enum LightingType {
        LIGHTING_NONE,
        LIGHTING_CUSTOM
    };

    enum UniformType {
        UNIFORM_FLOAT,
        UNIFORM_VEC4,
        UNIFORM_VEC3,
        UNIFORM_TEXTURE2D
    };

    struct uniform_data{
        void* data;
        UniformType type;
        GLuint dataID;
    };

    class Material : public Component{


    public :

                            Material();

        virtual             ~Material();

        /**
         * @brief Parse the material file to create a new Material object.
         * Load the material data, compile and link the sahders and returns the material.
         * @param filename of the material
         * @return A new material object.
         */
        static Material*    LoadMaterial(const std::string& filename);

        /**
         * @brief InitRenderData
         */
        void                InitRenderData();

        /**
         * @brief BindRenderData
         */
        void                BindRenderData();


        void                ReloadMaterial();

        void                CleanMaterial();

        void                SetUniformValue(const std::string& uniformName, void* value);

        void*               GetUniformValue(const std::string& uniformName);

        GLuint              GetShaderProgram();

    private :

        //Lod for shader ? later ?
        std::string                             mMaterialName;
        GLuint                                  mProgramShaderId;
        std::map<std::string, uniform_data>     mUniforms;

        /**
         * @brief Load the shader located in the given shader file, compiles and links them.
         * @param filename
         * @return the shader program ID.
         */
        static GLuint       loadShaders(const std::string& filename);
    };

}


#endif
