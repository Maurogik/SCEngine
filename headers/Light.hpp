/**********Sand Castle Engine**********/
/**************************************/
/******** AUTHOR : Gwenn AUBERT *******/
/********** FILE : Light.hpp **********/
/**************************************/
#ifndef SCE_LIGHT_HPP
#define SCE_LIGHT_HPP

#include "SCEDefines.hpp"
#include "Component.hpp"
#include <map>

#define LIGHT_POS_UNIFORM_STR "LightPos_worldspace"
#define LIGHT_COLOR_UNIFORM_STR "LightColor"

namespace SCE {

    REQUIRE_COMPONENT(Transform)

    class Light : public Component {

    public :

        Light();
        virtual             ~Light();
        virtual void        SetContainer(Container* cont);

        void                InitRenderDataForShader(const GLuint &shaderId);

        void                BindRenderDataForShader(const GLuint &shaderId);

    private :

        glm::vec4           mLightColor;
        //keep map of all shader/uniforms ID pairs ?
        std::map<GLuint, GLuint>    mLightPosByShader;
        std::map<GLuint, GLuint>    mLightColorByShader;

    };

}


#endif
