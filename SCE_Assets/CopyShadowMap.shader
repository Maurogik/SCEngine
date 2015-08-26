/******PROJECT:Sand Castle Engine******/
/**************************************/
/*********AUTHOR:Gwenn AUBERT**********/
/******FILE:CopyShadowMap.shader*******/
/**************************************/

VertexShader :
_{
#version 430 core

    in vec3 vertexPosition_modelspace;

    uniform mat4 MVP;

    void main()
    {
        gl_Position = MVP * vec4(vertexPosition_modelspace, 1.0);
    }
_}

FragmentShader :
_{
#version 430 core

//    uniform sampler2D ShadowMapTexture;
    out vec3 color;

    void main()
    {
//        vec2 uv = gl_FragCoord.xy / vec2(1024, 768);
        color = vec3(gl_FragCoord.z, 1.0, 0.0);//texture2D(ShadowMapTexture, uv);
    }
_}
