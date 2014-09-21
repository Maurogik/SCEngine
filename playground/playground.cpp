// Include standard headers
#include <stdio.h>
#include <stdlib.h>

#include "../headers/SCECore.hpp"
#include "Rotator.hpp"

using namespace SCE;
using namespace std;

int main( void )
{
    auto engine = SCECore::InitEngine("Playground scene for SCE");

    Scene::CreateEmptyScene();

    //Light
    shared_ptr<Container> lightObject = make_shared<Container>("lightObject");

    lightObject->AddComponent<Light>();
    Transform& lightTransform = lightObject->AddComponent<Transform>();

    lightTransform.SetWorldOrientation(vec3(30, 0, 30));
    lightTransform.SetWorldPosition(vec3(0, 10, 20));


    //Suzanne model
    shared_ptr<Container> suzanneObject = make_shared<Container>("suzanneObject");

    suzanneObject->AddComponent<Material>("TestMaterial");

    Transform& suzanneTransform = suzanneObject->AddComponent<Transform>();
    suzanneTransform.SetWorldPosition(vec3(0, 0, 0));

    suzanneObject->AddComponent<Mesh>("suzanne.obj");
    suzanneObject->AddComponent<MeshRenderer>();
    suzanneObject->AddComponent<Rotator>();

    //Camera
    shared_ptr<Container> cameraObject = make_shared<Container>("cameraObject");
    Transform& cameraTransform = cameraObject->AddComponent<Transform>();
    cameraObject->AddComponent<Camera>(40.0f, 4.0f/3.0f, 0.1f, 100.0f);

    cameraTransform.SetWorldPosition(vec3(0, 0, 12));
    cameraTransform.LookAt(suzanneTransform.GetWorldPosition());

    //load scene here
    engine->RunEngine();

    return 0;
}

