[VertexShader]
_{
#version 430 core

    in vec3 vertexPosition_modelspace;
    in vec2 vertexUV;
    in vec3 vertexNormal_modelspace;
    in vec3 vertexTangent;
    in vec3 vertexBitangent;
    //in mat4 instanceMatrix;

    out vec2 fragUV;
    out mat3 tangentToWorldspace;
    out vec3 Position_worldspace;
    out vec3 Position_modelspace;

    uniform mat4 M;
    uniform mat4 V;
    uniform mat4 P;

    void main()
    {
        fragUV = vertexUV;
        Position_worldspace = ( M * vec4(vertexPosition_modelspace, 1.0) ).xyz;
        Position_modelspace = vertexPosition_modelspace;
        vec3 Normal_worldspace = ( M * vec4(vertexNormal_modelspace, 0.0) ).xyz;
        vec3 Tangent_worldspace = ( M * vec4(vertexTangent, 0.0) ).xyz;
        vec3 Bitangent_worldspace = ( M * vec4(vertexBitangent, 0.0) ).xyz;

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
    in vec3 Position_modelspace;

    layout (location = 0) out vec3 oPosition;
    layout (location = 1) out vec3 oColor;
    layout (location = 2) out vec4 oNormal;

    uniform mat4 V;
    uniform sampler2D BarkTex;
    uniform sampler2D BarkNormalMap;
    uniform sampler2D LeafTex;
    uniform float BarkRoughness;
    uniform float LeafRoughness;

    void main()
    {
        vec2 uv = vec2(fragUV);
        vec3 normal = vec3(0.0, 0.0, 1.0);

        if(length(Position_modelspace.xz) < 1.0)
        {
            normal = texture(BarkNormalMap, uv).xyz;
            normal = normal * 2.0 - vec3(1.0);
            oColor = texture(BarkTex, uv).xyz;
        }
        else
        {
            vec4 color = texture(LeafTex, uv);
            oColor = color.rgb;
            if(color.a < 0.5)
            {
                discard;
            }
        }

        normal = normalize(tangentToWorldspace * normal);
        oNormal.xyz = normal;
        oNormal.a = BarkRoughness;
        //convert color to linear space
        //will do gamma correction in the last shading pass
        oColor = pow(oColor, vec3(2.2));
        oPosition = Position_worldspace;
    }
_}
