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

    Mesh::AddSphereMesh(object, 1.0f, tesselation);
    object->AddComponent<MeshRenderer>();
    object->AddComponent<Rotator>();

    return object;
}

SCEHandle<Container> createCone(const string& name, const float& tesselation,
                                float angle, float length, const vec3& pos){
    //cube model
    SCEHandle<Container> object = SCEScene::CreateContainer(name);

    object->AddComponent<Material>(MATERIAL);

    SCEHandle<Transform> transform = object->AddComponent<Transform>();
    transform->SetWorldPosition(pos);

    Mesh::AddConeMesh(object, length, angle, tesselation);
    object->AddComponent<MeshRenderer>();
    object->AddComponent<Rotator>();

    return object;
}

SCEHandle<Container> createCube(const string& name, bool rotator, const vec3& pos){
    //cube model
    SCEHandle<Container> object = SCEScene::CreateContainer(name);

    object->AddComponent<Material>(MATERIAL);

    SCEHandle<Transform> transform = object->AddComponent<Transform>();
    transform->SetWorldPosition(pos);

    Mesh::AddCubeMesh(object, 1.0f);
    object->AddComponent<MeshRenderer>();
    if(rotator)
    {
        object->AddComponent<Rotator>();
    }

    return object;
}

SCEHandle<Container> createPlane(const string& name, float size, const vec3& pos){
    //cube model
    SCEHandle<Container> object = SCEScene::CreateContainer(name);

    object->AddComponent<Material>(MATERIAL);

    SCEHandle<Transform> transform = object->AddComponent<Transform>();
    transform->SetWorldPosition(pos);

    Mesh::AddQuadMesh(object, size, size);
    object->AddComponent<MeshRenderer>();

    return object;
}

SCEHandle<Container> createModel(const string& objectName, const string& filename, const vec3& pos){
    SCEHandle<Container> object = SCEScene::CreateContainer(objectName);

    object->AddComponent<Material>(MATERIAL);

    SCEHandle<Transform> transform = object->AddComponent<Transform>();
    transform->SetWorldPosition(pos);
    object->AddComponent<Mesh>(filename);
    object->AddComponent<MeshRenderer>();
    object->AddComponent<Rotator>();

    return object;
}

SCEHandle<Container> createLight(vec3 pos, vec3 orientation, LightType type){
    //Light
    SCEHandle<Container> lightObject = SCEScene::CreateContainer("lightObject");

    SCEHandle<Transform> lightTransform = lightObject->AddComponent<Transform>();
    SCEHandle<Light> light = lightObject->AddComponent<Light>(type);

    lightTransform->SetWorldOrientation(orientation);
    lightTransform->SetWorldPosition(pos);

    return lightObject;
}

SCEHandle<Container> createLightSphere(vec3 pos, vec4 color){

    SCEHandle<Container> light =createLight(pos, vec3(0, 0, 0), LightType::POINT_LIGHT);
    light->GetComponent<Light>()->SetLightColor(color);
    light->AddComponent<Material>(MATERIAL);

    Mesh::AddSphereMesh(light, 1.0f, 3.0f);
    light->AddComponent<MeshRenderer>();

    return light;
}

int main( void )
{
    SCECore engine;
    engine.InitEngine("Playground scene for SCE");

    SCEScene::CreateEmptyScene();

    SCEHandle<Container> dirLight = createLight(vec3(0, 100, 0),
                                                vec3(40, 0, 0),
                                                LightType::DIRECTIONAL_LIGHT);
    dirLight->GetComponent<Light>()->SetLightColor(vec4(0.8, 0.8, 1.0, 0.2));
    dirLight->GetComponent<Light>()->SetCastShadow(true);

//    SCEHandle<Container> light1 = createLight(vec3(2, 3, -1), vec3(30, 0, 30), LightType::POINT_LIGHT);
//    light1->GetComponent<Light>()->SetLightColor(vec4(1.0, 0.0, 0.0, 1.0));
//    light1->GetComponent<Light>()->SetLightReach(2.0f);

//    SCEHandle<Container> light2 = createLight(vec3(-5, 10, -5), vec3(60, 0, 0), LightType::SPOT_LIGHT);
//    light2->GetComponent<Light>()->SetLightReach(40.0f);
//    light2->GetComponent<Light>()->SetLightMaxAngle(45.0f);
//    light2->GetComponent<Light>()->SetLightColor(vec4(0.0, 1.0, 0.0, 1.0));


    //Suzanne model
    SCEHandle<Container> suz = createModel("suzanneObject", "suzanne.obj", vec3(0, 3, 0));
    suz->GetComponent<Transform>()->RotateAroundAxis(vec3(0.0f, 1.0f, 0.0f), 180.0f);

    float spreadDist = 10.0f;
    float nbSpheres = 2;
    for(float x = 0.0f; x < nbSpheres * 2.0f; ++x)
    {
        for(float z = -nbSpheres; z < nbSpheres; ++z)
        {
            createSphere("sphereObject", 4, vec3(x * spreadDist, 0.0f, z * spreadDist));

            createCube("cubeObject", true, vec3(-x * spreadDist, 0.0f, z * spreadDist));
        }
    }

    SCEHandle<Container> wallObj = createCube("cubeObject", false, vec3(0.0f, 3.0f, 5.0f));
    SCEHandle<Transform> wallTransform = wallObj->GetComponent<Transform>();
    wallTransform->SetLocalScale(vec3(10.0f, 10.0f, 2.0f));

//    createCone("coneObject", 2, 45.0f, 5.0f, vec3(0.0f, 2.0f, 3.0f));

//    SCEHandle<Container> cone = createCone("coneObject", 4, 45.0f, 20.0f, vec3(-5, 10, 0));
//    cone->GetComponent<Transform>()->SetWorldOrientation(vec3(45, 0, 0));

    SCEHandle<Container> plane = createPlane("plane", 150.0f, vec3(0.0f, -2.0f, 2.0f));
    plane->GetComponent<Transform>()->SetWorldOrientation(vec3(-90.0f, 180.0f, 0.0f));

    //Camera
    SCEHandle<Container> cameraObject = SCEScene::CreateContainer("cameraObject");
    SCEHandle<Transform> cameraTransform = cameraObject->AddComponent<Transform>();
    cameraObject->AddComponent<Camera>(40.0f, 4.0f/3.0f, 1.0f, 100.0f);
    cameraTransform->SetWorldPosition(vec3(0, 10, -25));
    cameraTransform->RotateAroundAxis(vec3(1.0f, 0.0f, 0.0f), 90.0f);

    //load scene here
    engine.RunEngine();

    return 0;
}

