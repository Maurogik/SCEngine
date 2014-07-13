#***************************************#
#***************************************#
#** CMakeLists for Sand Castle Engine **#
#******** Author : Gwenn AUBERT ********#
#***************************************#
#***************************************#

#CMake Debug Line
#-DCMAKE_BUILD_TYPE=Debug



# CMake entry point
cmake_minimum_required (VERSION 2.8)
project (SCEngine)

find_package(OpenGL REQUIRED)


if( CMAKE_BINARY_DIR STREQUAL CMAKE_SOURCE_DIR )
    message( FATAL_ERROR "Please select another Build Directory ! (and give it a clever name, like bin_Visual2012_64bits/)" )
endif()
if( CMAKE_SOURCE_DIR MATCHES " " )
    message( "Your Source Directory contains spaces. If you experience problems when compiling, this can be the cause." )
endif()
if( CMAKE_BINARY_DIR MATCHES " " )
    message( "Your Build Directory contains spaces. If you experience problems when compiling, this can be the cause." )
endif()


# Compile external dependencies
add_subdirectory (external)

# On Visual 2005 and above, this module can set the debug working directory
list(APPEND CMAKE_MODULE_PATH "${CMAKE_SOURCE_DIR}/external/rpavlik-cmake-modules-1c73e35")
include(CreateLaunchers)
include(MSVCMultipleProcessCompile) # /MP

if(INCLUDE_DISTRIB)
    add_subdirectory(distrib)
endif(INCLUDE_DISTRIB)


include_directories(SYSTEM
    external/AntTweakBar-1.16/include/
    external/glfw-3.0.3/include/GLFW/
    external/glm-0.9.4.0/
    external/glew-1.9.0/include/
    external/assimp-3.0.1270/include/
    external/bullet-2.81-rev2613/src/
    external/rapidjson/
    .
)

set(ALL_LIBS
    ${OPENGL_LIBRARY}
    GLFW_303
    GLEW_190
)

add_definitions(
    -DTW_STATIC
    -DTW_NO_LIB_PRAGMA
    -DTW_NO_DIRECT3D
    -DGLEW_STATIC
    -D_CRT_SECURE_NO_WARNINGS
)

set( TARGET playground)


set(HEADERS
        ./headers/SCE.hpp
        ./headers/SCECore.hpp
        ./headers/SCEDebug.hpp
        ./headers/Material.hpp
        ./headers/Mesh.hpp
        ./headers/MeshRenderer.hpp
        ./headers/Scene.hpp
        ./headers/SceneDescriptor.hpp
        ./headers/Transform.hpp
        ./headers/GameObject.hpp
        ./headers/Camera.hpp
        ./headers/Light.hpp
        ./headers/Container.hpp
        ./headers/Component.hpp
        ./headers/SCEMacros.hpp
        ./headers/SCEDefines.hpp
        ./headers/SCETools.hpp
        ./headers/SCETime.hpp
)


aux_source_directory(./sources SOURCES)
aux_source_directory(./common COMMONS)
aux_source_directory(${TARGET} APP_SOURCES)

# User playground
add_executable(${TARGET}
    ${APP_SOURCES}

    ${HEADERS}
    ${SOURCES}
    ${COMMONS}
)
target_link_libraries(${TARGET}
    ${ALL_LIBS}
)

#set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -Wno-long-long -Wno-variadic-macros -Wunused-local-typedefs -pedantic")
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -Wno-long-long -Wno-variadic-macros -pedantic")

#set_source_files_properties(
#  ${HEADERS}
#  PROPERTIES
#  COMPILE_FLAGS "${FLAGS}"
#)

#set_source_files_properties(
#  ${SOURCES}
#  PROPERTIES
#  COMPILE_FLAGS "${FLAGS}"
#)

#set_source_files_properties(
#  ${APP_SOURCES}
#  PROPERTIES
#  COMPILE_FLAGS "${FLAGS}"
#)

# Xcode and Visual working directories
set_target_properties(${TARGET} PROPERTIES XCODE_ATTRIBUTE_CONFIGURATION_BUILD_DIR "${CMAKE_CURRENT_SOURCE_DIR}/${TARGET}/")
create_target_launcher(${TARGET} WORKING_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}/${TARGET}/")

message("${CMAKE_CURRENT_SOURCE_DIR}/playground/")


SOURCE_GROUP(common REGULAR_EXPRESSION ".*/common/.*" )
SOURCE_GROUP(shaders REGULAR_EXPRESSION ".*/.*shader*" )


if (NOT ${CMAKE_GENERATOR} MATCHES "Xcode" )
add_custom_command(
   TARGET ${TARGET} POST_BUILD
   COMMAND ${CMAKE_COMMAND} -E copy "${CMAKE_CURRENT_BINARY_DIR}/${CMAKE_CFG_INTDIR}/${TARGET}${CMAKE_EXECUTABLE_SUFFIX}" "${CMAKE_CURRENT_SOURCE_DIR}/${TARGET}/"
)


elseif (${CMAKE_GENERATOR} MATCHES "Xcode" )

endif (NOT ${CMAKE_GENERATOR} MATCHES "Xcode" )

file( COPY "${TARGET}/ressources" DESTINATION "." )


#file(GLOB ressources "${TARGET}/ressources/*")

#foreach(file ${ressources})

#endforeach()

#add_custom_command(TARGET ${TARGET} POST_BUILD
#    COMMAND ${CMAKE_COMMAND} -E copy_directory
#        "${CMAKE_CURRENT_SOURCE_DIR}/playground/ressources"
#        "$<TARGET_FILE_DIR:playground>/ressources"
#        DEPENDS ${CMAKE_CURRENT_SOURCE_DIR}/playground/ressources ${CMAKE_CURRENT_SOURCE_DIR}/foo.hpp
#)

# ADD_CUSTOM_COMMAND(TARGET ${TARGET} POST_BUILD
#    COMMAND ${CMAKE_COMMAND} -E copy_directory
#        "${CMAKE_CURRENT_SOURCE_DIR}/${TARGET}/ressources"
#        "$<TARGET_FILE_DIR:playground>/ressources"
#    DEPENDS "${CMAKE_CURRENT_SOURCE_DIR}/${TARGET}/ressources/*"
#)

# ADD_CUSTOM_TARGET(copy_ressources
#    DEPENDS "${CMAKE_CURRENT_SOURCE_DIR}/${TARGET}/ressources/*"
#)
