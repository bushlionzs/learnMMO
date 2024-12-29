#define  M_PI  3.1415926535897932384626422832795028841971f
float2 octWrap_float2(float2 v)
{
    return (float2(1.f, 1.f) - float2(abs(v.y), abs(v.x))) * float2(v.x >= 0.f ? 1.f : -1.f, v.y >= 0.f ? 1.f : -1.f);
}


float3 decodeDir(float2 encN)
{
    encN = encN * 2.0f - float2(1.0f, 1.f);

    float3 n;
    n.z = 1.0f - abs(encN.x) - abs(encN.y);

    n.xy = n.z >= 0.0f ? encN.xy : octWrap_float2(encN.xy);
    n = normalize(n);
    return n;
}


struct BarycentricDeriv
{
	float3 m_lambda;
	float3 m_ddx;
	float3 m_ddy;
};

float3 rayTriangleIntersection(float3 p0, float3 p1, float3 p2, float3 o, float3 d)
{
	float3 v0v1 = p1-p0;
	float3 v0v2 = p2-p0;
	float3 pvec = cross(d,v0v2);
	float det = dot(v0v1,pvec);
	float invDet = 1/det;
	float3 tvec = o - p0;
	float u = dot(tvec,pvec) * invDet;
	float3 qvec = cross(tvec,v0v1);
	float v = dot(d,qvec) *invDet;
	float w = 1.0f - v - u;
	return float3(w,u,v);
}

BarycentricDeriv CalcRayBary(float3 pt0, float3 pt1, float3 pt2,float3 position,float3 positionDX,float3 positionDY,
								float3 camPos)
{
	BarycentricDeriv ret ;


	float3 curRay = position - camPos;
	float3 rayDX = positionDX - camPos;
	float3 rayDY = positionDY - camPos;

	float3 H = rayTriangleIntersection(pt0,pt1,pt2,camPos,normalize(curRay));
	float3 Hx = rayTriangleIntersection(pt0,pt1,pt2,camPos,normalize(rayDX));
	float3 Hy = rayTriangleIntersection(pt0,pt1,pt2,camPos,normalize(rayDY));
	ret.m_lambda = H;

	ret.m_ddx = Hx-H;
	ret.m_ddy = Hy-H;
	return ret;
}

BarycentricDeriv CalcFullBary(float4 pt0, float4 pt1, float4 pt2, float2 pixelNdc, float2 two_over_windowsize)
{
	BarycentricDeriv ret;
	float3 invW = rcp(float3(pt0.w, pt1.w, pt2.w));

	float2 ndc0 = pt0.xy * invW.x;
	float2 ndc1 = pt1.xy * invW.y;
	float2 ndc2 = pt2.xy * invW.z;






	float invDet = rcp(determinant(float2x2(ndc2 - ndc1, ndc0 - ndc1)));



	ret.m_ddx = float3(ndc1.y - ndc2.y, ndc2.y - ndc0.y, ndc0.y - ndc1.y) * invDet * invW;
	ret.m_ddy = float3(ndc2.x - ndc1.x, ndc0.x - ndc2.x, ndc1.x - ndc0.x) * invDet * invW;

	float ddxSum = dot(ret.m_ddx, float3(1,1,1));
	float ddySum = dot(ret.m_ddy, float3(1,1,1));


	float2 deltaVec = pixelNdc - ndc0;


	float interpInvW = invW.x + deltaVec.x*ddxSum + deltaVec.y*ddySum;
	float interpW = rcp(interpInvW);


	ret.m_lambda.x = interpW * (invW[0] + deltaVec.x*ret.m_ddx.x + deltaVec.y*ret.m_ddy.x);
	ret.m_lambda.y = interpW * (0.0f + deltaVec.x*ret.m_ddx.y + deltaVec.y*ret.m_ddy.y);
	ret.m_lambda.z = interpW * (0.0f + deltaVec.x*ret.m_ddx.z + deltaVec.y*ret.m_ddy.z);


	ret.m_ddx *= two_over_windowsize.x;
	ret.m_ddy *= two_over_windowsize.y;
	ddxSum *= two_over_windowsize.x;
	ddySum *= two_over_windowsize.y;

	ret.m_ddy *= -1.0f;
	ddySum *= -1.0f;







	float interpW_ddx = 1.0f / (interpInvW + ddxSum);
	float interpW_ddy = 1.0f / (interpInvW + ddySum);


	ret.m_ddx = interpW_ddx*(ret.m_lambda*interpInvW + ret.m_ddx) - ret.m_lambda;
	ret.m_ddy = interpW_ddy*(ret.m_lambda*interpInvW + ret.m_ddy) - ret.m_lambda;

	return ret;
}


struct GradientInterpolationResults
{
	float2 interp;
	float2 dx;
	float2 dy;
};

float sampleScreenSpaceShadowValue(uint2 uv)
{
	//uint shadowDepthSample = LoadTex2D(Get(ScreenSpaceShadowTexture), Get(clampBorderNearSampler), uv, 0).r;
	//return float(shadowDepthSample) /  0xFFFF ;
	return 0.0f;
}

float InterpolateWithDeriv_float3(BarycentricDeriv deriv, float3 v)
{
	return dot(v,deriv.m_lambda);
}

float3 InterpolateWithDeriv_float3x3(BarycentricDeriv deriv,f3x3 attributes)
{
	float3 attr0 = getCol0(attributes);
	float3 attr1 = getCol1(attributes);
	float3 attr2 = getCol2(attributes);
	return float3(dot(attr0,deriv.m_lambda),dot(attr1,deriv.m_lambda),dot(attr2,deriv.m_lambda));
}

GradientInterpolationResults Interpolate2DWithDeriv(BarycentricDeriv deriv,f3x2 attributes)
{
	float3 attr0 = getRow0(attributes);
	float3 attr1 = getRow1(attributes);

	GradientInterpolationResults result;

	result.interp.x = InterpolateWithDeriv_float3(deriv,attr0);
	result.interp.y = InterpolateWithDeriv_float3(deriv,attr1);


	result.dx.x = dot(attr0,deriv.m_ddx);
	result.dx.y = dot(attr1,deriv.m_ddx);
	result.dy.x = dot(attr0,deriv.m_ddy);
	result.dy.y = dot(attr1,deriv.m_ddy);
	return result;
}

struct DerivativesOutput
{
	float3 db_dx;
	float3 db_dy;
};

float Pow5(float x)
{
	float xx = x * x;
	return xx * xx * x;
}

DerivativesOutput Cal3DDeriv(BarycentricDeriv deriv,f3x3 attributes)
{
	DerivativesOutput result;
	result.db_dx = mul(attributes,deriv.m_ddx);
	result.db_dy = mul(attributes,deriv.m_ddy);

	return result;
}

float3 perturb_normal(float3 normalMap, float3 normalVertex, DerivativesOutput dposition, DerivativesOutput duv)
{
	float3 dPdx = dposition.db_dx;
	float3 dPdy = dposition.db_dy;

	float2 dUVdx = duv.db_dx.xy;
	float2 dUVdy = duv.db_dy.xy;

	float3 crossPdyN = cross(dPdy, normalVertex);
	float3 crossNPdx = cross(normalVertex, dPdx);

	float3 tangent = crossPdyN * dUVdx.x + crossNPdx * dUVdy.x;
	float3 binormal = crossPdyN * dUVdx.y + crossNPdx * dUVdy.y;

	float invScale = rsqrt(max(dot(tangent, tangent), dot(binormal, binormal)));

	float3x3 TBN = make_f3x3_rows(tangent * invScale, binormal * invScale, normalVertex);

	return normalize(mul(normalMap, TBN));
}


float D_GGX(float a2, float NoH)
{
	float d = (NoH * a2 - NoH) * NoH + 1.0f;
	return a2 / (M_PI*d*d);
}


float Vis_SmithJointApprox(float a2, float NoV, float NoL)
{
	float a = sqrt(a2);
	float Vis_SmithV = NoL * (NoV * (1.0f - a) + a);
	float Vis_SmithL = NoV * (NoL * (1.0f - a) + a);
	return 0.5f * rcp(max(Vis_SmithV + Vis_SmithL, 0.001f));
}

float3 F_Schlick(float3 SpecularColor, float VoH)
{
	float Fc = Pow5(1.0f - VoH);
	return saturate(SpecularColor.g) * Fc + (1.0f - Fc) * SpecularColor;
}

float3 SpecularGGX(float Roughness, inout(float3) SpecularColor, float NoL, float Nov, float NoH, float VoH)
{
	float a = Roughness * Roughness;
	float a2 = a * a;


	float D = D_GGX(a2, NoH);
	float Vis = Vis_SmithJointApprox(a2, Nov, NoL);
	SpecularColor = F_Schlick(SpecularColor, VoH);

	return (D * Vis) * SpecularColor;
}

float3 PBR_shadowFactor(float NoL, float NoV, float3 LightVec, float3 ViewVec,
	float3 HalfVec, float3 NormalVec, inout(float3) albedo,
	inout(float3) specColor, float Roughness, float Metallic, bool isBackface, float shadowFactor)
{
	float NoH = saturate(dot(NormalVec, HalfVec));
	float VoH = saturate(dot(ViewVec, HalfVec));


	float3 specularTerm = f3(0.0f);
	if (!isBackface)
		specularTerm = lerp(f3(0.0f), SpecularGGX(Roughness, specColor, NoL, NoV, NoH, VoH), shadowFactor);

	return (albedo + specularTerm);
}


float3 calculateSpecular(float3 specularColor, float3 camPos, float3 pixelPos, float3 normalizedDirToLight, float3 normal)
{
	float3 viewVec = normalize(camPos - pixelPos);
	float3 halfVec = normalize(viewVec + normalizedDirToLight);
	float specIntensity = 128.0f;
	float specular = pow(saturate(dot(halfVec, normal)), specIntensity);

	return specularColor * specular;
}

float3 calculateIllumination(
	float3 normal,
	float3 ViewVec,
	float3 HalfVec,
	float NoL,
	float NoV,
	float3 camPos,
	float3 normalizedDirToLight, float3 position,
	float3 albedo,
	float3 specularColor,
	float Roughness,
	float Metallic,
	bool isBackface,
	bool isPBR,
	float shadowFactor)
{
	float3 finalColor;

	if (isPBR)
	{
		finalColor = PBR_shadowFactor(NoL, NoV, normalizedDirToLight, ViewVec, HalfVec, normal, albedo, specularColor, Roughness, Metallic, isBackface, shadowFactor);
	}
	else
	{
		specularColor = calculateSpecular(specularColor, camPos, position, normalizedDirToLight, normal);

		finalColor = albedo + lerp(float3(0.0, 0.0, 0.0), specularColor, shadowFactor);
	}

	finalColor *= shadowFactor;

	return finalColor;
}