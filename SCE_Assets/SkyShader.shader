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
    uniform vec3        SunPosition_worldspace;
    uniform sampler2D   PositionTex;

    uniform mat4 V;
    uniform mat4 P;

    out vec4 color;

    vec3 ndcToFarWorldspace(vec2 screenNdc)
    {
        mat4 ndcToWorld = inverse(P*V);
        vec4 frag_ndc = vec4(screenNdc, 1.0, 1.0);
        frag_ndc /= gl_FragCoord.w;

        vec3 frag_worldspace = (ndcToWorld * frag_ndc).xyz;
        return frag_worldspace;
    }

    vec3 getSkyColor(vec2 nbcUv)
    {
        vec3 frag_worldspace = ndcToFarWorldspace(nbcUv);
        vec3 fade = vec3(normalize(frag_worldspace).y);
        fade = clamp(fade, vec3(0.0), vec3(0.7));
        fade = vec3(1.0) - pow(vec3(1.0) - fade, vec3(7.0, 7.0, 5.0));

        vec3 lowerColor = vec3(0.65, 0.9, 1.0);
        vec3 upperColor = vec3(0.06, 0.4, 0.85);
        vec3 skyColor = mix(lowerColor, upperColor, fade);

        vec3 sun_cameraspace = (V * vec4(SunPosition_worldspace, 1.0)).xyz;

        vec3 eyeToSun = normalize(sun_cameraspace);
        vec2 fixedNdc = nbcUv * vec2(1.0, SCE_ScreenSize.y/SCE_ScreenSize.x); //compensate for aspect ratio
        vec3 eyeToSky = normalize(vec3(fixedNdc, 1.0));

        float sun = clamp(dot(eyeToSun, eyeToSky), 0.0, 1.0);
        sun = pow(sun, 2.0) * 0.5 + smoothstep(0.0, 0.0035, sun - 0.996);
        sun = clamp(sun, 0.0, 1.0);

        vec3 sunColor = vec3(1.0, 1.0, 0.9);
        return mix(skyColor, sunColor, sun);
    }

    void main()
    {
        //use position in model space as uv since we are rendering a simple quad
        vec2 uv = gl_FragCoord.xy / SCE_ScreenSize;
        vec3 Position_worldpsace = texture2D(PositionTex, uv).xyz;//vec4(getSkyColor(), 1.0);

        float fogStr = 0.001;
        float dist = abs((V * vec4(Position_worldpsace, 1.0)).z);
        float fogAmount = 1.0 - exp( -dist * fogStr );
        fogAmount = min(1.0, fogAmount + step(dot(Position_worldpsace, Position_worldpsace), 0.01));

        color.rgb = getSkyColor(uv * 2.0 - vec2(1.0));
        color.a = fogAmount;
    }
_}
