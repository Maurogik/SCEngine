// Include standard headers
#include <stdio.h>
#include <stdlib.h>

#include "../headers/SCECore.hpp"
#include "Rotator.hpp"

using namespace SCE;

int main( void )
{
    SCECore::InitEngine("Playground scene for SCE");

    SCE_DEBUG_LOG("Test logger");
    SCE_DEBUG_LOG("Test logger %d", 2);

    Scene::CreateEmptyScene();

    Material* mat = Material::LoadMaterial("TestMaterial");

    Container* suzanneObject    = new Container();

    Transform* suzanneTransform = new Transform();
    suzanneTransform->SetWorldPosition(vec3(0, 0, 0));
    Mesh* suzanneMesh           = Mesh::LoadMesh("suzanne.obj" );
    MeshRenderer* renderer      = new MeshRenderer();
    Rotator *rotator            = new Rotator();

    ADD_COMPONENT_TO(suzanneObject, Material, mat);
    ADD_COMPONENT_TO(suzanneObject, Transform, suzanneTransform);
    ADD_COMPONENT_TO(suzanneObject, Mesh, suzanneMesh);
    ADD_COMPONENT_TO(suzanneObject, MeshRenderer, renderer);
    ADD_COMPONENT_TO(suzanneObject, Rotator, rotator)


    Container* cameraObject     = new Container();

    Camera* camera              = new Camera(40.0f, 4.0f/3.0f, 0.1f, 100.0f);
    Transform* cameraTransform  = new Transform();
    cameraTransform->SetWorldPosition(vec3(0, 0, 12));

    cameraTransform->LookAt(suzanneTransform->GetWorldPosition());
    ADD_COMPONENT_TO(cameraObject, Transform, cameraTransform);
    ADD_COMPONENT_TO(cameraObject, Camera, camera);

    //load scene here
    SCECore::RunEngine();

    SCECore::CleanUpEngine();

    return 0;
}

