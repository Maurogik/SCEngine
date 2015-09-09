/******PROJECT:Sand Castle Engine******/
/**************************************/
/*********AUTHOR:Gwenn AUBERT**********/
/********FILE:SkyShader.shader*********/
/**************************************/
/**************** 1700µs **************/

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
    uniform vec3        SunPosition_worldspace;
    layout (location = 0) uniform sampler2D   FinalColorTex;
    layout (location = 1) uniform sampler2D   PositionTex;

    uniform mat4 V;
    uniform mat4 P;

    out vec4 color;

    //variables
    vec3 sunColor = vec3(1.0, 1.0, 0.9) ;

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
        vec3 fade = vec3(normalize(frag_worldspace).y);
        fade = clamp(fade, vec3(0.0), vec3(0.7));
        fade = vec3(1.0) - pow(vec3(1.0) - fade, vec3(7.0, 7.0, 5.0));

        vec3 lowerColor = vec3(0.65, 0.9, 1.0);
        vec3 upperColor = vec3(0.06, 0.4, 0.85);
        vec3 skyColor = mix(lowerColor, upperColor, fade);

        vec2 fixedNdc = ndcUv * vec2(1.0, SCE_ScreenSize.y/SCE_ScreenSize.x); //compensate for aspect ratio
        vec2 ndcToSun = sun_projectionspace.xy - fixedNdc;
        float sun = clamp(1.0 - length(ndcToSun), 0.0, 1.0) * step(0.001, sun_projectionspace.z);
        sun = pow(sun, 4.0) * 0.8 + smoothstep(0.0, 0.050, sun - 0.92) * 3.0;
        float sunStrength = 10.0;
        sun = clamp(sun, 0.0, sunStrength);
        return mix(skyColor, sunColor, sun);
    }

//#define LIGHT_SHAFTS
#if LIGHT_SHAFTS

    float computeVolumetricLight(vec2 uv, vec2 sunUV)
    {
        int nbSamples = 32;
        float decay = 4.0;
        float density = 0.15;
        float weight = 0.02;

        vec2 stepToFrag = (sunUV - uv);
        stepToFrag *= 1.0 / float(nbSamples) * density;

        float illuminationMult = 1.0;
        float illum = 0.0;
        vec2 sampleUV = uv;

        for(int i = 0; i < nbSamples; ++i)
        {
            sampleUV += stepToFrag;
            vec3 Position_worldpsace = texture(PositionTex, sampleUV).xyz;
            float sampleVal = step(dot(Position_worldpsace, Position_worldpsace), 0.0001);
            sampleVal *= pow(float(nbSamples - i) / float(nbSamples), decay) * weight;
            illum += sampleVal;
        }

        return illum;
    }
#endif

    void main()
    {
        //use position in model space as uv since we are rendering a simple quad
        vec2 uv = gl_FragCoord.xy / SCE_ScreenSize;
        vec3 Position_worldpsace = texture(PositionTex, uv).xyz;
        vec4 sceneColor = texture(FinalColorTex, uv);
        vec3 sun_cameraspace = (V * vec4(SunPosition_worldspace, 1.0)).xyz;


        float fogStr = 0.001;
        float dist = abs((V * vec4(Position_worldpsace, 1.0)).z);
        float fogAmount = 1.0 - exp( -dist * fogStr );
        float notOccludedByScene = step(dot(Position_worldpsace, Position_worldpsace), 0.0001);
        fogAmount = min(1.0, fogAmount + notOccludedByScene);

        vec2 ndcUv = uv * 2.0 - vec2(1.0);
        vec4 sun_projectionspace = (P * vec4(sun_cameraspace, 1.0));
        sun_projectionspace /= sun_projectionspace.w;
        vec2 sunUV = sun_projectionspace.xy * 0.5 + vec2(0.5);

        vec4 skyColor = vec4(getSkyColor(ndcUv, sun_projectionspace.xyz), 1.0);

#if LIGHT_SHAFTS
        float scaterring = 0.0;
        float scaterringStrength = 1.0 - dot(sun_projectionspace.xy, sun_projectionspace.xy) * 0.2;
        if(scaterringStrength > 0.0)
        {
            scaterring = computeVolumetricLight(uv, sunUV) * scaterringStrength;
        }
#endif

        color = mix(sceneColor, skyColor, fogAmount);
        //color.rgb += sunColor * scaterring;
    }
_}
