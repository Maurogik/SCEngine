/**********Sand Castle Engine**********/
/**************************************/
/******** AUTHOR : Gwenn AUBERT *******/
/******** FILE : Container.cpp ********/
/**************************************/

//template implementation

template < class T, class... Args >
SCE::SCEHandle<T>          SCE::Container::AddComponent(Args&&... args)
{
    T* compo = new SCEInternalComponent<T> (SCEHandle<Container>(this), args...);
    mComponents.push_back(compo);
    return SCEHandle<T>((SCEHandleTarget*) compo);
}

template < class T >
SCE::SCEHandle<T>          SCE::Container::GetComponent()
{
    return fetchComponent<T>();
}

template < class T >
const SCE::SCEHandle<T>    SCE::Container::GetComponent() const
{
    return fetchComponent<T>();
}

template < class T >
void                    SCE::Container::RemoveComponent()
{

}

template < class T >
bool                    SCE::Container::HasComponent() const
{
    int typeHash = SCEInternalComponent<T>::sTypeHash;
    auto it = std::find_if(
                  begin(mComponents)
                , end(mComponents)
                , [&typeHash] (const Component* compo){ return compo->GetTypeHash() == typeHash; }
    );
    return it != end(mComponents);
}

template < class T >
SCE::SCEHandle<T>          SCE::Container::fetchComponent() const
{
    int typeHash = SCEInternalComponent<T>::sTypeHash;
    auto it = std::find_if(
                  begin(mComponents)
                , end(mComponents)
                , [&typeHash] (const Component* compo) { return compo->GetTypeHash() == typeHash; }
    );
    //Debug::Assert(it != end(mComponents), "Could not find component on container : " + mName + ", check if component exists with HasComponent");
    T* resultPtr = (T*)*it;
    if(it == end(mComponents))
    {
        resultPtr = 0l;
        SCE::SCEInternal::InternalMessage("Could not find component on container : " + mName );

    }
    return SCEHandle<T>(resultPtr);
}
