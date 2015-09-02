// Include standard headers
#include <stdio.h>
#include <stdlib.h>

#include "../headers/SCECore.hpp"
#include "Rotator.hpp"
#include "LookAtTarget.hpp"

using namespace SCE;
using namespace std;

#define MATERIAL "Materials/TestMaterial"
#define GROUND_MATERIAL "Materials/GroundMaterial"
#define WALL_MATERIAL "Materials/WallMaterial"

SCEHandle<Container> createSphere(const string& name, const float& tesselation, const vec3& pos){
    //cube model
    SCEHandle<Container> object = SCEScene::CreateContainer(name);

    object->AddComponent<Material>(MATERIAL);

    SCEHandle<Transform> transform = object->AddComponent<Transform>();
    transform->SetWorldPosition(pos);

    Mesh::AddSphereMesh(object, tesselation);
    object->AddComponent<MeshRenderer>();    

    return object;
}

SCEHandle<Container> createCone(const string& name, const float& tesselation,
                                float angle, float length, const vec3& pos){
    //cube model
    SCEHandle<Container> object = SCEScene::CreateContainer(name);

    object->AddComponent<Material>(MATERIAL);

    SCEHandle<Transform> transform = object->AddComponent<Transform>();
    transform->SetWorldPosition(pos);

    Mesh::AddConeMesh(object, angle, tesselation);
    object->AddComponent<MeshRenderer>();

    return object;
}

SCEHandle<Container> createCube(const string& name, const vec3& pos,
                                const string& material){
    //cube model
    SCEHandle<Container> object = SCEScene::CreateContainer(name);

    object->AddComponent<Material>(material);

    SCEHandle<Transform> transform = object->AddComponent<Transform>();
    transform->SetWorldPosition(pos);

    Mesh::AddCubeMesh(object);
    object->AddComponent<MeshRenderer>();

    return object;
}

SCEHandle<Container> createPlane(const string& name, float size, const vec3& pos){
    //cube model
    SCEHandle<Container> object = SCEScene::CreateContainer(name);

    object->AddComponent<Material>(GROUND_MATERIAL);

    SCEHandle<Transform> transform = object->AddComponent<Transform>();
    transform->SetWorldPosition(pos);
    transform->SetLocalScale(vec3(size, size, 1.0f));

    Mesh::AddQuadMesh(object);
    object->AddComponent<MeshRenderer>();

    return object;
}

SCEHandle<Container> createModel(const string& objectName, const string& filename, const vec3& pos,
                                 bool windCW = false){
    SCEHandle<Container> object = SCEScene::CreateContainer(objectName);

    object->AddComponent<Material>(MATERIAL);

    SCEHandle<Transform> transform = object->AddComponent<Transform>();
    transform->SetWorldPosition(pos);
    object->AddComponent<Mesh>(filename, windCW);
    object->AddComponent<MeshRenderer>();

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

void scene1()
{
    SCEHandle<Container> wallObj = createCube("cubeObject", vec3(0.0f, 3.0f, 5.0f),
                                              WALL_MATERIAL);
    SCEHandle<Transform> wallTransform = wallObj->GetComponent<Transform>();
    wallTransform->SetLocalScale(vec3(10.0f, 10.0f, 2.0f));
    wallTransform->RotateAroundAxis(vec3(0.0, 0.0, 1.0), 90.0);


    SCEHandle<Container> plane = createPlane("plane", 500.0f, vec3(0.0f, -2.0f, 2.0f));
    plane->GetComponent<Transform>()->SetWorldOrientation(vec3(-90.0f, 180.0f, 0.0f));

    //Suzanne model
    SCEHandle<Container> suz = createModel("suzanneObject", "Meshes/suzanne.obj", vec3(0, 3, 0));
    suz->GetComponent<Transform>()->RotateAroundAxis(vec3(0.0f, 1.0f, 0.0f), 180.0f);
    suz->AddComponent<Rotator>();

    float spreadDist = 10.0f;
    float nbSpheres = 5;
    for(float x = 0.0f; x < nbSpheres * 2.0f; ++x)
    {
        for(float z = -nbSpheres; z < nbSpheres; ++z)
        {
            createSphere("sphereObject", 2, vec3(2.0 + x * spreadDist, 0.0f, z * spreadDist));

            SCEHandle<Container> cube = createCube("cubeObject", vec3(-2.0 - x * spreadDist, 0.0f, z * spreadDist),
                       MATERIAL);
            cube->AddComponent<Rotator>();
            cube->GetComponent<Transform>()->RotateAroundAxis(vec3(0.0, 1.0, 0.0), 45.0f);
        }
    }
}

void scene2()
{
    SCEHandle<Container> wallObj = createCube("cubeObject", vec3(0.0f, 3.0f, 5.0f),
                                              WALL_MATERIAL);
    SCEHandle<Transform> wallTransform = wallObj->GetComponent<Transform>();
    wallTransform->SetLocalScale(vec3(10.0f, 10.0f, 2.0f));
    wallTransform->RotateAroundAxis(vec3(0.0, 0.0, 1.0), 90.0);

    SCEHandle<Container> cubeObj = createCube("cubeObject", vec3(0.0f, -0.9f, 0.0f),
                                              WALL_MATERIAL);
    SCEHandle<Transform> cube = cubeObj->GetComponent<Transform>();
    cube->SetLocalScale(vec3(2.0f, 2.0f, 2.0f));


    SCEHandle<Container> plane = createPlane("plane", 500.0f, vec3(0.0f, -2.0f, 2.0f));
    plane->GetComponent<Transform>()->SetWorldOrientation(vec3(-90.0f, 180.0f, 0.0f));

    float tileScale = 0.5;
    float tileSize = 9.192f * tileScale;
    float groundHalfWidth = 20.0f;
    for(float x = -groundHalfWidth; x < groundHalfWidth; x += tileSize)
    {
        for(float z = -groundHalfWidth; z < groundHalfWidth; z += tileSize)
        {
            SCEHandle<Container> groundObj = createModel("groundObject",
                                                         "Meshes/CobbleStones2.obj",
                                                         vec3(x, -2.1f, z));
            groundObj->GetComponent<Transform>()->SetLocalScale(vec3(tileScale));
        }

    }


//    SCEHandle<Container> tmp = createModel("tmp",
//                                              "Meshes/Goku.obj",
//                                              vec3(-2.0f, -1.75f, 2.0f), true);
//    tmp->GetComponent<Transform>()->SetLocalScale(vec3(0.105f));
//    tmp->GetComponent<Transform>()->RotateAroundAxis(vec3(0.0, 1.0, 0.0), 180.0f);

    SCEHandle<Container> eagle1 = createModel("eagle1",
                                              "Meshes/EAGLE_2.OBJ",
                                              vec3(0.0f, 4.0f, 0.0f));
    eagle1->GetComponent<Transform>()->SetLocalScale(vec3(3));
    eagle1->GetComponent<Transform>()->RotateAroundAxis(vec3(0.0, 1.0, 0.0), 180.0f);

    SCEHandle<Container> eagle2 = createModel("eagle2",
                                              "Meshes/EAGLE_3.OBJ",
                                              vec3(-15.23f, 5.42f, 7.62f));
    eagle2->GetComponent<Transform>()->SetLocalScale(vec3(3));
    eagle2->GetComponent<Transform>()->RotateAroundAxis(vec3(0.0, 1.0, 0.0), 180.0f);

    SCEHandle<Container> house = createModel("house", "Meshes/house_obj.obj",
                                              vec3(-15.0f, -2.1f, 15.0f));
    house->GetComponent<Transform>()->SetLocalScale(vec3(0.015f));
    house->GetComponent<Transform>()->RotateAroundAxis(vec3(0.0, 1.0, 0.0), 180.0f);


    SCEHandle<Container> wolf = createModel("wolf", "Meshes/Wolf.obj",
                                              vec3(-8.0f, -1.9f, 0.0f));
    wolf->GetComponent<Transform>()->SetLocalScale(vec3(0.5f));


    SCEHandle<Container> avion = createModel("avion", "Meshes/Eurofighter.obj",
                                              vec3(10.0f, 5.0f, 0.0f), false);
    avion->GetComponent<Transform>()->SetLocalScale(vec3(1.0f));


    SCEHandle<Container> goku = createModel("goku", "Meshes/Goku.obj",
                                              vec3(-2.0f, -1.85f, 2.0f), true);
    goku->GetComponent<Transform>()->SetLocalScale(vec3(0.105f));
    goku->GetComponent<Transform>()->RotateAroundAxis(vec3(0.0, 1.0, 0.0), 180.0f);

    SCEHandle<Container> trex = createModel("t-rex",
                                              "Meshes/T_REX.OBJ",
                                              vec3(-20.0f, -1.9f, 0.0f), false);
    trex->GetComponent<Transform>()->SetLocalScale(vec3(1.0f));
    trex->GetComponent<Transform>()->RotateAroundAxis(vec3(0.0, 1.0, 0.0), 135.0f);


    SCEHandle<Container> scorpion = createModel("scorpion",
                                              "Meshes/SCORPION.OBJ",
                                              vec3(-15.0f, -1.87f, -5.0f), false);
    scorpion->GetComponent<Transform>()->SetLocalScale(vec3(30.0f));
    scorpion->GetComponent<Transform>()->RotateAroundAxis(vec3(0.0, 1.0, 0.0), -45.0f);


    SCEHandle<Container> dragon = createModel("dragon",
                                              "Meshes/GringottsDragon.obj",
                                              vec3(-20.0f, 50.0f, 10.0f), false);
    dragon->GetComponent<Transform>()->SetLocalScale(vec3(30.0f));
    dragon->GetComponent<Transform>()->RotateAroundAxis(vec3(1.0, 0.0, 0.0), -90.0f);

    SCEHandle<Container> mecha = createModel("mecha",
                                              "Meshes/RB-SideSwipe.obj",
                                              vec3(-25.0f, -2.0f, -10.0f));
    mecha->GetComponent<Transform>()->SetLocalScale(vec3(0.025f));
    mecha->GetComponent<Transform>()->RotateAroundAxis(vec3(1.0, 0.0, 0.0), -90.0f);
    mecha->GetComponent<Transform>()->RotateAroundAxis(vec3(0.0, 0.0, 1.0), 30.0f);

}

int main( void )
{
    SCECore engine;
    engine.InitEngine("Playground scene for SCE");

    SCEScene::CreateEmptyScene();

    SCEHandle<Container> dirLight = createLight(vec3(0, 200, -200),
                                                vec3(40, 0, 0),
                                                LightType::DIRECTIONAL_LIGHT);
    dirLight->GetComponent<Light>()->SetLightColor(vec4(0.8, 0.8, 1.0, 0.2));
    dirLight->GetComponent<Light>()->SetIsSunLight(true);

//    SCEHandle<Container> light1 = createLight(vec3(2, 3, -1), vec3(30, 0, 30), LightType::POINT_LIGHT);
//    light1->GetComponent<Light>()->SetLightColor(vec4(1.0, 0.0, 0.0, 1.0));
//    light1->GetComponent<Light>()->SetLightReach(2.0f);

//    SCEHandle<Container> light2 = createLight(vec3(-15, 2, -5), vec3(25, 0, 0), LightType::SPOT_LIGHT);
//    light2->GetComponent<Light>()->SetLightReach(40.0f);
//    light2->GetComponent<Light>()->SetLightMaxAngle(45.0f);
//    light2->GetComponent<Light>()->SetLightColor(vec4(0.0, 1.0, 0.0, 1.0));


    scene1();
//    scene2();

    //Camera
    SCEHandle<Container> cameraObject = SCEScene::CreateContainer("cameraObject");
    SCEHandle<Transform> cameraTransform = cameraObject->AddComponent<Transform>();
    cameraObject->AddComponent<Camera>(40.0f, 16.0f/9.0f, 1.0f, 500.0f);
    cameraTransform->SetWorldPosition(vec3(0, 2, -25));
//    cameraTransform->RotateAroundAxis(vec3(1.0f, 0.0f, 0.0f), 90.0f);

    //load scene here
    engine.RunEngine();

    return 0;
}

