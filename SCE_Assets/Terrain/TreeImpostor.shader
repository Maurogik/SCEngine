[VertexShader]
_{
#version 400 core

    layout(location = 0)in vec3 vertexPosition_modelspace;
    layout(location = 1)in vec2 vertexUV;
    layout(location = 2)in vec3 vertexNormal_modelspace;
    layout(location = 3)in vec3 vertexTangent;
    layout(location = 4)in vec3 vertexBitangent;

    layout(location = 5)in uint instanceCustomData;
    layout(location = 6)in mat4 instanceMatrix;

    out vec2 FragUV;
    out mat3 TangentToWorldspace;
    out vec3 Position_worldspace;

    uniform mat4 ScaleInvertMat;
    uniform mat4 V;
    uniform mat4 P;

#define NA 16
#define NAF 4.0
    vec4 TexMappings[NA]=vec4[](
        vec4(0.0/NAF, 0.0/NAF, 1.0/NAF, 1.0/NAF),
        vec4(1.0/NAF, 0.0/NAF, 2.0/NAF, 1.0/NAF),
        vec4(2.0/NAF, 0.0/NAF, 3.0/NAF, 1.0/NAF),
        vec4(3.0/NAF, 0.0/NAF, 4.0/NAF, 1.0/NAF),

        vec4(0.0/NAF, 1.0/NAF, 1.0/NAF, 2.0/NAF),
        vec4(1.0/NAF, 1.0/NAF, 2.0/NAF, 2.0/NAF),
        vec4(2.0/NAF, 1.0/NAF, 3.0/NAF, 2.0/NAF),
        vec4(3.0/NAF, 1.0/NAF, 4.0/NAF, 2.0/NAF),

        vec4(0.0/NAF, 2.0/NAF, 1.0/NAF, 3.0/NAF),
        vec4(1.0/NAF, 2.0/NAF, 2.0/NAF, 3.0/NAF),
        vec4(2.0/NAF, 2.0/NAF, 3.0/NAF, 3.0/NAF),
        vec4(3.0/NAF, 2.0/NAF, 4.0/NAF, 3.0/NAF),

        vec4(0.0/NAF, 3.0/NAF, 1.0/NAF, 4.0/NAF),
        vec4(1.0/NAF, 3.0/NAF, 2.0/NAF, 4.0/NAF),
        vec4(2.0/NAF, 3.0/NAF, 3.0/NAF, 4.0/NAF),
        vec4(3.0/NAF, 3.0/NAF, 4.0/NAF, 4.0/NAF)
    );

    float map(float f, vec2 m)
    {
        float d = m.y - m.x;
        return m.x + f*d;
    }

    void main()
    {
        FragUV = vertexUV;
//        uint ind = uint(gl_InstanceID)%16;
        uint ind = instanceCustomData;
        if(ind > 15)
        {
            ind = 15;
        }
        if(ind < 0)
        {
            ind = 0;
        }

        vec4 mapping = TexMappings[ind];
        FragUV = vec2(map(FragUV.x, mapping.xz), map(FragUV.y, mapping.yw));

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

#define USE_ARRAY 0

    in vec2 FragUV;
    in mat3 TangentToWorldspace;
    in vec3 Position_worldspace;

#if USE_ARRAY
    uniform sampler2DArray ImpostorTex;
    uniform sampler2DArray ImpostorNormalTex;
#else
    uniform sampler2D ImpostorTex;
    uniform sampler2D ImpostorNormalTex;
#endif

    layout (location = 0) out vec3 oPosition;
    layout (location = 1) out vec4 oColor;
    layout (location = 2) out vec4 oNormal;

    float ImpostorTranslucency = 0.1;
    float ImpostorRoughness = 0.95;

    void main()
    {
#if USE_ARRAY
        vec3 arrayUv = vec3(FragUV, 0.0);
#else
        vec2 arrayUv = FragUV;
#endif
        vec4 color = texture(ImpostorTex, arrayUv);
        //not needed because generated textures are not gamma corrected
        //color.rgb = pow(color.rgb, vec3(2.2));

        //vec2 screenUV = gl_FragCoord.xy / SCE_ScreenSize;

        if(color.a > 0.2)
        {
            vec3 normal = texture(ImpostorNormalTex, arrayUv).xyz;
            normal = pow(normal, vec3(2.2));
            normal = normal*2.0 - vec3(1.0);
            normal = normalize(TangentToWorldspace*normal);
            oNormal.xyz = normal;

            oColor = vec4(color.xyz, ImpostorTranslucency);
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
