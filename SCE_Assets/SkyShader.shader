/******PROJECT:Sand Castle Engine******/
/**************************************/
/*********AUTHOR:Gwenn AUBERT**********/
/********FILE:SkyShader.shader*********/
/**************************************/
/**************** 1700µs **************/

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
    uniform vec4        SunColor;
    uniform vec3        SkyTopColor;
    uniform vec3        SkyBottomColor;
    uniform vec3        FogColor;

    uniform float       SkyFadeFactor;

    layout (location = 0) uniform sampler2D   FinalColorTex;
    layout (location = 1) uniform sampler2D   PositionTex;
    layout (location = 2) uniform sampler2D   SunTex;
    layout (location = 3) uniform sampler2D   LightShaftTex;

    uniform mat4 V;
    uniform mat4 P;

    out vec4 color;

    vec3 ndcToFarWorldspace(vec2 screenNdc)
    {
        mat4 ndcToWorld = inverse(P*V);
        vec4 frag_ndc = vec4(screenNdc, 1.0, 1.0);
        frag_ndc *= 500.0; //perform inverse of perspective divide as if the point was on far plane

        vec3 frag_worldspace = (ndcToWorld * frag_ndc).xyz;
        return frag_worldspace;
    }

    vec3 getSkyColor(vec2 ndcUv)
    {
        vec3 frag_worldspace = ndcToFarWorldspace(ndcUv);
        float height_worldspace = normalize(frag_worldspace).y + 0.1;

        float maxHeight = 1.0;
        float fade = height_worldspace;
        fade = clamp(fade, 0.0, maxHeight) / maxHeight;
        fade = 1.0 - pow(1.0 - fade, SkyFadeFactor);

        return mix(SkyBottomColor, SkyTopColor, fade);
    }

    vec3 getFogColor(float height_worldspace)
    {
        return FogColor;
    }

    void main()
    {
        vec2 uv = gl_FragCoord.xy / SCE_ScreenSize;
        vec4 sceneColor = texture(FinalColorTex, uv);
        vec3 Position_worldspace = texture(PositionTex, uv).xyz;
        vec2 sunAndFlare = texture(SunTex, uv).rg;
        vec3 coloredSun = sunAndFlare.r * SunColor.rgb * SunColor.a;
        vec3 flareColor = sunAndFlare.g * SunColor.rgb * SunColor.a;
        float lightScatering = texture(LightShaftTex, uv).r;

        float notOccludedByScene = step(dot(Position_worldspace, Position_worldspace), 0.0001);

        //compute fog strength
        float height = Position_worldspace.y;
//        float fogStr = 0.000000002;
        float fogStr = 0.0000000001;
        float heightDensity = 0.0005;
        float dist = abs((V * vec4(Position_worldspace, 1.0)).z);
        dist *= dist;
        float fogAmount = (1.0 - exp( -dist * fogStr )) * exp(-height * heightDensity);

        color = vec4(0.0, 0.0, 0.0, 1.0);

        if(notOccludedByScene > 0.0)
        {
            //compute normalized device coord
            vec2 ndcUv = uv * 2.0 - vec2(1.0);
            vec3 skyColor = getSkyColor(ndcUv);
            skyColor += coloredSun;

            color.rgb += skyColor * notOccludedByScene;
        }
        color.rgb += mix(sceneColor.rgb, getFogColor(height), fogAmount) * (1.0 - notOccludedByScene);
        color.rgb += lightScatering * SunColor.rgb * SunColor.a;
        color.rgb += flareColor;
    }
_}
