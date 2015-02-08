VertexShader : 
_{
    #version 400 core


    uniform mat4 MVP;
    uniform mat4 M;
    uniform mat4 V;
    uniform mat4 P;

    in vec3 vertexPosition_modelspace;
    in vec2 vertexUV;
    in vec3 vertexNormal_modelspace;

    out vec3 worldPos;

    void main(){
        worldPos = (M * vec4(vertexPosition_modelspace,1)).xyz;
        gl_Position = MVP * vec4(vertexPosition_modelspace,1);//MVP * vec4(vertexPosition_modelspace,1);
    }
_}

FragmentShader : 
_{
    #version 400 core

    in vec3 worldPos;
    out vec4 color;

    void main(){
        color = vec4(worldPos.z, 0.0f, worldPos.x, 1.0f);
    }
_}
