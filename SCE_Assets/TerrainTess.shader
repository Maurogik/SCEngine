/***** PROJECT:Sand Castle Engine *****/
/**************************************/
/******** AUTHOR:Gwenn AUBERT *********/
/****** FILE:TerrainTess.shader *******/
/**************************************/
/**************** ????µs **************/

[VertexShader]
_{
#version 430 core

    uniform mat4 QuadToTerrainSpace;

    in vec3 vertexPosition_modelspace;
//    in vec3 vertexNormal_modelspace;

    out vec2 VS_terrainTexCoord;

    void main()
    {
        vec4 pos_terrainspace = QuadToTerrainSpace * vec4(vertexPosition_modelspace, 1.0);
        VS_terrainTexCoord = pos_terrainspace.xz * 0.5 + vec2(0.5);
        gl_Position = vec4(vertexPosition_modelspace, 1.0);
    }
_}

[TCS]
_{
#version 430 core

    uniform sampler2D TerrainHeightMap;
    uniform mat4 MV;
    uniform mat4 MVP;

    // Inputs
    in vec2 VS_terrainTexCoord[];

    // Outputs
    layout(vertices = 4) out;

    patch out float gl_TessLevelOuter[4];
    patch out float gl_TessLevelInner[2];

    out vec2 TCS_terrainTexCoord[];


    float tesselationFromDist(vec4 p0, vec4 p1, vec2 t0, vec2 t1)
    {

        vec2 centerUv = (t0 - t1) * 0.5 + t1;
        float height = texture(TerrainHeightMap, centerUv);

        float farPlane = 500.0;
        vec4 center_cameraspace = MV * ((p0 - p1) * 0.5 + p1 + vec4(0.0, height, 0.0, 0.0));
        float tess = center_cameraspace.z / farPlane * 64;//map to 0..64 range

        return tess;//between 0 and 64
    }

    bool offscreen(vec4 vertex_modelspace){
        vec4 vert_ndc = MVP * vertex_modelspace;
        return vert_ndc.z < -0.7 || abs(vert_ndc.x) > 1.0 || abs(vert_ndc.y) > 1.0;
    }

    void main(void)
    {
        if(gl_InvocationID == 0)
        {
            vec4 p0 = gl_in[0].gl_Position;
            vec4 p1 = gl_in[1].gl_Position;
            vec4 p2 = gl_in[2].gl_Position;
            vec4 p3 = gl_in[3].gl_Position;

           /* if( offscreen(p0) &&
                offscreen(p1) &&
                offscreen(p2) &&
                offscreen(p3))
            { //Quad is offscreen, discard tesselation
                gl_TessLevelInner[0] = 0;
                gl_TessLevelInner[1] = 0;
                gl_TessLevelOuter[0] = 0;
                gl_TessLevelOuter[1] = 0;
                gl_TessLevelOuter[2] = 0;
                gl_TessLevelOuter[3] = 0;
            }
            else*/
            {
                // Outer tessellation level
                gl_TessLevelOuter[0] = tesselationFromDist( p3, p0,
                        VS_terrainTexCoord[3], VS_terrainTexCoord[0]);

                gl_TessLevelOuter[1] = tesselationFromDist( p0, p1,
                        VS_terrainTexCoord[0], VS_terrainTexCoord[1]);

                gl_TessLevelOuter[2] = tesselationFromDist( p1, p2,
                        VS_terrainTexCoord[1], VS_terrainTexCoord[2]);

                gl_TessLevelOuter[3] = tesselationFromDist( p2, p3,
                        VS_terrainTexCoord[2], VS_terrainTexCoord[3]);

                // Inner tessellation level
                gl_TessLevelInner[0] = 0.5 * (gl_TessLevelOuter[0] + gl_TessLevelOuter[3]);
                gl_TessLevelInner[1] = 0.5 * (gl_TessLevelOuter[2] + gl_TessLevelOuter[1]);
            }
        }

        // Pass the patch verts along
        gl_out[gl_InvocationID].gl_Position = gl_in[gl_InvocationID].gl_Position;

        // Pass texture coordinates along
        TCS_terrainTexCoord[gl_InvocationID] = VS_terrainTexCoord[gl_InvocationID];
    }

_}

[TES]
_{
#version 430 core

    uniform mat4 MVP;
    uniform mat4 M;
    uniform sampler2D TerrainHeightMap;

    // Inputs
    layout(quads, fractional_even_spacing) in;

    patch in float gl_TessLevelOuter[4];
    patch in float gl_TessLevelInner[2];

    in vec2 TCS_terrainTexCoord[];

    // Outputs
    out float TES_tessLevel;
    out vec3 TES_Position_worldspace;

    void main()
    {
        vec4 bottomEdge = mix(gl_in[0].gl_Position, gl_in[1].gl_Position, gl_TessCoord.x);
        vec4 topEdge = mix(gl_in[3].gl_Position, gl_in[2].gl_Position, gl_TessCoord.x);
        vec4 position = mix(bottomEdge, topEdge, gl_TessCoord.y);

        // Interpolate texture coordinates
        vec2 bottomTerrainUv = mix(TCS_terrainTexCoord[0], TCS_terrainTexCoord[1], gl_TessCoord.x);
        vec2 topTerrainUv = mix(TCS_terrainTexCoord[3], TCS_terrainTexCoord[2], gl_TessCoord.x);
        vec2 terrainTexCoord = mix(bottomTerrainUv, topTerrainUv, gl_TessCoord.y);

        float height = texture(TerrainHeightMap ,terrainTexCoord);
        position.y += height;

        // Send along the tessellation level (for color coded wireframe)
        TES_tessLevel = gl_TessLevelOuter[0];

        // Project the vertex to clip space and send it along
        gl_Position = MVP * position;
        TES_Position_worldspace = (M * position).xyz;
    }
_}

[Geometry]
_{
#version 430 core

    layout(triangles) in;
    layout(triangle_strip, max_vertices = 4) out;

    in float TES_tessLevel[];
    in vec3 TES_Position_worldspace[];

    out vec3 Normal_worldspace;
    out vec3 Position_worldspace;
    out vec4 TESS_color;

    vec4 wireframeColor(float tessLevel)
    {
        if (tessLevel == 64.0)
            return vec4(0.0, 0.0, 1.0, 1.0);
        else if (tessLevel > 32.0)
            return vec4(0.0, 1.0, 1.0, 1.0);
        else if (tessLevel > 16.0)
            return vec4(1.0, 1.0, 0.0, 1.0);
        else
            return vec4(1.0, 0.0, 0.0, 1.0);
    }

    void main()
    {
        vec3 A = TES_Position_worldspace[2] - TES_Position_worldspace[0];
        vec3 B = TES_Position_worldspace[1] - TES_Position_worldspace[0];
        Normal_worldspace = normalize(cross(A, B));

//        TESS_color = wireframeColor(TES_tessLevel[0]);
//        Position_worldspace = TES_Position_worldspace[0];
//        gl_Position = gl_in[0].gl_Position;
//        EmitVertex();

//        TESS_color = wireframeColor(TES_tessLevel[1]);
//        Position_worldspace = TES_Position_worldspace[1];
//        gl_Position = gl_in[1].gl_Position;
//        EmitVertex();

//        TESS_color = wireframeColor(TES_tessLevel[2]);
//        Position_worldspace = TES_Position_worldspace[2];
//        gl_Position = gl_in[2].gl_Position;
//        EmitVertex();

//        TESS_color = wireframeColor(TES_tessLevel[3]);
//        Position_worldspace = TES_Position_worldspace[3];
//        gl_Position = gl_in[3].gl_Position;
//        EmitVertex();

        // Output verts
        for(int i = 0; i < gl_in.length(); ++i)
        {
            TESS_color = wireframeColor(TES_tessLevel[i]);
            Position_worldspace = TES_Position_worldspace[i];
            gl_Position = gl_in[i].gl_Position;
            EmitVertex();
        }

        TESS_color = wireframeColor(TES_tessLevel[0]);
        Position_worldspace = TES_Position_worldspace[0];
        gl_Position = gl_in[0].gl_Position;
        EmitVertex();

        EndPrimitive();
    }

_}

[FragmentShader]
_{
#version 430 core

    layout (location = 0) out vec3 oPosition;
    layout (location = 1) out vec3 oColor;
    layout (location = 2) out vec4 oNormal;

    in vec3 Normal_worldspace;
    in vec3 Position_worldspace;
    in vec3 TES_Position_worldspace;
    in vec4 TESS_color;

    void main()
    {
        oNormal = vec4(1.0);
        oColor = TESS_color.xyz;
        oPosition = TES_Position_worldspace;
    }
_}
