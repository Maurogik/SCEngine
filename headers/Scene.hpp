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

    class Scene {

    public :
        Scene();
        ~Scene();
        void            RenderScene();
        void            UpdateScene();

        /*****Static*****/

        //basic scene functions
        static void     CreateEmptyScene();
        static void     LoadScene(const std::string& scenePath);
        static void     Run();
        static void     DestroyScene();

        //object related functions
        static Handle<Container> CreateContainer(const std::string& name);
        static void      DestroyContainer(const Handle<Container>& container);

        static std::vector<Handle<Container> > FindContainersWithTag(const std::string& tag);
        static std::vector<Handle<Container> > FindContainersWithLayer(const std::string& layer);

        static void     RegisterGameObject(Handle<GameObject> gameObject);
        static void     UnregisterGameObject(Handle<GameObject> gameObject);

        static void     RegisterLight(Handle<Light> light);
        static void     UnregisterLight(Handle<Light> light);

        static void     InitLightRenderData(const GLuint &shaderId);
        static void     BindLightRenderData(const GLuint &shaderId);

    private :

        static void     RemoveContainer(const int& objId);

        void            renderSceneWithCamera(const Handle<Camera> &camera);

        std::vector<Container*>             mContainers;
        std::vector<Handle<Light> >         mLights;
        std::vector<Handle<GameObject> >    mGameObjects;
        int                                 mLastId;

        /*****Static*****/
        static Scene*                       s_scene;

    };

}


#endif





