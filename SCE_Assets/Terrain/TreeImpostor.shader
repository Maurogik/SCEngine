[VertexShader]
_{
#version 400 core

    in vec3 vertexPosition_modelspace;
    in vec2 vertexUV;
    in vec3 vertexNormal_modelspace;
    in mat4 instanceMatrix;
    in vec3 vertexTangent;
    in vec3 vertexBitangent;

    out vec2 FragUV;
    out mat3 TangentToWorldspace;
    out vec3 Position_worldspace;

    uniform mat4 ScaleInvertMat;
    uniform mat4 V;
    uniform mat4 P;

    void main()
    {
        FragUV = vertexUV;
        Position_worldspace = (instanceMatrix*vec4(vertexPosition_modelspace, 1.0)).xyz;
        mat4 normalModelMatrix = instanceMatrix;ScaleInvertMat;
        vec3 Normal_worldspace = ( normalModelMatrix*vec4(vertexNormal_modelspace, 0.0) ).xyz;
        vec3 Tangent_worldspace = ( normalModelMatrix*vec4(vertexTangent, 0.0) ).xyz;
        vec3 Bitangent_worldspace = ( normalModelMatrix*vec4(vertexBitangent, 0.0) ).xyz;

        TangentToWorldspace = (mat3(
            Tangent_worldspace,
            Bitangent_worldspace,
            Normal_worldspace
        ));

        gl_Position = P*V*vec4(Position_worldspace, 1.0);
    }
_}

[FragmentShader]
_{
#version 430 core

    in vec2 FragUV;
    in mat3 TangentToWorldspace;
    in vec3 Position_worldspace;

    uniform sampler2D ImpostorTex;
    uniform sampler2D ImpostorNormalTex;

//    uniform vec2 SCE_ScreenSize;

//    uniform sampler2D iDiffuseTex;
//    uniform sampler2D iPositionTex;
//    uniform sampler2D iNormalTex;

    layout (location = 0) out vec3 oPosition;
    layout (location = 1) out vec4 oColor;
    layout (location = 2) out vec4 oNormal;

    float ImpostorTranslucency = 0.1;
    float ImpostorRoughness = 0.95;

    void main()
    {
        vec4 color = texture(ImpostorTex, FragUV);
        color.rgb = pow(color.rgb, vec3(2.2));        

        //vec2 screenUV = gl_FragCoord.xy / SCE_ScreenSize;

        if(color.a > 0.6)
        {
            vec3 normal = texture(ImpostorNormalTex, FragUV).xyz;
            normal = pow(normal, vec3(2.2));
            normal = normal*2.0 - vec3(1.0);
            normal = normalize(TangentToWorldspace*normal);
            oNormal.xyz = normal;

            oColor = vec4(color.rgb, ImpostorTranslucency);
            oPosition = Position_worldspace;
            oNormal.a = ImpostorRoughness;
        }
        else
        {
//            oColor = texture(iDiffuseTex, screenUV);
//            oNormal = texture(iNormalTex, screenUV);
//            oPosition = texture(iPositionTex, screenUV).xyz;
            discard;
        }
    }
_}
