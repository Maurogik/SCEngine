/**********Sand Castle Engine**********/
/**************************************/
/******** AUTHOR : Gwenn AUBERT *******/
/********* FILE : Material.hpp ********/
/**************************************/
#ifndef SCE_MATERIAL_HPP
#define SCE_MATERIAL_HPP


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
 * DATA
 * {
 *      Name : name -- Type : type
 *      Name : AmbientColor -- Type : vec4
 *      Name : Outline -- Type : float
 *      Name : Texture1 -- Type : texture
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
 * Pass_2
 * {
 *      vertex shader ...
 *      fragment shader ...
 * }
 *
 * Pass_n ...
 *
 */

namespace SCE {

    class Material {

    public :

    private :

    };

}


#endif
