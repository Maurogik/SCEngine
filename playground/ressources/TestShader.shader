VertexShader : 
_{
    #version 400 core

	in vec3 vertexPosition_modelspace;
	in vec2 vertexUV;
	in vec3 vertexNormal_modelspace;

	out vec3 fragColor;
	out vec2 fragUV;
	out vec3 Normal_cameraspace;
        out vec3 LightDirection_cameraspace;
        out vec3 EyeDirection_cameraspace;

	uniform mat4 MVP;
	uniform mat4 M;
	uniform mat4 V;
	uniform mat4 P;

	uniform vec3 LightPos_worldspace;

	void main(){
 
	    gl_Position =  MVP * vec4(vertexPosition_modelspace,1);
	    fragUV = vertexUV;

            vec3 Position_worldspace = (M * vec4(vertexPosition_modelspace,1)).xyz;

            vec3 vertexPosition_cameraspace = ( V * M * vec4(vertexPosition_modelspace,1)).xyz;
            EyeDirection_cameraspace = vec3(0,0,0) - vertexPosition_cameraspace;

            vec3 LightPosition_cameraspace = ( V * vec4(LightPos_worldspace,1)).xyz;
            LightDirection_cameraspace = LightPosition_cameraspace + EyeDirection_cameraspace;

            // Normal of the the vertex, in camera space
            Normal_cameraspace = ( V * M * vec4(vertexNormal_modelspace,0)).xyz; // Only correct if ModelMatrix does not scale the model ! Use its inverse transpose if not.

            //float distance = length( LightPos_worldspace - Position_worldspace );
	}
_}

FragmentShader : 
_{
    #version 400 core

    in vec3 fragColor;
    in vec2 fragUV;
    in vec3 Normal_cameraspace;
    in vec3 LightDirection_cameraspace;
    in vec3 EyeDirection_cameraspace;

    out vec3 color;

    uniform vec3 AmbientColor;
    uniform vec4 LightColor;
    uniform sampler2D MainTex;

    void main(){

        // Normal of the computed fragment, in camera space
        vec3 n = normalize( Normal_cameraspace );
        // Direction of the light (from the fragment to the light)
        vec3 l = normalize( LightDirection_cameraspace );

        float cosTheta = clamp( dot( n,l ), 0,1 );

        vec3 E = normalize(EyeDirection_cameraspace);
        vec3 R = reflect(-l,n);
        float cosAlpha = clamp( dot( E,R ), 0,1 );

        vec3 MaterialDiffuseColor = texture2D(MainTex, fragUV).xyz;//vec3(fragUV.xy, 1);
        vec3 MaterialAmbientColor = vec3(0.1,0.1,0.1) * AmbientColor;
        vec3 MaterialSpecularColor = vec3(0.3,0.3,0.3);

        color = // Ambient : simulates indirect lighting
                MaterialAmbientColor +
                // Diffuse : "color" of the object
                MaterialDiffuseColor * LightColor.rgb * LightColor.a * cosTheta + // / (distance*distance) +
                // Specular : reflective highlight, like a mirror
                MaterialSpecularColor * LightColor.rgb * LightColor.a * pow(cosAlpha,5); // / (distance*distance)*/
        //color = MaterialDiffuseColor;
    }
_}
