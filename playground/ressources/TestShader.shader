VertexShader : 
_{
#version 400 core

    in vec3 vertexPosition_modelspace;
    in vec2 vertexUV;
    in vec3 vertexNormal_modelspace;
    in vec3 vertexTangent;
    in vec3 vertexBitangent;

    out vec2 fragUV;
    out mat3 tangentToWorldspace;
    out vec3 Normal_worldspace;
    out vec3 Position_worldspace;

    uniform mat4 MVP;
    uniform mat4 M;
    uniform mat4 V;
    uniform mat4 P;

    void main()
    {
        gl_Position = MVP * vec4(vertexPosition_modelspace, 1.0);
        fragUV = vertexUV;
        Position_worldspace = ( M * vec4(vertexPosition_modelspace, 1.0) ).xyz;
        Normal_worldspace = ( M * vec4(vertexNormal_modelspace, 0.0) ).xyz;
        vec3 Tangent_worldspace = ( M * vec4(vertexTangent, 0.0) ).xyz;
        vec3 Bitangent_worldspace = ( M * vec4(vertexBitangent, 0.0) ).xyz;

        tangentToWorldspace = (mat3(
                Tangent_worldspace,
                Bitangent_worldspace,
                Normal_worldspace
            ));
    }
_}

FragmentShader : 
_{
#version 400 core

    in vec2 fragUV;
    in mat3 tangentToWorldspace;
    in vec3 Normal_worldspace;
    in vec3 Position_worldspace;

    layout (location = 0) out vec3 oPosition;
    layout (location = 1) out vec3 oColor;
    layout (location = 2) out vec3 oNormal;

    uniform mat4 M;
    uniform vec3 AmbientColor;
    uniform sampler2D MainTex;
    uniform sampler2D NormalMap;
    uniform float ScaleU;
    uniform float ScaleV;

    void main()
    {
        oColor = texture2D(MainTex, fragUV * vec2(ScaleU, ScaleV)).xyz;
        //gamma expansion of texture because it is store gamma corrected and we will do
        //our own gamma correction in the last shading pass
        oColor = pow(oColor, vec3(2.2));
        oPosition = Position_worldspace;
        vec3 normal = texture2D(NormalMap, fragUV * vec2(ScaleU, ScaleV)).xyz;
        normal = normal * 2.0 - vec3(1.0);
        normal = normalize(tangentToWorldspace * normal);
//        normal = normalize(M * vec4(normal, 0.0)).xyz;
        oNormal = normal;

//        oColor = oNormal;
    }
_}
