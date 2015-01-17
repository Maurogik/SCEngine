TEMPLATE = app
CONFIG += console
CONFIG -= app_bundle
CONFIG -= qt

SOURCES += \
    ../../sources/Camera.cpp \
    ../../sources/Component.cpp \
    ../../sources/Container.cpp \
    ../../sources/GameObject.cpp \
    ../../sources/Light.cpp \
    ../../sources/Material.cpp \
    ../../sources/Mesh.cpp \
    ../../sources/MeshRenderer.cpp \
    ../../sources/SCE_GLDebug.cpp \
    ../../sources/SCECore.cpp \
    ../../sources/SCEInternal.cpp \
    ../../sources/Scene.cpp \
    ../../sources/SCETime.cpp \
    ../../sources/SCETools.cpp \
    ../../sources/Transform.cpp \
    ../../templates/Container.tpp \
    ../../templates/Handle.tpp

include(deployment.pri)
qtcAddDeployment()

HEADERS += \
    ../../headers/Camera.hpp \
    ../../headers/Component.hpp \
    ../../headers/Container.hpp \
    ../../headers/GameObject.hpp \
    ../../headers/Handle.hpp \
    ../../headers/HandleTarget.hpp \
    ../../headers/InternalComponent.hpp \
    ../../headers/Light.hpp \
    ../../headers/Material.hpp \
    ../../headers/Mesh.hpp \
    ../../headers/MeshRenderer.hpp \
    ../../headers/SCE.hpp \
    ../../headers/SCE_GLDebug.hpp \
    ../../headers/SCECore.hpp \
    ../../headers/SCEDefines.hpp \
    ../../headers/SCEEvent.hpp \
    ../../headers/SCEInternal.hpp \
    ../../headers/Scene.hpp \
    ../../headers/SceneDescriptor.hpp \
    ../../headers/SCETime.hpp \
    ../../headers/SCETools.hpp \
    ../../headers/Transform.hpp \
    ../../playground/Rotator.hpp


win32:CONFIG(release, debug|release): LIBS += -L$$PWD/../../external/SOIL/release/ -lSOIL
else:win32:CONFIG(debug, debug|release): LIBS += -L$$PWD/../../external/SOIL/debug/ -lSOIL
else:unix: LIBS += -L$$PWD/../../external/SOIL/ -lSOIL

INCLUDEPATH += $$PWD/../../external/SOIL
DEPENDPATH += $$PWD/../../external/SOIL

win32-g++:CONFIG(release, debug|release): PRE_TARGETDEPS += $$PWD/../../external/SOIL/release/libSOIL.a
else:win32-g++:CONFIG(debug, debug|release): PRE_TARGETDEPS += $$PWD/../../external/SOIL/debug/libSOIL.a
else:win32:!win32-g++:CONFIG(release, debug|release): PRE_TARGETDEPS += $$PWD/../../external/SOIL/release/SOIL.lib
else:win32:!win32-g++:CONFIG(debug, debug|release): PRE_TARGETDEPS += $$PWD/../../external/SOIL/debug/SOIL.lib
else:unix: PRE_TARGETDEPS += $$PWD/../../external/SOIL/libSOIL.a

OTHER_FILES +=
