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

    layout (local_size_x = 1, local_size_y = 128, local_size_z = 1) in;

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
        int y = int(gl_GlobalInvocationID.y);

        int dstMinX = DstRectArea.x;
        int dstMaxX = DstRectArea.x + DstRectArea.z;
        int dstY = y + DstRectArea.y;
        int dstX = 0;
        vec2 srcTexSize = textureSize(TexSrc, 0);//size at lod 0

        if(y < DstRectArea.w)
        {
            float fKernelSize = float(KernelHalfSize)*2.0;

            vec4 colourSum = SAMPLE_SRC(0, y)*fKernelSize*0.5;

            for( int x = 0; x <= KernelHalfSize; x++)
            {
                colourSum += SAMPLE_SRC(x, y);
            }

            for( int x = 0; x < DstRectArea.z; x++)
            {
                dstX = dstMinX + x;

    //            vec4 col = imageLoad(TexSrc, ivec2(x, y)); //vec4(colourSum/fKernelSize)
                vec4 col = SAMPLE_SRC(x, y);//*vec4(1.0, 1.0, 0.0, 0.0);
                imageStore(TexDst, ivec2(dstX, dstY), col);

                // move window to the next
                vec4 leftBorder     = SAMPLE_SRC(max(dstX - KernelHalfSize, dstMinX), dstY);
                vec4 rightBorder    = SAMPLE_SRC(min(dstX + KernelHalfSize + 1, dstMaxX - 1), dstY);

                colourSum -= leftBorder;
                colourSum += rightBorder;
            }
        }
    }
_}

