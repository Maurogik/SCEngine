/******PROJECT:Sand Castle Engine******/
/**************************************/
/*********AUTHOR:Gwenn AUBERT**********/
/***********FILE:Handle.tpp************/
/**************************************/




template <class T>
SCE::Handle<T>::Handle() : mTarget(0l) {}

template <class T>
SCE::Handle<T>::Handle (SCE::HandleTarget* target)
    : mTarget (target)
{
    if(mTarget != 0l){
        mDestroyedDelegate.connect(this, &SCE::Handle<T>::invalidateTarget, mTarget->mInvalidateHandleEvent);
    }
}

template <class T>
SCE::Handle<T>::Handle (SCE::Handle<T>& handle)
    : mTarget(handle.mTarget)
{
    if(mTarget != 0l){
        mDestroyedDelegate.connect(this, &SCE::Handle<T>::invalidateTarget, mTarget->mInvalidateHandleEvent);
    }
}

template <class T>
SCE::Handle<T>::Handle (const SCE::Handle<T>& handle)
    : mTarget(handle.mTarget)
{
    if(mTarget != 0l){
        mDestroyedDelegate.connect(this, &SCE::Handle<T>::invalidateTarget, mTarget->mInvalidateHandleEvent);
    }
}

template <class T>
SCE::Handle<T>::Handle (SCE::Handle<T>&& handle)
    : mTarget(handle.mTarget)
{
    if(mTarget != 0l){
        mDestroyedDelegate.connect(this, &SCE::Handle<T>::invalidateTarget, mTarget->mInvalidateHandleEvent);
    }
}

template <class T>
SCE::Handle<T>& SCE::Handle<T>::operator= (const SCE::Handle<T>& rhs)
{
    mTarget = rhs.mTarget;
    //disconnect from previous target
    mDestroyedDelegate.disconnectAll();
    if(mTarget != 0l){
        mDestroyedDelegate.connect(this, &SCE::Handle<T>::invalidateTarget, mTarget->mInvalidateHandleEvent);
    }
    return *this;
}

template <class T>
SCE::Handle<T>& SCE::Handle<T>::operator= (SCE::HandleTarget* rhs)
{
    mTarget = rhs;
    //disconnect from previous target
    mDestroyedDelegate.disconnectAll();
    if(mTarget != 0l){
        mDestroyedDelegate.connect(this, &SCE::Handle<T>::invalidateTarget, mTarget->mInvalidateHandleEvent);
    }
    return *this;
}

template <class T>
T*                      SCE::Handle<T>::operator->()
{
    return (T*)mTarget;
}

template <class T>
const T*                SCE::Handle<T>::operator->() const
{
    return (T*)mTarget;
}

template<class T>
SCE::Handle<T>::operator bool() const
{
    return mTarget != 0l;
}

template <class T>
bool SCE::Handle<T>::operator== (const SCE::Handle<T>& rhs)
{
    return mTarget == rhs.mTarget;
}

template <class T>
bool SCE::Handle<T>::operator!= (const SCE::Handle<T>& rhs)
{
    return mTarget != rhs.mTarget;
}

template <class T>
void                    SCE::Handle<T>::invalidateTarget() {
    mTarget = 0l;
}
