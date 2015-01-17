/******PROJECT:Sand Castle Engine******/
/**************************************/
/*********AUTHOR:Gwenn AUBERT**********/
/***********FILE:SCEHandle.tpp************/
/**************************************/




template <class T>
SCE::SCEHandle<T>::SCEHandle() : mTarget(0l) {}

template <class T>
SCE::SCEHandle<T>::SCEHandle (SCE::SCEHandleTarget* target)
    : mTarget (target)
{
    if(mTarget != 0l){
        mDestroyedDelegate.connect(this, &SCE::SCEHandle<T>::invalidateTarget, mTarget->mInvalidateHandleEvent);
    }
}

template <class T>
SCE::SCEHandle<T>::SCEHandle (SCE::SCEHandle<T>& SCEHandle)
    : mTarget(SCEHandle.mTarget)
{
    if(mTarget != 0l){
        mDestroyedDelegate.connect(this, &SCE::SCEHandle<T>::invalidateTarget, mTarget->mInvalidateHandleEvent);
    }
}

template <class T>
SCE::SCEHandle<T>::SCEHandle (const SCE::SCEHandle<T>& SCEHandle)
    : mTarget(SCEHandle.mTarget)
{
    if(mTarget != 0l){
        mDestroyedDelegate.connect(this, &SCE::SCEHandle<T>::invalidateTarget, mTarget->mInvalidateHandleEvent);
    }
}

template <class T>
SCE::SCEHandle<T>::SCEHandle (SCE::SCEHandle<T>&& SCEHandle)
    : mTarget(SCEHandle.mTarget)
{
    if(mTarget != 0l){
        mDestroyedDelegate.connect(this, &SCE::SCEHandle<T>::invalidateTarget, mTarget->mInvalidateHandleEvent);
    }
}

template <class T>
SCE::SCEHandle<T>& SCE::SCEHandle<T>::operator= (const SCE::SCEHandle<T>& rhs)
{
    mTarget = rhs.mTarget;
    //disconnect from previous target
    mDestroyedDelegate.disconnectAll();
    if(mTarget != 0l){
        mDestroyedDelegate.connect(this, &SCE::SCEHandle<T>::invalidateTarget, mTarget->mInvalidateHandleEvent);
    }
    return *this;
}

template <class T>
SCE::SCEHandle<T>& SCE::SCEHandle<T>::operator= (SCE::SCEHandleTarget* rhs)
{
    mTarget = rhs;
    //disconnect from previous target
    mDestroyedDelegate.disconnectAll();
    if(mTarget != 0l){
        mDestroyedDelegate.connect(this, &SCE::SCEHandle<T>::invalidateTarget, mTarget->mInvalidateHandleEvent);
    }
    return *this;
}

template <class T>
T*                      SCE::SCEHandle<T>::operator->()
{
    return (T*)mTarget;
}

template <class T>
const T*                SCE::SCEHandle<T>::operator->() const
{
    return (T*)mTarget;
}

template<class T>
SCE::SCEHandle<T>::operator bool() const
{
    return mTarget != 0l;
}

template <class T>
bool SCE::SCEHandle<T>::operator== (const SCE::SCEHandle<T>& rhs)
{
    return mTarget == rhs.mTarget;
}

template <class T>
bool SCE::SCEHandle<T>::operator!= (const SCE::SCEHandle<T>& rhs)
{
    return mTarget != rhs.mTarget;
}

template <class T>
void                    SCE::SCEHandle<T>::invalidateTarget() {
    mTarget = 0l;
}
