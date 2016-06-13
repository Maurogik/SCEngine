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

        //Terrain
        static void                                 AddTerrain(float terrainSize, float patchSize,
                                                               float baseHeight, int nbRepeat = 1);

        static void                                 RemoveTerrain();
        static void                                 SetRootWorldspacePosition(glm::vec3 const& pos_worldspace);
        static glm::vec3                            GetFrameRootPosition();

#ifdef SCE_DEBUG_ENGINE
        static void                                 ReloadAllMaterials();
#endif

    private :

        SCEScene();
        ~SCEScene();

        static void     RemoveContainer(int objId);

        void            renderSceneWithCamera(const SCEHandle<Camera> &camera);        

        std::vector<Container*>             mContainers;
        std::vector<SCEHandle<GameObject>>  mGameObjects;
        int                                 mLastId;
        glm::vec3                           mScenePosition;
        glm::vec3                           mPrevScenePosition;


        /*****Static*****/
        static SCEScene*                    s_scene;

    };

}


#endif





