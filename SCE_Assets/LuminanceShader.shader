/******PROJECT:Sand Castle Engine******/
/**************************************/
/*********AUTHOR:Gwenn AUBERT**********/
/********FILE:SkyShader.shader*********/
/**************************************/

VertexShader :
_{
#version 430 core

    in vec3 vertexPosition_modelspace;
    in vec3 vertexNormal_modelspace;

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
    layout (location = 0) uniform sampler2D   FinalColorTex;
    layout (location = 1) uniform sampler2D   LuminanceTex;
    layout (location = 2) uniform float       DeltaTime;

    out float oLuminance;

    void main()
    {
        float adaptationDuration = 1.5f;

        vec2 uv = gl_FragCoord.xy / SCE_ScreenSize;
        vec4 sceneColor = texture(FinalColorTex, uv);
        float oldLum = texture(LuminanceTex, uv).r;
        float newLum = dot(vec4(0.30, 0.59, 0.11, 0.0), sceneColor);
        float dt = DeltaTime * 1.0f;
        oLuminance = (oldLum * adaptationDuration + newLum * dt)/(adaptationDuration + dt);
    }
_}
