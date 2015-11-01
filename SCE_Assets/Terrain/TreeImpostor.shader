[VertexShader]
_{
#version 400 core

    in vec3 vertexPosition_modelspace;
    in vec2 vertexUV;
    in vec3 vertexNormal_modelspace;
    in mat4 instanceMatrix;
    in vec3 vertexTangent;
    in vec3 vertexBitangent;

    out vec2 fragUV;
    out mat3 TangentToWorldspace;
    out vec3 Position_worldspace;

    uniform mat4 V;
    uniform mat4 P;

    void main()
    {
        fragUV = vertexUV;
        Position_worldspace = (instanceMatrix * vec4(vertexPosition_modelspace, 1.0)).xyz;
        vec3 Normal_worldspace = ( instanceMatrix * vec4(vertexNormal_modelspace, 0.0) ).xyz;
        vec3 Tangent_worldspace = ( instanceMatrix * vec4(vertexTangent, 0.0) ).xyz;
        vec3 Bitangent_worldspace = ( instanceMatrix * vec4(vertexBitangent, 0.0) ).xyz;

        TangentToWorldspace = (mat3(
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
    in mat3 TangentToWorldspace;
    in vec3 Position_worldspace;

    uniform sampler2D ImpostorTex;
    uniform sampler2D ImpostorNormalTex;

    layout (location = 0) out vec3 oPosition;
    layout (location = 1) out vec3 oColor;
    layout (location = 2) out vec4 oNormal;

    void main()
    {
        vec4 color = texture(ImpostorTex, fragUV);
        color.rgb = pow(color.rgb, vec3(2.2));

        if(color.a > 0.999)
        {
            vec3 normal = texture(ImpostorNormalTex, fragUV).xyz;
            normal = pow(normal, vec3(2.2));
            normal = normal * 2.0 - vec3(1.0);
            normal = normalize(TangentToWorldspace * normal);
            oNormal.xyz = normal;

            oColor = color.rgb;
            oPosition = Position_worldspace;
            oNormal.a = 0.0;
        }
        else
        {
            discard;
        }
    }
_}