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
        static void     AddContainer(std::shared_ptr<Container> obj);
        static void     RemoveContainer(std::shared_ptr<Container> obj);

        static std::vector<std::shared_ptr<Container> > FindContainersWithTag(const std::string& tag);
        static std::vector<std::shared_ptr<Container> > FindContainersWithLayer(const std::string& layer);

        static void     RegisterGameObject(std::shared_ptr<GameObject> gameObject);
        static void     UnregisterGameObject(std::shared_ptr<GameObject> gameObject);

        static void     RegisterLight(std::shared_ptr<Light> light);
        static void     UnregisterLight(std::shared_ptr<Light> light);

        static void     InitLightRenderData(const GLuint &shaderId);
        static void     BindLightRenderData(const GLuint &shaderId);

    private :

        void            renderSceneWithCamera(const Camera& camera);

        std::vector<std::shared_ptr<Container> >    mContainers;
        std::vector<std::shared_ptr<Light> >        mLights;
        std::vector<std::shared_ptr<GameObject> >   mGameObjects;
        static bool                                 mCleaningScene;

        /*****Static*****/
        static Scene*                               s_scene;

    };

}


#endif





