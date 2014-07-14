/**********Sand Castle Engine**********/
/**************************************/
/******** AUTHOR : Gwenn AUBERT *******/
/********** FILE : Scene.hpp **********/
/**************************************/
#ifndef SCE_SCENE_HPP
#define SCE_SCENE_HPP

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
        static void     AddContainer(Container *obj);
        static void     RemoveContainer(Container *obj);
        static std::vector<Container*> FindContainersWithTag(const std::string& tag);
        static std::vector<Container*> FindContainersWithLayer(const std::string& layer);
        static void     RegisterLight(Light* light);
        static void     UnregisterLight(Light* light);
        static void     InitLightRenderData(const GLuint &shaderId);
        static void     BindLightRenderData(const GLuint &shaderId);

    private :

        void            renderSceneWithCamera(Camera *camera);

        std::vector<Container*>         mContainers;
        std::vector<Light*>             mLights;

        /*****Static*****/
        static Scene*                   s_scene;

    };

}


#endif





