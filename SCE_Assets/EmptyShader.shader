/******PROJECT:Sand Castle Engine******/
/**************************************/
/*********AUTHOR:Gwenn AUBERT**********/
/*******FILE:emptyShader.shader********/
/**************************************/

VertexShader :
_{
#version 430 core

    in vec3 vertexPosition_modelspace;

    uniform mat4 MVP;

    void main()
    {
        gl_Position                 = MVP * vec4(vertexPosition_modelspace, 1.0);
    }
_}

FragmentShader :
_{
#version 430 core

    out vec4 color;

    void main()
    {
        color = vec4(gl_FragCoord.z, 0.0, 0.0, 1.0);
    }
_}
