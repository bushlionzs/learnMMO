#define LoadByte(BYTE_BUFFER, ADDRESS) ((BYTE_BUFFER)[(ADDRESS)>>2])
#define LoadByte2(BYTE_BUFFER, ADDRESS) uint2(((BYTE_BUFFER)[((ADDRESS) >> 2) + 0]), ((BYTE_BUFFER)[((ADDRESS) >> 2) + 1]))
#define LoadByte3(BYTE_BUFFER, ADDRESS) uint3(((BYTE_BUFFER)[((ADDRESS) >> 2) + 0]), ((BYTE_BUFFER)[((ADDRESS) >> 2) + 1]), ((BYTE_BUFFER)[((ADDRESS) >> 2) + 2]))
#define LoadByte4(BYTE_BUFFER, ADDRESS) uint4(((BYTE_BUFFER)[((ADDRESS) >> 2) + 0]), ((BYTE_BUFFER)[((ADDRESS) >> 2) + 1]), ((BYTE_BUFFER)[((ADDRESS) >> 2) + 2]), ((BYTE_BUFFER)[((ADDRESS) >> 2) + 3]))

#define StoreByte(BYTE_BUFFER, ADDRESS, VALUE)  (BYTE_BUFFER)[((ADDRESS) >> 2) + 0] = VALUE;
#define StoreByte2(BYTE_BUFFER, ADDRESS, VALUE) (BYTE_BUFFER)[((ADDRESS) >> 2) + 0] = VALUE[0]; (BYTE_BUFFER)[((ADDRESS) >> 2) + 1] = VALUE[1];
#define StoreByte3(BYTE_BUFFER, ADDRESS, VALUE) (BYTE_BUFFER)[((ADDRESS) >> 2) + 0] = VALUE[0]; (BYTE_BUFFER)[((ADDRESS) >> 2) + 1] = VALUE[1]; (BYTE_BUFFER)[((ADDRESS) >> 2) + 2] = VALUE[2];
#define StoreByte4(BYTE_BUFFER, ADDRESS, VALUE) (BYTE_BUFFER)[((ADDRESS) >> 2) + 0] = VALUE[0]; (BYTE_BUFFER)[((ADDRESS) >> 2) + 1] = VALUE[1]; (BYTE_BUFFER)[((ADDRESS) >> 2) + 2] = VALUE[2]; (BYTE_BUFFER)[((ADDRESS) >> 2) + 3] = VALUE[3];

#define AtomicAdd(DEST, VALUE, ORIGINAL_VALUE) \
    {ORIGINAL_VALUE = atomicAdd(DEST, VALUE);}
#define AtomicStore(DEST, VALUE) \
    (DEST) = (VALUE)
#define AtomicLoad(SRC) SRC

#define f4(X) vec4(X)
#define f3(X) vec3(X)
#define f2(X) vec2(X)
#define u4(X) uvec4(X)
#define u3(X) uvec3(X)
#define u2(X) uvec2(X)
#define i4(X) ivec4(X)
#define i3(X) ivec3(X)
#define i2(X) ivec2(X)	
#define int2 ivec2
#define int3 ivec3
#define int4 ivec4
#define uint2 uvec2
#define uint3 uvec3
#define uint4 uvec4
#define float float
#define float2 vec2
#define float3 vec3
#define packed_float3 vec3
#define float4 vec4
#define float2x2 mat2
#define float3x3 mat3
#define float3x2 mat3x2
#define float2x3 mat2x3
#define float4x4 mat4

#define f2x2 mat2x2
#define f2x3 mat2x3
#define f2x4 mat2x4
#define f3x2 mat3x2
#define f3x3 mat3x3
#define f3x4 mat3x4
#define f4x2 mat4x2
#define f4x3 mat4x3
#define f4x4 mat4x4

#define inout(T) inout T
#define out(T) out T
#define in(T) in T
#define Get(v) v



bvec3 Equal(vec3 X, float Y) { return equal(X, vec3(Y));}

bvec2 LessThan(in(vec2) a, in(float) b)      { return lessThan(a, vec2(b)); }
bvec2 LessThan(in(vec2) a, in(vec2) b)       { return lessThan(a, b);}
bvec3 LessThan(in(vec3) X, in(float) Y)      { return lessThan(X, f3(Y)); }
bvec3 LessThanEqual(in(vec3) X, in(float) Y) { return lessThanEqual(X, f3(Y)); }
bvec2 LessThanEqual(in(vec2) a, in(vec2) b)  { return lessThanEqual(a, b);}

bvec2 GreaterThan(in(vec2) a, in(float) b)      { return greaterThan(a, vec2(b)); }
bvec3 GreaterThan(in(vec3) a, in(float) b)      { return greaterThan(a, f3(b)); }
bvec2 GreaterThan(in(uvec2) a, in(uint) b)      { return greaterThan(a, uvec2(b)); }
bvec2 GreaterThan(in(vec2) a, in(vec2) b)       { return greaterThan(a, b);}
bvec2 GreaterThanEqual(in(vec2) a, in(vec2) b)  { return greaterThanEqual(a, b);}
bvec4 GreaterThan(in(vec4) a, in(vec4) b)       { return greaterThan(a, b); }
bvec3 GreaterThanEqual(in(vec3) a, in(float) b) { return greaterThanEqual(a, vec3(b)); }

bvec2 And(in(bvec2) a, in(bvec2) b)
{ return bvec2(a.x && b.x, a.y && b.y); }

bool any(vec2 x) { return any(notEqual(x, vec2(0))); }
bool any(vec3 x) { return any(notEqual(x, vec3(0))); }

mat4 mul(mat4 a, mat4 b)  { return a * b; }
vec4 mul(mat4 a, vec4 b)  { return a * b; }
vec4 mul(vec4 a, mat4 b)  { return a * b; }

mat3 mul(mat3 a, mat3 b)  { return a * b; }
vec3 mul(mat3 a, vec3 b)  { return a * b; }
vec3 mul(vec3 a, mat3 b)  { return a * b; }

vec2 mul(mat2 a, vec2 b)  { return a * b; }
vec3 mul(vec3 a, float b) { return a * b; }


#define UNROLL_N(X)
#define UNROLL
#define LOOP
#define FLATTEN
#define sincos(angle, s, c) { (s) = sin(angle); (c) = cos(angle); }

#define RES(TYPE, NAME, FREQ, REG, BINDING) layout(FREQ, BINDING) uniform TYPE NAME

#define RWTex1D(T)           VK_T_##T(image1D)
#define RWTex2D(T)           VK_T_##T(image2D)
#define RWTex3D(T)           VK_T_##T(image3D)

#define SampleLvlTex2D(NAME, SAMPLER, COORD, LEVEL) \
textureLod(sampler2D(NAME, SAMPLER), COORD, LEVEL)
#define SampleLvlTex3D(NAME, SAMPLER, COORD, LEVEL) \
textureLod(sampler3D(NAME, SAMPLER), COORD, LEVEL)
#define SampleLvlTex2DArray(NAME, SAMPLER, COORD, LEVEL ) \
textureLod(sampler2DArray(NAME, SAMPLER), COORD, LEVEL)
#define SampleLvlTexCube(NAME, SAMPLER, COORD, LEVEL ) \
textureLod(samplerCube(NAME, SAMPLER), COORD, LEVEL)
#define SampleLvlTexCubeArray(NAME, SAMPLER, COORD, LEVEL ) \
textureLod(samplerCubeArray(NAME, SAMPLER), float4(COORD, LEVEL), 0)

#define LoadTex2D(TEX, SMP, P, LOD) _LoadTex2D((TEX), (SMP), ivec2((P).xy), int(LOD))
vec4 _LoadTex2D( texture2D TEX, sampler SMP, ivec2 P, int lod) { return texelFetch( sampler2D(TEX, SMP), P, lod); }
uvec4 _LoadTex2D(utexture2D TEX, sampler SMP, ivec2 P, int lod) { return texelFetch(usampler2D(TEX, SMP), P, lod); }
ivec4 _LoadTex2D(itexture2D TEX, sampler SMP, ivec2 P, int lod) { return texelFetch(isampler2D(TEX, SMP), P, lod); }

#define LoadRWTex2D(TEX, P) imageLoad(TEX, ivec2(P))
#define LoadRWTex3D(TEX, P) imageLoad(TEX, ivec3(P))

#define SampleGradTex2D(TEX, SMP, P, DX, DY) \
textureGrad(sampler2D(TEX, SMP), P, DX, DY)

#define SampleTex2D(TEX, SMP, P) \
texture(sampler2D(TEX, SMP), P)

vec4  _to4(in(vec4)  x)  { return x; }
vec4  _to4(in(vec3)  x)  { return vec4(x, 0); }
vec4  _to4(in(vec2)  x)  { return vec4(x, 0, 0); }
vec4  _to4(in(float) x)  { return vec4(x, 0, 0, 0); }
uvec4 _to4(in(uvec4) x)  { return x; }
uvec4 _to4(in(uvec3) x)  { return uvec4(x, 0); }
uvec4 _to4(in(uvec2) x)  { return uvec4(x, 0, 0); }
uvec4 _to4(in(uint)  x)  { return uvec4(x, 0, 0, 0); }
ivec4 _to4(in(ivec4) x)  { return x; }
ivec4 _to4(in(ivec3) x)  { return ivec4(x, 0); }
ivec4 _to4(in(ivec2) x)  { return ivec4(x, 0, 0); }
ivec4 _to4(in(int)   x)  { return ivec4(x, 0, 0, 0); }

#define Write2D(TEX, P, V) imageStore(TEX, ivec2((P).xy),  _to4(V))
#define Write3D(TEX, P, V) imageStore(TEX, ivec3((P.xyz)), _to4(V))
#define Write2DArray(TEX, P, I, V) imageStore(TEX, ivec3(P, I), _to4(V))

#define make_f2x2_cols(C0, C1) f2x2(C0, C1)
#define make_f2x2_rows(R0, R1) transpose(f2x2(R0, R1))
#define make_f2x2_col_elems f2x2
#define make_f2x3_cols(C0, C1) f2x3(C0, C1)
#define make_f2x3_rows(R0, R1, R2) transpose(f3x2(R0, R1, R2))
#define make_f2x3_col_elems f2x3
#define make_f2x4_cols(C0, C1) f2x4(C0, C1)
#define make_f2x4_rows(R0, R1, R2, R3) transpose(f4x2(R0, R1, R2, R3))
#define make_f2x4_col_elems f2x4

#define make_f3x2_cols(C0, C1, C2) f3x2(C0, C1, C2)
#define make_f3x2_rows(R0, R1) transpose(f2x3(R0, R1))
#define make_f3x2_col_elems f3x2
#define make_f3x3_cols(C0, C1, C2) f3x3(C0, C1,C2)
#define make_f3x3_rows(R0, R1, R2) transpose(f3x3(R0, R1, R2))
#define make_f3x3_col_elems f3x3
#define make_f3x3_row_elems(E00, E01, E02, E10, E11, E12, E20, E21, E22) \
    f3x3(E00, E10, E20, E01, E11, E21, E02, E12, E22)
#define f3x4_cols(C0, C1, C2) f3x4(C0, C1,C2)
#define f3x4_rows(R0, R1, R2, R3) transpose(f4x3(R0, R1, R2, R3))
#define f3x4_col_elems f3x4

#define make_f4x2_cols(C0, C1, C2, C3) f4x2(C0, C1, C2, C3)
#define make_f4x2_rows(R0, R1) transpose(f2x4(R0, R1))
#define make_f4x2_col_elems f4x2
#define make_f4x3_cols(C0, C1, C2, C3) f4x3(C0, C1, C2, C3)
#define make_f4x3_rows(R0, R1, R2) transpose(f3x4(R0, R1, R2))
#define make_f4x3_col_elems f4x3
#define make_f4x4_cols(C0, C1, C2, C3) f4x4(C0, C1, C2, C3)
#define make_f4x4_rows(R0, R1, R2, R3) transpose(f4x4(R0, R1, R2, R3))
#define make_f4x4_col_elems f4x4
#define make_f4x4_row_elems(E00, E01, E02, E03, E10, E11, E12, E13, E20, E21, E22, E23, E30, E31, E32, E33) \
f4x4(E00, E10, E20, E30, E01, E11, E21, E31, E02, E12, E22, E32, E03, E13, E23, E33)

f4x4 Identity() { return f4x4(1.0f); }

#define setElem(M, I, J, V) {M[I][J] = V;}
#define getElem(M, I, J) (M[I][J])

#define frac(VALUE)			fract(VALUE)
#define lerp mix
#define rsqrt(VALUE)		inversesqrt(VALUE)
float saturate(float VALUE) { return clamp(VALUE, 0.0f, 1.0f); }
vec2 saturate(vec2 VALUE) { return clamp(VALUE, 0.0f, 1.0f); }
vec3 saturate(vec3 VALUE) { return clamp(VALUE, 0.0f, 1.0f); }
vec4 saturate(vec4 VALUE) { return clamp(VALUE, 0.0f, 1.0f); }
// #define saturate(VALUE)		clamp(VALUE, 0.0f, 1.0f)
#define ddx(VALUE)			dFdx(VALUE)
#define ddy(VALUE)			dFdy(VALUE)
#define ddx_fine(VALUE)		dFdx(VALUE) // fallback
#define ddy_fine(VALUE)		dFdy(VALUE) // dFdxFine/dFdyFine available in opengl 4.5
#define rcp(VALUE)			(1.0f / (VALUE))
#define atan2(X, Y)			atan(X, Y)
#define reversebits(X)		bitfieldReverse(X)
#define asuint(X)			floatBitsToUint(X)
#define asfloat(X)			uintBitsToFloat(X)
#define mad(a, b, c)		(a) * (b) + (c)
#define VK_T_uint(T)   u ## T
#define VK_T_uint2(T)  u ## T
#define VK_T_uint4(T)  u ## T
#define VK_T_int(T)    i ## T
#define VK_T_int2(T)   i ## T
#define VK_T_int4(T)   i ## T
#define VK_T_float(T)       T
#define VK_T_float2(T)      T
#define VK_T_float4(T)      T
#define VK_T_half(T)       T
#define VK_T_half2(T)      T
#define VK_T_half4(T)      T

// TODO: get rid of these
#define VK_T_uint3(T)  u ## T
#define VK_T_int3(T)   i ## T
#define VK_T_float3(T)      T
#define VK_T_half3(T)      T

#define TexCube(T)           VK_T_##T(textureCube)
#define TexCubeArray(T)      VK_T_##T(textureCubeArray)

#define Tex1D(T)             VK_T_##T(texture1D)
#define Tex2D(T)             VK_T_##T(texture2D)
#define Tex3D(T)             VK_T_##T(texture3D)

#define Depth2D(T)              VK_T_##T(texture2D)
#define Depth2DMS(T, S)         VK_T_##T(texture2DMS)
#define Depth2DArray(T)         VK_T_##T(texture2DArray)
#define Depth2DArrayMS(T, S)    VK_T_##T(texture2DMSArray)



#define SamplerState sampler
#define SamplerComparisonState sampler
#define NO_SAMPLER 0u

#define getCol(M, I) M[I]
#define getCol0(M) getCol(M, 0)
#define getCol1(M) getCol(M, 1)
#define getCol2(M) getCol(M, 2)
#define getCol3(M) getCol(M, 3)

vec4 getRow(in(mat4)   M, uint i) { return vec4(M[0][i], M[1][i], M[2][i], M[3][i]); }
vec4 getRow(in(mat4x3) M, uint i) { return vec4(M[0][i], M[1][i], M[2][i], M[3][i]); }
vec4 getRow(in(mat4x2) M, uint i) { return vec4(M[0][i], M[1][i], M[2][i], M[3][i]); }

vec3 getRow(in(mat3x4) M, uint i) { return vec3(M[0][i], M[1][i], M[2][i]); }
vec3 getRow(in(mat3)   M, uint i) { return vec3(M[0][i], M[1][i], M[2][i]); }
vec3 getRow(in(mat3x2) M, uint i) { return vec3(M[0][i], M[1][i], M[2][i]); }

vec2 getRow(in(mat2x4) M, uint i) { return vec2(M[0][i], M[1][i]); }
vec2 getRow(in(mat2x3) M, uint i) { return vec2(M[0][i], M[1][i]); }
vec2 getRow(in(mat2)   M, uint i) { return vec2(M[0][i], M[1][i]); }

#define getRow0(M) getRow(M, 0)
#define getRow1(M) getRow(M, 1)
#define getRow2(M) getRow(M, 2)
#define getRow3(M) getRow(M, 3)

#define asfloat(X)			uintBitsToFloat(X)

#define STRUCT(NAME) struct NAME
#define DATA(TYPE, NAME, SEM) TYPE NAME

#define CBUFFER(NAME, FREQ, REG, BINDING) layout(std140, FREQ, BINDING) uniform NAME

#ifndef UPDATE_FREQ_NONE
    #define UPDATE_FREQ_NONE      set = 0
#endif
#ifndef UPDATE_FREQ_PER_FRAME
    #define UPDATE_FREQ_PER_FRAME set = 1
#endif
#ifndef UPDATE_FREQ_PER_BATCH
    #define UPDATE_FREQ_PER_BATCH set = 2
#endif
#ifndef UPDATE_FREQ_PER_DRAW
    #define UPDATE_FREQ_PER_DRAW  set = 3
#endif

#define _NONE
#define NUM_THREADS(X, Y, Z) layout (local_size_x = X, local_size_y = Y, local_size_z = Z) in(_NONE);

// case list
#define REPEAT_TEN(base) CASE(base) CASE(base+1) CASE(base+2) CASE(base+3) CASE(base+4) CASE(base+5) CASE(base+6) CASE(base+7) CASE(base+8) CASE(base+9)
#define REPEAT_HUNDRED(base)	REPEAT_TEN(base) REPEAT_TEN(base+10) REPEAT_TEN(base+20) REPEAT_TEN(base+30) REPEAT_TEN(base+40) REPEAT_TEN(base+50) \
                                REPEAT_TEN(base+60) REPEAT_TEN(base+70) REPEAT_TEN(base+80) REPEAT_TEN(base+90)

#define CASE_LIST_256 CASE(0)	REPEAT_HUNDRED(1) REPEAT_HUNDRED(101) \
                            REPEAT_TEN(201) REPEAT_TEN(211) REPEAT_TEN(221) REPEAT_TEN(231) REPEAT_TEN(241) \
                            CASE(251) CASE(252) CASE(253) CASE(254) CASE(255)

#define EARLY_FRAGMENT_TESTS layout(early_fragment_tests) in;

#define GroupMemoryBarrier() { \
    groupMemoryBarrier(); \
    barrier(); \
}

#define clip(COND) if( (COND) < 0 ) \
{ \
    discard;\
    return;\
}

STRUCT(VBConstants)
{
	DATA(uint, indexOffset, None);
	DATA(uint, pad_0, None);
	DATA(uint, pad_1, None);
	DATA(uint, pad_2, None);
};

STRUCT(Transform)
{
	DATA(mat4, vp, None);
	DATA(mat4, mvp, None);
	DATA(mat4, projection, None);
	DATA(mat4, invVP, None);
};

STRUCT(CullingViewPort)
{
	DATA(float2, windowSize, None);
	DATA(uint, sampleCount, None);
	DATA(uint, _pad0, None);
};

STRUCT(FilterDispatchGroupData)
{
	DATA(uint, meshIndex, None);
	DATA(uint, indexOffset, None);
    DATA(uint, instanceDataIndex, None);
	DATA(uint, geometrySet_faceCount, None);
};

STRUCT(MeshConstants)
{
	DATA(uint, indexOffset, None);
	DATA(uint, vertexOffset, None);
	DATA(uint, materialID, None);
	DATA(uint, twoSided, None);
};

STRUCT(VertexData)
{
    DATA(vec3, vertexPosition, None);
	DATA(vec3, vertexNormal, None);
	DATA(vec2, vertexTextureUV, None);
};

struct RayDesc
{
	float3 Origin;
	float3 Direction;
	float TMin;
	float TMax;
};