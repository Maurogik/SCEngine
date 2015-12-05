[VertexShader]
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

[FragmentShader]
_{
#version 400 core

    in vec2 fragUV;
    in vec3 Normal_worldspace;
    in vec3 Position_worldspace;

    layout (location = 0) out vec3 oPosition;
    layout (location = 1) out vec3 oColor;
    layout (location = 2) out vec4 oNormal;

    uniform sampler2D MainTex;
    uniform float ScaleU;
    uniform float ScaleV;
    uniform vec3 MainColor;
    uniform float Roughness;

    void main()
    {
        vec2 uv = vec2(fragUV.x, fragUV.y);
        oColor = texture2D(MainTex, uv * vec2(ScaleU, ScaleV)).xyz;
        //gamma expansion of texture because it is store gamma corrected and we will do
        //our own gamma correction in the last shading pass
        oColor = pow(oColor, vec3(2.2));
        oColor *= MainColor;

        oPosition = Position_worldspace;
        //compressed normal
        oNormal.xyz = normalize(Normal_worldspace);
        //specularity
        oNormal.a = Roughness;
    }
_}
