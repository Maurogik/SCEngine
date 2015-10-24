// Include standard headers
#include <stdio.h>
#include <stdlib.h>

#include "../headers/SCEMeshLoader.hpp"
#include "../headers/SCE.hpp"
#include "Rotator.hpp"
#include "CameraControl.hpp"
#include "PlayerControl.hpp"

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

    ui16 meshId = SCE::MeshLoader::CreateSphereMesh(tesselation);
    object->AddComponent<MeshRenderer>(meshId);

    return object;
}

SCEHandle<Container> createCone(const string& name, const float& tesselation,
                                float angle, float length, const vec3& pos){
    //cube model
    SCEHandle<Container> object = SCEScene::CreateContainer(name);

    object->AddComponent<Material>(MATERIAL);

    SCEHandle<Transform> transform = object->AddComponent<Transform>();
    transform->SetWorldPosition(pos);
    transform->SetLocalScale(vec3(length));

    ui16 meshId = SCE::MeshLoader::CreateConeMesh(angle, tesselation);
    object->AddComponent<MeshRenderer>(meshId);

    return object;
}

SCEHandle<Container> createCube(const string& name, const vec3& pos,
                                const string& material){
    //cube model
    SCEHandle<Container> object = SCEScene::CreateContainer(name);

    object->AddComponent<Material>(material);

    SCEHandle<Transform> transform = object->AddComponent<Transform>();
    transform->SetWorldPosition(pos);

    ui16 meshId = SCE::MeshLoader::CreateCubeMesh();
    object->AddComponent<MeshRenderer>(meshId);

    return object;
}

SCEHandle<Container> createPlane(const string& name, const string& mat, float size, const vec3& pos){
    //cube model
    SCEHandle<Container> object = SCEScene::CreateContainer(name);

    object->AddComponent<Material>(mat);

    SCEHandle<Transform> transform = object->AddComponent<Transform>();
    transform->SetWorldPosition(pos);
    transform->SetLocalScale(vec3(size, size, size));

    ui16 meshId = SCE::MeshLoader::CreateQuadMesh();
    object->AddComponent<MeshRenderer>(meshId);

    return object;
}

SCEHandle<Container> createModel(const string& objectName, const string& filename,  const string& mat,
                                 const vec3& pos){
    SCEHandle<Container> object = SCEScene::CreateContainer(objectName);

    object->AddComponent<Material>(mat);

    SCEHandle<Transform> transform = object->AddComponent<Transform>();
    transform->SetWorldPosition(pos);
    object->AddComponent<MeshRenderer>(filename);

    return object;
}

SCEHandle<Container> createModel(const string& objectName, const string& filename, const vec3& pos){
    return createModel(objectName, filename, string(MATERIAL), pos);
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

float simpleGround()
{
    SCEHandle<Container> plane = createPlane("plane", GROUND_MATERIAL, 500.0f, vec3(0.0f, -2.0f, 0.0f));
    plane->GetComponent<Transform>()->SetWorldOrientation(vec3(-90.0f, 180.0f, 0.0f));
    return -2.0f;
}

float complexGround()
{
    //stones
    float tileScale = 1.0;
    float tileSize = 9.192f * tileScale;
    float groundHalfWidth = 40.0f;
    for(float x = -groundHalfWidth; x < groundHalfWidth; x += tileSize)
    {
        for(float z = -groundHalfWidth; z < groundHalfWidth; z += tileSize)
        {
            SCEHandle<Container> groundObj = createModel("groundObject",
                                                         "Meshes/CobbleStones2.obj", "Materials/CobbleStone2",
                                                         vec3(x, -0.1f, z));
            groundObj->GetComponent<Transform>()->SetLocalScale(vec3(tileScale));
        }

    }

    return -1.6f;
}

float waterGround(float height)
{
    SCEHandle<Container> plane = createPlane("plane", "Materials/Water", 4000.0f, vec3(0.0f, height, 0.0f));
    plane->GetComponent<Transform>()->SetWorldOrientation(vec3(-90.0f, 180.0f, 0.0f));
    return height;
}

void scene1()
{
    SCEHandle<Container> wallObj = createCube("cubeObject", vec3(0.0f, 3.0f, 5.0f),
                                              WALL_MATERIAL);
    SCEHandle<Transform> wallTransform = wallObj->GetComponent<Transform>();
    wallTransform->SetLocalScale(vec3(10.0f, 10.0f, 10.0f));


    SCEHandle<Container> plane = createPlane("plane", GROUND_MATERIAL, 500.0f, vec3(0.0f, -2.0f, 2.0f));
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
    SCEHandle<Container> wallObj = createPlane("plane", WALL_MATERIAL, 500.0f, vec3(0.0f, -2.0f, 2.0f));
    SCEHandle<Transform> wallTransform = wallObj->GetComponent<Transform>();
    wallTransform->SetLocalScale(vec3(10.0f, 10.0f, 10.0f));

    SCEHandle<Container> cubeObj = createCube("cubeObject", vec3(0.0f, -0.9f, 0.0f),
                                              WALL_MATERIAL);
    SCEHandle<Transform> cube = cubeObj->GetComponent<Transform>();
    cube->SetLocalScale(vec3(2.0f, 2.0f, 2.0f));


    SCEHandle<Container> plane = createPlane("plane", GROUND_MATERIAL, 500.0f, vec3(0.0f, -2.0f, 2.0f));
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

    SCEHandle<Container> eagle1 = createModel("eagle1",
                                              "Meshes/EAGLE_2.OBJ",
                                              vec3(0.0f, 4.0f, 0.0f));
    eagle1->GetComponent<Transform>()->RotateAroundAxis(vec3(0.0, 1.0, 0.0), 180.0f);

    SCEHandle<Container> eagle2 = createModel("eagle2",
                                              "Meshes/EAGLE_3.OBJ",
                                              vec3(-15.23f, 5.42f, 7.62f));
    eagle2->GetComponent<Transform>()->RotateAroundAxis(vec3(0.0, 1.0, 0.0), 180.0f);

    SCEHandle<Container> house = createModel("house", "Meshes/house_obj.obj",
                                              vec3(-15.0f, -2.1f, 15.0f));
    house->GetComponent<Transform>()->RotateAroundAxis(vec3(0.0, 1.0, 0.0), 180.0f);


    SCEHandle<Container> wolf = createModel("wolf", "Meshes/Wolf.obj",
                                              vec3(-8.0f, -1.9f, 0.0f));


    SCEHandle<Container> avion = createModel("avion", "Meshes/Eurofighter.obj",
                                              vec3(10.0f, 5.0f, 0.0f));


    SCEHandle<Container> goku = createModel("goku", "Meshes/Goku.obj",
                                              vec3(-2.0f, -1.85f, 2.0f));
    goku->GetComponent<Transform>()->RotateAroundAxis(vec3(0.0, 1.0, 0.0), 180.0f);

    SCEHandle<Container> trex = createModel("t-rex",
                                              "Meshes/T_REX.OBJ",
                                              vec3(-20.0f, -1.9f, 0.0f));
    trex->GetComponent<Transform>()->RotateAroundAxis(vec3(0.0, 1.0, 0.0), 135.0f);


    SCEHandle<Container> scorpion = createModel("scorpion",
                                              "Meshes/SCORPION.OBJ",
                                              vec3(-15.0f, -1.87f, -5.0f));
    scorpion->GetComponent<Transform>()->RotateAroundAxis(vec3(0.0, 1.0, 0.0), -45.0f);


    SCEHandle<Container> dragon = createModel("dragon",
                                              "Meshes/GringottsDragon.obj",
                                              vec3(-20.0f, 50.0f, 10.0f));
    dragon->GetComponent<Transform>()->RotateAroundAxis(vec3(1.0, 0.0, 0.0), -90.0f);

    SCEHandle<Container> mecha = createModel("mecha",
                                              "Meshes/RB-SideSwipe.obj",
                                              vec3(-25.0f, -2.0f, -10.0f));
    mecha->GetComponent<Transform>()->RotateAroundAxis(vec3(1.0, 0.0, 0.0), -90.0f);
    mecha->GetComponent<Transform>()->RotateAroundAxis(vec3(0.0, 0.0, 1.0), 30.0f);

}


void scene3()
{
    SCEHandle<Container> cubeObj = createCube("cubeObject", vec3(5.0f, -1.0f, -5.0f), WALL_MATERIAL);
    SCEHandle<Transform> cube = cubeObj->GetComponent<Transform>();
    cube->SetLocalScale(vec3(2.0f, 2.0f, 2.0f));

    //ground
    float groundY = simpleGround();//complexGround();

    //house
    SCEHandle<Container> house = createModel("house", "Meshes/house_obj.obj", "Materials/House",
                                              vec3(-5.0f, groundY - 0.15f, 5.0f));
    house->GetComponent<Transform>()->RotateAroundAxis(vec3(0.0, 1.0, 0.0), 195.0f);


    SCEHandle<Container> avion = createModel("avion", "Meshes/Eurofighter.obj", "Materials/Plane",
                                              vec3(10.0f, 5.0f, 0.0f));


    SCEHandle<Container> trex = createModel("t-rex",
                                              "Meshes/T_REX.OBJ", "Materials/Rex",
                                              vec3(-10.0f, groundY - 0.1f, -5.0f));
    trex->GetComponent<Transform>()->RotateAroundAxis(vec3(0.0, 1.0, 0.0), 135.0f);


    SCEHandle<Container> scorpion = createModel("scorpion",
                                              "Meshes/SCORPION.OBJ", string("Materials/Scorpion"),
                                              vec3(-5.0f, groundY, -10.0f));
    scorpion->GetComponent<Transform>()->RotateAroundAxis(vec3(0.0, 1.0, 0.0), -45.0f);
}

void lightOutdoor()
{
        SCEHandle<Container> dirLight = createLight(vec3(0, 2000, -2000),
                                                    vec3(40, 0, 0),
                                                    LightType::DIRECTIONAL_LIGHT);
        dirLight->GetComponent<Light>()->SetLightColor(vec4(1.0, 1.0, 0.8, 0.7));
        dirLight->GetComponent<Light>()->SetIsSunLight(true);
}

void redAndGreen()
{
    SCEHandle<Container> light1 = createLight(vec3(2, 3, -1), vec3(30, 0, 30), LightType::POINT_LIGHT);
    light1->GetComponent<Light>()->SetLightColor(vec4(1.0, 0.0, 0.0, 1.0));
    light1->GetComponent<Light>()->SetLightReach(30.0f);

    SCEHandle<Container> light2 = createLight(vec3(-2, 15, -17), vec3(45, -45, 0), LightType::SPOT_LIGHT);
    light2->GetComponent<Light>()->SetLightReach(40.0f);
    light2->GetComponent<Light>()->SetLightMaxAngle(50.0f);
    light2->GetComponent<Light>()->SetLightColor(vec4(0.0, 1.0, 0.0, 1.0));

}

void streetLights()
{
    float nbLights = 5.0;
    float offset = 25.0f;
    for(float x = -nbLights / 2.0f; x < nbLights / 2.0f; ++x)
    {
        for(float z = -nbLights / 2.0f; z < nbLights / 2.0f; ++z)
        {
            SCEHandle<Container> light = createLight(vec3(x * offset, 15, z * offset),
                                                     vec3(90, 0, 0), LightType::SPOT_LIGHT);
            light->GetComponent<Light>()->SetLightReach(30.0f);
            light->GetComponent<Light>()->SetLightMaxAngle(75.0f);
            light->GetComponent<Light>()->SetLightColor(vec4(0.5, 1.0, 0.0, 1.0));
        }
    }
}

#define EAGLE

int sizeV(const vector<int>& x)
{
    return x.size();
}

int main( void )
{
    SCECore engine;
    engine.InitEngine("Playground scene for SCE");

    SCEScene::CreateEmptyScene();

    SCEScene::AddTerrain(3000.0f, 100.0f, 0.0f);

    lightOutdoor();
//    redAndGreen();
//    streetLights();

//    scene1();
//    scene2();
//    scene3();

//    waterGround(7.0f);

    vec3 startPos = vec3(0, 350, -25);
    //Camera
    SCEHandle<Container> cameraObject = SCEScene::CreateContainer("cameraObject");
    SCEHandle<Transform> cameraTransform = cameraObject->AddComponent<Transform>();
    cameraObject->AddComponent<Camera>(40.0f, 16.0f/9.0f, 1.0f, 8000.0f);

#ifdef EAGLE
    SCEHandle<Container> eagle = createModel("eagle2",
                                              "Meshes/EAGLE_2.OBJ", "Materials/Eagle",
                                              startPos);
    SCEHandle<Transform> eagleTransform = eagle->GetComponent<Transform>();
//    eagleTransform->RotateAroundAxis(vec3(0.0, 1.0, 0.0), 180.0f);
    eagle->AddComponent<PlayerControl>();
    cameraTransform->SetWorldPosition(startPos + vec3(0.0, 1.0, -5.0));
    SCEHandle<CameraControl> camControl = cameraObject->AddComponent<CameraControl>(eagleTransform);
#else
    cameraTransform->SetLocalPosition(startPos);
    cameraObject->AddComponent<PlayerControl>();
#endif

    //load scene here
    engine.RunEngine();

    return 0;
}

