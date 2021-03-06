// Include standard headers
#include <stdio.h>
#include <stdlib.h>

#include "../headers/SCEMeshLoader.hpp"
#include "../headers/SCE.hpp"
#include "Rotator.hpp"
#include "SunCycle.hpp"
#include "CameraControl.hpp"
#include "PlayerControl.hpp"

using namespace SCE;
using namespace std;

#define MATERIAL "Materials/TestMaterial"
#define GROUND_MATERIAL "Materials/GroundMaterial"
#define WALL_MATERIAL "Materials/WallMaterial"
//#define GROUND_MATERIAL "Materials/TestMaterial"
//#define WALL_MATERIAL "Materials/TestMaterial"

SCEHandle<Container> createSphere(const string& name, const float& tesselation, const vec3& pos){
    //cube model
    SCEHandle<Container> object = SCEScene::CreateContainer(name);

    object->AddComponent<Material>(MATERIAL);

    SCEHandle<Transform> transform = object->AddComponent<Transform>();
    transform->SetScenePosition(pos);

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
    transform->SetScenePosition(pos);
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
    transform->SetScenePosition(pos);

    ui16 meshId = SCE::MeshLoader::CreateCubeMesh();
    object->AddComponent<MeshRenderer>(meshId);

    return object;
}

SCEHandle<Container> createPlane(const string& name, const string& mat, float size, const vec3& pos){
    //cube model
    SCEHandle<Container> object = SCEScene::CreateContainer(name);

    object->AddComponent<Material>(mat);

    SCEHandle<Transform> transform = object->AddComponent<Transform>();
    transform->SetScenePosition(pos);
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
    transform->SetScenePosition(pos);
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

    lightTransform->SetSceneOrientation(orientation);
    lightTransform->SetScenePosition(pos);

    return lightObject;
}

float simpleGround()
{
    SCEHandle<Container> plane = createPlane("plane", GROUND_MATERIAL, 500.0f, vec3(0.0f, -2.0f, 0.0f));
    plane->GetComponent<Transform>()->SetSceneOrientation(vec3(-90.0f, 180.0f, 0.0f));
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
    plane->GetComponent<Transform>()->SetSceneOrientation(vec3(-90.0f, 180.0f, 0.0f));
    return height;
}

void scene1()
{
    SCEHandle<Container> wallObj = createCube("cubeObject", vec3(0.0f, 3.0f, 5.0f),
                                              WALL_MATERIAL);
    SCEHandle<Transform> wallTransform = wallObj->GetComponent<Transform>();
    wallTransform->SetLocalScale(vec3(10.0f, 10.0f, 10.0f));


    SCEHandle<Container> plane = createPlane("plane", GROUND_MATERIAL, 500.0f, vec3(0.0f, -2.0f, 2.0f));
    plane->GetComponent<Transform>()->SetSceneOrientation(vec3(-90.0f, 180.0f, 0.0f));

    //Suzanne model
    SCEHandle<Container> suz = createModel("suzanneObject", "Meshes/suzanne.obj", vec3(0, 3, 0));
    suz->GetComponent<Transform>()->RotateAroundAxis(vec3(0.0f, 1.0f, 0.0f), 180.0f);

    float spreadDist = 10.0f;
    float nbSpheres = 5;
    for(float x = 0.0f; x < nbSpheres * 2.0f; ++x)
    {
        for(float z = -nbSpheres; z < nbSpheres; ++z)
        {
            SCEHandle<Container> sphere =
                    createSphere("sphereObject", 3.0f, vec3(2.0 + x * spreadDist, 0.0f, z * spreadDist));
            sphere->GetComponent<Material>()->SetUniformValue<float>("Roughness",
                                                                     glm::max(x/(nbSpheres*2.0f), 0.02f));

            SCEHandle<Container> cube =
                    createCube("cubeObject", vec3(-2.0 - x * spreadDist, 0.0f, z * spreadDist),
                       MATERIAL);
            cube->GetComponent<Material>()->SetUniformValue<float>("Roughness",
                                                                     glm::max(x/(nbSpheres*2.0f), 0.02f));

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
    plane->GetComponent<Transform>()->SetSceneOrientation(vec3(-90.0f, 180.0f, 0.0f));

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
    scorpion->GetComponent<Transform>()->SetLocalScale(vec3(5.0));
}

void sceneTerrain()
{
//    SCEScene::AddTerrain(3000.0f, 300.0f, 0.0f);
//    SCEScene::AddTerrain(9000.0f, 600.0f, 0.0f);
    SCEScene::AddTerrain(16000.0f, 600.0f, 0.0f, 2);
}

void sceneTrees()
{
    SCEHandle<Container> plane = createPlane("plane", GROUND_MATERIAL, 500.0f, vec3(0.0f, -2.0f, 2.0f));
    plane->GetComponent<Transform>()->SetSceneOrientation(vec3(-90.0f, 180.0f, 0.0f));

    //Suzanne model
    SCEHandle<Container> suz = createModel("suzanneObject", "Meshes/suzanne.obj", vec3(-20, 3, 0));
    suz->GetComponent<Transform>()->RotateAroundAxis(vec3(0.0f, 1.0f, 0.0f), 180.0f);

    float spreadDist = 10.0f;
    float nbSpheres = 5;
    for(float x = 0.0f; x < nbSpheres * 2.0f; ++x)
    {
        for(float z = -nbSpheres; z < nbSpheres; ++z)
        {
            SCEHandle<Container> sphere =
                    createSphere("sphereObject", 3.0f, vec3(2.0 + x * spreadDist, 0.0f, z * spreadDist));
            sphere->GetComponent<Material>()->SetUniformValue<float>("Roughness",
                                                                     glm::max(x/(nbSpheres*2.0f), 0.02f));
        }
    }

    std::string treeRoot = "Terrain/TreePack/";
    std::string extension = ".obj";
    std::vector<string> treeModelNames =
    {
        "tree_1/1/", "tree_1/2/",
        "tree_2/1/", "tree_2/2/",
        "tree_3/1/", "tree_3/2/",
        "tree_4/1/", "tree_4/2/",
    };


    float treeSpacing = 15.0f;
    float zIter = 30.0f;

    for(float z = -zIter*0.5f; z < zIter*0.5f; ++z)
    {
        glm::vec3 treePos = glm::vec3(-float(treeModelNames.size()) * 3.0f * 0.5f * treeSpacing, -2.2f, -80.0f);
        treePos.z += z*treeSpacing;
        for(uint i = 0; i < treeModelNames.size(); ++i)
        {
            for(int lod = 0; lod < 3; ++lod)
            {
                treePos.x += treeSpacing;
                SCEHandle<Container> tree = createModel(treeModelNames[i],
                                                        treeRoot + treeModelNames[i] +
                                                        "lod" + std::to_string(lod) + extension,
                                                        "Terrain/TreePack/Tree",
                                                        treePos);

                int treeInd = i / 2 + 1;
                SCEHandle<Material> mat = tree->GetComponent<Material>();
                std::string treeFolderPath = treeRoot + "tree_" + std::to_string(treeInd) + "/";

                GLuint barkTex =
                        SCE::TextureUtils::LoadTexture(treeFolderPath + "bark" + std::to_string(treeInd) + ".png");
                GLuint barkNormal =
                        SCE::TextureUtils::LoadTexture(treeFolderPath + "bark" + std::to_string(treeInd) + "_nmp"+ ".png");
                GLuint leafTex =
                        SCE::TextureUtils::LoadTexture(treeFolderPath + "leafs" + std::to_string(treeInd) + ".png");

                mat->SetUniformValue<GLuint>("BarkTex", barkTex);
                mat->SetUniformValue<GLuint>("BarkNormalMap", barkNormal);
                mat->SetUniformValue<GLuint>("LeafTex", leafTex);
            }
        }
    }

}

//#define START_NIGHT
void lightOutdoor()
{
    SCEHandle<Container> sunObject = SCEScene::CreateContainer("sunObject");
    sunObject->AddComponent<SunCycle>(1.0f, glm::vec3(1.0, 0.0, 0.0));    
#ifdef START_NIGHT
    glm::vec3 sunPos = glm::vec3(10.0, 0.0, 20000.0);
    sunObject->GetComponent<Transform>()->SetWorldPosition(sunPos);
#endif
}

void redAndGreen()
{
    SCEHandle<Container> light1 = createLight(vec3(2, 3, -1), vec3(30, 0, 30), LightType::POINT_LIGHT);
    light1->GetComponent<Light>()->SetLightColor(vec4(1.0, 0.0, 0.0, 1.0));
    light1->GetComponent<Light>()->SetLightReach(30.0f);

    SCEHandle<Container> light2 = createLight(vec3(8, 25, -27), vec3(45, -45, 0), LightType::SPOT_LIGHT);
    light2->GetComponent<Light>()->SetLightReach(100.0f);
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
            light->GetComponent<Light>()->SetLightColor(vec4(0.75, 1.0, 0.0, 1.0));
        }
    }
}

#define EAGLE
//#define PLANE

int sizeV(const vector<int>& x)
{
    return x.size();
}

int main( void )
{
    SCECore engine;
    engine.InitEngine("Playground scene for SCE");

    SCEScene::CreateEmptyScene();  

    lightOutdoor();
//    redAndGreen();
//    streetLights();

//    scene1();
    sceneTerrain();
//    sceneTrees();
//    scene2();
//    scene3();

//    waterGround(7.0f);

    vec3 startPos = vec3(0, 350, -25);
    //Camera
    SCEHandle<Container> cameraObject = SCEScene::CreateContainer("cameraObject");
    SCEHandle<Transform> cameraTransform = cameraObject->AddComponent<Transform>();
    cameraObject->AddComponent<Camera>(40.0f, 16.0f/9.0f, 1.0f, 30000.0f);


    SCEHandle<Container> eagle = SCEScene::CreateContainer("eagle");

    SCEHandle<Transform> eagleTransform = eagle->AddComponent<Transform>();
    eagleTransform->SetScenePosition(startPos);

    glm::vec3 distanceToTarget;

#ifdef EAGLE
    distanceToTarget = glm::vec3(0.0f, 1.0f, -3.5f);
    eagle->AddComponent<Material>("Materials/Eagle");
    SCEHandle<MeshRenderer> eagleRenderer = eagle->AddComponent<MeshRenderer>("Meshes/eagle_low.obj");
    eagleRenderer->SetIsCastingShadow(false);
#endif

#ifdef PLANE
    distanceToTarget = glm::vec3(0.0f, 3.0f, -10.5f);
    SCEHandle<Container> planeObj = SCEScene::CreateContainer("planeObj");
    SCEHandle<Transform> planeTransform = planeObj->AddComponent<Transform>();
    eagleTransform->AddChild(planeTransform);

    planeTransform->SetLocalOrientation(glm::vec3(0.0f, 180.0f, 0.0f));
    planeTransform->SetLocalPosition(glm::vec3(0.0));

    planeObj->AddComponent<Material>("Materials/Plane");
    planeObj->AddComponent<MeshRenderer>("Meshes/Eurofighter.obj");

#endif

    eagle->AddComponent<PlayerControl>();
    cameraTransform->SetScenePosition(startPos + vec3(0.0, 1.0, -5.0));
    cameraObject->AddComponent<CameraControl>(eagleTransform, distanceToTarget);

    //load scene here
    engine.RunEngine();

    return 0;
}

