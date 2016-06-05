[VertexShader]
_{
#version 430 core

    in vec3 vertexPosition_modelspace;
    in vec2 vertexUV;
    in vec3 vertexNormal_modelspace;
    in mat4 instanceMatrix;

    out vec2 fragUV;
    out vec3 Normal_worldspace;
    out vec3 Position_worldspace;

    uniform mat4 V;
    uniform mat4 P;    
    uniform vec3 SCE_RootPosition;

    void main()
    {
        fragUV = vertexUV;
        mat4 modelMatrix = instanceMatrix;
        modelMatrix[3] -= vec4(SCE_RootPosition, 0.0);

        Position_worldspace = ( modelMatrix * vec4(vertexPosition_modelspace, 1.0) ).xyz;
        Normal_worldspace = ( modelMatrix * vec4(vertexNormal_modelspace, 0.0) ).xyz;

        gl_Position = P * V * vec4(Position_worldspace, 1.0);
    }
_}

[FragmentShader]
_{
#version 430 core

    in vec2 fragUV;
    in vec3 Normal_worldspace;
    in vec3 Position_worldspace;

    layout (location = 0) out vec3 oPosition;
    layout (location = 1) out vec4 oColor;
    layout (location = 2) out vec4 oNormal;

    uniform mat4 V;
    uniform sampler2D LeafTex;

//    uniform vec2 SCE_ScreenSize;

//    uniform sampler2D iDiffuseTex;
//    uniform sampler2D iPositionTex;
//    uniform sampler2D iNormalTex;

    float Roughness = 0.9;
    uniform float Translucency;// = 0.5;

    void main()
    {
        vec2 uv = vec2(fragUV);
//        vec2 screenUV = gl_FragCoord.xy / SCE_ScreenSize;
        vec4 texColor = texture(LeafTex, uv);
        if(texColor.a > 0.6)
        {
            //discard;
            vec3 color = texColor.rgb;

            oNormal.xyz = normalize(Normal_worldspace);
            oNormal.a = Roughness;
            //convert color to linear space
            //will do gamma correction in the last shading pass
            color = pow(color, vec3(2.2));
            oColor = vec4(color, Translucency);
            oPosition = Position_worldspace;
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
