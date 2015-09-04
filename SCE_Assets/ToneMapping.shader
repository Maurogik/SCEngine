/******PROJECT:Sand Castle Engine******/
/**************************************/
/*********AUTHOR:Gwenn AUBERT**********/
/********FILE:SkyShader.shader*********/
/**************************************/
/**************** 800µs ***************/

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
    uniform float       SCE_Exposure;
    uniform float       SCE_MaxBrightness;
    layout (location = 0) uniform sampler2D   FinalColorTex;
    layout (location = 1) uniform sampler2D   LuminanceTex;

    out vec4 color;

    float A = 0.15;
    float B = 0.50;
    float C = 0.10;
    float D = 0.20;
    float E = 0.02;
    float F = 0.30;
    float W = 11.2;

    vec3 Uncharted2Tonemap(vec3 x)
    {
        return ((x*(A*x+C*B)+D*E)/(x*(A*x+B)+D*F))-E/F;
    }

    void main()
    {
        vec2 uv = gl_FragCoord.xy / SCE_ScreenSize;
        vec4 sceneColor = texture(FinalColorTex, uv);

        float maxSize = max(SCE_ScreenSize.x, SCE_ScreenSize.y);
        float numLevels = 1.0 + floor(log2(maxSize));

        vec3 whitePoint = vec3(7.0);

        float lum = textureLod(LuminanceTex, uv, numLevels-1).r;
        float exposure = 1.5 / lum;
        vec3 hdrColor = sceneColor.xyz;
        color.rgb = hdrColor;
        color.rgb = Uncharted2Tonemap(hdrColor * exposure) / Uncharted2Tonemap(whitePoint);

        //gamma correction
        color.rgb = pow(color.rgb, vec3(1.0/2.2));
        color.a = 1.0;
    }
_}
