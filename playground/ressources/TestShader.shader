VertexShader : 
_{
#version 400 core

    in vec3 vertexPosition_modelspace;
    in vec2 vertexUV;
    in vec3 vertexNormal_modelspace;

    out vec2 fragUV;
    out vec3 Normal_worldspace;
    out vec3 Position_worldspace;

    uniform mat4 MVP;
    uniform mat4 M;
    uniform mat4 V;
    uniform mat4 P;

    void main()
    {
        gl_Position = MVP * vec4(vertexPosition_modelspace, 1.0);
        fragUV = vertexUV;
        Position_worldspace = ( M * vec4(vertexPosition_modelspace, 1.0) ).xyz;
        Normal_worldspace = ( M * vec4(vertexNormal_modelspace, 0.0) ).xyz;
    }
_}

FragmentShader : 
_{
#version 400 core

    in vec2 fragUV;
    in vec3 Normal_worldspace;
    in vec3 Position_worldspace;

    //out vec3 oColor;

    layout (location = 0) out vec3 oPosition;
    layout (location = 1) out vec3 oColor;
    layout (location = 2) out vec3 oNormal;
    //layout (location = 3) out vec3 oUV;

    uniform vec3 AmbientColor;
    uniform sampler2D MainTex;

    void main()
    {
        oColor = texture2D(MainTex, fragUV).xyz;
        //gamma expansion of texture because it is store gamma corrected and we will do
        //our own gamma correction in the last shading pass
        oColor = pow(oColor, vec3(2.2));
        oPosition = Position_worldspace;
        oNormal = Normal_worldspace;
        //oUV = fragUV;
    }
_}
