/**********Sand Castle Engine**********/
/**************************************/
/******** AUTHOR : Gwenn AUBERT *******/
/********** FILE : Scene.hpp **********/
/**************************************/
#ifndef SCE_SCENE_HPP
#define SCE_SCENE_HPP

#include "GameObject.hpp"
#include "Container.hpp"
#include "SCEDefines.hpp"
#include "Light.hpp"

namespace SCE {

    class Camera;

    class SCEScene {

    public :
        SCEScene();
        ~SCEScene();
        void            RenderScene();
        void            UpdateScene();

        /*****Static*****/

        //basic scene functions
        static void     CreateEmptyScene();
        static void     LoadScene(const std::string& scenePath);
        static void     Run();
        static void     DestroyScene();

        //object related functions
        static SCEHandle<Container>                 CreateContainer(const std::string& name);
        static void                                 DestroyContainer(const SCEHandle<Container>& container);

        static std::vector<SCEHandle<Container> >   FindContainersWithTag(const std::string& tag);
        static std::vector<SCEHandle<Container> >   FindContainersWithLayer(const std::string& layer);

        static void                                 RegisterGameObject(SCEHandle<GameObject> gameObject);
        static void                                 UnregisterGameObject(SCEHandle<GameObject> gameObject);

        static void                                 RegisterLight(SCEHandle<Light> light);
        static void                                 UnregisterLight(SCEHandle<Light> light);
        static void                                 InitLightRenderData(GLuint shaderId);
        static void                                 BindLightRenderData(GLuint shaderId);
        static std::vector<SCEHandle<Light> >       FindLightsInRange(const glm::vec3& worldPosition);

    private :

        static void     RemoveContainer(int objId);

        void            renderSceneWithCamera(const SCEHandle<Camera> &camera);        

        std::vector<Container*>             mContainers;
        std::vector<SCEHandle<Light>>       mLights;
        std::vector<SCEHandle<GameObject>>  mGameObjects;
        int                                 mLastId;       


        /*****Static*****/
        static SCEScene*                       s_scene;

    };

}


#endif





