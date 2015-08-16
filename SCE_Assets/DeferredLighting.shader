VertexShader : 
_{
#version 430 core

    in vec3 vertexPosition_modelspace;
    in vec3 vertexNormal_modelspace;

    out vec3 LightDirection_cameraspace;
    out vec3 EyeDirection_cameraspace;
    out vec3 LightPosition_cameraspace;
    out float LightReach_cameraspace;

    uniform mat4 MVP;
    uniform mat4 M;
    uniform mat4 V;
    uniform mat4 P;

    uniform vec3    SCE_LightPosition_worldspace;
    uniform vec3    SCE_LightDirection_worldspace;
    uniform float   SCE_LightReach_worldspace;
    uniform vec4    SCE_LightColor;
    uniform float   SCE_LightMaxAngle;
    uniform float   SCE_LightCutoff;

    void main(){

        gl_Position                 = MVP * vec4(vertexPosition_modelspace, 1.0);

        vec3 Position_worldspace    = (M * vec4(vertexPosition_modelspace, 1.0)).xyz;

        vec3 VertexPosition_cameraspace = ( V * M * vec4(vertexPosition_modelspace, 1.0)).xyz;

        EyeDirection_cameraspace    = VertexPosition_cameraspace;

        LightPosition_cameraspace   = ( V * vec4(SCE_LightPosition_worldspace, 1.0)).xyz;

        LightDirection_cameraspace  = ( V * vec4(SCE_LightDirection_worldspace, 0.0)).xyz;

        LightReach_cameraspace = length( V * vec4(SCE_LightReach_worldspace, 0.0, 0.0, 0.0));
    }
_}

FragmentShader : 
_{
#version 430 core

    uniform vec2    SCE_ScreenSize;
    uniform vec3    SCE_LightPosition_worldspace;
    uniform vec3    SCE_LightDirection_worldspace;
    uniform float   SCE_LightReach_worldspace;
    uniform vec4    SCE_LightColor;
    uniform float   SCE_LightMaxAngle;
    uniform float   SCE_LightCutoff;

#define LIGHT_SUBROUTINE_PARAMS in vec3 in_LightDirection_cameraspace,\
    in vec3 in_Normal_cameraspace,\
    in vec3 in_EyeDirection_cameraspace,\
    in vec3 in_LightToFrag_cameraspace,\
    in float in_LightReach_cameraspace


    float mapToRange(float fromMin, float fromMax, float toMin, float toMax, float val)
    {
        val = max(fromMin, (min(fromMax, val)));//clamp in range if outside
        float fromSize = fromMax - fromMin;
        val = (val - fromMin) / fromSize;
        return mix(toMin, toMax, val);
    }

    //define a subroutine signature
    subroutine vec4 SCE_ComputeLightType(LIGHT_SUBROUTINE_PARAMS);

    //Directional light option
    subroutine (SCE_ComputeLightType) vec4 SCE_ComputeDirectionalLight(LIGHT_SUBROUTINE_PARAMS) {

        //Diffuse component
        vec3 dirToLight = normalize(-in_LightDirection_cameraspace);
        float NdotL     = dot(in_Normal_cameraspace, dirToLight);
        NdotL           = clamp(NdotL, 0, 1);

        vec3 eyeDir         = normalize(in_EyeDirection_cameraspace);
        vec3 reflectedLight = reflect(dirToLight, in_Normal_cameraspace);
        float EdotL         = clamp( dot( eyeDir, reflectedLight ), 0, 1 );

        float distCoef  = 1.0;

        vec4 lightCol   = vec4(1, 1, 1, 1); //SCE_LightColor;
        vec4 light      = lightCol * NdotL / (distCoef * distCoef)
                + lightCol * pow(EdotL, 5) / (distCoef * distCoef);

        //return vec4(1.0, 0.0, 0.0, 1.0);
        return light;
    }

    //Point light option
    subroutine (SCE_ComputeLightType) vec4 SCE_ComputePointLight(LIGHT_SUBROUTINE_PARAMS) {

        //Diffuse component
        vec3 dirToLight = normalize(-in_LightToFrag_cameraspace);
        float NdotL     = dot(in_Normal_cameraspace, dirToLight);
        NdotL           = clamp(NdotL, 0.0, 1.0);

        vec3 eyeDir         = normalize(in_EyeDirection_cameraspace);
        vec3 reflectedLight = reflect(dirToLight, in_Normal_cameraspace);
        float EdotL         = clamp( dot(eyeDir, reflectedLight), 0.0 ,1.0 );

        float lightReach    = in_LightReach_cameraspace;

        float dist              = length(in_LightToFrag_cameraspace);
        float d = max(dist - lightReach, 0);
        // calculate basic attenuation
        float denom = d/lightReach + 1;
        float attenuation = 1 / (denom*denom);

        // scale and bias attenuation such that:
        //   attenuation == 0 at extent of max influence
        //   attenuation == 1 when d == 0
        attenuation = (attenuation - SCE_LightCutoff) / (1 - SCE_LightCutoff);
        attenuation = max(attenuation, 0);

        vec3 light  = vec3(
                    NdotL, //diffuse lighting
                    pow(EdotL, 5.0), //specular component
                    1.0);

        light *= attenuation;
        return vec4(light, 1.0);
    }

    //Spot light option
    subroutine (SCE_ComputeLightType) vec4 SCE_ComputeSpotLight(LIGHT_SUBROUTINE_PARAMS) {

        return vec4(0.0, 1.0, 0.0, 1.0);
    }

    //uniform variable declaration for the light function subroutine
    subroutine uniform SCE_ComputeLightType SCE_ComputeLight;


    in vec3     LightDirection_cameraspace;
    in vec3     EyeDirection_cameraspace;
    in vec3     LightPosition_cameraspace;
    in float    LightReach_cameraspace;

    out vec4 color;

    uniform sampler2D PositionTex;
    uniform sampler2D DiffuseTex;
    uniform sampler2D NormalTex;

    void main(){

        vec2 uv = gl_FragCoord.xy / SCE_ScreenSize;
        vec3 MaterialDiffuseColor   = texture2D(DiffuseTex, uv).xyz;
        vec3 MaterialSpecularColor  = vec3(0.3,0.3,0.3);
        vec3 Normal_cameraspace     = texture2D(NormalTex, uv).xyz;
        vec3 Position_cameraspace   = texture2D(PositionTex, uv).xyz;

        vec3 LightToFrag_cameraspace = Position_cameraspace - LightPosition_cameraspace;

        vec4 lightCol = SCE_ComputeLight(
                    LightDirection_cameraspace,
                    Normal_cameraspace,
                    EyeDirection_cameraspace,
                    LightToFrag_cameraspace,
                    LightReach_cameraspace
                    );

        color = vec4( //Diffuse
                      MaterialDiffuseColor * lightCol.x * SCE_LightColor.rgb * SCE_LightColor.a +
                      //Specular
                      + lightCol.y, 1.0);

        //gamma correction
        color = pow(color, vec4(1.0/2.2));
    }
_}
