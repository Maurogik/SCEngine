/******PROJECT:Sand Castle Engine******/
/**************************************/
/*********AUTHOR:Gwenn AUBERT**********/
/********FILE:Blur_Box_H.shader*********/
/**************************************/
/****************       ***************/

[ComputeShader]
_{
#version 430 core

//#define COMPUTE_BLOCK_SIZE 128

    layout (local_size_x = 128, local_size_y = 1, local_size_z = 1) in;

    uniform ivec4 SrcRectArea;
    uniform ivec4 DstRectArea;
    uniform int KernelHalfSize;
//    layout(rgba32f, binding = 0) uniform readonly image2D TexSrc;
    uniform sampler2D TexSrc;
    uniform writeonly image2D TexDst;

    vec2 pixToUV(int x, int y, ivec4 rectArea, vec2 texSize)
    {
        vec2 res;
        res.x = float(rectArea.x + x);
        res.x /= texSize.x;
        res.y = float(rectArea.y + y);
        res.y /= texSize.y;

        return res;
    }

#define SAMPLE_SRC(x, y) texture(TexSrc, pixToUV(x, y, SrcRectArea, srcTexSize))

    void main()
    {        
        int x = int(gl_GlobalInvocationID.x);

        int dstMinY = DstRectArea.y;
        int dstMaxY = DstRectArea.y + DstRectArea.w;
        int dstY = 0;
        int dstX = x + DstRectArea.x;

        vec2 srcTexSize = textureSize(TexSrc, 0);//size at lod 0

        if(x < DstRectArea.z)
        {
            float fKernelSize = float(KernelHalfSize)*2.0;

            vec4 colourSum = SAMPLE_SRC(x, 1)*fKernelSize*0.5;

            for( int y = 0; y <= KernelHalfSize; y++)
            {
                colourSum += SAMPLE_SRC(x, y);
            }

            for( int y = 0; y < DstRectArea.w; y++)
            {
                dstY = dstMinY + y;

//                vec4 col = SAMPLE_SRC(x, y); //vec4(colourSum/fKernelSize)
                vec4 col = vec4(colourSum/fKernelSize);//SAMPLE_SRC(x, y);//*vec4(1.0, 0.0, 0.0, 0.0);
                imageStore(TexDst, ivec2(dstX, dstY), col);

                // move window to the next
                vec4 leftBorder     = SAMPLE_SRC(dstX, max(dstY - KernelHalfSize, dstMinY));
                vec4 rightBorder    = SAMPLE_SRC(dstX, min(dstY + KernelHalfSize + 1, dstMaxY - 1));

                colourSum -= leftBorder;
                colourSum += rightBorder;
            }
        }
    }
_}

