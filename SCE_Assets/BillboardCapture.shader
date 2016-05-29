[VertexShader]
_{
#version 400 core

    in vec3 vertexPosition_modelspace;
    in vec2 vertexUV;
    in vec3 vertexNormal_modelspace;
    in vec3 vertexTangent;
    in vec3 vertexBitangent;

    out vec2 fragUV;
    out mat3 tangentToCameraspace;
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
        vec3 Normal_cameraspace = ( V *  M * vec4(vertexNormal_modelspace, 0.0) ).xyz;
        vec3 Tangent_cameraspace= ( V * M * vec4(vertexTangent, 0.0) ).xyz;
        vec3 Bitangent_cameraspace = ( V * M * vec4(vertexBitangent, 0.0) ).xyz;

        tangentToCameraspace = (mat3(
                Tangent_cameraspace,
                Bitangent_cameraspace,
                Normal_cameraspace
            ));
    }
_}

[FragmentShader]
_{
#version 400 core

    in vec2 fragUV;
    in mat3 tangentToCameraspace;
    in vec3 Position_worldspace;

    layout (location = 0) out vec4 oColor;
    layout (location = 1) out vec4 oNormal;

    uniform sampler2D MainTex;
    uniform sampler2D NormalMap;

    void main()
    {
        vec2 uv = fragUV;
        oColor = texture(MainTex, uv);
        if(oColor.a > 0.5)
        {
            //convert color to linear space
            //will do gamma correction in the last shading pass
            oColor = pow(oColor, vec4(2.2));
        }
        else
        {
            discard;
        }

        vec3 normal = texture(NormalMap, uv).xyz;
        normal = normal * 2.0 - vec3(1.0);
        normal = normalize(tangentToCameraspace * normal);
        oNormal = vec4(normal, 1.0);
    }
_}
