#include "common.hlsl"


// The MIT License
// Copyright (c) 2016-2017 Mohamad Moneimne and Contributors
//
// This fragment shader defines a reference implementation for Physically Based Shading of
// a microfacet surface material defined by a glTF model.
//
// References:
// [1] Real Shading in Unreal Engine 4
//     http://blog.selfshadow.com/publications/s2013-shading-course/karis/s2013_pbs_epic_notes_v2.pdf
// [2] Physically Based Shading at Disney
//     http://blog.selfshadow.com/publications/s2012-shading-course/burley/s2012_pbs_disney_brdf_notes_v3.pdf
// [3] README.md - Environment Maps
//     https://github.com/KhronosGroup/glTF-WebGL-PBR/#environment-maps
// [4] "An Inexpensive BRDF Model for Physically based Rendering" by Christophe Schlick
//     https://www.cs.virginia.edu/~jdl/bib/appearance/analytic%20models/schlick94b.pdf



// Encapsulate the various inputs used by the various functions in the shading equation
// We store values in this struct to simplify the integration of alternative implementations
// of the shading terms, outlined in the Readme.MD Appendix.
struct PBRInfo
{
    float NdotL;                  // cos angle between normal and light direction
    float NdotV;                  // cos angle between normal and view direction
    float NdotH;                  // cos angle between normal and half vector
    float LdotH;                  // cos angle between light direction and half vector
    float VdotH;                  // cos angle between view direction and half vector
    float perceptualRoughness;    // roughness value, as authored by the model creator (input to shader)
    float metalness;              // metallic value at the surface
    float3 reflectance0;            // full reflectance color (normal incidence angle)
    float3 reflectance90;           // reflectance color at grazing angle
    float alphaRoughness;         // roughness mapped to a more linear change in the roughness (proposed by [2])
    float3 diffuseColor;            // color contribution from diffuse lighting
    float3 specularColor;           // color contribution from specular lighting
};

#define  M_PI  3.141592653589793
float c_MinRoughness = 0.04;

#define  albedoIndex  0
#define ambientOcclusionIndex  1
#define normalMapIndex  2
#define emissiveIndex  3
#define metalRoughnessIndex 4
#define brdfLutIndex  5
#define iblDiffuseIndex  0
#define iblSpecularIndex  1
float4 SRGBtoLINEAR(float4 srgbIn)
{
    #ifdef MANUAL_SRGB
    #ifdef SRGB_FAST_APPROXIMATION
    float3 linOut = pow(srgbIn.xyz,float3(2.2, 2.2, 2.2));
    #else //SRGB_FAST_APPROXIMATION
    float3 bLess = step(float3(0.04045),srgbIn.xyz);
    float3 linOut = mix( srgbIn.xyz/float3(12.92), pow((srgbIn.xyz+float3(0.055))/float3(1.055),float3(2.4)), bLess );
    #endif //SRGB_FAST_APPROXIMATION
    return float4(linOut,srgbIn.w);;
    #else //MANUAL_SRGB
    return srgbIn;
    #endif //MANUAL_SRGB
}



float3 NormalSampleToWorldSpace(float3 normalMapSample, float3 unitNormalW, float3 tangentW, float3 worldPos, float2 uv)
{
	// Uncompress each component from [0,1] to [-1,1].
	float3 normalT = 2.0f*normalMapSample - 1.0f;
	
	float3 q1 = ddx(worldPos);
    float3 q2 = ddy(worldPos);
    float2 st1 = ddx(uv);
    float2 st2 = ddy(uv);

	// Build orthonormal basis.
	float3 N = unitNormalW;
	float3 T = normalize(tangentW - dot(tangentW, N)*N);
	//float3 T = normalize(q1 * st2.t - q2 * st1.t);
	float3 B = cross(N, T);

	float3x3 TBN = float3x3(T, B, N);

	// Transform from tangent space to world space.
	float3 bumpedNormalW = mul(normalT, TBN);

	return bumpedNormalW;
}

float3 getPbrNormal(float3 worldPos, float3 normal, float2 uv)
{
    float3 tangentNormal = (normal_pbr.Sample(normalSampler, uv).xyz * 2.0f) - 1.0f.xxx;
    float3 q1 = ddx(worldPos);
    float3 q2 = ddy(worldPos);
    float2 st1 = ddx(uv);
    float2 st2 = ddy(uv);
    float3 N = normalize(normal);
    float3 T = normalize((q1 * st2.y) - (q2 * st1.y));
    float3 B = -normalize(cross(N, T));
    float3x3 TBN = float3x3(float3(T), float3(B), float3(N));
    return normalize(mul(tangentNormal, TBN));
}


float4 SRGBtoLINEAR2(float4 srgbIn)
{
    float3 bLess = step(0.040449999272823333740234375f.xxx, srgbIn.xyz);
    float3 linOut = lerp(srgbIn.xyz / 12.9200000762939453125f.xxx, pow((srgbIn.xyz + 0.054999999701976776123046875f.xxx) / 1.05499994754791259765625f.xxx, 2.400000095367431640625f.xxx), bLess);
    return float4(linOut, srgbIn.w);
}

float3 Uncharted2Tonemap(float3 color)
{
    float A = 0.1500000059604644775390625f;
    float B = 0.5f;
    float C = 0.100000001490116119384765625f;
    float D = 0.20000000298023223876953125f;
    float E = 0.0199999995529651641845703125f;
    float F = 0.300000011920928955078125f;
    float W = 11.19999980926513671875f;
    return (((color * ((color * A) + (C * B).xxx)) + (D * E).xxx) / ((color * ((color * A) + B.xxx)) + (D * F).xxx)) - (E / F).xxx;
}

float4 tonemap(float4 color)
{
    float exposure = 4.5f;
    float gamma = 2.2000000476837158203125f;
    float3 param = color.xyz * exposure;
    float3 outcol = Uncharted2Tonemap(param);
    float3 param_1 = 11.19999980926513671875f.xxx;
    outcol *= (1.0f.xxx / Uncharted2Tonemap(param_1));
    return float4(pow(outcol, (1.0f / gamma).xxx), color.w);
}

#ifdef USE_IBL
// Calculation of the lighting contribution from an optional Image Based Light source.
// Precomputed Environment Maps are required uniform inputs and are computed as outlined in [1].
// See our README.md on Environment Maps [3] for additional discussion.
float3 getIBLContribution(PBRInfo pbrInputs, float3 n, float3 reflection)
{
    float mipCount = 4.0f;
    float lod = pbrInputs.perceptualRoughness * mipCount;
    float4 param = brdflut_pbr.Sample(brdflutSampler, float2(pbrInputs.NdotV, 1.0f - pbrInputs.perceptualRoughness));
    float3 brdf = SRGBtoLINEAR2(param).xyz;
    float4 param_1 = irradianceCube.Sample(irradianceSampler, n);
    float4 param_2 = tonemap(param_1);
    float3 diffuseLight = SRGBtoLINEAR2(param_2).xyz;
    float4 param_3 = prefilteredCube.SampleLevel(prefilteredSampler, reflection, lod);
    float4 param_4 = tonemap(param_3);
    float3 specularLight = SRGBtoLINEAR2(param_4).xyz;
    float3 diffuse_1 = diffuseLight * pbrInputs.diffuseColor;
    float3 specular = specularLight * ((pbrInputs.specularColor * brdf.x) + brdf.y.xxx);
    diffuse_1 *= u_ScaleIBLAmbient.x;
    specular *= u_ScaleIBLAmbient.y;
    return (diffuse_1 + specular) * 0.35f;
}
#endif

// Basic Lambertian diffuse
// Implementation from Lambert's Photometria https://archive.org/details/lambertsphotome00lambgoog
// See also [1], Equation 1
float3 diffuse(PBRInfo pbrInputs)
{
    return pbrInputs.diffuseColor / M_PI;
}

// The following equation models the Fresnel reflectance term of the spec equation (aka F())
// Implementation of fresnel from [4], Equation 15
float3 specularReflection(PBRInfo pbrInputs)
{
    return pbrInputs.reflectance0 + (pbrInputs.reflectance90 - pbrInputs.reflectance0) * pow(clamp(1.0 - pbrInputs.VdotH, 0.0, 1.0), 5.0);
}

// This calculates the specular geometric attenuation (aka G()),
// where rougher material will reflect less light back to the viewer.
// This implementation is based on [1] Equation 4, and we adopt their modifications to
// alphaRoughness as input as originally proposed in [2].
float geometricOcclusion(PBRInfo pbrInputs)
{
    float NdotL = pbrInputs.NdotL;
    float NdotV = pbrInputs.NdotV;
    float r = pbrInputs.alphaRoughness;

    float attenuationL = 2.0 * NdotL / (NdotL + sqrt(r * r + (1.0 - r * r) * (NdotL * NdotL)));
    float attenuationV = 2.0 * NdotV / (NdotV + sqrt(r * r + (1.0 - r * r) * (NdotV * NdotV)));
    return attenuationL * attenuationV;
}

// The following equation(s) model the distribution of microfacet normals across the area being drawn (aka D())
// Implementation from "Average Irregularity Representation of a Roughened Surface for Ray Reflection" by T. S. Trowbridge, and K. P. Reitz
// Follows the distribution function recommended in the SIGGRAPH 2013 course notes from EPIC Games [1], Equation 3.
float microfacetDistribution(PBRInfo pbrInputs)
{
    float roughnessSq = pbrInputs.alphaRoughness * pbrInputs.alphaRoughness;
    float f = (pbrInputs.NdotH * roughnessSq - pbrInputs.NdotH) * pbrInputs.NdotH + 1.0;
    return roughnessSq / (M_PI * f * f);
}



struct VertexIn
{
	VKLOCATION(0) float3 PosL    : POSITION;
    VKLOCATION(1) float3 NormalL : NORMAL;
	VKLOCATION(3) float2 TexC    : TEXCOORD;
};

struct VertexOut
{
	float4 PosH    : SV_POSITION;
    float3 PosW    : POSITION1;
    float3 NormalW : NORMAL;
	float2 v_UV    : TEXCOORD;
};

VertexOut VS(VertexIn vIn)
{
    VertexOut vOut;
	
    float4 posW = mul(gWorld, float4(vIn.PosL, 1.0f));

    vOut.PosH = mul(gViewProj, posW);
    vOut.PosW = posW.xyz / posW.w;
    vOut.NormalW = mul((float3x3) gWorld, vIn.NormalL);
    vOut.v_UV = vIn.TexC;

    return vOut;
}



float4 PS(VertexOut pin) : SV_Target
{
    // Metallic and Roughness material properties are packed together
    // In glTF, these factors can be specified by fixed scalar values
    // or from a metallic-roughness map
#ifdef HAS_METALROUGHNESSMAP
    // Roughness is stored in the 'g' channel, metallic is stored in the 'b' channel.
    // This layout intentionally reserves the 'r' channel for (optional) occlusion map data
    float4 mrSample = metal_roughness_pbr.Sample(metalRoughnessSampler, pin.v_UV);
	float metalness = mrSample.b;
    float roughness = mrSample.g;
#else
    float roughness = u_MetallicRoughnessValues.y;
    float metalness = u_MetallicRoughnessValues.x;
	roughness = clamp(roughness, c_MinRoughness, 1.0);
    metalness = clamp(metalness, 0.0, 1.0);
#endif
  

    #ifdef HAS_METALMAP
        metalness = metal_roughness_pbr.Sample(metalRoughnessSampler, pin.v_UV).r;
    #endif

    #ifdef HAS_ROUGHNESSMAP
        roughness = roughness_pbr.Sample(roughnessSampler, pin.v_UV).r;
    #endif

    // The albedo may be defined from a base texture or a flat color
	float4 baseColorSource = float4(1.0f, 1.0f, 1.0f, 1.0f);
#ifdef HAS_BASECOLORMAP
    baseColorSource = albedo_pbr.Sample(albedoSampler, pin.v_UV);
    float4 baseColor = SRGBtoLINEAR(baseColorSource) * u_BaseColorFactor;
	
	if (alpha_mask > 0)
	{
		if (baseColorSource.a < alpha_mask_cutoff)
		{
			discard;
		}
	}
#else
    float4 baseColor = vec4(0.04f, 0.04f, 0.04f, 1.0f);
#endif
    float3 f0 = float3(0.04, 0.04, 0.04);
	
	float alphaRoughness = roughness * roughness;
	
	float3 diffuseColor = baseColor.rgb * (float3(1.0, 1.0, 1.0) - f0);
	diffuseColor *= 1.0 - metalness;
	float3 specularColor = lerp(f0, baseColor.rgb, metalness);
	float reflectance = max(max(specularColor.r, specularColor.g), specularColor.b);
	
	float reflectance90 = clamp(reflectance * 25.0, 0.0, 1.0);
	float3 specularEnvironmentR0 = specularColor.rgb;
	float3 specularEnvironmentR90 = float3(1.0, 1.0, 1.0) * reflectance90;
	
	
	float3 inWorldPos = pin.PosW;
	float3 inNormal = pin.NormalW;
#ifdef HAS_NORMALMAP
	float3 n = getPbrNormal(inWorldPos, inNormal, pin.v_UV);
#else
    float3 n = inNormal;
#endif
    //n.y *= -1.0f;
	
	float3 v = normalize(gEyePosW - inWorldPos);        // Vector from surface point to camera
	
	float3 l = normalize(gDirLights[0].Direction); 
    
	float3 h = normalize(l+v);
	float3 reflection = normalize(reflect(-v, n));
	
    float NdotL = clamp(dot(n, l), 0.001, 1.0);
	float NdotV = clamp(abs(dot(n, v)), 0.001, 1.0);
	float NdotH = clamp(dot(n, h), 0.0, 1.0);
	float LdotH = clamp(dot(l, h), 0.0, 1.0);
	float VdotH = clamp(dot(v, h), 0.0, 1.0);
    PBRInfo pbrInputs = {
        NdotL,
        NdotV,
        NdotH,
        LdotH,
        VdotH,
        roughness,
        metalness,
        specularEnvironmentR0,
        specularEnvironmentR90,
        alphaRoughness,
        diffuseColor,
        specularColor
    };

	// Calculate the shading terms for the microfacet specular shading model
	float3 F = specularReflection(pbrInputs);
	float G = geometricOcclusion(pbrInputs);
	float D = microfacetDistribution(pbrInputs);

	const float3 u_LightColor = float3(1.0, 1.0, 1.0);

	// Calculation of analytical lighting contribution
	float3 diffuseContrib = (1.0 - F) * diffuse(pbrInputs);
	float3 specContrib = F * G * D / (4.0 * NdotL * NdotV);
	// Obtain final intensity as reflectance (BRDF) scaled by the energy of the light (cosine law)
	float3 directColor = NdotL * u_LightColor * (diffuseContrib + specContrib);
	
	float3 color = directColor;
    // Calculate lighting contribution from image based lighting source (IBL)
	float3 ibl = float3(0.0f, 0.0f, 0.0f);
#ifdef USE_IBL
	ibl = getIBLContribution(pbrInputs, n, reflection);
    color += ibl;
#endif
    // Apply optional PBR terms for additional (optional) shading
	float ao = 0.0f;
#ifdef HAS_OCCLUSIONMAP
	ao = ao_pbr.Sample(aoSampler, pin.v_UV).r;
    color = lerp(color, color * ao, u_OcclusionStrength);
#endif

#ifdef HAS_EMISSIVEMAP
	float3 emissive = emissive_pbr.Sample(emissiveSampler ,pin.v_UV).rgb * u_EmissiveFactor;
    color += emissive;
#endif
    
	float4 outColor = float4(color, baseColor.a);
		
	if(debugRenderMode > 0)
	{
        switch (debugRenderMode)
		{
		default:
		case 0: break;
		case 1: color = baseColorSource.rgb; break;
		case 2: color =  n; break;
		case 3: color = float3(roughness, roughness, roughness); break;
		case 4: color = float3(metalness, metalness, metalness); break;
		case 5: color = float3(ao, ao, ao); break;
		case 6: color = n; break;
		case 7: color = F;break;
		case 8: color = float3(G, G, G);break;
		case 9: color = float3(D, D, D);break;
		}
		outColor = float4(color, baseColorSource.a);
		if(debugRenderMode < 6)
		{
		    outColor = SRGBtoLINEAR(outColor);
		}
	}
	
	return outColor;
}
