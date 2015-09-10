[VertexShader]
_{
#version 400 core

    in vec3 vertexPosition_modelspace;
    in vec2 vertexUV;
    in vec3 vertexNormal_modelspace;
    in vec3 vertexTangent;
    in vec3 vertexBitangent;

    out vec2 fragUV;
    out mat3 tangentToWorldspace;
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
        vec3 Normal_worldspace = ( M * vec4(vertexNormal_modelspace, 0.0) ).xyz;
        vec3 Tangent_worldspace = ( M * vec4(vertexTangent, 0.0) ).xyz;
        vec3 Bitangent_worldspace = ( M * vec4(vertexBitangent, 0.0) ).xyz;

        tangentToWorldspace = (mat3(
                Tangent_worldspace,
                Bitangent_worldspace,
                Normal_worldspace
            ));
    }
_}

[FragmentShader]
_{
#version 400 core

    in vec2 fragUV;
    in mat3 tangentToWorldspace;
    in vec3 Position_worldspace;

    layout (location = 0) out vec3 oPosition;
    layout (location = 1) out vec3 oColor;
    layout (location = 2) out vec4 oNormal;

    uniform sampler2D MainTex;
    uniform sampler2D NormalMap;
    uniform sampler2D SpecMap;
    uniform float Specularity;
    uniform float ScaleU;
    uniform float ScaleV;

    void main()
    {
        vec2 uv = vec2(fragUV) * vec2(ScaleU, ScaleV);
        oColor = texture(MainTex, uv).xyz;
        //convert color to linear space
        //will do gamma correction in the last shading pass
        oColor = pow(oColor, vec3(2.2));

        oPosition = Position_worldspace;

        vec3 normal = texture(NormalMap, uv).xyz;
        normal = normal * 2.0 - vec3(1.0);
        normal = normalize(tangentToWorldspace * normal);
        oNormal.xyz = normal;
        //specularity
        vec4 specularity = texture(SpecMap, uv);
        oNormal.a = Specularity * length(specularity);
    }
_}
