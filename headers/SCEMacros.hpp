/**********Sand Castle Engine**********/
/**************************************/
/******** AUTHOR : Gwenn AUBERT *******/
/********* FILE : SCEMacro.hpp ********/
/**************************************/
#ifndef SCE_MACROS_HPP
#define SCE_MACROS_HPP

#include "SCEDefines.hpp"

#include <assert.h>
#include <stdio.h>


/**
Variadic macros : macros with a variable list of argument
ex : #define errPrint(format, ...) <- varaible list of args
Use __VA_ARGS__ to get the arguments
To remove a preceding comma use [before], ## __VA_ARGS__ [after]
ex : printf(format, ## __VA_ARGS__);
**/

#define SCE_LOG(...) \
    fflush(stdout); \
    printf(__VA_ARGS__); \
    printf("\n");\
    fflush(stdout); \

#ifdef SCE_DEBUG_ENGINE
#define SCE_DEBUG_LOG(...) \
    fflush(stdout); \
    printf(__VA_ARGS__); \
    printf("\n");\
    fflush(stdout); \

#else
    #define SCE_DEBUG_LOG(...) \

#endif

#define SECURE_DELETE(obj) \
    if((obj)) { \
        delete((obj)); \
        (obj) = 0l; \
    }\

#define SECURE_DELETE_ARRAY(obj) \
    if((obj)) { \
        delete[]((obj)); \
        (obj) = 0l; \
    }\

#define SECURE_EXEC(obj, call) \
    if((obj)) { \
        (obj)->call; \
    } \


#ifdef SCE_DEBUG
    #define SCE_ASSERT(condition, ...) \
        if(!(condition)) { \
            std::cerr << "Assertion `" #condition "` failed in " << __FILE__ \
                      << " line " << __LINE__ << std::endl; \
            fflush(stdout); \
            printf(__VA_ARGS__); \
            printf("\n");\
            fflush(stdout); \
            abort();\
        }
#else
    #define SCE_ASSERT(condition,...) do { } while (false)
#endif


#ifdef SCE_DEBUG
    #define SCE_ERROR(...) \
        fflush(stdout); \
        printf(__VA_ARGS__); \
        printf("\n");\
        fflush(stdout); \
        abort();
#else
    #define SCE_ERROR(...) do { } while (false)
#endif


#define STRINGIFY(val) \
     #val

#define REQUIRE_COMPONENT(componentName) \
    class componentName;\

#define GET_COMPONENT(componentName) \
    (componentName*)(GetContainer()->GetComponent(#componentName)); \

#define GET_COMPONENT_FROM(objPtr, componentName) \
    (componentName*)((objPtr)->GetComponent(#componentName)); \

#define ADD_COMPONENT_TO(objPtr, componentName, componentPtr) \
    (objPtr)->AddComponent(#componentName, (componentPtr)); \
    (componentPtr)->SetContainer(objPtr); \

#define ADD_NEW_COMPONENT_TO(objPtr, componentName, ...) \
    componentName* compo = new componentName(__VA_ARGS__); \
    (objPtr)->AddComponent(#componentName, compo); \
    compo->SetContainer(objPtr); \

#define REMOVE_COMPONENT_FROM(objPtr, componentName) \
    (objPtr)->RemoveComponent(#componentName); \



/**

  Component principles :
    A container can have (any number of ?) components of any types
    A component is stored in the container in an hashMap
        ex : hashMap["transform"] = transformPtr

    A component can be added to an object in two ways
        1 : ADD_COMPONENT(objPtr, componentPtr)
        2 : ADD_NEW_COMPONENT(objPtr, componentType, componentArguments
    Adding a component to an object also tells the component which object it belongs to

    A component can access other components of the same object with :
        GET_COMPONENT(componentName) which return the component's pointer

    Components can be accessed from outside with :
        GET_COMPONENT_FROM(objPtr, componentName)
 **/


#endif
