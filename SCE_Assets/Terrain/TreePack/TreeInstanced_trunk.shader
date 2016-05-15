[VertexShader]
_{
#version 430 core

    in vec3 vertexPosition_modelspace;
    in vec2 vertexUV;
    in vec3 vertexNormal_modelspace;
    in vec3 vertexTangent;
    in vec3 vertexBitangent;
    in mat4 instanceMatrix;

    out vec2 fragUV;
    out mat3 tangentToWorldspace;
    out vec3 Position_worldspace;

    uniform mat4 V;
    uniform mat4 P;

    void main()
    {
        fragUV = vertexUV;
        Position_worldspace = ( instanceMatrix * vec4(vertexPosition_modelspace, 1.0) ).xyz;
        vec3 Normal_worldspace = ( instanceMatrix * vec4(vertexNormal_modelspace, 0.0) ).xyz;
        vec3 Tangent_worldspace = ( instanceMatrix * vec4(vertexTangent, 0.0) ).xyz;
        vec3 Bitangent_worldspace = ( instanceMatrix * vec4(vertexBitangent, 0.0) ).xyz;

        tangentToWorldspace = (mat3(
                Tangent_worldspace,
                Bitangent_worldspace,
                Normal_worldspace
            ));

        gl_Position = P * V * vec4(Position_worldspace, 1.0);
    }
_}

[FragmentShader]
_{
#version 430 core

    in vec2 fragUV;
    in mat3 tangentToWorldspace;
    in vec3 Position_worldspace;

    layout (location = 0) out vec3 oPosition;
    layout (location = 1) out vec3 oColor;
    layout (location = 2) out vec4 oNormal;

    uniform mat4 V;
    uniform sampler2D BarkTex;
    uniform sampler2D BarkNormalMap;

    float BarkRoughness = 0.9;

    void main()
    {
        vec2 uv = vec2(fragUV);
        vec3 normal = texture(BarkNormalMap, uv).xyz;
        normal = normal * 2.0 - vec3(1.0);
        oColor = texture(BarkTex, uv).xyz;
//        oColor = vec3(0.12, 0.02, 0.0);

        normal = normalize(tangentToWorldspace * normal);
        oNormal.xyz = normal;
        oNormal.a = BarkRoughness;
        //convert color to linear space
        //will do gamma correction in the last shading pass
        oColor = pow(oColor, vec3(2.2));
        oPosition = Position_worldspace;
    }
_}
