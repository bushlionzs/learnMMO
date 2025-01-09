/* Copyright (c) 2023, Sascha Willems
 *
 * SPDX-License-Identifier: MIT
 *
 */

#version 460
#extension GL_EXT_ray_tracing : enable

layout(location = 0) rayPayloadInEXT vec3 hitValue;

void main()
{
    hitValue = vec3(0.678431f, 0.847058f, 0.901960f);
}