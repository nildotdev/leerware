#include "visuals.h"

// used: source sdk
#include "../sdk/interfaces/iengineclient.h"
#include "../sdk/entity.h"

// used: overlay
#include "visuals/overlay.h"
#include "visuals/chams.h"

#include "../core/sdk.h"

using namespace F;

bool F::VISUALS::Setup()
{
	if (!CHAMS::Initialize())
	{
		L_PRINT(LOG_ERROR) << CS_XOR("failed to initialize chams");
		return false;
	}

	return true;
}

void F::VISUALS::OnDestroy()
{
	CHAMS::Destroy();
}

void VISUALS::OnFrame(const int nStage)
{
	if (nStage == FRAME_RENDER_END)
	{
		// check is render initialized
		if (!D::bInitialized)
			return;

		/*
		 * game and our gui are based on immediate render mode principe
		 * this means that we should always reset draw data from previous frame and re-store it again
		 */
		D::ResetDrawData();

		if (CCSPlayerController* pLocal = CCSPlayerController::GetLocalPlayerController(); pLocal != nullptr)
		{
			OVERLAY::OnFrameStageNotify(pLocal);
		}

		D::SwapDrawData();
	}
}

bool F::VISUALS::OnDrawObject(void* pAnimatableSceneObjectDesc, void* pDx11, CMeshData* arrMeshDraw, int nDataCount, void* pSceneView, void* pSceneLayer, void* pUnk, void* pUnk2)
{
	return CHAMS::OnDrawObject(pAnimatableSceneObjectDesc, pDx11, arrMeshDraw, nDataCount, pSceneView, pSceneLayer, pUnk, pUnk2);
}

void F::VISUALS::DrawTaserParticles(C_CSPlayerPawn* targetPawn)
{
	using fOriginal = void* CS_FASTCALL(const char*, int, C_BaseEntity*, bool, int, bool, int, int, bool);
	auto oOriginal = reinterpret_cast<fOriginal*>(MEM::GetAbsoluteAddress(MEM::FindPattern(CLIENT_DLL, "E8 ? ? ? ? F3 0F 10 44 24 ? F3 0F 11 87 ? ? ? ? FF C3"), 0x1));
	oOriginal("particles/blood_impact/impact_taser_bodyfx.vpcf", 3, targetPawn, false, 0, false, -1, 0, false);
}
