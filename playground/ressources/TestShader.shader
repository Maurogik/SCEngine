VertexShader : 
_{
	attribute vec3 vertexPosition_modelspace;
	attribute vec2 vertexUV;
	attribute vec3 vertexNormal_modelspace;

	varying vec3 fragColor;
	varying vec2 fragUV;
	varying vec3 fragNormal;

	uniform mat4 MVP;
	uniform mat4 M;
	uniform mat4 V;

	uniform vec3 AmbientColor;

	void main(){
 
	    gl_Position =  MVP * vec4(vertexPosition_modelspace,1);
	    //fragUV = vertexUV;
	    //fragNormal = vertexNormal_modelspace;
	    fragColor = AmbientColor;
	}
_}

FragmentShader : 
_{
	varying vec3 fragColor;
	varying vec2 fragUV;
	varying vec3 fragNormal;

	//uniform sampler2D myTextureSampler;

	void main(){

	    //gl_FragColor = texture2D( myTextureSampler, UV );
	    gl_FragColor = vec4(fragColor, 1);
	    //gl_FragColor = vec4(fragColor.z, fragUV.x * 0.01f, fragNormal.x * 0.01f, 1.0f);
	    //vec4(1, fragUV.x, fragNormal.x, 1);
	}
_}