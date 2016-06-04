/***** PROJECT:Sand Castle Engine *****/
/**************************************/
/******** AUTHOR:Gwenn AUBERT *********/
/****** FILE:TerrainTess.shader *******/
/**************************************/
/**************** ????µs **************/

[VertexShader]
_{
#version 430 core

    uniform mat4 WorldToTerrainSpace;
    uniform mat4 M;

    in vec3 vertexPosition_modelspace;

    out vec2 VS_terrainTexCoord;

    void main()
    {
        vec4 pos_terrainspace = WorldToTerrainSpace * M * vec4(vertexPosition_modelspace, 1.0);
        VS_terrainTexCoord = pos_terrainspace.zx * 0.5 + vec2(0.5);
        gl_Position = vec4(vertexPosition_modelspace, 1.0);
    }
_}

[TCS]
_{
#version 430 core

    uniform sampler2D TerrainHeightMap;
    uniform float MaxTessDistance;
    uniform float TessLodMultiplier;
    uniform float PatchSize;
    uniform mat4 M;
    uniform mat4 V;
    uniform mat4 MVP;

    //in
    in vec2 VS_terrainTexCoord[];

    //out
    layout(vertices = 4) out;

    patch out float gl_TessLevelOuter[4];
    patch out float gl_TessLevelInner[2];

    out vec2 TCS_terrainTexCoord[];

    float tesselationFromDist(vec4 p0, vec4 p1, vec2 t0, vec2 t1)
    {
        vec2 centerUv = (t0 - t1) * 0.5 + t1;
        float height = texture(TerrainHeightMap, centerUv).a;

        float farDist = MaxTessDistance + 10.0;
        vec4 center_cameraspace = V * ( M * ((p0 - p1) * 0.5 + p1) + vec4(0.0, height, 0.0, 0.0));
        float dist = length(center_cameraspace);
        float tess = 1.0 - clamp((dist - PatchSize)/ farDist, 0.0, 1.0);//map to 0..64 range
        tess = pow(tess, 12.0) / TessLodMultiplier;

        return clamp(tess * 64.0, 4.0, 64.0);//between 0 and 64
    }

    void main(void)
    {
        if(gl_InvocationID == 0)
        {
            vec4 p0 = gl_in[0].gl_Position;
            vec4 p1 = gl_in[1].gl_Position;
            vec4 p2 = gl_in[2].gl_Position;
            vec4 p3 = gl_in[3].gl_Position;

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

        // Pass the patch verts along
        gl_out[gl_InvocationID].gl_Position = gl_in[gl_InvocationID].gl_Position;

        // Pass texture coordinates along
        TCS_terrainTexCoord[gl_InvocationID] = VS_terrainTexCoord[gl_InvocationID];
    }

_}

[TES]
_{
#version 430 core

    uniform mat4 M;
    uniform mat4 V;
    uniform mat4 P;
    uniform sampler2D TerrainHeightMap;    

    //in
    layout(quads, fractional_odd_spacing, ccw) in;
//    layout(quads, equal_spacing, ccw) in;

    patch in float gl_TessLevelOuter[4];
    patch in float gl_TessLevelInner[2];

    in vec2 TCS_terrainTexCoord[];

    //out
    out float TES_tessLevel;
    out vec3 TES_Position_worldspace;
    out vec2 TES_terrainTexCoord;

    void main()
    {
        //Interpolate position
        vec4 bottomEdge = mix(gl_in[0].gl_Position, gl_in[1].gl_Position, gl_TessCoord.x);
        vec4 topEdge = mix(gl_in[3].gl_Position, gl_in[2].gl_Position, gl_TessCoord.x);
        vec4 position = mix(bottomEdge, topEdge, gl_TessCoord.y);

        // Interpolate texture coordinates
        vec2 bottomTerrainUv = mix(TCS_terrainTexCoord[0], TCS_terrainTexCoord[1], gl_TessCoord.x);
        vec2 topTerrainUv = mix(TCS_terrainTexCoord[3], TCS_terrainTexCoord[2], gl_TessCoord.x);
        vec2 terrainTexCoord = mix(bottomTerrainUv, topTerrainUv, gl_TessCoord.y);

        vec4 normAndHeight = texture(TerrainHeightMap, terrainTexCoord);
        vec3 norm = normAndHeight.xyz;

        float height = normAndHeight.a;

        TES_tessLevel = gl_TessLevelOuter[0];
        TES_Position_worldspace = (M * position).xyz;
        TES_Position_worldspace.y += height;
        TES_terrainTexCoord = terrainTexCoord;

        gl_Position = P * V * vec4(TES_Position_worldspace, 1.0);
    }
_}

[Geometry]
_{
#version 430 core

//#define WIREFRAME

    uniform vec2 SCE_ScreenSize;

    //in
    layout(triangles, invocations = 1) in;

    in float TES_tessLevel[];
    in vec3 TES_Position_worldspace[];
    in vec2 TES_terrainTexCoord[];

    //out
    layout(triangle_strip, max_vertices = 4) out;

    noperspective out vec3 gs_edgeDist; //for wireframe
    out vec3 Position_worldspace;
    out vec3 GS_color;
    out vec2 GS_terrainTexCoord;


    vec3 wireframeColor()
    {
        float fTess = TES_tessLevel[0]/64.0;
        return vec3(fTess, 1.0 - fTess, 0.0);
    }

    void main(void)
    {
#ifdef WIREFRAME
        float ha, hb, hc;

        vec2 p0 = vec2(SCE_ScreenSize * (gl_in[0].gl_Position.xy / gl_in[0].gl_Position.w));
        vec2 p1 = vec2(SCE_ScreenSize * (gl_in[1].gl_Position.xy / gl_in[1].gl_Position.w));
        vec2 p2 = vec2(SCE_ScreenSize * (gl_in[2].gl_Position.xy / gl_in[2].gl_Position.w));

        float a = length(p1 - p2);
        float b = length(p2 - p0);
        float c = length(p1 - p0);
        float alpha = acos( (b*b + c*c - a*a) / (2.0*b*c) );
        float beta = acos( (a*a + c*c - b*b) / (2.0*a*c) );
        ha = abs( c * sin( beta ) );
        hb = abs( c * sin( alpha ) );
        hc = abs( b * sin( alpha ) );

#endif

        // Output verts
        for(int i = 0; i < gl_in.length(); ++i)
        {
            GS_color = vec3(0.0);
            Position_worldspace = TES_Position_worldspace[i];
            gl_Position = gl_in[i].gl_Position;
            GS_terrainTexCoord = TES_terrainTexCoord[i];

#ifdef WIREFRAME
            if (i == 0)
                gs_edgeDist = vec3(ha, 0, 0);
            else if (i == 1)
                gs_edgeDist = vec3(0, hb, 0);
            else
                gs_edgeDist = vec3(0, 0, hc);
            GS_color = wireframeColor();
#endif
            EmitVertex();
        }

        EndPrimitive();
    }

_}

[FragmentShader]
_{
#version 430 core

//#define WIREFRAME

    uniform sampler2D TerrainHeightMap;
    uniform sampler2D GrassTex;
    uniform sampler2D DirtTex;
    uniform sampler2D SnowTex;
    uniform mat4 M;
    uniform vec2 SCE_ScreenSize;
    uniform float HeightScale;
    uniform float TextureTileScale;

    //in
    in vec3 Position_worldspace;
    in vec3 GS_color;
    in vec2 GS_terrainTexCoord;

#ifdef WIREFRAME
    noperspective in vec3 gs_edgeDist;
#endif

    //out
    layout (location = 0) out vec3 oPosition;
    layout (location = 1) out vec3 oColor;
    layout (location = 2) out vec4 oNormal;

    vec4 TerrainColor(vec4 normAndHeight, vec2 uv)
    {
        uv *= TextureTileScale;

        vec4 snowColor = vec4(1.0, 1.0, 1.0, 0.2);
        vec4 dirtColor = vec4(0.7, 0.4, 0.2, 0.55);
        vec4 grassColor = vec4(0.2, 0.7, 0.1, 0.65);

        grassColor.rgb = pow(texture(GrassTex, uv).rgb, vec3(2.2));
        dirtColor.rgb = pow(texture(DirtTex, uv).rgb, vec3(2.2));
        snowColor.rgb += pow(texture(SnowTex, uv).rgb, vec3(2.2));

        float height = normAndHeight.a / HeightScale;
        float flatness = dot(normAndHeight.xyz, vec3(0.0, 1.0, 0.0));

        float grassStr = smoothstep(0.25, 0.0, height);
        grassStr += smoothstep(0.4, 0.1, height) * pow(flatness, 1.5);
        grassStr = clamp(grassStr, 0.0, 1.0);

        float snowStr = smoothstep(0.4, 0.6, height);
        snowStr += smoothstep(0.35, 0.4, height) * pow(flatness, 8.0) * 2.0;
        snowStr = clamp(snowStr, 0.0, 1.0);

        vec4 resColor = dirtColor;

        resColor = mix(resColor, grassColor, grassStr);
        resColor = mix(resColor, snowColor, snowStr);

        return resColor;
    }

    void main()
    {
        vec2 uv = gl_FragCoord.xy / SCE_ScreenSize;
        vec4 normAndHeight = texture(TerrainHeightMap, GS_terrainTexCoord);

        vec4 colorAndRough = TerrainColor(normAndHeight, GS_terrainTexCoord);

        vec3 norm = normAndHeight.xyz;
        oNormal = vec4(norm, colorAndRough.a);
        oColor = colorAndRough.rgb;
        oPosition = Position_worldspace;

#ifdef WIREFRAME
        float d = min(gs_edgeDist.x, gs_edgeDist.y);
        d = min(d, gs_edgeDist.z);
        float LineWidth = 1.5;
        float mixVal = smoothstep(LineWidth - 1.0, LineWidth + 1.0, d);
        oColor = mix(vec3(0.0, 0.0, 0.0), oColor, mixVal);

#endif
    }
_}
