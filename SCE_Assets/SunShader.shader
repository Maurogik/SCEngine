/******PROJECT:Sand Castle Engine******/
/**************************************/
/*********AUTHOR:Gwenn AUBERT**********/
/********FILE:SunShader.shader*********/
/**************************************/
/*************** 1700µs ***************/

[VertexShader]
_{
#version 430 core

    in vec3 vertexPosition_modelspace;
    in vec3 vertexNormal_modelspace;

    uniform mat4 MVP;
    uniform mat4 M;
    uniform mat4 V;
    uniform mat4 P;

    void main()
    {
        gl_Position                 = MVP * vec4(vertexPosition_modelspace, 1.0);
    }
_}

[FragmentShader]
_{
#version 430 core

    uniform vec2        SCE_ScreenSize;
    uniform vec3        SunPosition_worldspace;
    uniform float       SizeQuality;
    layout (location = 0) uniform sampler2D   PositionTex;

    uniform mat4 V;
    uniform mat4 P;

    out vec3 color;

    float pent(in vec2 fragCoord){
        vec2 sunToFrag = abs(fragCoord);
        return max(max(sunToFrag.x*1.176-fragCoord.y*0.385,
                       sunToFrag.x*0.727+fragCoord.y), -fragCoord.y*1.237)*1.;
    }

    //originaly from https://www.shadertoy.com/view/XtS3DD
    vec2 flare(vec2 fragCoord, vec2 sunCoord)
    {
        vec2 res;
        vec2 sunToFrag = fragCoord - sunCoord;
        float a = atan(sunToFrag.x,sunToFrag.y);

        float fra = fract(a * 0.8 + 0.12);
        float rz = 0.55*(pow(abs(fra-0.5),3.0)*exp2((-dot(sunToFrag,sunToFrag)*4.))); //Spokes
        //squared spokes
//        float rz = pow(abs(fract(a * 0.61) - 0.5), 4.0)*exp2((-dot(sunToFrag,sunToFrag)*4.));
        res.r = rz;
        rz = 0.0;

        vec2 pds = fragCoord * (length(fragCoord)) * 0.75;
        rz += max(1.0/(1.0+32.0*pent(pds+0.8*sunCoord)),.0)*0.2; //Projected ghost (main lens)

        vec2 p2 = mix(fragCoord,pds,-.5); //Reverse distort
        rz += max(0.01-pow(pent(p2 + 0.4*sunCoord),2.2),.0)*3.0;
        rz += max(0.01-pow(pent(p2 + 0.2*sunCoord),5.5),.0)*3.0;
        rz += max(0.01-pow(pent(p2 - 0.1*sunCoord),1.6),.0)*4.0;
        rz += max(0.01-pow(pent(-(p2 + 1.*sunCoord)),2.5),.0)*5.0;
        rz += max(0.01-pow(pent(-(p2 - .5*sunCoord)),2.),.0)*4.0;
        rz += max(0.01-pow(pent(-(p2 + .7*sunCoord)),5.),.0)*3.0;

        res.g = clamp(rz,0.,1.);
        return res;
    }


    vec2 getSunColor(vec2 ndcUv, vec3 sun_projectionspace)
    {
        float sunStrength = 10.0;

        sunStrength *= step(0.001, sun_projectionspace.z) *
                step(abs(sun_projectionspace.x), 1.2) *
                step(abs(sun_projectionspace.y), 1.2);

        vec2 fixedNdc = ndcUv * vec2(1.0, SCE_ScreenSize.y/SCE_ScreenSize.x); //compensate for aspect ratio
        sun_projectionspace.xy *= vec2(1.0, SCE_ScreenSize.y/SCE_ScreenSize.x);
        vec2 ndcToSun = sun_projectionspace.xy - fixedNdc;
        float sun = clamp(1.0 - length(ndcToSun), 0.0, 1.0);
        sun = pow(sun, 4.0) * 0.8 + smoothstep(0.0, 0.050, sun - 0.92) * 3.0;

        vec2 flare = flare(fixedNdc, sun_projectionspace.xy);

        return vec2(clamp(sun + flare.r*2.0, 0.0, sunStrength),
                    clamp(flare.g, 0.0, sunStrength));
    }

//#define LIGHT_SHAFTS
#ifdef LIGHT_SHAFTS
    float computeVolumetricLight(vec2 uv, vec2 sunUV)
    {
        float nbSamples = 16.0;
        float decay = 1.0;
        float density = 0.15;
        float weight = 0.005;

        vec2 stepToFrag = (sunUV - uv);
        stepToFrag *= 1.0 / float(nbSamples) * density;

        float illum = 0.0;
        vec2 sampleUV = uv;

        for(float i = 0; i < nbSamples; ++i)
        {
            sampleUV += stepToFrag;
            vec3 Position_worldpsace = texture(PositionTex, sampleUV).xyz;
            float sampleVal = step(dot(Position_worldpsace, Position_worldpsace), 0.0001);
            sampleVal *= pow((nbSamples - i) / nbSamples, decay) * weight;
            illum += sampleVal;
        }

        return illum;
    }
#endif

    void main()
    {
        vec2 uv = gl_FragCoord.xy / (SCE_ScreenSize * SizeQuality);
        vec3 Position_worldpsace = texture(PositionTex, uv).xyz;
        vec3 sun_cameraspace = (V * vec4(SunPosition_worldspace, 1.0)).xyz;

        float notOccludedByScene = step(dot(Position_worldpsace, Position_worldpsace), 0.0001);

        //compute normalized device coord and screenspace sun position
        vec2 ndcUv = uv * 2.0 - vec2(1.0);
        vec4 sun_projectionspace = (P * vec4(sun_cameraspace, 1.0));
        sun_projectionspace /= sun_projectionspace.w;
        vec2 sunUV = sun_projectionspace.xy * 0.5 + vec2(0.5);

        float sunSqLen = dot(sun_projectionspace.xy, sun_projectionspace.xy);

        color = vec3(0.0);

        if(sunSqLen < 2.0)
        {
            float sunStrength = 1.1 - sunSqLen * 0.5;
            sunStrength *= step(0.0, sun_cameraspace.z);

            vec2 sunColor = getSunColor(ndcUv, sun_projectionspace.xyz);
            color.r = sunColor.r * notOccludedByScene * sunStrength;
#ifdef LIGHT_SHAFTS
            color.g = computeVolumetricLight(uv, sunUV) * sunStrength;
#endif
            color.b = sunColor.g * pow(sunStrength, 2.0);
        }
    }
_}
