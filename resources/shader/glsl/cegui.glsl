#version 450
#ifdef VERTEX_SHADER
#include "common.glsl"


layout (location = 0) in vec3 position;
layout (location = 2) in vec4 color_diffuse;
layout (location = 3) in vec2 texcoord;

layout (location = 0) out vec4 outColor;
layout (location = 1) out vec2 outTexC;

void main() {
    gl_Position =  cbPass.gViewProj * cbPerObject.gWorld * vec4(position, 1.0);
	outColor = color_diffuse;
	outTexC = texcoord;
}
#endif //VERTEX_SHADER

#ifdef FRAGMENT_SHADER
#include "common.glsl"

layout (location = 0) in vec4 inColor;
layout (location = 1) in vec2 inTexC;

layout(location = 0) out vec4 outColor;

void main() {
	outColor = texture(sampler2D(first, firstSampler), inTexC) * inColor;
	if(outColor.a < 0.5f)
	{
	    discard;
	}
}
#endif //FRAGMENT_SHADER