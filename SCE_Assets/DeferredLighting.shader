[VertexShader]
_{
#version 430 core

    in vec3 vertexPosition_modelspace;
    in vec3 vertexNormal_modelspace;

    uniform mat4 MVP;

    void main()
    {
        gl_Position                 = MVP * vec4(vertexPosition_modelspace, 1.0);
    }
_}

[FragmentShader]
_{
#version 430 core

    #define CASCADE_COUNT 2
    #define SHADOW_MAP_SIZE 4096.0
    #define M_PI 3.14159265359

    uniform vec2    SCE_ScreenSize;
    uniform vec3    SCE_EyePosition_worldspace;
    uniform vec3    SCE_LightPosition_worldspace;
    uniform vec3    SCE_LightDirection_worldspace;
    uniform float   SCE_LightReach_worldspace;
    uniform vec4    SCE_LightColor;
    uniform float   SCE_LightMaxDotAngle;
    uniform float   SCE_ShadowStrength;
    uniform float   ShadowCrossFade;

    uniform mat4    DepthConvertMat[CASCADE_COUNT];
    uniform mat4    V;
    uniform mat4    P;

    uniform float   FarSplits_cameraspace[CASCADE_COUNT];

    uniform sampler2D   PositionTex;
    uniform sampler2D   DiffuseTex;
    uniform sampler2D   NormalTex;
    uniform sampler2DArrayShadow ShadowTex;

    out vec4 color;


#define LIGHT_SUBROUTINE_PARAMS \
    in vec3 in_Position_worldspace,\
    in vec3 in_LightDirection_worldspace,\
    in vec3 in_Normal_worldspace,\
    in vec3 in_EyeToFrag_worldspace,\
    in vec3 in_LightToFrag_worldspace,\
    in float in_LightReach_worldspace,\
    in float in_Surface_Roughness,\
    in float in_Light_Intensity,\
    in float in_Translucency\


    float mapToRange(float fromMin, float fromMax, float toMin, float toMax, float val)
    {
        val = max(fromMin, (min(fromMax, val)));//clamp in range if outside
        float fromSize = fromMax - fromMin;
        val = (val - fromMin) / fromSize;
        return mix(toMin, toMax, val);
    }

    float getShadowDepth(vec3 pos_worldspace, vec3 normal_worldspace, vec3 lightDir_worldspace);      


    ///// PBR ///////

    //Fresnel effect using Schlick's approximation (assuming transport elem is air)
    //f0 is the reflectance coef for incoming light paralel to normal
    //f90 is the reflectance coef for incoming light at right angle (usually 1.0 or close)
    float F_Schlick(float f0, float f90, float dot )
    {
        return f0 + ( f90 - f0 ) * pow (1.0 - dot , 5.0 ) ;
    }

    float Fr_DisneyDiffuse(float NdotV, float NdotL, float LdotH, float linearRoughness)
    {
        float energyBias = mix (0.0 , 0.5 , linearRoughness ) ;

        float energyFactor = mix (1.0 , 1.0 / 1.51, linearRoughness);
        float fd90 = energyBias + 2.0 * LdotH * LdotH * linearRoughness;
        float f0 = 1.0;
        float lightScatter = F_Schlick( f0 , fd90 , NdotL );
        float viewScatter = F_Schlick( f0 , fd90 , NdotV );
        return lightScatter * viewScatter * energyFactor ;
    }

    //Normal Distribution Function to approximate how many micro-facets should reflect
    //Uses Trowbridge-Reitz (GGX)
    float D_GGX( float NdotH , float a2 )
    {
        float f = ( NdotH * a2 - NdotH ) * NdotH + 1;
        return a2 / ( M_PI * f * f ) ;
    }

    float G_1_Schlick(float dot, float k)
    {
        return dot/(dot*(1 - k) + k);
    }

    float G_Schlick_GGX(float NdotV, float NdotL, float a2)
    {
        float k = a2*0.5;//Follow UE4 usage of k=a/2 to match Schlick approx to the Smith geom func
        return G_1_Schlick(NdotV, k)*G_1_Schlick(NdotL, k);
    }

    vec3 PBR_Lighting(vec3 dirToLight, vec3 dirToEye, vec3 normal, float roughness)
    {
        float NdotL         = max(dot(normal, dirToLight), 0.0);
        vec3 halfway        = normalize(dirToEye + dirToLight);
        float HdotN         = max(dot(normal, halfway), 0.0);
        float NdotV         = max(dot(normal, dirToEye), 0.1); //strongly enforce the dot is not
        //too low to avoid artifacts on low poly meshes
        float VdotH         = dot(dirToEye, halfway);
        float LdotH         = max(dot(dirToLight, halfway), 0.0);

        // Calculate color at normal incidence
//        float ior = 1.5;//1.0 + roughness;
//        float f0 = abs ((1.0 - ior) / (1.0 + ior));
//        f0 = f0 * f0;

        float f0    = 0.028;
        float f90   = 1.0;//0.85;
        float F     = F_Schlick(f0, f90, VdotH);

        float a2    = roughness*roughness;

        float refl = 0.0;

        //avoid dividing by zero
        if(NdotL > 0.0)
        {
            float Vis   = G_Schlick_GGX(NdotV, NdotL, a2);
            float D     = D_GGX(HdotN, a2);
            refl        = Vis * D / (4.0 * NdotL * NdotV);
        }

        //Diffuse BRDF
        float diff = 1.0 / M_PI;

        //use fresnel as weights for diff/spec balance
        vec2 light = vec2(diff * (1.0 - F), refl * F);
        return vec3(light * NdotL, 1.0 /*full shadow impact*/);
    }

    ///// Vegetation Lighting //////

    float PowWrappedDiffuse(vec3 normal, vec3 dirToLight, float w, float n)
    {
        // w is between 0 and 1
        // n is not -1
        float wrappedDiffuse = pow(clamp((dot(normal, dirToLight) + w)/(1.0f + w), 0.0, 1.0), n) * (n + 1) / (2 * (1 + w));
        return wrappedDiffuse;
    }

    vec3 VegetationBackLighting(vec3 dirToLight, vec3 dirToEye, vec3 normal,
                                float roughness, float translucency)
    {
        float shadowImpact = 1.0 - pow(translucency, 5.0);
        float EdotL = clamp(dot(dirToEye, dirToLight), 0.0, 1.0);
        float PowEdotL = EdotL * EdotL;
        PowEdotL *= PowEdotL;
        // Back diffuse shading, wrapped slightly
//        float LdotNBack = clamp(dot(normal, dirToLight)*0.6+0.4, 0.0, 1.0);
        float LdotNBack = PowWrappedDiffuse(normal, dirToLight, 0.5, 2.0);
        // Allow artists to tweak view dependency.
        float diff = mix(LdotNBack, PowEdotL, translucency);
        // Apply material back diffuse color.
        return vec3(diff, 0.0, shadowImpact);
    }

    vec3 VegetationFrontLighting(vec3 dirToLight, vec3 dirToEye, vec3 normal,
                                 float roughness, float translucency)
    {
        float shadowImpact = 1.0 - pow(translucency, 5.0);

        float NdotL = clamp(dot(normal, dirToLight), 0.0, 1.0);
        vec3 H = normalize(dirToLight + dirToEye);
        float HdotN = clamp(dot(H, normal), 0.0, 1.0);
        float spec = pow(HdotN, 1.0);
//        float diff = PowWrappedDiffuse(normal, dirToLight, 0.1, 1.0);
        float diff = NdotL;
        return vec3(diff, 0.0, shadowImpact);
    }

    vec3 PerformLighting(vec3 dirToLight, vec3 dirToEye, vec3 normal,
                         float roughness, float translucency)
    {
        if(translucency > 0.01)
        {
            float NdotL = dot(normal, dirToLight);
            float EdotL = dot(dirToEye, dirToLight);

            //return vec3(NdotL*0.5+0.5, EdotL*0.5+0.5, 0.0);



//            if(NdotL > 0.0)
//            {
//                backNormal = -normal;
//            }

            if(NdotL < 0.0)
            {
                normal = -normal;
            }

            vec3 backNormal = normal;
            vec3 frontNormal = normal;

            vec3 backLit = VegetationBackLighting(-dirToLight, dirToEye, backNormal,
                                              roughness, translucency);

            vec3 frontLit = VegetationFrontLighting(dirToLight, dirToEye, frontNormal,
                                               roughness, translucency);

//            frontLit = vec3(1.0, 0.0, 0.0);
//            backLit = vec3(0.0, 1.0, 0.0);
            EdotL = 1.0/(1.0 + exp(-EdotL*20));
//            EdotL = clamp(EdotL, -1.0, 1.0);
//            EdotL = EdotL*0.5 + 0.5;
            return mix(backLit, frontLit, EdotL);
        }
        else
        {
            return PBR_Lighting(dirToLight, dirToEye, normal, roughness);
        }
    }

//    #define DEBUG

    ///////////////////

    //define a subroutine signature
    subroutine vec3 SCE_ComputeLightType(LIGHT_SUBROUTINE_PARAMS);

    //Directional light option
    subroutine (SCE_ComputeLightType) vec3 SCE_ComputeDirectionalLight(LIGHT_SUBROUTINE_PARAMS)
    {

        float shadow = 0.0f;
        if(SCE_ShadowStrength > 0.0f)
        {
            shadow = getShadowDepth(in_Position_worldspace,
                                    in_Normal_worldspace,
                                    SCE_LightDirection_worldspace)*SCE_ShadowStrength;
        }

        vec3 diffSpecShadow = PerformLighting(normalize(-in_LightDirection_worldspace),
                                        normalize(-in_EyeToFrag_worldspace),
                                        in_Normal_worldspace,
                                        in_Surface_Roughness, in_Translucency);

        vec3 light = vec3(
                    in_Light_Intensity*diffSpecShadow.x,
                    in_Light_Intensity*diffSpecShadow.y,
                    shadow*diffSpecShadow.z);

        return light;
    }

    //Point light option
    subroutine (SCE_ComputeLightType) vec3 SCE_ComputePointLight(LIGHT_SUBROUTINE_PARAMS)
    {
        vec3 diffSpecShadow = PerformLighting(normalize(-in_LightToFrag_worldspace),
                                        normalize(-in_EyeToFrag_worldspace),
                                        in_Normal_worldspace,
                                        in_Surface_Roughness, in_Translucency);

        vec3 light = vec3(
                    in_Light_Intensity*diffSpecShadow.x,
                    in_Light_Intensity*diffSpecShadow.y,
                    0.0);

        float lightReach    = in_LightReach_worldspace;
        //hackish attenuation, but works well
        float squaredDist   = dot(in_LightToFrag_worldspace, in_LightToFrag_worldspace);
        float attenuation   = mix(lightReach*in_Light_Intensity/squaredDist,
                                  0,
                                  squaredDist/(lightReach*lightReach));

        light *= attenuation;
        return light;
    }

    //Spot light option
    subroutine (SCE_ComputeLightType) vec3 SCE_ComputeSpotLight(LIGHT_SUBROUTINE_PARAMS)
    {
        vec3 dirToLight = normalize(-in_LightToFrag_worldspace);
        vec3 invLightDir = normalize(-in_LightDirection_worldspace);
        vec3 diffSpecShadow = PerformLighting(dirToLight,
                                        normalize(-in_EyeToFrag_worldspace),
                                        in_Normal_worldspace,
                                        in_Surface_Roughness, in_Translucency);

        vec3 light = vec3(
                    in_Light_Intensity*diffSpecShadow.x,
                    in_Light_Intensity*diffSpecShadow.y,
                    0.0);

        float lightReach    = in_LightReach_worldspace;
        //use very simple fallof approximation to fade spot light with distance
        float squaredDist   = dot(in_LightToFrag_worldspace, in_LightToFrag_worldspace);
        float attenuation   = mix(lightReach*in_Light_Intensity/squaredDist,
                                  0,
                                  squaredDist/(lightReach*lightReach));

        float fragDotL = dot(dirToLight, invLightDir);
        float surfDotL = SCE_LightMaxDotAngle;
        float spotAttenuation = mapToRange(surfDotL, 1.0, 0.0, 1.0, fragDotL);
        spotAttenuation = pow(spotAttenuation, 4.0);

        light *= attenuation * spotAttenuation;
        return light;
    }

    //uniform variable declaration for the light function subroutine
    subroutine uniform SCE_ComputeLightType SCE_ComputeLight;


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

    //rand [0, 1[
    float rand(vec4 seed4)
    {
        float dot_product = dot(seed4, vec4(12.9898,78.233,45.164,94.673));
        return fract(sin(dot_product) * 43758.5453);
    }

    float sampleMyTex(vec2 uv, float cascade, float z)
    {
        return texture(ShadowTex, vec4(uv, cascade, z));
    }

    float samplePoisson(int sampleCount, float z_depthspace, vec2 uv,
                        vec3 pos_worldspace, float cascade)
    {
        int poissonSize = 16;
        float poissonSpread = 1.0 / 1200.0;
        float shadow = 0.0;
        vec4 seed4 = vec4(0.0);
        float fSamples = float(sampleCount);
        int index = 0;
        float cascadeNum = cascade + 1.0;
        for (int off = 0; off < sampleCount; off++)
        {
            seed4 = vec4(floor(pos_worldspace * 500.0 / cascadeNum), off);
            index = int((float(poissonSize)*rand(seed4))) % poissonSize;
            uv += poissonDisk[index] * poissonSpread / cascadeNum;
            shadow += (1.0 - sampleMyTex(uv, cascade, z_depthspace)) / fSamples;
        }

        return shadow;
    }

    float samplePCF(int sampleCount, float z_depthspace, vec2 uv, float cascade)
    {
        float fsamples = float(sampleCount);
        float kernelSize = sqrt(fsamples);
        float shadow = 0.0;

        float baseValue = (1.0 - sampleMyTex(uv, cascade, z_depthspace));

        for(float off = 0.0; off < fsamples; ++off)
        {
            float u = floor(off / kernelSize);
            float v = mod(off, kernelSize);
            vec2 uvOffset = vec2(u, v) / SHADOW_MAP_SIZE;
            shadow += (1.0 - sampleMyTex(uv + uvOffset, cascade, z_depthspace)) / fsamples;
        }

        return shadow;
    }

    #define SHADOW_BIAS 0.00001

    float getShadowDepth(vec3 pos_worldspace, vec3 normal_worldspace, vec3 lightDir_worldspace)
    {
        int sampleCount = 8;
        float eyeDist = distance(pos_worldspace, SCE_EyePosition_worldspace);
        vec3 pos_cameraspace = (V * vec4(pos_worldspace, 1.0)).xyz;

//        float biasOffset = min(1.0 - dot(-lightDir_worldspace, normal_worldspace), 0.5);
        float biasOffset = 1.0 - abs(dot(-lightDir_worldspace, normal_worldspace));
        float bias = SHADOW_BIAS * biasOffset;

        float shadow = 0.0;
        vec4 position_depthspace = vec4(0.0);
        float shadowStrength = 1.0;
        float fadeDist = ShadowCrossFade;//cross fabe distance

        for(int i = 0; i < CASCADE_COUNT; ++i)
        {
            position_depthspace = DepthConvertMat[i] * vec4(pos_worldspace, 1.0);
            //do a cross fade between shadow cascade
            float splitStrength = step(pos_cameraspace.z, FarSplits_cameraspace[i]);
            splitStrength *= clamp((FarSplits_cameraspace[i] - pos_cameraspace.z)/fadeDist,
                                         0.0, 1.0);
            splitStrength *= shadowStrength;
            shadowStrength = clamp(shadowStrength - splitStrength, 0.0, 1.0);

//            if(pos_cameraspace.z <= FarSplits_cameraspace[i])
            {
//              shadow = samplePoisson(sampleCount, position_depthspace.z, position_depthspace.xy,
//                                      pos_worldspace, float(i));
                shadow += splitStrength*samplePCF(sampleCount, position_depthspace.z - bias,
                                   position_depthspace.xy, float(i));
//                shadow = (1.0 - sampleMyTex(position_depthspace.xy, float(i),
//                                            position_depthspace.z + bias));
//                return shadow;
            }            
        }
        return shadow;
    }

//#define DEBUG

    void main()
    {
        vec3 ambiantColor = vec3(1.0, 1.0, 1.0) * 0.001;

        vec2 uv = gl_FragCoord.xy / SCE_ScreenSize;
        vec3 Position_worldspace    = texture2D(PositionTex, uv).xyz;

        //bail early if texture wasn't filled at this position
        if(dot(Position_worldspace, Position_worldspace) < 0.0001)
        {
            color = vec4(0.0, 0.0, 0.0, 1.0);
            return;
        }

        vec4 ColorAndTrans          = texture2D(DiffuseTex, uv);
        vec3 MaterialDiffuseColor   = ColorAndTrans.rgb;
        vec4 normSpec               = texture2D(NormalTex, uv);
        vec3 Normal_worldspace      = normSpec.rgb;

        float Roughness             = clamp(normSpec.a, 0.0, 1.0);

        color = vec4(MaterialDiffuseColor, 1.0);

        vec3 LightToFrag_cameraspace = Position_worldspace - SCE_LightPosition_worldspace;
        vec3 EyeToFrag_cameraspace = Position_worldspace - SCE_EyePosition_worldspace;

        vec3 lightCol = SCE_ComputeLight(
                    Position_worldspace,
                    SCE_LightDirection_worldspace,
                    Normal_worldspace,
                    EyeToFrag_cameraspace,
                    LightToFrag_cameraspace,
                    SCE_LightReach_worldspace,
                    Roughness,
                    SCE_LightColor.a,
                    ColorAndTrans.a);


        color.rgb =
                //Diffuse
                (MaterialDiffuseColor * lightCol.x * SCE_LightColor.rgb)
                //Specular
                + (SCE_LightColor.rgb * lightCol.y);
        //shadow and ambiant
        color.rgb = color.rgb * (1.0 - lightCol.z)
                     + SCE_ShadowStrength * ambiantColor * MaterialDiffuseColor;



#ifdef DEBUG
//        color = vec4(MaterialDiffuseColor, 1.0);
//        color = vec4((Normal_worldspace * 0.5 + vec3(0.5)), 1.0);
        color = vec4(lightCol, 1.0);
//        color = vec4(Specularity, Specularity, Specularity, 1.0);
#endif
    }
_}
