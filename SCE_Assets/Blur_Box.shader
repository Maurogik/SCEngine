/******PROJECT:Sand Castle Engine******/
/**************************************/
/*********AUTHOR:Gwenn AUBERT**********/
/********FILE:TextDebug.shader*********/
/**************************************/
/****************       ***************/

[ComputeShader]
_{
#version 430 core

//#define COMPUTE_BLOCK_SIZE 128

//    layout(local_size_x = 1, local_size_y = 128, local_size_z = 1​) in;
    layout (local_size_x = 1, local_size_y = 128, local_size_z = 1) in;

//    uniform int TexSize;
//    uniform int KernelHalfDist;
    layout (rgba32f, location = 1) uniform readonly image2D TexSrc;
    layout (rgba32f, location = 2) uniform writeonly image2D TexDst;

    int TexSize = 512;
    int KernelHalfDist = 128;

    void main()
    {
        uint y = gl_GlobalInvocationID.y;

        float fKernelSize = float(KernelHalfDist)*2.0;
        vec4 colourSum = imageLoad(TexSrc, ivec2(0, y))*fKernelSize*0.5;

        for( int x = 0; x <= KernelHalfDist; x++)
        {
            colourSum += imageLoad(TexSrc, ivec2(x, y));
        }

        for( int x = 0; x < TexSize; x++)
        {
            imageStore(TexDst, ivec2(x, y), vec4(1.0));//vec4(colourSum/fKernelSize));

            // move window to the next
            vec4 leftBorder     = imageLoad(TexSrc, ivec2(max(x - KernelHalfDist, 0), y));
            vec4 rightBorder    = imageLoad(TexSrc, ivec2(min(x + KernelHalfDist + 1, TexSize - 1), y));

            colourSum -= leftBorder;
            colourSum += rightBorder;
        }

    }
_}

