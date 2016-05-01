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
static int mWidth = -1;
static int mHeight = -1;
static vec2* cornersGradient = NULL; //order of corners : x0y0, x0y1, x1y0, x1y1

void randomizeNormalizedVectors(vec2* vectors, int count)
{
    for(int i = 0; i < count; ++i)
    {
        vectors[i].x = SCE::Math::RandRange(-1.0, 1.0);
        vectors[i].y = SCE::Math::RandRange(-1.0, 1.0);
        glm::normalize(vectors[i]);
    }
}

void MakePerlin(ui16 gridWidth, ui16 gridHeight)
{
    //initalize gradient grid
    mWidth = gridWidth;
    mHeight = gridHeight;
    int length = mWidth * mHeight;
    vec2* gradients = (vec2*)malloc(sizeof(glm::vec2)*length);
    randomizeNormalizedVectors(gradients, length);

    //store each 'square' of gradients to increase access speed (load all needed corner at once)
    cornersGradient = (vec2*)malloc(sizeof(vec2) * length * 4);
    int x1, y1;
    int pos;
    for(int x = 0; x < mWidth; ++x)
    {
        x1 = (x + 1) % mWidth;
        for(int y = 0; y < mHeight; ++y)
        {
            y1 = (y + 1) % mHeight;
            pos = x * mHeight * 4 + y * 4;
            cornersGradient[pos] 	 = gradients[x  * mHeight + y ];
            cornersGradient[pos + 1] = gradients[x  * mHeight + y1];
            cornersGradient[pos + 2] = gradients[x1 * mHeight + y ];
            cornersGradient[pos + 3] = gradients[x1 * mHeight + y1];
        }
    }

    free(gradients);
    gradients = nullptr;
}

void DestroyPerlin()
{
    free(cornersGradient);
    cornersGradient = NULL;
}

//result in -0.5 .. 0.5
float GetPerlinAt(float x, float y)
{
    x = glm::mod(x, (float)mWidth);
    y = glm::mod(y, (float)mHeight);
    //the four corners of this grid cell
    float x0, y0, x1, y1;
    x0 = glm::floor(x);
    y0 = glm::floor(y);
    x1 = glm::ceil(x);
    y1 = glm::ceil(y);

    vec2 point (x, y);
    vec2 corners[4] = { vec2(x0, y0),
                        vec2(x0, y1),
                        vec2(x1, y0),
                        vec2(x1, y1) };
    //compute vectorsToPoint from corners to wanted point
    float gradientDotVector[4];
    int cornerPos = ((int)x0 % mWidth) * mHeight * 4 + ((int)y0 % mHeight) * 4;
    for(int i = 0; i < 4; ++i)
    {
        vec2 vectorToPoint 	 = point - corners[i];
        float expL = 1.0;//0.2 + glm::pow(glm::length(vectorToPoint), 2.0);
        gradientDotVector[i] = expL * glm::dot(cornersGradient[cornerPos + i], vectorToPoint);
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
        perlinSum += GetPerlinAt(x * frequency, y * frequency) * amplitude;
        frequency *= 2;
        maxValue += amplitude;
        amplitude /= persistence;
    }
    return perlinSum / maxValue;
}
}

}
