/******PROJECT:Sand Castle Engine******/
/**************************************/
/*********AUTHOR:Gwenn AUBERT**********/
/********FILE:TextDebug.shader*********/
/**************************************/
/****************       ***************/

[VertexShader]
_{
#version 430 core

    in vec3 vertexPosition_modelspace;
    in vec2 vertexUV;

    out vec2 fragUV;

    uniform mat4 MVP;

    void main()
    {
        gl_Position = MVP * vec4(vertexPosition_modelspace, 1.0);
        fragUV = vertexUV;
    }
_}

[FragmentShader]
_{
#version 430 core

//#define DEBUG

    uniform vec3 TextColor;
    uniform sampler2D FontAtlas;

    in vec2 fragUV;

    out vec4 color;

    void main()
    {
        //gamma correction
        float fontAlpha = texture(FontAtlas, fragUV).r;
        color = vec4(TextColor, fontAlpha);
    }
_}
