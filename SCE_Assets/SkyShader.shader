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
    uniform vec3        SunColor;
    uniform vec3        SkyTopColor;
    uniform vec3        SkyBottomColor;
    uniform vec3        FogColor;

    uniform float       SkyFadeFactor;

    layout (location = 0) uniform sampler2D   FinalColorTex;
    layout (location = 1) uniform sampler2D   PositionTex;
    layout (location = 2) uniform sampler2D   SunTex;

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
        vec4 sunData = texture(SunTex, uv);
        vec3 sunColor = sunData.r * SunColor;
        float lightScatering = sunData.g;

        float notOccludedByScene = step(dot(Position_worldspace, Position_worldspace), 0.0001);

        //compute normalized device coord and screenspace sun position
        vec3 sun_cameraspace = (V * vec4(SunPosition_worldspace, 1.0)).xyz;
        vec2 ndcUv = uv * 2.0 - vec2(1.0);
        vec4 sun_projectionspace = (P * vec4(sun_cameraspace, 1.0));
        sun_projectionspace /= sun_projectionspace.w;
        vec2 sunUV = sun_projectionspace.xy * 0.5 + vec2(0.5);

        //compute fog strength
        float height = Position_worldspace.y;
        float fogStr = 0.0000001;
        float heightDensity = 0.0005;
        float dist = abs((V * vec4(Position_worldspace, 1.0)).z);
        dist *= dist;
        float fogAmount = (1.0 - exp( -dist * fogStr )) * exp(-height * heightDensity);

        vec3 skyColor = getSkyColor(ndcUv);
        skyColor += sunColor;

        color = vec4(0.0, 0.0, 0.0, 1.0);
        color.rgb += skyColor * notOccludedByScene;
        color.rgb += mix(sceneColor.rgb, getFogColor(height), fogAmount) * (1.0 - notOccludedByScene);
        color.rgb += lightScatering * SunColor;
    }
_}
