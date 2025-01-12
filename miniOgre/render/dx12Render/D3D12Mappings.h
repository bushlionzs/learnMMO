/*
-----------------------------------------------------------------------------
This source file is part of OGRE
(Object-oriented Graphics Rendering Engine)
For the latest info, see http://www.ogre3d.org/

Copyright (c) 2000-2014 Torus Knot Software Ltd

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in
all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
THE SOFTWARE.
-----------------------------------------------------------------------------
*/
#ifndef __D3D12MAPPINGS_H__
#define __D3D12MAPPINGS_H__

#include "d3dx12.h"
#include "RenderSystem.h"
#include "OgreBlendMode.h"
#include "OgreCommon.h"
#include "OgreHardwareVertexBuffer.h"

namespace Ogre 
{
    class  D3D12Mappings
	{
	public:

		/// return a D3D11 equivalent for a Ogre SceneBlendFactor value
		static D3D12_BLEND get(SceneBlendFactor sbf, bool forAlpha);
		/// return a D3D11 equivalent for a Ogre SceneBlendOperation value
		static D3D12_BLEND_OP get(SceneBlendOperation sbo);
		/// return a D3D11 equivalent for a Ogre CompareFunction value
		static D3D12_COMPARISON_FUNC get(CompareFunction cf);
		

		static LPCSTR getSemanticName(VertexElementSemantic sem);

		static PixelFormat _getClosestSupportedPF(PixelFormat ogrePF);

		/// utility method, convert Ogre pixel format to D3D11 pixel format
		static DXGI_FORMAT _getPF(PixelFormat ogrePF);
		/// utility method, optionally maps plain format to _SRGB counterparts
		static DXGI_FORMAT _getGammaFormat(DXGI_FORMAT format, bool appendSRGB);

		static DXGI_FORMAT dx12GetType(VertexElementType vType);

		static D3D12_CULL_MODE getCullMode(backend::CullingMode mode);
		static D3D12_BLEND getBlendFactor(BlendFunction mode);

		static D3D12_BLEND_OP getBlendOp(BlendEquation equation);

		static D3D12_COMPARISON_FUNC getComparisonFunc(RasterState::DepthFunc depthFunc);

		static D3D12_DESCRIPTOR_RANGE_TYPE getRangeType(D3D_SHADER_INPUT_TYPE inputType);

		static D3D12_SHADER_VISIBILITY getShaderVisibility(uint8_t stages);

		static Ogre::PixelFormat getPixelFormat(DXGI_FORMAT format);

		static D3D12_RESOURCE_STATES util_to_dx12_resource_state(uint32_t state);
		static DXGI_FORMAT util_to_dx12_srv_format(DXGI_FORMAT defaultFormat);
		static D3D12_TEXTURE_ADDRESS_MODE getWrapMode(filament::backend::SamplerWrapMode mode);
		static D3D12_FILTER getFilter(const filament::backend::SamplerParams& params);

		static float getMaxLod(filament::backend::SamplerMipMapMode mipMapMode);
		static D3D12_COMPARISON_FUNC getCompareOp(filament::backend::SamplerCompareFunc func);

		static D3D12_RAYTRACING_ACCELERATION_STRUCTURE_BUILD_FLAGS 
			util_to_dx_acceleration_structure_build_flags(AccelerationStructureBuildFlags flags);

		static D3D12_RAYTRACING_ACCELERATION_STRUCTURE_TYPE ToDXRASType(AccelerationStructureType type);

		static D3D12_RAYTRACING_GEOMETRY_FLAGS util_to_dx_geometry_flags(AccelerationStructureGeometryFlags flags);

		static D3D12_RAYTRACING_INSTANCE_FLAGS util_to_dx_instance_flags(AccelerationStructureInstanceFlags flags);

		static D3D12_RESOURCE_STATES util_to_dx12_resource_state(ResourceState state);
	};
}
#endif
