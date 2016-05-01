/******PROJECT:Sand Castle Engine******/
/**************************************/
/*********AUTHOR:Gwenn AUBERT**********/
/**********FILE:SCEDebug.hpp***********/
/**************************************/
#ifndef SCE_SCEDEBUG_HPP
#define SCE_SCEDEBUG_HPP

#ifdef SCE_DEBUG_ENGINE
namespace SCE
{
    namespace Debug
    {
        bool    ToggleDebugMenu();
        void    UpdateDebugMenu();
    }
}
#endif

#endif
