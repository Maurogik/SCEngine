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
    layout (location = 0) uniform sampler2D   FinalColorTex;
    layout (location = 1) uniform sampler2D   PositionTex;
    layout (location = 2) uniform sampler2D   SunTex;

    uniform mat4 V;
    uniform mat4 P;

    out vec4 color;

    //variables
    vec3 vSunColor = vec3(1.0, 1.0, 0.9);
    vec3 vLowerSkyColor = vec3(0.65, 0.9, 1.0);
    vec3 vUpperSkyColor = vec3(0.06, 0.4, 0.85);

    vec3 ndcToFarWorldspace(vec2 screenNdc)
    {
        mat4 ndcToWorld = inverse(P*V);
        vec4 frag_ndc = vec4(screenNdc, 1.0, 1.0);
        frag_ndc /= gl_FragCoord.w;

        vec3 frag_worldspace = (ndcToWorld * frag_ndc).xyz;
        return frag_worldspace;
    }

    vec3 getSkyColor(vec2 ndcUv, vec3 sun_projectionspace)
    {
        vec3 frag_worldspace = ndcToFarWorldspace(ndcUv);
        float fade = normalize(frag_worldspace).y;
        fade = clamp(fade, 0.0, 1.0);
        fade = 1.0 - pow(1.0 - fade, 7.0);

        return mix(vLowerSkyColor, vUpperSkyColor, fade);
    }

    void main()
    {
        vec2 uv = gl_FragCoord.xy / SCE_ScreenSize;
        vec4 sceneColor = texture(FinalColorTex, uv);
        vec3 Position_worldpsace = texture(PositionTex, uv).xyz;
        vec4 sunData = texture(SunTex, uv);
        vec3 sunColor = sunData.rgb;

        //compute fog strength
        float fogStr = 0.001;
        float dist = abs((V * vec4(Position_worldpsace, 1.0)).z);
        float fogAmount = 1.0 - exp( -dist * fogStr );
        float notOccludedByScene = step(dot(Position_worldpsace, Position_worldpsace), 0.0001);
        fogAmount = min(1.0, fogAmount + notOccludedByScene);

        //compute normalized device coord and screenspace sun position
        vec3 sun_cameraspace = (V * vec4(SunPosition_worldspace, 1.0)).xyz;
        vec2 ndcUv = uv * 2.0 - vec2(1.0);
        vec4 sun_projectionspace = (P * vec4(sun_cameraspace, 1.0));
        sun_projectionspace /= sun_projectionspace.w;
        vec2 sunUV = sun_projectionspace.xy * 0.5 + vec2(0.5);

        vec3 skyColor = getSkyColor(ndcUv, sun_projectionspace.xyz);
        skyColor += sunColor;

        color = vec4(0.0, 0.0, 0.0, 1.0);
        color.rgb = mix(sceneColor.rgb, skyColor, fogAmount);
        color.rgb += sunData.a * vSunColor;
    }
_}
