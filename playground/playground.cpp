// Include standard headers
#include <stdio.h>
#include <stdlib.h>

#include "../headers/SCECore.hpp"
#include "Rotator.hpp"
#include "LookAtTarget.hpp"

using namespace SCE;
using namespace std;

#define TEST_MAT "TestMaterial"
#define DEBUG_MAT "Debug" //"TestMaterial"
#define MATERIAL "TestMaterial"

SCEHandle<Container> createSphere(const string& name, const float& tesselation, const vec3& pos){
    //cube model
    SCEHandle<Container> object = SCEScene::CreateContainer(name);

    object->AddComponent<Material>(MATERIAL);

    SCEHandle<Transform> transform = object->AddComponent<Transform>();
    transform->SetWorldPosition(pos);

    //Mesh::AddCubeMesh(cubeObject, 1.0f);
    Mesh::AddSphereMesh(object, 1.0f, tesselation);
    object->AddComponent<MeshRenderer>();
    object->AddComponent<Rotator>();

    return object;
}

SCEHandle<Container> createCone(const string& name, const float& tesselation, const vec3& pos){
    //cube model
    SCEHandle<Container> object = SCEScene::CreateContainer(name);

    object->AddComponent<Material>(MATERIAL);

    SCEHandle<Transform> transform = object->AddComponent<Transform>();
    transform->SetWorldPosition(pos);

    //Mesh::AddCubeMesh(cubeObject, 1.0f);
    Mesh::AddConeMesh(object, 1.0f, 45.0f, tesselation);
    object->AddComponent<MeshRenderer>();

    return object;
}

void createModel(const string& objectName, const string& filename, const vec3& pos){
    SCEHandle<Container> object = SCEScene::CreateContainer(objectName);

    object->AddComponent<Material>(MATERIAL);

    SCEHandle<Transform> transform = object->AddComponent<Transform>();
    transform->SetWorldPosition(pos);
    object->AddComponent<Mesh>(filename);
    object->AddComponent<MeshRenderer>();
    object->AddComponent<Rotator>();
}

SCEHandle<Container> createLight(vec3 pos, vec3 orientation, LightType type){
    //Light
    SCEHandle<Container> lightObject = SCEScene::CreateContainer("lightObject");
    lightObject->SetLayer("Light");

    SCEHandle<Transform> lightTransform = lightObject->AddComponent<Transform>();
    lightObject->AddComponent<Light>(type);

    lightTransform->SetWorldOrientation(orientation);
    lightTransform->SetWorldPosition(pos);

    return lightObject;
}

SCEHandle<Container> createLightSphere(vec3 pos, vec4 color){

    SCEHandle<Container> light =createLight(pos, vec3(0, 0, 0), LightType::POINT_LIGHT);
    light->GetComponent<Light>()->SetLightColor(color);
    light->AddComponent<Material>(MATERIAL);

    //Mesh::AddCubeMesh(cubeObject, 1.0f);
    Mesh::AddSphereMesh(light, 1.0f, 3);
    light->AddComponent<MeshRenderer>();

    return light;
}

int main( void )
{
    SCECore engine;
    engine.InitEngine("Playground scene for SCE");

    SCEScene::CreateEmptyScene();

    //createLight(vec3(4, 4, 1), vec3(30, 0, 30), LightType::DIRECTIONAL_LIGHT);
    SCEHandle<Container> light1 = createLight(vec3(4, 4, 1), vec3(30, 0, 30), LightType::POINT_LIGHT);
    //SCEHandle<Container> light2 = createLight(vec3(-4, 4, 1), vec3(30, 0, 30), LightType::POINT_LIGHT);
    light1->GetComponent<Light>()->SetLightReach(2.0f);
    //light2->GetComponent<Light>()->SetLightReach(2.0f);

    //SCEHandle<Container> light = createLightSphere(vec3(4, 4, 1), vec4(1, 1, 1, 1));
    //SCEHandle<Container> light2 = createLightSphere(vec3(-4, 4, 1), vec4(0, 0, 1, 1));

    //Suzanne model
    createModel("suzanneObject", "suzanne.obj", vec3(0, 0, 0));
    //createCone("coneObject", 3.0f, vec3(0, 0, 0));
    int startX = -4;
    for(int i = 0; i < 5; ++i){
        createSphere("sphereObject", i+2, vec3(startX + i * 5, -i-3, 0));
    }

    //Camera
    SCEHandle<Container> cameraObject = SCEScene::CreateContainer("cameraObject");
    SCEHandle<Transform> cameraTransform = cameraObject->AddComponent<Transform>();
    cameraObject->AddComponent<Camera>(40.0f, 4.0f/3.0f, 0.1f, 100.0f);
    //cameraObject->AddComponent<LookAtTarget>();

    cameraTransform->SetWorldPosition(vec3(0, 0, 12));
    cameraTransform->RotateAroundAxis(vec3(0.0f, 1.0f, 0.0f), 180.0f);

    //load scene here
    engine.RunEngine();

    return 0;
}

