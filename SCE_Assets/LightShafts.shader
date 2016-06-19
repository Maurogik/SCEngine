/******PROJECT:Sand Castle Engine******/
/**************************************/
/*********AUTHOR:Gwenn AUBERT**********/
/********FILE:SunShader.shader*********/
/**************************************/
/*************** 1700µs ***************/

[VertexShader]
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

[FragmentShader]
_{
#version 430 core

    uniform vec2        SCE_ScreenSize;
    uniform vec3        SunPosition_worldspace;
    uniform float       SizeQuality;
    uniform int         VolumetricLightEnabled;
    layout (location = 0) uniform sampler2D   PositionTex;

    uniform mat4 V;
    uniform mat4 P;

    out float color;

    float computeVolumetricLight(vec2 uv, vec2 sunUV)
    {
        float nbSamples = 32.0;
        float decay = 2.0;
        float density = 0.3;
        float weight = 0.01;

        vec2 stepToFrag = (sunUV - uv)*density/float(nbSamples);

        float illum = 0.0;
        vec2 sampleUV = uv;

        for(float i = 0; i < nbSamples; ++i)
        {
            sampleUV += stepToFrag;
            vec3 Position_worldpsace = texture(PositionTex, sampleUV).xyz;
            float sampleVal = step(dot(Position_worldpsace, Position_worldpsace), 0.0001);
            sampleVal *= pow((nbSamples - i) / nbSamples, decay) * weight;
            illum += sampleVal;
        }

        return illum;
    }

    void main()
    {
        vec2 uv = gl_FragCoord.xy / (SCE_ScreenSize * SizeQuality);
        vec3 Position_worldpsace = texture(PositionTex, uv).xyz;
        vec3 sun_cameraspace = (V * vec4(SunPosition_worldspace, 1.0)).xyz;
        vec4 sun_projectionspace = (P * vec4(sun_cameraspace, 1.0));
        sun_projectionspace /= sun_projectionspace.w;

        vec2 sunUV = sun_projectionspace.xy * 0.5 + vec2(0.5);

        float sunSqLen = dot(sun_projectionspace.xy, sun_projectionspace.xy);

        color = 0.0;

        if(sunSqLen < 3.0)
        {
            float sunStrength = (3.0 - sunSqLen)/3.0;
            sunStrength *= step(0.0, sun_cameraspace.z);
            color = computeVolumetricLight(uv, sunUV) * sunStrength;
        }
    }
_}
