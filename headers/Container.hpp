/**********Sand Castle Engine**********/
/**************************************/
/******** AUTHOR : Gwenn AUBERT *******/
/******** FILE : Container.hpp ********/
/**************************************/
#ifndef SCE_CONTAINER_HPP
#define SCE_CONTAINER_HPP

#include "SCEDefines.hpp"
#include "Component.hpp"
#include "InternalComponent.hpp"
#include <typeinfo>
//#include <map>
#include <type_traits>

namespace SCE {

    //class Component;//forward declaration of component

    //Container class should not be inherited
    class Container {

    public :

        //Prevent creation of copy constructor
                        Container(const Container&)     = delete;
        //Prevent creation of move Contructor
                         Container(Container&&)          = delete;

                        Container(const std::string& name);
                        ~Container();

        //Only provide template functions for Component derived classes.
        template < class T, class... Args,
                   class = typename std::enable_if<std::is_base_of<Component, T>::value>::type>
        T&                 AddComponent(Args&&... args)
        {
            T* compo = new InternalComponent<T> (*this, args...);
            mComponents.push_back(std::shared_ptr<Component>(compo));
            return *compo;
        }



        template < class T,
                   class = typename std::enable_if<std::is_base_of<Component, T>::value>::type>
        T&                  GetComponent()
        {
            return fetchComponent<T>();
        }



        template < class T,
                   class = typename std::enable_if<std::is_base_of<Component, T>::value>::type>
        const T&                  GetComponent() const
        {
            return fetchComponent<T>();
        }



        template < class T,
                   class = typename std::enable_if<std::is_base_of<Component, T>::value>::type>
        void                RemoveComponent()
        {

        }


        template < class T,
                   class = typename std::enable_if<std::is_base_of<Component, T>::value>::type>
        bool                HasComponent() const
        {
            int typeHash = InternalComponent<T>::sTypeHash;
            auto it = std::find_if(begin(mComponents), end(mComponents),
                                   [&typeHash] (const std::shared_ptr<Component>& compo){
                                        return compo->GetTypeHash() == typeHash;
                                    }
            );
            return it != end(mComponents);
        }

        const std::string&  GetTag() const;
        const std::string&  GetLayer() const;
        void                SetTag(const std::string& tag);
        void                SetLayer(const std::string& layer);

        const std::string&  GetName() const;
        void                SetName(const std::string &name);

    private :


        template < class T,
                   class = typename std::enable_if<std::is_base_of<Component, T>::value>::type>
        T&                      fetchComponent() const
        {
            int typeHash = InternalComponent<T>::sTypeHash;
            auto it = std::find_if(begin(mComponents), end(mComponents),
                                   [&typeHash] (const std::shared_ptr<Component>& compo){
                                        return compo->GetTypeHash() == typeHash;
                                    }
            );
            Debug::Assert(it != end(mComponents), "Could not find component on container : " + mName);
            return *(T*)it->get();
        }


        std::vector<std::shared_ptr<Component> >            mComponents;
        std::string                                         mTag;
        std::string                                         mLayer;
        std::string                                         mName;

    };

}


#endif
