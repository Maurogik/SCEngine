VertexShader : 
_{
#version 430 core

    in vec3 vertexPosition_modelspace;
    in vec3 vertexNormal_modelspace;

    uniform mat4 MVP;
    uniform mat4 M;
    uniform mat4 V;
    uniform mat4 P;

    uniform vec3    SCE_EyePosition_worldspace;
    uniform vec3    SCE_LightPosition_worldspace;
    uniform vec3    SCE_LightDirection_worldspace;
    uniform float   SCE_LightReach_worldspace;
    uniform vec4    SCE_LightColor;
    uniform float   SCE_SpotAttenuation;
    uniform float   SCE_LightCutoff;

    void main()
    {
        gl_Position                 = MVP * vec4(vertexPosition_modelspace, 1.0);
    }
_}

FragmentShader : 
_{
#version 430 core

    uniform vec2    SCE_ScreenSize;
    uniform vec3    SCE_EyePosition_worldspace;
    uniform vec3    SCE_LightPosition_worldspace;
    uniform vec3    SCE_LightDirection_worldspace;
    uniform float   SCE_LightReach_worldspace;
    uniform vec4    SCE_LightColor;
    uniform float   SCE_SpotAttenuation;
    uniform float   SCE_LightCutoff;
    uniform float   SCE_ShadowStrength;

#define LIGHT_SUBROUTINE_PARAMS \
    in vec3 in_LightDirection_worldspace,\
    in vec3 in_Normal_worldspace,\
    in vec3 in_EyeToFrag_worldspace,\
    in vec3 in_LightToFrag_worldspace,\
    in float in_LightReach_worldspace


    float mapToRange(float fromMin, float fromMax, float toMin, float toMax, float val)
    {
        val = max(fromMin, (min(fromMax, val)));//clamp in range if outside
        float fromSize = fromMax - fromMin;
        val = (val - fromMin) / fromSize;
        return mix(toMin, toMax, val);
    }

    //define a subroutine signature
    subroutine vec2 SCE_ComputeLightType(LIGHT_SUBROUTINE_PARAMS);

    //Directional light option
    subroutine (SCE_ComputeLightType) vec2 SCE_ComputeDirectionalLight(LIGHT_SUBROUTINE_PARAMS)
    {
        //Diffuse component
        vec3 dirToLight = normalize(-in_LightDirection_worldspace);
        float NdotL     = dot(in_Normal_worldspace, dirToLight);
        NdotL           = clamp(NdotL, 0, 1);

        vec3 dirToEye       = normalize(-in_EyeToFrag_worldspace);
        vec3 halway         = normalize(dirToEye + dirToLight);
        float EdotL         = clamp( dot(in_Normal_worldspace, halway), 0.0 ,1.0 );

        vec2 light  = vec2(
                    NdotL, //diffuse lighting
                    pow(EdotL, 16.0)); //specular component

        return light;
    }

    //Point light option
    subroutine (SCE_ComputeLightType) vec2 SCE_ComputePointLight(LIGHT_SUBROUTINE_PARAMS)
    {
        //Diffuse component
        vec3 dirToLight = normalize(-in_LightToFrag_worldspace);
        float NdotL     = dot(in_Normal_worldspace, dirToLight);
        NdotL           = clamp(NdotL, 0.0, 1.0);

        vec3 dirToEye       = normalize(-in_EyeToFrag_worldspace);
        vec3 halway         = normalize(dirToEye + dirToLight);
        float EdotL         = clamp( dot(in_Normal_worldspace, halway), 0.0 ,1.0 );

        float lightReach    = in_LightReach_worldspace;

        float dist          = length(in_LightToFrag_worldspace);
        float d = max(dist - lightReach, 0);
        // calculate basic attenuation
        float denom = d/lightReach + 1;
        float attenuation = 1 / (denom*denom);
        attenuation = (attenuation - SCE_LightCutoff) / (1 - SCE_LightCutoff);
        attenuation = max(attenuation, 0);

        vec2 light  = vec2(
                    NdotL, //diffuse lighting
                    pow(EdotL, 16.0)); //specular component

        light *= attenuation;
        return light;
    }

    //Spot light option
    subroutine (SCE_ComputeLightType) vec2 SCE_ComputeSpotLight(LIGHT_SUBROUTINE_PARAMS)
    {
        //Diffuse component
        vec3 dirToLight     = normalize(-in_LightToFrag_worldspace);
        vec3 invLightDir    = normalize(-in_LightDirection_worldspace);
        float NdotL         = dot(in_Normal_worldspace, dirToLight);
        NdotL               = clamp(NdotL, 0.0, 1.0);

        vec3 dirToEye       = normalize(-in_EyeToFrag_worldspace);
        vec3 halway         = normalize(dirToEye + dirToLight);
        float EdotL         = clamp( dot(in_Normal_worldspace, halway), 0.0 ,1.0 );

        float lightReach    = in_LightReach_worldspace;

        //use very simple fallof approximation to fade spot light with distance
        float dist          = length(in_LightToFrag_worldspace);
        float d             = mapToRange(0.0, lightReach, 1.0, 0.0, dist);
        float attenuation   = d*d;

        float spotAttenuation = max(dot(dirToLight, invLightDir), 0.0);
        spotAttenuation = pow(spotAttenuation, SCE_SpotAttenuation);

        vec2 light  = vec2(
                    NdotL, //diffuse lighting
                    pow(EdotL, 16.0)); //specular component

        light *= attenuation * spotAttenuation;
        return light;
    }

    //uniform variable declaration for the light function subroutine
    subroutine uniform SCE_ComputeLightType SCE_ComputeLight;


    #define CASCADE_COUNT 4
    #define SHADOW_BIAS 0.00005

    out vec4 color;

    uniform mat4        DepthConvertMat[CASCADE_COUNT];
    uniform mat4        V;
    uniform float       FarSplits_cameraspace[CASCADE_COUNT];

    uniform sampler2D   PositionTex;
    uniform sampler2D   DiffuseTex;
    uniform sampler2D   NormalTex;
//    uniform sampler2D   ShadowTex;
    uniform sampler2DArray ShadowTex;


    vec2 poissonDisk[16] = vec2[](
            vec2( -0.94201624, -0.39906216 ),
            vec2( 0.94558609, -0.76890725 ),
            vec2( -0.094184101, -0.92938870 ),
            vec2( 0.34495938, 0.29387760 ),
            vec2( -0.91588581, 0.45771432 ),
            vec2( -0.81544232, -0.87912464 ),
            vec2( -0.38277543, 0.27676845 ),
            vec2( 0.97484398, 0.75648379 ),
            vec2( 0.44323325, -0.97511554 ),
            vec2( 0.53742981, -0.47373420 ),
            vec2( -0.26496911, -0.41893023 ),
            vec2( 0.79197514, 0.19090188 ),
            vec2( -0.24188840, 0.99706507 ),
            vec2( -0.81409955, 0.91437590 ),
            vec2( 0.19984126, 0.78641367 ),
            vec2( 0.14383161, -0.14100790 )
         );

//    vec2 poissonDisk[4] = vec2[](
//      vec2( -0.94201624, -0.39906216 ),
//      vec2( 0.94558609, -0.76890725 ),
//      vec2( -0.094184101, -0.92938870 ),
//      vec2( 0.34495938, 0.29387760 )
//    );

    //rand [0, 1[
    float rand(vec4 seed4)
    {
        float dot_product = dot(seed4, vec4(12.9898,78.233,45.164,94.673));
        return fract(sin(dot_product) * 43758.5453);
    }

    float sampleMyTex(vec2 uv, int i, float z)
    {
        return textureGather(ShadowTex, vec3(uv, float(i)));
    }

    float getShadowDepth(vec3 pos_worldspace, vec3 normal_worldspace, vec3 lightDir_worldspace)
    {
        int sampleCount = 4;
        int poissonSize = 16;
        float poissonSpread = 1.0 / 900.0;
        float eyeDist = distance(pos_worldspace, SCE_EyePosition_worldspace);

        vec3 cameraPos = (V * vec4(pos_worldspace, 1.0)).xyz;

        //float bias = SHADOW_BIAS * (1.0 - dot(lightDir_worldspace, - normal_worldspace));
        float cosTheta = clamp(dot(-lightDir_worldspace, normal_worldspace), 0.0, 1.0);
        float bias = SHADOW_BIAS * tan(acos(cosTheta));

        for(int i = 0; i < CASCADE_COUNT; ++i)
        {
            if(cameraPos.z <= FarSplits_cameraspace[i])
            {                
                vec4 position_lightspace = DepthConvertMat[i] * vec4(pos_worldspace, 1.0);

                float shadow = 0.0;
                for (int off = 0; off < sampleCount; off++)
                {
                    float fi = float(i + 1);
                    vec4 seed4 = vec4(floor(pos_worldspace * 500.0 / fi), off);
                    int index = int((float(poissonSize)*rand(seed4))) % poissonSize;
                    vec2 uv = position_lightspace.xy + poissonDisk[index] * poissonSpread / fi;
                    float depth_lightspace = sampleMyTex(uv, i, position_lightspace.z);

                    shadow += step(depth_lightspace + bias, position_lightspace.z) * 1.0/float(sampleCount);
                }

                return shadow;
//                float depthCenter = sampleMyTex(position_lightspace.xy, i, position_lightspace.z);
//                return step(depthCenter + bias, position_lightspace.z);
            }
        }
        return 0.0f;
    }

    void main()
    {
        vec2 uv = gl_FragCoord.xy / SCE_ScreenSize;
        vec3 MaterialDiffuseColor   = texture2D(DiffuseTex, uv).xyz;
        vec3 Normal_worldspace      = normalize(texture2D(NormalTex, uv).xyz);
        vec3 Position_worldspace    = texture2D(PositionTex, uv).xyz;

        vec3 LightToFrag_cameraspace = Position_worldspace - SCE_LightPosition_worldspace;
        vec3 EyeToFrag_cameraspace = Position_worldspace - SCE_EyePosition_worldspace;

        vec2 lightCol = SCE_ComputeLight(
                    SCE_LightDirection_worldspace,
                    Normal_worldspace,
                    EyeToFrag_cameraspace,
                    LightToFrag_cameraspace,
                    SCE_LightReach_worldspace
                    );

        color = vec4( //Diffuse
                      (MaterialDiffuseColor * lightCol.x * SCE_LightColor.rgb * SCE_LightColor.a)
                      //Specular
                      + (SCE_LightColor.rgb * lightCol.y * SCE_LightColor.a), 1.0);

        float shadow = getShadowDepth(Position_worldspace, Normal_worldspace, SCE_LightDirection_worldspace);

        color.rgb *= 1.0 - shadow * SCE_ShadowStrength;
//        color.rgb = texture2D(ShadowTex, uv).rgb * SCE_ShadowStrength;

//        color.rgb *= getShadowDepth(Position_worldspace);

        /*vec2 uv2 = uv * 0.5;

        if(uv.x < 0.5 && uv.y < 0.5)
        {
            color.rgb *=  0.5 + sampleMyTex(uv2, 0).r;
        }

        if(uv.x > 0.5 && uv.y < 0.5)
        {
            color.rgb *=  0.5 + sampleMyTex(uv2, 1).r;
        }

        if(uv.x < 0.5 && uv.y > 0.5)
        {
            color.rgb *=  0.5 + sampleMyTex(uv2, 2).r;
        }

        if(uv.x > 0.5 && uv.y > 0.5)
        {
            color.rgb *=  0.5 + sampleMyTex(uv2, 3).r;
        }*/

        //gamma correction
        color = pow(color, vec4(1.0/2.2));
    }
_}
