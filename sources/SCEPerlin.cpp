/******PROJECT:Sand Castle Engine******/
/**************************************/
/*********AUTHOR:Gwenn AUBERT**********/
/**********FILE:SCEPerlin.cpp**********/
/**************************************/

#include "../headers/SCEPerlin.hpp"
#include "../headers/SCETools.hpp"
#include <cstdlib>

namespace SCE
{
namespace Perlin
{

#define EXP_ARRAY_SIZE 256

static int mSize = -1;
static vec2* gradients;
static int* permutations;
static float* exponents;

void randomizeNormalizedVectors(vec2* vectors, int count)
{
    for(int i = 0; i < count; ++i)
    {
        vectors[i].x = SCE::Math::RandRange(-1.0, 1.0);
        vectors[i].y = SCE::Math::RandRange(-1.0, 1.0);
        glm::normalize(vectors[i]);
    }
}

void MakePerlin(ui16 gridSize)
{
    //initalize gradient grid
    mSize = gridSize;
    int length = mSize*mSize;
    gradients = (vec2*)malloc(sizeof(glm::vec2)*length);
    randomizeNormalizedVectors(gradients, length);

    permutations = (int*) malloc(sizeof(int)*EXP_ARRAY_SIZE);

    for(int i = 0; i < EXP_ARRAY_SIZE; ++i)
    {
        permutations[i] = i;
    }

    for(int i = EXP_ARRAY_SIZE - 1; i > 0; i--)
    {
        std::swap(permutations[i], permutations[rand()%(i + 1)]);
    }

    float mExpSrc = glm::pow(2.0f, -126.0f/float(EXP_ARRAY_SIZE - 1));
    mExpSrc = glm::mix(mExpSrc, 1.0f, 0.5f);

    exponents = (float*)malloc(sizeof(float)*EXP_ARRAY_SIZE);
    float s = 1.0; //current magnitude
    for(int i = 0; i < EXP_ARRAY_SIZE; i++)
    {
        exponents[i] = s;
        s /= mExpSrc;
    }
}

void DestroyPerlin()
{
    free(gradients);
    gradients = NULL;

    free(permutations);
    permutations = NULL;

    free(exponents);
    exponents = NULL;
}

float GetPerlinAt(float x, float y)
{
    return GetPerlinAt(x, y, mSize);
}

int hash(int x, int y, int size)
{
    return permutations[(permutations[x] + y)%size];
}

//result in -0.5 .. 0.5
float GetPerlinAt(float x, float y, float period)
{
    //the four corners of this grid cell
    float x0, y0, x1, y1;
    x0 = glm::floor(x);
    y0 = glm::floor(y);
    x1 = x0 + 1;
    y1 = y0 + 1;

    vec2 point (x, y);
    vec2 corners[4] = { vec2(x0, y0),
                        vec2(x0, y1),
                        vec2(x1, y0),
                        vec2(x1, y1) };

    float expL = 1.0;

    //compute vectorsToPoint from corners to wanted point
    float gradientDotVector[4];
    for(int i = 0; i < 4; ++i)
    {
        vec2 vectorToPoint 	 = point - corners[i];

        int xg = int(glm::mod(corners[i].x, period))%mSize;
        int yg = int(glm::mod(corners[i].y, period))%mSize;

        vec2 gradient = gradients[ xg  * mSize + yg ];
//        vec2 gradient = gradients[hash(xg, yg, mSize)];
        //expL = exponents[hash(xg%EXP_ARRAY_SIZE, yg%EXP_ARRAY_SIZE, EXP_ARRAY_SIZE)];
        gradientDotVector[i] = expL * glm::dot(gradient, vectorToPoint);
    }

    //compute average of the 4 dot products
    float u,v;
    u = (x - x0);
    v = (y - y0);
    //apply s-curve function to u & v
    u = u * u * u * (u * (u * 6 - 15) + 10);//6t5-15t4+10t3
    v = v * v * v * (v * (v * 6 - 15) + 10);
    float avgY0 = glm::mix(gradientDotVector[0], gradientDotVector[2], u);
    float avgY1 = glm::mix(gradientDotVector[1], gradientDotVector[3], u);
    return glm::mix(avgY0, avgY1, v);
}

//Computes and combine perlin noise at several frequencies
float GetLayeredPerlinAt(float x, float y, int layers, float persistence)
{
    float perlinSum = 0.0;
    float frequency = 1.0;
    float amplitude = 1.0;
    float maxValue = 0.0;
    for(int l = 0; l < layers; ++l)
    {
        perlinSum += GetPerlinAt(x * frequency, y * frequency, mSize) * amplitude;
        frequency *= 2;
        maxValue += amplitude;
        amplitude /= persistence;
    }
    return perlinSum / maxValue;
}

}

}
