/* Copyright (c) 2018-2023, Sascha Willems
 *
 * SPDX-License-Identifier: MIT
 *
 */

#version 450

layout(binding = 1, std140) uniform  PushConstsStruct{
    mat4 mvp;
	float roughness;
	uint numSamples;
} pushConsts;

layout (location = 0) in vec3 position;
layout (location = 0) out vec3 outUVW;

void main() 
{
	outUVW = position;
	gl_Position = pushConsts.mvp * vec4(position.xyz, 1.0);
}
