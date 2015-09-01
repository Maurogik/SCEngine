/******PROJECT:Sand Castle Engine******/
/**************************************/
/*********AUTHOR:Gwenn AUBERT**********/
/********FILE:SkyShader.shader*********/
/**************************************/

VertexShader :
_{
#version 430 core

    in vec3 vertexPosition_modelspace;

    uniform mat4 MVP;
    uniform mat4 M;
    uniform mat4 V;
    uniform mat4 P;

    void main()
    {
        gl_Position                 = MVP * vec4(vertexPosition_modelspace, 1.0);
    }
_}

FragmentShader :
_{
#version 430 core

    uniform vec2        SCE_ScreenSize;
    uniform sampler2D   finalTex;

    out vec4 color;

    void main()
    {
        float brightMax = 10.0;
        float exposure = 1.0;

        vec4 sceneColor = texture(finalTex, gl_FragCoord.xy / SCE_ScreenSize);

        float Y = dot(vec4(0.30, 0.59, 0.11, 0.0), sceneColor);
        float YD = exposure * (exposure/brightMax + 1.0) / (exposure + 1.0);
        color = sceneColor * YD;

        //gamma correction
        color = pow(color, vec4(1.0/2.2));
    }
_}
