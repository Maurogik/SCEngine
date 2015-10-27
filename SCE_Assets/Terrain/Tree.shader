[VertexShader]
_{
#version 400 core

    in vec3 vertexPosition_modelspace;
    in vec2 vertexUV;
    in vec3 vertexNormal_modelspace;
    in mat4 instanceMatrix;

    out vec2 fragUV;
    out vec3 Normal_worldspace;
    out vec3 Position_worldspace;
    out vec3 Position_modelspace;

    uniform mat4 WorldToTerrainSpace;
    uniform mat4 V;
    uniform mat4 P;
    uniform float PatchSize;
    uniform sampler2D TerrainHeightMap;

    void main()
    {
        Position_modelspace = vertexPosition_modelspace;
        Position_worldspace = ( instanceMatrix * vec4(vertexPosition_modelspace, 1.0) ).xyz;

        vec4 pos_terrainspace = WorldToTerrainSpace * vec4(Position_worldspace, 1.0);
        float height = texture(TerrainHeightMap, pos_terrainspace.zx * 0.5 + vec2(0.5)).a;

        fragUV = vertexUV;
        Normal_worldspace = ( instanceMatrix * vec4(vertexNormal_modelspace, 0.0) ).xyz;

        gl_Position = P * V * vec4(Position_worldspace, 1.0);
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

    vec3 LeavesColor = vec3(0.0, 0.05, 0.0);
    vec3 TruncColor = vec3(0.10, 0.025, 0.0);
    float Specularity = 0.02;

    void main()
    {
        float trunc = step(Position_modelspace.y, 8.9);
        oColor = trunc * TruncColor + (1.0 - trunc) * LeavesColor;

        oPosition = Position_worldspace;

        oNormal.xyz = normalize(Normal_worldspace);
        oNormal.a = Specularity;
    }
_}
