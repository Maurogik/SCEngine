VertexShader : 
_{
	attribute vec3 vertexPosition_modelspace;
	attribute vec2 vertexUV;
	attribute vec3 vertexNormal_modelspace;

	varying vec3 fragColor;
	//varying vec2 fragUV;
	//varying vec3 fragNormal;

	uniform mat4 MVP;
	uniform mat4 M;
	uniform mat4 V;
	uniform mat4 P;

	uniform vec3 LightPos_worldspace;
	uniform vec4 LightColor;
	uniform vec3 AmbientColor;

	void main(){
 
	    gl_Position =  MVP * vec4(vertexPosition_modelspace,1);
	    //fragUV = vertexUV;
	    //fragNormal = vertexNormal_modelspace;

		vec3 Position_worldspace = (M * vec4(vertexPosition_modelspace,1)).xyz;

		vec3 vertexPosition_cameraspace = ( V * M * vec4(vertexPosition_modelspace,1)).xyz;
		vec3 EyeDirection_cameraspace = vec3(0,0,0) - vertexPosition_cameraspace;

		vec3 LightPosition_cameraspace = ( V * vec4(LightPos_worldspace,1)).xyz;
		vec3 LightDirection_cameraspace = LightPosition_cameraspace + EyeDirection_cameraspace;
		
		// Normal of the the vertex, in camera space
		vec3 Normal_cameraspace = ( V * M * vec4(vertexNormal_modelspace,0)).xyz; // Only correct if ModelMatrix does not scale the model ! Use its inverse transpose if not.

		float distance = length( LightPos_worldspace - Position_worldspace );

		vec3 n = normalize( Normal_cameraspace );
		vec3 l = normalize( LightDirection_cameraspace );
		float cosTheta = clamp( dot( n,l ), 0,1 );
		

		vec3 E = normalize(EyeDirection_cameraspace);
		vec3 R = reflect(-l,n);
		float cosAlpha = clamp( dot( E,R ), 0,1 );
		
		vec3 MaterialDiffuseColor = AmbientColor;
		vec3 MaterialAmbientColor = vec3(0.1,0.1,0.1) * MaterialDiffuseColor;
		vec3 MaterialSpecularColor = vec3(0.3,0.3,0.3);

		fragColor.rgb = 
			// Ambient : simulates indirect lighting
			MaterialAmbientColor +
			// Diffuse : "color" of the object
			MaterialDiffuseColor * LightColor.rgb * LightColor.a * cosTheta + // / (distance*distance) +
			// Specular : reflective highlight, like a mirror
			MaterialSpecularColor * LightColor.rgb * LightColor.a * pow(cosAlpha,5); // / (distance*distance);
	}
_}

FragmentShader : 
_{
	varying vec3 fragColor;
	//varying vec2 fragUV;
	//varying vec3 fragNormal;

	//uniform sampler2D myTextureSampler;

	void main(){
	    //gl_FragColor = texture2D( myTextureSampler, UV );
	    gl_FragColor = vec4(fragColor, 1);

	}
_}