[VertexShader]
_{
#version 400 core

    in vec3 vertexPosition_modelspace;
    in vec2 vertexUV;
    in vec3 vertexNormal_modelspace;

    out vec2 fragUV;
    out vec3 Normal_worldspace;
    out vec3 Position_worldspace;
    out vec3 Position_modelspace;

    uniform mat4 MVP;
    uniform mat4 M;
    uniform mat4 V;
    uniform mat4 P;

    void main()
    {
        gl_Position = MVP * vec4(vertexPosition_modelspace, 1.0);
        fragUV = vertexUV;
        Position_modelspace = vertexPosition_modelspace;
        Position_worldspace = ( M * vec4(vertexPosition_modelspace, 1.0) ).xyz;
        Normal_worldspace = ( M * vec4(vertexNormal_modelspace, 0.0) ).xyz;
    }
_}

[FragmentShader]
_{
#version 400 core

    in vec2 fragUV;
    in vec3 Normal_worldspace;
    in vec3 Position_worldspace;
    in vec3 Position_modelspace;

    layout (location = 0) out vec3 oPosition;
    layout (location = 1) out vec3 oColor;
    layout (location = 2) out vec4 oNormal;

    uniform vec3  LeavesColor;
    uniform vec3  TruncColor;
    uniform float Specularity;

    void main()
    {
        float trunc = step(Position_modelspace.y, 3.0);
        oColor = trunc * TruncColor + (1.0 - trunc) * LeavesColor;

        oPosition = Position_worldspace;

        oNormal.xyz = Normal_worldspace;
        oNormal.a = Specularity;
    }
_}
