#pragma once
// used: stronghandle
#include "../../sdk/datatypes/stronghandle.h"

static constexpr char szVMatBufferWhiteVisible[] =
R"(<!-- kv3 encoding:text:version{e21c7f3c-8a33-41c5-9977-a76d3a32aa0d} format:generic:version{7412167c-06e9-4698-aff2-e63eb59037e7} -->
{
	shader = "csgo_unlitgeneric.vfx"

	F_PAINT_VERTEX_COLORS = 1
	F_TRANSLUCENT = 1
	F_BLEND_MODE = 1

	g_vColorTint = [1, 1, 1, 1]

	TextureAmbientOcclusion = resource:"materials/default/default_mask_tga_fde710a5.vtex"
	g_tAmbientOcclusion = resource:"materials/default/default_mask_tga_fde710a5.vtex"
	g_tColor = resource:"materials/default/default_mask_tga_fde710a5.vtex"
	g_tNormal = resource:"materials/default/default_mask_tga_fde710a5.vtex"
	g_tTintMask = resource:"materials/default/default_mask_tga_fde710a5.vtex"
})";

static constexpr char szVMatBufferWhiteInvisible[] =
R"(<!-- kv3 encoding:text:version{e21c7f3c-8a33-41c5-9977-a76d3a32aa0d} format:generic:version{7412167c-06e9-4698-aff2-e63eb59037e7} -->
{
	shader = "csgo_unlitgeneric.vfx"

	F_PAINT_VERTEX_COLORS = 1
	F_TRANSLUCENT = 1
	F_BLEND_MODE = 1
	F_DISABLE_Z_BUFFERING = 1

	g_vColorTint = [1, 1, 1, 1]

	TextureAmbientOcclusion = resource:"materials/default/default_mask_tga_fde710a5.vtex"
	g_tAmbientOcclusion = resource:"materials/default/default_mask_tga_fde710a5.vtex"
	g_tColor = resource:"materials/default/default_mask_tga_fde710a5.vtex"
	g_tNormal = resource:"materials/default/default_mask_tga_fde710a5.vtex"
	g_tTintMask = resource:"materials/default/default_mask_tga_fde710a5.vtex"
})";

static constexpr char szVMatBufferIlluminateVisible[] =
R"(<!-- kv3 encoding:text:version{e21c7f3c-8a33-41c5-9977-a76d3a32aa0d} format:generic:version{7412167c-06e9-4698-aff2-e63eb59037e7} -->
{
    "shader" = "csgo_effects.vfx"
 
     "g_tColor" = resource:"materials/dev/primary_white_color_tga_21186c76.vtex"
 
	 "g_tMask1" = resource:"materials/default/default_mask_tga_344101f8.vtex"
	 "g_tMask2" = resource:"materials/default/default_mask_tga_344101f8.vtex"
	 "g_tMask3" = resource:"materials/default/default_mask_tga_344101f8.vtex"
	 
	 "g_flColorBoost" = 20 
	 "g_flOpacityScale" = 0.6999999 
	 "g_flFresnelExponent" = 10 
	 "g_flFresnelFalloff" = 10 
	 "g_flFresnelMax" = 0
	 "g_flFresnelMin" = 1
	 
	 "F_BLEND_MODE" = 1
	 "F_TRANSLUCENT" = 1
	 "F_IGNOREZ" = 0
 
	 "F_DISABLE_Z_BUFFERING" = 0
	 
	 "F_RENDER_BACKFACES" = 0
	 
	 "g_vColorTint" = [1.00000, 1.00000, 1.00000]
})";

static constexpr char szVMatBufferIlluminateInvisible[] =
R"(<!-- kv3 encoding:text:version{e21c7f3c-8a33-41c5-9977-a76d3a32aa0d} format:generic:version{7412167c-06e9-4698-aff2-e63eb59037e7} -->
{
    "shader" = "csgo_effects.vfx"
 
     "g_tColor" = resource:"materials/dev/primary_white_color_tga_21186c76.vtex"
 
	 "g_tMask1" = resource:"materials/default/default_mask_tga_344101f8.vtex"
	 "g_tMask2" = resource:"materials/default/default_mask_tga_344101f8.vtex"
	 "g_tMask3" = resource:"materials/default/default_mask_tga_344101f8.vtex"
	 
	 "g_flColorBoost" = 20 
	 "g_flOpacityScale" = 0.6999999 
	 "g_flFresnelExponent" = 10 
	 "g_flFresnelFalloff" = 10 
	 "g_flFresnelMax" = 0
	 "g_flFresnelMin" = 1
	 
	 "F_BLEND_MODE" = 1
	 "F_TRANSLUCENT" = 1
	 "F_IGNOREZ" = 1
 
	 "F_DISABLE_Z_BUFFERING" = 1
	 
	 "F_RENDER_BACKFACES" = 0
	 
	 "g_vColorTint" = [1.00000, 1.00000, 1.00000]
})";

static constexpr char szVMatBufferBubbleVisible[] =
R"(<!-- kv3 encoding:text:version{e21c7f3c-8a33-41c5-9977-a76d3a32aa0d} format:generic:version{7412167c-06e9-4698-aff2-e63eb59037e7} -->
{
    shader = "csgo_glass.vfx"
    g_bFogEnabled = 1
    
    GlassMaskColor = resource:"materials/glass/glass01_vmat_g_tglassdust_5a468a25.vtex"
    GlassMaskTransmission = resource:"materials/glass/glass01_vmat_g_tglasstintcolor_ef8a35db.vtex"
    TextureNormal = resource:"materials/glass/glass01_vmat_g_tnormal_151129e7.vtex"
    g_tGlassDust = resource:"materials/glass/glass01_vmat_g_tglassdust_5a468a25.vtex"
    g_tGlassTintColor = resource:"materials/glass/glass01_vmat_g_tglasstintcolor_ef8a35db.vtex"
    g_tNormal = resource:"materials/glass/glass01_vmat_g_tnormal_151129e7.vtex"
})";

class CMaterial2;
class CMeshData;

namespace F::VISUALS::CHAMS
{
	bool Initialize();
	void Destroy();

	bool OnDrawObject(void* pAnimatableSceneObjectDesc, void* pDx11, CMeshData* arrMeshDraw, int nDataCount, void* pSceneView, void* pSceneLayer, void* pUnk, void* pUnk2);

	// @note: bDisableZBuffering == true to create invisible material
	CStrongHandle<CMaterial2> CreateMaterial(const char* szMaterialName, const char szVmatBuffer[]);

	bool OverrideMaterial(void* pAnimatableSceneObjectDesc, void* pDx11, CMeshData* arrMeshDraw, int nDataCount, void* pSceneView, void* pSceneLayer, void* pUnk, void* pUnk2);

	inline bool bInitialized = false;
}
