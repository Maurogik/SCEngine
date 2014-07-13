/**********Sand Castle Engine**********/
/**************************************/
/******** AUTHOR : Gwenn AUBERT *******/
/********** FILE : Light.hpp **********/
/**************************************/
#ifndef SCE_LIGHT_HPP
#define SCE_LIGHT_HPP

#include "SCEDefines.hpp"
#include "Component.hpp"

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
    };

}


#endif
