/**********Sand Castle Engine**********/
/**************************************/
/******** AUTHOR : Gwenn AUBERT *******/
/******** FILE : Container.hpp ********/
/**************************************/
#ifndef SCE_CONTAINER_HPP
#define SCE_CONTAINER_HPP

#include "SCEDefines.hpp"
#include "SCEInternal.hpp"
#include "Component.hpp"
#include "SCEInternalComponent.hpp"
#include "SCEHandle.hpp"
#include <typeinfo>
#include <type_traits>

namespace SCE {
    //Container class should not be inherited but the final keyword doesn't seem to work here
    class Container : public SCEHandleTarget{
        friend class SCEScene;

    public :

                            ~Container();

        template < class T, class... Args >
        SCEHandle<T>           AddComponent(Args&&... args);


        template < class T >
        SCEHandle<T>           GetComponent();



        template < class T >
        const SCEHandle<T>     GetComponent() const;


        template < class T >
        void                RemoveComponent();

        template < class T >
        bool                HasComponent() const;

        const std::string&  GetTag() const;
        const std::string&  GetLayer() const;
        const std::string&  GetName() const;
        void                SetTag(const std::string& tag);
        void                SetLayer(const std::string& layer);
        void                SetName(const std::string &name);

        const int&          GetContainerId() const;

    private :

        //Prevent creation of copy constructor for now
                            Container(const Container&)     = delete;
        //Prevent creation of move Contructor for now
                            Container(Container&&)          = delete;

                            Container(const std::string& name, const int& id);

        template < class T >
        SCEHandle<T>           fetchComponent() const;

        std::vector<Component*>                             mComponents;
        std::string                                         mTag;
        std::string                                         mLayer;
        std::string                                         mName;
        int                                                 mContainerId;

    };
}

#include "../templates/Container.tpp"

#endif
