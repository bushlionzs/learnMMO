#version 450
#extension GL_ARB_separate_shader_objects : enable
#extension GL_ARB_shading_language_include : enable
#include "common.glsl"

layout (location = 0) in vec4 inPosH;
layout (location = 1) in vec4 inShadowPosH;
layout (location = 2) in vec3 inPosW;
layout (location = 3) in vec3 inNormalW;
layout (location = 4) in vec2 inTexC;

layout(location = 0) out vec4 outColor;

void main() {
    outColor = texture(sampler2D(first, firstSampler), inTexC) * cbMaterial.gDiffuseAlbedo;
	if(outColor.a < 0.5f)
	{
	    discard;
	}
}