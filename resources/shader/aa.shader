shader basic
{
     shader_unit
	 {
		shader_type directx
		vertex_shader basic.hlsl VS
		frag_shader basic.hlsl PS
		//vertex_shader basic.vert.glsl main
		//frag_shader basic.frag.glsl main
	 }
	 
	 shader_unit
	 {
		shader_type vulkan
		//vertex_shader basic.vert.glsl main
		//frag_shader basic.frag.glsl main
		vertex_shader basic.hlsl VS
		frag_shader basic.hlsl PS
	 }
}

shader basic2
{
     shader_unit
	 {
		shader_type directx
		vertex_shader basic2.hlsl VS
		frag_shader basic2.hlsl PS
	 }
	 
	 shader_unit
	 {
		shader_type vulkan
		vertex_shader basic2.hlsl VS
		frag_shader basic2.hlsl PS
	 }
}

shader mygui
{
    shader_unit
	 {
		shader_type directx
		vertex_shader mygui.hlsl VS
		frag_shader mygui.hlsl PS
	 }
	 
	 shader_unit
	 {
		shader_type vulkan
		vertex_shader mygui.vert.glsl main
		frag_shader mygui.frag.glsl main
	 }
}

shader cegui
{
    shader_unit
	 {
		shader_type directx
		vertex_shader cegui.hlsl VS
		frag_shader cegui.hlsl PS
	 }
	 
	 shader_unit
	 {
		shader_type vulkan
		vertex_shader cegui.glsl main
		frag_shader cegui.glsl main
	 }
}


shader ogreparticle
{
     shader_unit
	 {
		shader_type directx
		vertex_shader ogreparticle.hlsl VS
		frag_shader ogreparticle.hlsl PS
	 }
	 
	 shader_unit
	 {
		shader_type vulkan
		vertex_shader ogreparticle.vert.glsl main
		frag_shader ogreparticle.frag.glsl main
	 }
}

shader ogresky
{
     shader_unit
	 {
		shader_type directx
		vertex_shader sky.hlsl VS
		frag_shader sky.hlsl PS
	 }
	 
	 shader_unit
	 {
		shader_type vulkan
		vertex_shader sky.vert.glsl main
		frag_shader sky.frag.glsl main
	 }
}

shader prefilterenvmap
{
     shader_unit
	 {
		shader_type directx
		vertex_shader sky.hlsl VS
		frag_shader sky.hlsl PS
	 }
	 
	 shader_unit
	 {
		shader_type vulkan
		vertex_shader filtercube.vert main
		frag_shader prefilterenvmap.frag main
	 }
}

shader ogresimple
{
     shader_unit
	 {
		shader_type directx
		vertex_shader ogresimple.hlsl VS
		frag_shader ogresimple.hlsl PS
	 }
	 
	 shader_unit
	 {
		shader_type vulkan
		vertex_shader ogresimple.glsl main
		frag_shader ogresimple.glsl main
	 }
}

shader addblend
{
     shader_unit
	 {
		shader_type directx
		vertex_shader addblend.hlsl VS
		frag_shader addblend.hlsl PS
	 }
	 
	 shader_unit
	 {
		shader_type vulkan
		vertex_shader addblend.vert.glsl main
		frag_shader addblend.frag.glsl main
	 }
}

shader ogreblend
{
     shader_unit
	 {
		shader_type directx
		vertex_shader blend.hlsl VS
		frag_shader blend.hlsl PS
	 }
	 
	 shader_unit
	 {
		shader_type vulkan
		vertex_shader blend.vert.glsl main
		frag_shader blend.frag.glsl main
	 }
}


shader add
{
     shader_unit
	 {
		shader_type directx
		vertex_shader add.hlsl VS
		frag_shader add.hlsl PS
	 }
	 
	 shader_unit
	 {
		shader_type vulkan
		vertex_shader blend.vert.glsl main
		frag_shader blend.frag.glsl main
	 }
}

shader ogreterrain_onelayer
{
     shader_unit
	 {
		shader_type directx
		vertex_shader ogreterrain.hlsl onelayer_vs
		frag_shader ogreterrain.hlsl onelayer_ps
	 }
	 
	 shader_unit
	 {
		shader_type vulkan
		vertex_shader onelayer.vert.glsl main
		frag_shader onelayer.frag.glsl main
	 }
}

shader ogreterrain_onelayer_lightmap
{
     shader_unit
	 {
		shader_type directx
		vertex_shader ogreterrain.hlsl onelayer_lightmap_vs
		frag_shader ogreterrain.hlsl onelayer_lightmap_ps
	 }
	 
	 shader_unit
	 {
		shader_type vulkan
		vertex_shader onelayer_lightmap.vert.glsl main
		frag_shader onelayer_lightmap.frag.glsl main
	 }
}

shader ogreterrain_twolayer
{
     shader_unit
	 {
		shader_type directx
		vertex_shader ogreterrain.hlsl twolayer_vs
		frag_shader ogreterrain.hlsl twolayer_vs
	 }
	 
	 shader_unit
	 {
		shader_type vulkan
		vertex_shader twolayer.vert.glsl main
		frag_shader twolayer.frag.glsl main
	 }
}

shader ogreterrain_twolayer_lightmap
{
     shader_unit
	 {
		shader_type directx
		vertex_shader ogreterrain.hlsl twolayer_lightmap_vs
		frag_shader ogreterrain.hlsl twolayer_lightmap_ps
	 }
	 
	 shader_unit
	 {
		shader_type vulkan
		vertex_shader twolayer_lightmap.vert.glsl main
		frag_shader twolayer_lightmap.frag.glsl main
	 }
}

shader water
{
     shader_unit
	 {
		shader_type directx
		vertex_shader water.hlsl vs_main
		frag_shader water.hlsl ps_main
	 }
	 
	 shader_unit
	 {
		shader_type vulkan
		vertex_shader water.glsl vs
		frag_shader water.glsl ps
	 }
}

shader mipmap
{
    shader_unit
	 {
		shader_type directx
		vertex_shader mipmap.hlsl VS
		frag_shader mipmap.hlsl PS
	 }
	 
	 shader_unit
	 {
		shader_type vulkan
		vertex_shader mipmap.glsl VS
		frag_shader mipmap.glsl PS
	 }
}

shader fbx
{
    shader_unit
	 {
		shader_type directx
		vertex_shader fbx.hlsl VS
		frag_shader fbx.hlsl PS
	 }
	 
	 shader_unit
	 {
		shader_type vulkan
		vertex_shader fbx.hlsl VS
		frag_shader fbx.hlsl PS
	 }
}

shader shadow
{
    shader_unit
	 {
		shader_type directx
		vertex_shader shadow.hlsl vs
		frag_shader shadow.hlsl ps
	 }
	 
	 shader_unit
	 {
		shader_type vulkan
		vertex_shader shadow.glsl vs
		frag_shader shadow.glsl ps
	 }
}


shader shadows
{
    shader_unit
	 {
		shader_type directx
		vertex_shader shadows.hlsl vs
		frag_shader shadows.hlsl ps
	 }
	 
	 shader_unit
	 {
		shader_type vulkan
		vertex_shader shadow.glsl vs
		frag_shader shadow.glsl ps
	 }
}

shader wowterrain
{
    shader_unit
	 {
		shader_type directx
		vertex_shader wowterrain.hlsl vs
		frag_shader wowterrain.hlsl ps
	 }
	 
	 shader_unit
	 {
		shader_type vulkan
		vertex_shader wowterrain.glsl vs
		frag_shader wowterrain.glsl ps
	 }
}


shader projector
{
     shader_unit
	 {
		shader_type directx
		vertex_shader projector.hlsl VS
		frag_shader projector.hlsl PS
	 }
	 
	 shader_unit
	 {
		shader_type vulkan
		vertex_shader projector.glsl main
		frag_shader projector.glsl main
	 }
}

