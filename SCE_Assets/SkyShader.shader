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

//    out vec3 vertexPosition_worldspace;
//    out vec3 eyePosition_worldspace;
    out vec3 vertexPosition_model;

    uniform mat4 MVP;
    uniform mat4 M;
    uniform mat4 V;
    uniform mat4 P;

    void main()
    {
        gl_Position                 = MVP * vec4(vertexPosition_modelspace, 1.0);
//        vertexPosition_worldspace   = (M * vec4(vertexPosition_modelspace, 1.0)).xyz;
//        eyePosition_worldspace      = (inverse(V) * vec4(0.0, 0.0, 0.0, 1.0)).xyz;
        vertexPosition_model = vertexPosition_modelspace;
    }
_}

FragmentShader :
_{
#version 430 core

    uniform vec3 SunPosition_worldspace;

    uniform mat4 V;
    uniform mat4 P;

//    in vec3 vertexPosition_worldspace;
//    in vec3 eyePosition_worldspace;
    in vec3 vertexPosition_model;

    out vec4 color;

    vec3 ndcToFarWorldspace(vec2 screenNdc)
    {
        mat4 ndcToWorld = inverse(P*V);
        vec4 frag_ndc = vec4(screenNdc, 1.0, 1.0);
        frag_ndc /= gl_FragCoord.w;

        vec3 frag_worldspace = (ndcToWorld * frag_ndc).xyz;
        return frag_worldspace;
    }

    vec3 getSkyColor()
    {
        vec3 frag_worldspace = ndcToFarWorldspace(vertexPosition_model.xy);
        vec3 fade = vec3(normalize(frag_worldspace).y + 0.1);
        fade = clamp(fade, vec3(0.0), vec3(0.7));
        fade = vec3(1.0) - pow(vec3(1.0) - fade, vec3(7.0, 7.0, 5.0));

        vec3 lowerColor = vec3(0.65, 0.9, 1.0);
        vec3 upperColor = vec3(0.06, 0.4, 0.85);
        return mix(lowerColor, upperColor, fade);
    }

    void main()
    {
        color = vec4(getSkyColor(), 1.0);
    }
_}
