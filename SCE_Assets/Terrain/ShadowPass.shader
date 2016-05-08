/***** PROJECT:Sand Castle Engine *****/
/**************************************/
/******** AUTHOR:Gwenn AUBERT *********/
/******* FILE:ShadowPass.shader *******/
/**************************************/
/**************** ????µs **************/

[VertexShader]
_{
#version 430 core

    uniform mat4 MVP;

    in vec3 vertexPosition_modelspace;

    void main()
    {        
        gl_Position = MVP*vec4(vertexPosition_modelspace, 1.0);
    }
_}


[FragmentShader]
_{
#version 430 core

    #define MAX_RAY_STEPS 86
    #define MAX_HEIGHT_INC 2.0

    uniform vec3        SunPosition_worldspace;
    uniform sampler2D   PositionTex;
    uniform sampler2D   FinalTex;

    uniform vec2        SCE_ScreenSize;

    uniform sampler2D   TerrainHeightMap;
    uniform mat4        WorldToTerrainSpace;
    uniform float       HeightScale;



    //out
    out vec3 color;

    float terrainHeightAt(vec3 pos_worldspace)
    {
        vec4 pos_terrainspace = WorldToTerrainSpace * vec4(pos_worldspace, 1.0);
        vec2 terrainUV = pos_terrainspace.zx * 0.5 + vec2(0.5);
        return texture(TerrainHeightMap, terrainUV).a;//only get height
    }

    float raymarchTerrainShadow(vec3 start_worldspace, vec3 dir_worldspace, float maxLen, float rayStep)
    {
        float k = 100.0;
        vec3 pos_worldspace = start_worldspace;
        float softshadow = 0.0;
        float fMaxIter = float(MAX_RAY_STEPS);
        float t = 0.0;
        for(int i = 0;
            i < MAX_RAY_STEPS && float(i)*rayStep < maxLen && pos_worldspace.y/HeightScale < 1.01;
            ++i)
        {
            float advanceRatio = float(i)/fMaxIter;
            t += rayStep + advanceRatio;
            pos_worldspace = start_worldspace + dir_worldspace * t;
            float height = terrainHeightAt(pos_worldspace);
            if(pos_worldspace.y <= height)
            {
                return 1.0f;
            }
            softshadow = max(1.0 - min((pos_worldspace.y - height)*k/t, 1.0), softshadow);
        }
        return softshadow;
    }

    float raymarchTerrainShadow2(vec3 start_worldspace, vec3 dir_worldspace, float minT, float maxLen,
                                 float maxDot, float sunDot)
    {
        float k = 30.0;
        vec3 pos_worldspace = start_worldspace;
        float softshadow = 0.0;
        float fMaxIter = float(MAX_RAY_STEPS);
        float t = 0.0;
        float d = minT;
        float hyp, step;
        float dotRatio = sunDot/(maxDot)*5.0;

        for(int i = 0;
            i < MAX_RAY_STEPS && t < maxLen
            //prevents going to high
            && pos_worldspace.y/HeightScale < 1.01;
            ++i)
        {
            hyp = d / maxDot;
            step = sqrt(hyp*hyp - d*d)*(1.0 + dotRatio);

            step *= 0.5 + float(i)/fMaxIter;

            t += step;
            pos_worldspace = start_worldspace + dir_worldspace * t;
            float height = terrainHeightAt(pos_worldspace);
            d = pos_worldspace.y - height;
            if(d < 0.1f || i == MAX_RAY_STEPS - 1)
            {
                return 1.0;
            }
            softshadow = max(1.0 - min(d*k/t, 1.0), softshadow);
        }
        return softshadow;
    }

    void main()
    {
        vec2 uv = gl_FragCoord.xy / SCE_ScreenSize;
        vec3 finalColor = texture(FinalTex, uv).rgb;
        vec3 pos_worldspace = texture(PositionTex, uv).xyz;

        float occludedByScene = step(0.0001, dot(pos_worldspace, pos_worldspace));

        vec3 dirToSun = SunPosition_worldspace - pos_worldspace;
        float maxLen = length(dirToSun);
        dirToSun = normalize(dirToSun);

        vec3 maxTerrainStep = normalize(vec3(1.0, MAX_HEIGHT_INC, 0.0));
        vec3 upVec = vec3(0.0, 1.0, 0.0);
        float maxDot = dot(maxTerrainStep, upVec);
        float dirDot = dot(dirToSun, upVec);

        if(occludedByScene > 0.0 && maxDot >= dirDot)
        {
            float currentHeight = terrainHeightAt(pos_worldspace);
            //correct worldspace position height (because low poly terrain might have a 'wrong' height)
            pos_worldspace.y = currentHeight;


//            float h = 5.0;//currentHeight - pos_worldspace.y;
//            float dy = dirToSun.y;
//            float dx = sqrt(dirToSun.x*dirToSun.x + dirToSun.z*dirToSun.z);
//            float M = acos(maxDot);

//            float t1 = h / ( M*dx - dy);
//            float t2 = h / (-M*dx - dy);
//            float dt = max(t1, t2);
//            if( t1<0.0 && t2<0.0 )
//            {
//                dt = 1.0;
//            }
//            float rayStep = dt;

////            float rayStep = dirDot/maxDot * 0.8;
////            rayStep += step(1.0, rayStep)*maxLen;

//            float light = 1.0 - raymarchTerrainShadow(pos_worldspace + upVec * 0.1,
//                                                      dirToSun, maxLen, rayStep);
            float startOffset = 0.5;


            float light = 1.0 - raymarchTerrainShadow2(
                        pos_worldspace + upVec*startOffset,
                        dirToSun,
                        startOffset,
                        maxLen,
                        maxDot,
                        dirDot);


            color = finalColor*clamp(light, 0.2, 1.0);
        }
        else
        {
            color = finalColor;
        }
    }
_}
