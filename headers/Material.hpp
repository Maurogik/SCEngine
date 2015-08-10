/**********Sand Castle Engine**********/
/**************************************/
/******** AUTHOR : Gwenn AUBERT *******/
/********* FILE : Material.hpp ********/
/**************************************/
#ifndef SCE_MATERIAL_HPP
#define SCE_MATERIAL_HPP

#include "SCEDefines.hpp"
#include "SCETools.hpp"
#include "Component.hpp"
#include "Light.hpp"
#include <map>


namespace SCE {

    enum LightingType {
        LIGHTING_NONE,   //Do nothing & do not pass the light uniforms
        LIGHTING_CUSTOM, //Pass the light uniforms but do nothing
        LIGHTING_DEFAULT //Pass the uniforms & insert stardard functions to process lights
    };


    /**
     * @brief The Material class, basically contains a shader and related uniforms
     */
    class Material : public Component{

        enum UniformType {
            UNIFORM_FLOAT,
            UNIFORM_VEC4,
            UNIFORM_VEC3,
            UNIFORM_TEXTURE2D
        };

        struct uniform_data{
            std::string     name;
            void*           data;
            UniformType     type;
            GLuint          dataID;
        };

    public :

        virtual             ~Material();
        void                BindMaterialData();
        void                ReloadMaterial();
        void                CleanMaterial();

        template<typename T>
        void                SetUniformValue(const std::string& uniformName, const T& value);

        template<typename T>
        const T&            GetUniformValue(const std::string& uniformName) const;

        const GLuint&       GetShaderProgram() const;

    protected :

                            Material(SCEHandle<Container>& container, const std::string &filename, const std::string& typeName = "");

    private :

        /**
         * @brief Parse the material file to load the Material object.
         * Load the material data, compile and link the shaders and returns the material.
         * @param filename of the material
         */
        void LoadMaterial(const std::string& filename);

        /**
         * @brief Load the shader located in the given shader file, compiles and links them.
         * @param filename
         * @return the shader program ID.
         */
        static GLuint       loadShaders(const std::string& filename);

        //TODO add LOD/verions for shader ?
        std::string                             mMaterialName;
        GLuint                                  mProgramShaderId;
        std::map<std::string, uniform_data>     mUniforms;
    };

}

#include "../templates/Material.tpp"

#endif
