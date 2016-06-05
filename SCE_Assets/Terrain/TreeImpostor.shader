[VertexShader]
_{
#version 400 core

    layout(location = 0)in vec3 vertexPosition_modelspace;
    layout(location = 1)in vec2 vertexUV;
    layout(location = 2)in vec3 vertexNormal_modelspace;
    layout(location = 3)in vec3 vertexTangent;
    layout(location = 4)in vec3 vertexBitangent;

    layout(location = 5)in vec4 instanceCustomData;
    layout(location = 6)in mat4 instanceMatrix;

    out vec2 FragUV;
    out mat3 TangentToWorldspace;
    out vec3 Position_worldspace;

    uniform mat4 V;
    uniform mat4 P;
    uniform vec3 SCE_RootPosition;

    float map(float f, vec2 m)
    {
        return m.x + f*m.y;
    }

    void main()
    {
        FragUV = vertexUV;
        vec4 mapping = instanceCustomData;
//        if(gl_InstanceID%2 == 0)
        {

            FragUV = vec2(map(FragUV.x, mapping.xz), map(FragUV.y, mapping.yw));
        }
        mat4 modelMatrix = instanceMatrix;
        modelMatrix[3] -= vec4(SCE_RootPosition, 0.0);

        Position_worldspace = (modelMatrix*vec4(vertexPosition_modelspace, 1.0)).xyz;
        vec3 Normal_worldspace = ( modelMatrix*vec4(vertexNormal_modelspace, 0.0) ).xyz;
        vec3 Tangent_worldspace = ( modelMatrix*vec4(vertexTangent, 0.0) ).xyz;
        vec3 Bitangent_worldspace = ( modelMatrix*vec4(vertexBitangent, 0.0) ).xyz;

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

    float ImpostorRoughness = 0.95;

    void main()
    {
#if USE_ARRAY
        vec3 arrayUv = vec3(FragUV, 0.0);
#else
        vec2 arrayUv = FragUV;
#endif
        vec4 color = texture(ImpostorTex, arrayUv);

        if(color.a > 0.2)
        {
            vec4 normalAndTranslu = texture(ImpostorNormalTex, arrayUv);
            vec3 normal = normalAndTranslu.xyz;
            normal = normal * 2.0 - vec3(1.0);
            normal = normalize(TangentToWorldspace*normal);
            oNormal.xyz = normal;

            oColor = vec4(color.xyz, normalAndTranslu.w);
            oPosition = Position_worldspace;
            oNormal.a = ImpostorRoughness;
        }
        else
        {
            discard;
        }
    }
_}
