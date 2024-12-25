#version 450
#define float2 vec2
#define float3 vec3
#define float4 vec4
#define f3x3 mat3
#define lerp mix
#define ddx dFdx
#define ddy dFdy
#define rsqrt(VALUE)		inversesqrt(VALUE)
#define make_f3x3_rows(R0, R1, R2) transpose(f3x3(R0, R1, R2))
#ifdef VERTEX_SHADER
#include "common.glsl"

layout (location = 0) in vec3 position;
layout (location = 1) in vec3 normal;
layout (location = 3) in vec2 texcoord;



layout (location = 0) out vec3 outWorldPos;
layout (location = 1) out vec3 outNormal;
layout (location = 2) out vec3 outTagent;
layout (location = 3) out vec2 outUV0;
layout (location = 4) out vec2 outUV1;
layout (location = 5) out vec4 outColor0;



void main()
{	
    vec4 locPos = (cbPerObject.gWorld * float4(position, 1.0f));
    outWorldPos = locPos.xyz / locPos.w;
    outNormal = mat3(cbPerObject.gWorld) * normal;
    outUV0 = texcoord;
	gl_Position = cbPass.gViewProj * float4(outWorldPos, 1.0f);
}

#endif //VERTEX_SHADER

#ifdef FRAGMENT_SHADER
#include "common.glsl"
#include "pbrutil.glsl"


#define c_MinRoughness  0.04



float saturate(float VALUE) { return clamp(VALUE, 0.0f, 1.0f); }

float3 ReconstructNormal(float4 sampleNormal, float intensity)
{
	float3 tangentNormal;
	tangentNormal.xy = (sampleNormal.rg * 2 - 1) * intensity;
	tangentNormal.z = sqrt(1 - saturate(dot(tangentNormal.xy, tangentNormal.xy)));
	return tangentNormal;
}

vec3 getNormal(float3 viewDirection, float3 normal, float2 uv)
{
    float4  sampleNormal = texture(sampler2D(normal_pbr, normalSampler), uv);
	// Perturb normal, see http://www.thetenthplanet.de/archives/1180
	vec3 tangentNormal = ReconstructNormal(sampleNormal, 0.56);
	

	float3 dPdx = ddx(viewDirection);
	float3 dPdy = ddy(viewDirection);
	float2 dUVdx = ddx(uv);
	float2 dUVdy = ddy(uv);

	float3 N = normalize(normal);
	float3 crossPdyN = cross(dPdy, N);
	float3 crossNPdx = cross(N, dPdx); 

	float3 T = crossPdyN * dUVdx.x + crossNPdx * dUVdy.x;
	float3 B = crossPdyN * dUVdx.y + crossNPdx * dUVdy.y;

	float invScale = rsqrt(max(dot(T, T), dot(B, B)));

	f3x3 TBN = make_f3x3_rows(T * invScale, B * invScale, N);
	
	return normalize(tangentNormal * TBN);
}


vec3 Uncharted2Tonemap(vec3 color)
{
	float A = 0.15;
	float B = 0.50;
	float C = 0.10;
	float D = 0.20;
	float E = 0.02;
	float F = 0.30;
	float W = 11.2;
	return ((color*(A*color+C*B)+D*E)/(color*(A*color+B)+D*F))-E/F;
}

vec4 tonemap(vec4 color)
{
    float exposure = 4.5f;
	float gamma = 2.2f;
	vec3 outcol = Uncharted2Tonemap(color.rgb * exposure);
	outcol = outcol * (1.0f / Uncharted2Tonemap(vec3(11.2f)));	
	return vec4(pow(outcol, vec3(1.0f / gamma)), color.a);
}

vec4 tonemap2(vec4 color)
{
    return color;
    float exposure = 4.5f;
	float gamma = 2.2f;
	vec3 outcol = Uncharted2Tonemap(color.rgb * exposure);
	outcol = outcol * (1.0f / Uncharted2Tonemap(vec3(11.2f)));	
	return vec4(pow(outcol, vec3(1.0f / gamma)), color.a);
}

vec4 SRGBtoLINEAR2(vec4 srgbIn)
{
	vec3 bLess = step(vec3(0.04045),srgbIn.xyz);
	vec3 linOut = mix( srgbIn.xyz/vec3(12.92), pow((srgbIn.xyz+vec3(0.055))/vec3(1.055),vec3(2.4)), bLess );
	return vec4(linOut,srgbIn.w);;
}

float Pow5(float x)
{
	float xx = x * x;
	return xx * xx * x;
}

float3 F_Schlick(float3 SpecularColor, float VoH)
{
	float Fc = Pow5(1.0f - VoH);		// 1 sub, 3 mul
										//return Fc + (1 - Fc) * SpecularColor;		// 1 add, 3 mad

	// Anything less than 2% is physically impossible and is instead considered to be shadowing
	return saturate(SpecularColor.g) * Fc + (1.0f - Fc) * SpecularColor;
}

float D_GGX(float a2, float NoH)
{
	float d = (NoH * a2 - NoH) * NoH + 1.0f;	// 2 mad
	return a2 / (M_PI*d*d);					// 4 mul, 1 rcp
}

float GeometrySchlickGGX(float NdotV, float roughness)
{
	// note that we use a different k for IBL
	float a = roughness;
	float k = (a * a) / 2.0;

	float nom = NdotV;
	float denom = NdotV * (1.0 - k) + k;

	return nom / denom;
}

float GeometrySmith(float NdotV, float NdotL, float roughness)
{
	float ggx2 = GeometrySchlickGGX(NdotV, roughness);
	float ggx1 = GeometrySchlickGGX(NdotL, roughness);

	return ggx1 * ggx2;
}

float3 BRDF(float NoV, float NoL, float3 N, float3 V, float3 L, float3 albedo, float3 F0, float roughness, float metalness)
{	
	const float3 H = normalize(V + L);
	float NoH = saturate(dot(N, H));

	// Calculate surface reflection..
	float3 F = F_Schlick(F0, NoH);

	float3 kS = F;
	float3 kD = (float3(1.0f, 1.0f, 1.0f) - kS) * (1.0f - metalness);


	float a = roughness*roughness;
	float a2 = a*a;

	// Calculate normal distribution.. Approx. microfacets..
	float NDF = D_GGX(a2, NoH);
	// Calculate Geometry func.. Self-shadowing for microfacets..
	float G = GeometrySmith(NoV, NoL, roughness);

	// Id & Is: diffuse & specular illumination
	float3 Is = NDF * G * F / (4.0f * NoV * NoL + 0.001f);
	// Lambert diffuse..
	float3 Id = kD * albedo / M_PI;

	return Id + Is;
}

#ifdef USE_IBL
// Calculation of the lighting contribution from an optional Image Based Light source.
// Precomputed Environment Maps are required uniform inputs and are computed as outlined in [1].
// See our README.md on Environment Maps [3] for additional discussion.
float3 getIBLContribution(PBRInfo pbrInputs, float3 n, float3 reflection)
{
    float mipCount = 4.0; // resolution of 512x512
    float lod = (pbrInputs.perceptualRoughness * mipCount);
    // retrieve a scale and bias to F0. See [1], Figure 3
    float3 brdf = SRGBtoLINEAR2(texture(sampler2D(brdflut, brdflutSampler), float2(pbrInputs.NdotV, 1.0 - pbrInputs.perceptualRoughness))).rgb;
    float3 diffuseLight = SRGBtoLINEAR2(tonemap(texture(samplerCube(irradianceCube, irradianceSampler), n))).rgb;

    float3 specularLight = SRGBtoLINEAR2(tonemap(textureLod(samplerCube(prefilteredCube, prefilteredSampler), reflection, lod))).rgb;

    float3 diffuse = diffuseLight * pbrInputs.diffuseColor;
    float3 specular = specularLight * (pbrInputs.specularColor * brdf.x + brdf.y);

    // For presentation, this allows us to disable IBL terms
    diffuse *= pbrMaterial.u_ScaleIBLAmbient.x;
    specular *= pbrMaterial.u_ScaleIBLAmbient.y;

    return (diffuse + specular)*0.35;
}

float3 FresnelSchlickRoughness(float cosTheta, float3 F0, float roughness)
{
	float3 ret = float3(0.0, 0.0, 0.0);
	float powTheta = pow(1.0 - cosTheta, 5.0);
	float invRough = float(1.0 - roughness);

	ret.x = F0.x + (max(invRough, F0.x) - F0.x) * powTheta;
	ret.y = F0.y + (max(invRough, F0.y) - F0.y) * powTheta;
	ret.z = F0.z + (max(invRough, F0.z) - F0.z) * powTheta;

	return ret;
}

float3 EnvironmentBRDF(float3 N, float3 V, float3 albedo, float roughness, float metalness)
{
	const float3 reflection = reflect(-V, N);

	// F0 represents the base reflectivity (calculated using IOR: index of refraction)
	float3 F0 = float3(0.04f, 0.04f, 0.04f);
	F0 = lerp(F0, albedo, metalness);

	float3 F = FresnelSchlickRoughness(max(dot(N, V), 0.0f), F0, roughness);

	float3 kS = F;
	float3 kD = (float3(1.0f, 1.0f, 1.0f) - kS) * (1.0f - metalness);
	float lod = roughness * 4.0f;
	
	float3 irradiance = SRGBtoLINEAR2(tonemap(texture(samplerCube(irradianceCube, irradianceSampler), N))).rgb;
	float3 specular = SRGBtoLINEAR2(tonemap(textureLod(samplerCube(prefilteredCube, prefilteredSampler), reflection, lod))).rgb;

	float2 maxNVRough = float2(max(dot(N, V), 0.0), roughness);
	float3 brdf = SRGBtoLINEAR2(texture(sampler2D(brdflut, brdflutSampler), maxNVRough)).rgb;
	// Id & Is: diffuse & specular illumination
	float3 Is = specular * (F * brdf.x + brdf.y);	
	float3 Id = kD * irradiance * albedo;
	

	return (Id + Is);
}
#endif

// Basic Lambertian diffuse
// Implementation from Lambert's Photometria https://archive.org/details/lambertsphotome00lambgoog
// See also [1], Equation 1






layout (location = 0) in vec3 inWorldPos;
layout (location = 1) in vec3 inNormal;
layout (location = 2) in vec3 inTagent;
layout (location = 3) in vec2 inUV0;
layout (location = 4) in vec2 inUV1;
layout (location = 5) in vec4 inColor0;

layout (location = 0) out vec4 outColor;

void main()
{
    // Metallic and Roughness material properties are packed together
    // In glTF, these factors can be specified by fixed scalar values
    // or from a metallic-roughness map
#ifdef HAS_METALROUGHNESSMAP
    // Roughness is stored in the 'g' channel, metallic is stored in the 'b' channel.
    // This layout intentionally reserves the 'r' channel for (optional) occlusion map data
    float4 mrSample = texture(sampler2D(metal_roughness_pbr, metalRoughnessSampler), inUV0);
	float metalness = mrSample.b;
    float roughness = mrSample.g;
#else
    float roughness = pbrMaterial.u_MetallicRoughnessValues.y;
    float metalness = pbrMaterial.u_MetallicRoughnessValues.x;
	roughness = clamp(roughness, c_MinRoughness, 1.0);
    metalness = clamp(metalness, 0.0, 1.0);
#endif
  

    #ifdef HAS_METALMAP
        metalness = texture(sampler2D(metal_roughness_pbr, metalRoughnessSampler), inUV0).r;
    #endif

    #ifdef HAS_ROUGHNESSMAP
        roughness = texture(sampler2D(roughness_pbr, roughnessSampler), inUV0).r;
    #endif

    // The albedo may be defined from a base texture or a flat color
	float4 baseColorSource = float4(1.0f, 1.0f, 1.0f, 1.0f);
#ifdef HAS_BASECOLORMAP
    baseColorSource = texture(sampler2D(albedo_pbr, albedoSampler), inUV0);
    float4 baseColor = SRGBtoLINEAR(baseColorSource) * pbrMaterial.u_BaseColorFactor;
	
	if (pbrMaterial.alpha_mask > 0)
	{
		if (baseColorSource.a < pbrMaterial.alpha_mask_cutoff)
		{
			discard;
		}
	}
#else
    float4 baseColor = vec4(0.04f, 0.04f, 0.04f, 1.0f);
#endif
    float3 f0 = float3(0.04, 0.04, 0.04);
	
	float alphaRoughness = roughness * roughness;
	
	vec3 diffuseColor = baseColor.rgb * (vec3(1.0) - f0);
	diffuseColor *= 1.0 - metalness;
	vec3 specularColor = mix(f0, baseColor.rgb, metalness);
	float reflectance = max(max(specularColor.r, specularColor.g), specularColor.b);
	
	float reflectance90 = clamp(reflectance * 25.0, 0.0, 1.0);
	vec3 specularEnvironmentR0 = specularColor.rgb;
	vec3 specularEnvironmentR90 = vec3(1.0, 1.0, 1.0) * reflectance90;
	
	
	
#ifdef HAS_NORMALMAP
	float3 n = getPbrNormal(inWorldPos, inNormal, inUV0);
#else
    float3 n = inNormal;
#endif
    //n.y *= -1.0f;
	
	float3 v = normalize(cbPass.gEyePosW - inWorldPos);        // Vector from surface point to camera
	
	vec3 l = normalize(cbPass.gDirLights[0].Direction); 
    
	vec3 h = normalize(l+v);
	vec3 reflection = normalize(reflect(-v, n));
	
    float NdotL = clamp(dot(n, l), 0.001, 1.0);
	float NdotV = clamp(abs(dot(n, v)), 0.001, 1.0);
	float NdotH = clamp(dot(n, h), 0.0, 1.0);
	float LdotH = clamp(dot(l, h), 0.0, 1.0);
	float VdotH = clamp(dot(v, h), 0.0, 1.0);
	PBRInfo pbrInputs = PBRInfo(
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
	);

	// Calculate the shading terms for the microfacet specular shading model
	vec3 F = specularReflection(pbrInputs);
	float G = geometricOcclusion(pbrInputs);
	float D = microfacetDistribution(pbrInputs);

	const vec3 u_LightColor = vec3(1.0);

	// Calculation of analytical lighting contribution
	vec3 diffuseContrib = (1.0 - F) * diffuse(pbrInputs);
	vec3 specContrib = F * G * D / (4.0 * NdotL * NdotV);
	// Obtain final intensity as reflectance (BRDF) scaled by the energy of the light (cosine law)
	vec3 directColor = NdotL * u_LightColor * (diffuseContrib + specContrib);
	
	float3 color = directColor;
    // Calculate lighting contribution from image based lighting source (IBL)
	float3 ibl = vec3(0.0f);
#ifdef USE_IBL
	ibl = getIBLContribution(pbrInputs, n, reflection);
    color += ibl;
#endif
    // Apply optional PBR terms for additional (optional) shading
	float ao = 0.0f;
#ifdef HAS_OCCLUSIONMAP
	ao = texture(sampler2D(ao_pbr, aoSampler), inUV0).r;
    //color = lerp(color, color * ao, pbrMaterial.u_OcclusionStrength);
#endif

#ifdef HAS_EMISSIVEMAP
	float3 emissive = texture(sampler2D(emissive_pbr, emissiveSampler),inUV0).rgb * pbrMaterial.u_EmissiveFactor;
    color += emissive;
#endif
    
	outColor = vec4(color, baseColor.a);
	
	//outColor = SRGBtoLINEAR(outColor);
	
	if(pbrMaterial.debugRenderMode > 0)
	{
        switch (pbrMaterial.debugRenderMode)
		{
		default:
		case 0: break;
		case 1: color = baseColorSource.rgb; break;
		//case 2: color =  texture(sampler2D(normal_pbr, normalSampler), inUV0).rgb; break;
		case 3: color = float3(roughness); break;
		case 4: color = float3(metalness); break;
		case 5: color = float3(ao); break;
		case 6: color = n; break;
		case 7: color = F;break;
		case 8: color = vec3(G);break;
		case 9: color = vec3(D);break;
		}
		outColor = vec4(color, baseColorSource.a);
		if(pbrMaterial.debugRenderMode < 6)
		{
		    outColor = SRGBtoLINEAR(outColor);
		}
	}
}

void main1()
{
    // Metallic and Roughness material properties are packed together
    // In glTF, these factors can be specified by fixed scalar values
    // or from a metallic-roughness map
#ifdef HAS_METALROUGHNESSMAP
    // Roughness is stored in the 'g' channel, metallic is stored in the 'b' channel.
    // This layout intentionally reserves the 'r' channel for (optional) occlusion map data
    float4 mrSample = texture(sampler2D(metal_roughness_pbr, metalRoughnessSampler), inUV0);
	float metalness = mrSample.b;
    float roughness = mrSample.g;
#else
    float roughness = pbrMaterial.u_MetallicRoughnessValues.y;
    float metalness = pbrMaterial.u_MetallicRoughnessValues.x;
#endif
    
#ifdef HAS_METALMAP
    metalness = texture(sampler2D(metal_roughness_pbr, metalRoughnessSampler), inUV0).r;
#endif

#ifdef HAS_ROUGHNESSMAP
  roughness = texture(sampler2D(roughness_pbr, roughnessSampler), inUV0).r;
#endif

    roughness = clamp(roughness, c_MinRoughness, 1.0);
    metalness = clamp(metalness, 0.0, 1.0);

    // The albedo may be defined from a base texture or a flat color
#ifdef HAS_BASECOLORMAP
    float4 baseColorSource = texture(sampler2D(albedo_pbr, albedoSampler), inUV0);
    float4 baseColor = SRGBtoLINEAR(baseColorSource) * pbrMaterial.u_BaseColorFactor;
	
	if (pbrMaterial.alpha_mask > 0)
	{
		if (baseColorSource.a < pbrMaterial.alpha_mask_cutoff)
		{
			discard;
		}
	}
#else
    float4 baseColor = vec4(0.04f, 0.04f, 0.04f, 1.0f);
#endif
    float3 f0 = float3(0.04, 0.04, 0.04);
	f0 = lerp(f0, baseColor.rgb, metalness);
	
	float3 v = normalize(cbPass.gEyePosW - inWorldPos);        // Vector from surface point to camera
	
#ifdef HAS_NORMALMAP
	float3 n = getNormal(-v, inNormal, inUV0);
#else
    float3 n = inNormal;
#endif

    //n.y *= -1.0f;
    float NdotV = clamp(abs(dot(n, v)), 0.001, 1.0);
    

    float3 directColor = vec3(0.0f);
	
	for (uint i = 0; i < cbPass.numDirLights; ++i)
	{
	    float3 l = normalize(cbPass.gDirLights[i].Direction);
	    float NdotL = clamp(dot(n, l), 0.001, 1.0);
	    directColor += BRDF(NdotV, NdotL, n, v, l, baseColor.rgb, f0, roughness, metalness);
	}
	
	float3 color = directColor;
    // Calculate lighting contribution from image based lighting source (IBL)
	float3 ibl = vec3(0.0f);
#ifdef USE_IBL
	
	for (uint i = 0; i < cbPass.numDirLights; ++i)
	{
	    ibl += EnvironmentBRDF(n, v, baseColor.rgb, roughness, metalness);
	}
	
    color += ibl;
#endif
    // Apply optional PBR terms for additional (optional) shading
	float ao = 0.0f;
#ifdef HAS_OCCLUSIONMAP
	ao = texture(sampler2D(ao_pbr, aoSampler), inUV0).r;
    color = lerp(color, color * ao, pbrMaterial.u_OcclusionStrength);
#endif

#ifdef HAS_EMISSIVEMAP
	float3 emissive = texture(sampler2D(emissive_pbr, emissiveSampler),inUV0).rgb * pbrMaterial.u_EmissiveFactor;
    color += emissive;
#endif
    
	color = color / (color + float3(1.0f, 1.0f, 1.0f));
	outColor = vec4(color, baseColorSource.a);
	if(pbrMaterial.debugRenderMode > 0)
	{
        switch (pbrMaterial.debugRenderMode)
		{
		default:
		case 0: break;
		case 1: color = baseColorSource.rgb; break;
		case 2: color =  n; break;
		case 3: color = float3(roughness); break;
		case 4: color = float3(metalness); break;
		case 5: color = float3(ao); break;
		case 6: color = directColor; break;
		}
		outColor = vec4(color, baseColorSource.a);
	}
}

#endif //FRAGMENT_SHADER