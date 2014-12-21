// Include standard headers
#include <stdio.h>
#include <stdlib.h>

#include "../headers/SCECore.hpp"
#include "Rotator.hpp"

using namespace SCE;
using namespace std;

void createSphere(const string& name, const ushort& tesselation, const vec3& pos){
    //cube model
    Handle<Container> object = Scene::CreateContainer(name);

    object->AddComponent<Material>("TestMaterial");

    Handle<Transform> transform = object->AddComponent<Transform>();
    transform->SetWorldPosition(pos);

    //Mesh::AddCubeMesh(cubeObject, 1.0f);
    Mesh::AddSphereMesh(object, 1.0f, tesselation);
    object->AddComponent<MeshRenderer>();
    object->AddComponent<Rotator>();
}

void createModel(const string& objectName, const string& filename, const vec3& pos){
    Handle<Container> object = Scene::CreateContainer(objectName);

    object->AddComponent<Material>("TestMaterial");

    Handle<Transform> transform = object->AddComponent<Transform>();
    transform->SetWorldPosition(pos);

    object->AddComponent<Mesh>(filename);
    object->AddComponent<MeshRenderer>();
    object->AddComponent<Rotator>();
}

int main( void )
{
    SCECore engine;
    engine.InitEngine("Playground scene for SCE");

    Scene::CreateEmptyScene();

    //Light
    Handle<Container> lightObject = Scene::CreateContainer("lightObject");
    Handle<Container> cont = Scene::CreateContainer("toto");

    lightObject->AddComponent<Light>();
    Handle<Transform> lightTransform = lightObject->AddComponent<Transform>();

    lightTransform->SetWorldOrientation(vec3(30, 0, 30));
    lightTransform->SetWorldPosition(vec3(0, 10, 20));


    //Suzanne model
    //createModel("suzanneObject", "suzanne.obj", vec3(1, 0, 0));

    int startX = -4;
    for(int i = 0; i < 5; ++i){
        createSphere("sphereObject", i, vec3(startX + i * 2, 0, 0));
    }

    //Camera
    Handle<Container> cameraObject = Scene::CreateContainer("cameraObject");
    Handle<Transform> cameraTransform = cameraObject->AddComponent<Transform>();
    cameraObject->AddComponent<Camera>(40.0f, 4.0f/3.0f, 0.1f, 100.0f);

    cameraTransform->SetWorldPosition(vec3(0, 0, 12));
    cameraTransform->LookAt(vec3(0, 0, 0));

    //load scene here
    engine.RunEngine();

    return 0;
}

