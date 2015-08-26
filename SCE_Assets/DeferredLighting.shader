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

    out vec4 color;

    uniform mat4        DepthConvertMat;

    uniform sampler2D   PositionTex;
    uniform sampler2D   DiffuseTex;
    uniform sampler2D   NormalTex;
    uniform sampler2D   ShadowTex;


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

        //convert frag pos to lightspace homogeneous coordinates (but in range [0.0, 1.0])
        vec4 position_lightspace = DepthConvertMat * vec4(Position_worldspace, 1.0);
        float depth_lightspace = texture2D(ShadowTex, position_lightspace.xy).r;

        float shadow = step(depth_lightspace + 0.001, position_lightspace.z);

        color.rgb *= 1.0 - shadow * SCE_ShadowStrength;
//        color.rgb = texture2D(ShadowTex, uv).rgb * SCE_ShadowStrength;
        //gamma correction
        color = pow(color, vec4(1.0/2.2));
    }
_}
