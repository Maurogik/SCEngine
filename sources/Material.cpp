/**********Sand Castle Engine**********/
/**************************************/
/******** AUTHOR : Gwenn AUBERT *******/
/********* FILE : Material.cpp ********/
/**************************************/

#include "../headers/Material.hpp"

#include "external/rapidjson/document.h" // rapidjson's DOM-style API
#include "external/rapidjson/prettywriter.h" // for stringify JSON
#include "external/rapidjson/filestream.h" // wrapper of C stream for prettywriter as output

#include <string>
#include <vector>
#include <iostream>
#include <fstream>

using namespace SCE;
using namespace std;


//define shader
//load material definition from text file
//
// Attributes : Normals, Vertex & UV are always part of the material shader
// Uniforms : LightPos & LightColor are part of every shaders too
//
//Text file :
/* MATERIAL : "exempleMat"
 *
 * Shader : "ShaderName"
 *
 * Uniforms:
 *  Name : AmbientColor -- Type : vec4, Value : vec4(r,g,b,a)
 *  Name : Outline -- Type : float, Value : 0.63f
 *  Name : Texture1 -- Type : texture, Value : "pathToText1"
 *  Name : Texture2 -- Type : texture, Value : "pathToText2"
 *
 */
// The uniforms defined in the material file must be defined & used in the shader
// The material file will be parsed and the shader will be compiled and linked
// The data (uniforms, attribute) from the file will be generated, binded and linked

//SHADER
/*
 *
 * NAME{ShaderName}
 *
 * /*ATTRIBUTES
 * {
 *      // Vertices, Normals, Uvs are mandatory and so are tangents and bi-tangents
 *      // (Use buffer index value to decide if used or not)
 * }*\/
 *
 *
 *
 * UNIFORMS
 * {
 *      // ModelViewProjection matrix
 *      // Model Matrix
 *      // View Matrix
 *      // Projection Matrix
 *
 *      LightPos, LightColor ?
 * }
 *
 * DATA //Aditional uniforms
 * {
 *      Name : name -- Type : type
 *      Name : AmbientColor -- Type : vec4
 *      Name : Outline -- Type : float
 *      Name : Texture1 -- Type : texture
 * }
 *
 * LIGHTING
 * {
 *      NONE / CUSTOM
 * }
 *
 * Pass_1
 * {
 *      vertex shader
 *      {
 *          //shader content
 *      }
 *
 *      fragment shader
 *      {
 *          //shader content
 *      }
 * }
 *
 * // To do later...
 *
 * Pass_2
 * {
 *      vertex shader ...
 *      fragment shader ...
 * }
 *
 * Pass_n ...
 *
 */


void Material::LoadMaterial(string filename)
{
    rapidjson::Document root;
    string fileStr = "";
    ifstream fileStream(filename.c_str(), ios::in);

    if(fileStream.is_open()){
        string currLine;
        while (getline(fileStream, currLine)) {
            fileStr += "\n" + currLine;
        }
        fileStream.close();

        if (root.Parse<0>(fileStr.c_str()).HasParseError()){
            return;
        }

        //Access values
        SCE_ASSERT(root.IsObject(), "Malformated Json material file\n");

        string name = root["Name"].GetString();
        SCE_LOG(name.c_str());

    } else {

    }
}
