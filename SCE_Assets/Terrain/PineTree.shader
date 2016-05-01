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

    void main()
    {
        Position_worldspace = ( instanceMatrix * vec4(vertexPosition_modelspace, 1.0) ).xyz;
        //because fuck it
        fragUV.x = vertexUV.y;
        fragUV.y = vertexUV.x;
        //fragUV.y *= -1.0;
        Normal_worldspace = ( instanceMatrix * vec4(vertexNormal_modelspace, 0.0) ).xyz;

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
    layout (location = 1) out vec3 oColor;
    layout (location = 2) out vec4 oNormal;

    uniform mat4 V;
    uniform sampler2D AlphaTex;
    uniform sampler2D ColorTex;

    float Roughness = 2.0;

    void main()
    {
        oColor = texture(ColorTex, fragUV).xyz;
        oColor = pow(oColor, vec3(2.2));

        oPosition = Position_worldspace;

        oNormal.xyz = normalize(Normal_worldspace);
        oNormal.a = Roughness;
    }
_}
