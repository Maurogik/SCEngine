/******PROJECT:Sand Castle Engine******/
/**************************************/
/*********AUTHOR:Gwenn AUBERT**********/
/*****FILE:standardLighting.shader*****/
/**************************************/

//required : basic model data

//uniform names

//DIRECTIONAL LIGHT
//SCE_LightDirection_worldspace
//SCE_LightColor

//POINT LIGHT
//SCE_LightPosition_worldspace
//SCE_LightColor
//SCE_LightReach_worldspace

//SPOT LIGHT
//SCE_LightPosition_worldspace
//SCE_LightDirection_worldspace
//SCE_LightReach_worldspace
//SCE_LightStartRadius_worldspace
//SCE_LightEndRadius_worldspace
//SCE_LightColor

#pragma include(lightingUniforms.shader)
//includes should set a boolean to true when they are used to avoid using them twice


//define a subroutine signature
subroutine vec4 SCE_ComputeLightColor(in vec3 in_Normal, in vec3 in_EyeDirection_cameraspace);//use fragment data

//Directional light option
subroutine (SCE_ComputeLightColor) vec4 SCE_ComputeDirectionalLight() {
    return vec4(1.0, 0.0, 0.0, 1.0);
}

//Point light option
subroutine (SCE_ComputeLightColor) vec4 SCE_ComputePointLight() {
    return vec4(0.0, 0.0, 1.0, 1.0);
}

//Spot light option
subroutine (SCE_ComputeLightColor) vec4 SCE_ComputeSpotLight() {
    return vec4(0.0, 1.0, 0.0, 1.0);
}
