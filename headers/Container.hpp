/**********Sand Castle Engine**********/
/**************************************/
/******** AUTHOR : Gwenn AUBERT *******/
/******** FILE : Container.hpp ********/
/**************************************/
#ifndef SCE_CONTAINER_HPP
#define SCE_CONTAINER_HPP

#include "SCEDefines.hpp"
#include "GameObject.hpp"

#include <map>

namespace SCE {

    class Component;//forward declaration of component

    class Container {

    public :

                            Container();
        virtual             ~Container();
        void                AddComponent(const std::string& componentName, Component* component);
        Component*          GetComponent(const std::string& componentName);
        void                RemoveComponent(const std::string& componentName);
        const std::string&  GetTag();
        const std::string&  GetLayer();
        void                SetTag(const std::string& tag);
        void                SetLayer(const std::string& layer);
        const std::vector<GameObject*>&
                            GetGameObjects();
        void                AddGameObject(GameObject* go);
        void                RemoveGameObject(GameObject* go);

    private :

        std::map<std::string, Component*>       mComponentMap;
        std::vector<GameObject*>                mGameObjects;
        std::string                             mTag;
        std::string                             mLayer;

    };

}


#endif
