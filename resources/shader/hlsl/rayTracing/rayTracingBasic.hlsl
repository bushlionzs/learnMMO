// Copyright 2020 Google LLC

RaytracingAccelerationStructure topLevelAS : register(t0);
RWTexture2D<float4> image : register(u1);

struct CameraProperties
{
	float4x4 viewInverse;
	float4x4 projInverse;
};
cbuffer cam : register(b2) { CameraProperties cam; };

struct Payload
{
   float4 hitValue;
};

struct Attributes
{
  float2 bary;
};

[shader("raygeneration")]
void genMain()
{
	uint3 LaunchID = DispatchRaysIndex();
	uint3 LaunchSize = DispatchRaysDimensions();

	const float2 pixelCenter = float2(LaunchID.xy) + float2(0.5, 0.5);
	const float2 inUV = pixelCenter/float2(LaunchSize.xy);
	float2 d = inUV * 2.0 - 1.0;
	float4 target = mul(cam.projInverse, float4(d.x, d.y, 1, 1));

	RayDesc rayDesc;
	rayDesc.Origin = mul(cam.viewInverse, float4(0,0,0,1)).xyz;
	rayDesc.Direction = mul(cam.viewInverse, float4(normalize(target.xyz), 0)).xyz;
	rayDesc.TMin = 0.001;
	rayDesc.TMax = 10000.0;

	Payload payload;
	TraceRay(topLevelAS, RAY_FLAG_FORCE_OPAQUE, 0xff, 0, 0, 0, rayDesc, payload);

	image[int2(LaunchID.xy)] = payload.hitValue;
}

struct DataBlock {
    uint data[25]; // 100 bytes / 4 bytes per uint = 25 elements
};

[shader("closesthit")]
void closethitMain(inout Payload p, in Attributes attribs)
{
  //uint instanceId = InstanceID();
  //uint geometryId = GeometryIndex();
  const float3 barycentricCoords = float3(1.0f - attribs.bary.x - attribs.bary.y, attribs.bary.x, attribs.bary.y);
  p.hitValue = float4(barycentricCoords, 0.0f);
}

[shader("miss")]
void missMain(inout Payload p)
{
    p.hitValue = float4(0.0, 0.0, 0.2, 0.0f);
}