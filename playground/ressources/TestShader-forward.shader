VertexShader : 
_{
    #version 400 core

	in vec3 vertexPosition_modelspace;
	in vec2 vertexUV;
	in vec3 vertexNormal_modelspace;

	out vec2 fragUV;
        out vec3 Normal_cameraspace;
        out vec3 LightDirection_cameraspace;
        out vec3 EyeDirection_cameraspace;
        out vec3 LightToFrag_cameraspace;

	uniform mat4 MVP;
	uniform mat4 M;
	uniform mat4 V;
	uniform mat4 P;

        uniform vec3    SCE_LightPosition_worldspace;
        uniform vec3    SCE_LightDirection_worldspace;
        uniform float   SCE_LightReach_worldspace;
        uniform vec4    SCE_LightColor;
        uniform float   SCE_LightMaxAngle;

	void main(){
 
            gl_Position = MVP * vec4(vertexPosition_modelspace,1);
	    fragUV = vertexUV;

            vec3 Position_worldspace = (M * vec4(vertexPosition_modelspace,1)).xyz;

            vec3 VertexPosition_cameraspace = ( V * M * vec4(vertexPosition_modelspace,1)).xyz;

            EyeDirection_cameraspace = VertexPosition_cameraspace;

            vec3 LightPosition_cameraspace = ( V * vec4(SCE_LightPosition_worldspace,1)).xyz;

            LightDirection_cameraspace = ( V * vec4(SCE_LightDirection_worldspace,1)).xyz;
            //LightDirection_cameraspace = LightPosition_cameraspace - VertexPosition_cameraspace;
            //LightDirection_cameraspace = Position_worldspace - SCE_LightPosition_worldspace;
            //LightDirection_cameraspace = LightDirection_cameraspace * 0.5 + vec3(1, 1, 1);

            LightToFrag_cameraspace = VertexPosition_cameraspace - LightPosition_cameraspace;

            // Normal of the the vertex, in camera space
            Normal_cameraspace = ( V * M * vec4(vertexNormal_modelspace,0)).xyz; // Only correct if ModelMatrix does not scale the model ! Use its inverse transpose if not.
	}
_}

FragmentShader : 
_{
    #version 400 core

    in vec2 fragUV;
    in vec3 Normal_cameraspace;
    in vec3 LightDirection_cameraspace;
    in vec3 EyeDirection_cameraspace;
    in vec3 LightToFrag_cameraspace;

    out vec4 color;

    uniform vec3 AmbientColor;
    uniform sampler2D MainTex;

    uniform vec3    SCE_LightPosition_worldspace;
    uniform vec3    SCE_LightDirection_worldspace;
    uniform float   SCE_LightReach_worldspace;
    uniform vec4    SCE_LightColor;
    uniform float   SCE_LightMaxAngle;


    //define a subroutine signature
    subroutine vec4 SCE_ComputeLightType(
            in vec3 in_LightDirection_cameraspace,
            in vec3 in_Normal_cameraspace,
            in vec3 in_EyeDirection_cameraspace,
            in vec3 in_LightToFrag_cameraspace);

    //Directional light option
    subroutine (SCE_ComputeLightType) vec4 SCE_ComputeDirectionalLight(
                in vec3 in_LightDirection_cameraspace,
                in vec3 in_Normal_cameraspace,
                in vec3 in_EyeDirection_cameraspace,
                in vec3 in_LightToFrag_cameraspace) {

        //Diffuse component
        vec3 dirToLight = normalize(-in_LightDirection_cameraspace);
        float NdotL = dot(in_Normal_cameraspace, dirToLight);
        NdotL = clamp(NdotL, 0, 1);

        vec3 eyeDir = normalize(in_EyeDirection_cameraspace);
        vec3 reflectedLight = reflect(dirToLight, in_Normal_cameraspace);
        float EdotL = clamp( dot( eyeDir, reflectedLight ), 0,1 );

        float distCoef = 10;

        vec4 lightCol = vec4(1, 1, 1, 1); //SCE_LightColor;
        vec4 light = lightCol * NdotL / (distCoef * distCoef)
        + lightCol * pow(EdotL, 5) / (distCoef * distCoef);

        return light;
    }

    //Point light option
    subroutine (SCE_ComputeLightType) vec4 SCE_ComputePointLight(
                in vec3 in_LightDirection_cameraspace,
                in vec3 in_Normal_cameraspace,
                in vec3 in_EyeDirection_cameraspace,
                in vec3 in_LightToFrag_cameraspace) {

        //Diffuse component
        vec3 dirToLight = normalize(-in_LightToFrag_cameraspace);
        float NdotL = dot(in_Normal_cameraspace, dirToLight);
        NdotL = clamp(NdotL, 0, 1);

        vec3 eyeDir = normalize(in_EyeDirection_cameraspace);
        vec3 reflectedLight = reflect(dirToLight, in_Normal_cameraspace);
        float EdotL = clamp( dot( eyeDir, reflectedLight ), 0,1 );

        float distance = length(in_LightToFrag_cameraspace);
        float falloffDist = 5.0f;//inverse square law falls to zero at approximatly 5
        float lightReach = SCE_LightReach_worldspace;
        float distCoef = (distance / lightReach) * falloffDist;

        vec4 lightCol = SCE_LightColor;
        vec4 light = lightCol * NdotL / (distCoef * distCoef)
        + lightCol * pow(EdotL, 5) / (distCoef * distCoef);

        return light;
    }

    //Spot light option
    subroutine (SCE_ComputeLightType) vec4 SCE_ComputeSpotLight(
                in vec3 in_LightDirection_cameraspace,
                in vec3 in_Normal_cameraspace,
                in vec3 in_EyeDirection_cameraspace,
                in vec3 in_LightToFrag_cameraspace) {

        return vec4(0.0, 1.0, 0.0, 1.0);
    }

    //uniform variable declaration
    subroutine uniform SCE_ComputeLightType SCE_ComputeLight;

    void main(){

        vec3 MaterialDiffuseColor = texture2D(MainTex, fragUV).xyz;//vec3(fragUV.xy, 1);
        vec3 MaterialAmbientColor = vec3(0.1,0.1,0.1) * AmbientColor;
        vec3 MaterialSpecularColor = vec3(0.3,0.3,0.3);

        vec4 lightCol =  SCE_ComputeLight(
                            LightDirection_cameraspace,
                            Normal_cameraspace,
                            EyeDirection_cameraspace,
                            LightToFrag_cameraspace
                        );

        color = vec4(MaterialDiffuseColor, 1.0) * lightCol;
    }
_}
