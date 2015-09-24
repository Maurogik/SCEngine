[VertexShader]
_{
#version 400 core

    in vec3 vertexPosition_modelspace;
    in vec2 vertexUV;
    in vec3 vertexNormal_modelspace;

    out vec2 fragUV;
    out vec3 Normal_worldspace;
    out vec3 Position_worldspace;
    out vec3 VertColor;

    uniform mat4 MVP;
    uniform mat4 M;
    uniform mat4 V;
    uniform mat4 P;
    uniform float SCE_TimeInSeconds;

    void main()
    {

        float wings = step(-0.2, abs(vertexPosition_modelspace.x) + vertexPosition_modelspace.z)
                * step(0.125, abs(vertexPosition_modelspace.x))
                * step(-0.025, vertexPosition_modelspace.y);

//        float wings = clamp(abs(vertexPosition_modelspace.x) + vertexPosition_modelspace.z + 0.3, 0.0, 1.0)
//                * clamp(abs(vertexPosition_modelspace.x) - 0.125, 0.0, 1.0)
//                * clamp(vertexPosition_modelspace.y + 0.025, 0.0, 1.0);

        float moveStr = abs(vertexPosition_modelspace.x) - 0.1;

        VertColor = vec3(wings, 0.0, 0.0);
        vec3 vertPos_modelspace = vertexPosition_modelspace;
        vertPos_modelspace.y += wings * sin(SCE_TimeInSeconds * 1.5) * 0.5 * moveStr;

        gl_Position = MVP * vec4(vertPos_modelspace, 1.0);
        fragUV = vertexUV;
        Position_worldspace = ( M * vec4(vertPos_modelspace, 1.0) ).xyz;
        Normal_worldspace = ( M * vec4(vertexNormal_modelspace, 0.0) ).xyz;
    }
_}

[FragmentShader]
_{
#version 400 core

    in vec2 fragUV;
    in vec3 Normal_worldspace;
    in vec3 Position_worldspace;
    in vec3 VertColor;

    layout (location = 0) out vec3 oPosition;
    layout (location = 1) out vec3 oColor;
    layout (location = 2) out vec4 oNormal;

    uniform sampler2D MainTex;
    uniform float ScaleU;
    uniform float ScaleV;
    uniform vec3 MainColor;
    uniform float Specularity;

    void main()
    {
        vec2 uv = vec2(fragUV.x, fragUV.y);
        oColor = texture2D(MainTex, uv * vec2(ScaleU, ScaleV)).xyz;
        //gamma expansion of texture because it is store gamma corrected and we will do
        //our own gamma correction in the last shading pass
        oColor = oColor * MainColor;
//        oColor = VertColor;
        oColor = pow(oColor, vec3(2.2));

        oPosition = Position_worldspace;
        //compressed normal
        oNormal.xyz = normalize(Normal_worldspace);
        //specularity
        oNormal.a = Specularity;
    }
_}
