#include "hooks.h"

// used: variables
#include "variables.h"

// used: game's sdk
#include "../sdk/interfaces/iswapchaindx11.h"
#include "../sdk/interfaces/iviewrender.h"
#include "../sdk/interfaces/cgameentitysystem.h"
#include "../sdk/interfaces/ccsgoinput.h"
#include "../sdk/interfaces/iinputsystem.h"
#include "../sdk/interfaces/iengineclient.h"
#include "../sdk/interfaces/inetworkclientservice.h"
#include "../sdk/interfaces/iglobalvars.h"
#include "../sdk/interfaces/imaterialsystem.h"
#include "../sdk/interfaces/ipvs.h"

// used: viewsetup
#include "../sdk/datatypes/viewsetup.h"

// used: entity
#include "../sdk/entity.h"

// used: get virtual function, find pattern, ...
#include "../utilities/memory.h"
// used: inputsystem
#include "../utilities/inputsystem.h"
// used: draw
#include "../utilities/draw.h"

// used: features callbacks
#include "../features.h"
// used: CRC rebuild
#include "../features/CRC.h"

// used: game's interfaces
#include "interfaces.h"
#include "sdk.h"

// used: menu
#include "menu.h"
#include "../features/prediction/engine.h"
#include <deque>

bool H::Setup()
{
	if (MH_Initialize() != MH_OK)
	{
		L_PRINT(LOG_ERROR) << CS_XOR("failed to initialize minhook");

		return false;
	}
	L_PRINT(LOG_INFO) << CS_XOR("minhook initialization completed");

	if (!hkPresent.Create(MEM::GetVFunc(I::SwapChain->pDXGISwapChain, VTABLE::D3D::PRESENT), reinterpret_cast<void*>(&Present)))
		return false;
	L_PRINT(LOG_INFO) << CS_XOR("\"Present\" hook has been created");

	if (!hkResizeBuffers.Create(MEM::GetVFunc(I::SwapChain->pDXGISwapChain, VTABLE::D3D::RESIZEBUFFERS), reinterpret_cast<void*>(&ResizeBuffers)))
		return false;
	L_PRINT(LOG_INFO) << CS_XOR("\"ResizeBuffers\" hook has been created");

	// creat swap chain hook
	IDXGIDevice* pDXGIDevice = NULL;
	I::Device->QueryInterface(IID_PPV_ARGS(&pDXGIDevice));

	IDXGIAdapter* pDXGIAdapter = NULL;
	pDXGIDevice->GetAdapter(&pDXGIAdapter);

	IDXGIFactory* pIDXGIFactory = NULL;
	pDXGIAdapter->GetParent(IID_PPV_ARGS(&pIDXGIFactory));

	if (!hkCreateSwapChain.Create(MEM::GetVFunc(pIDXGIFactory, VTABLE::DXGI::CREATESWAPCHAIN), reinterpret_cast<void*>(&CreateSwapChain)))
		return false;
	L_PRINT(LOG_INFO) << CS_XOR("\"CreateSwapChain\" hook has been created");

	pDXGIDevice->Release();
	pDXGIDevice = nullptr;
	pDXGIAdapter->Release();
	pDXGIAdapter = nullptr;
	pIDXGIFactory->Release();
	pIDXGIFactory = nullptr;

	// @ida: class CViewRender->OnRenderStart call GetMatricesForView
	if (!hkGetMatrixForView.Create(MEM::FindPattern(CLIENT_DLL, CS_XOR("40 53 48 81 EC ? ? ? ? 49 8B C1")), reinterpret_cast<void*>(&GetMatrixForView)))
		return false;
	L_PRINT(LOG_INFO) << CS_XOR("\"GetMatrixForView\" hook has been created");

	// @ida: #STR: cl: CreateMove clamped invalid attack history index %d in frame history to -1. Was %d, frame history size %d.\n
	if (!hkCreateMove.Create(MEM::FindPattern(CLIENT_DLL, CS_XOR("85 D2 0F 85 ? ? ? ? 48 8B C4 44 88 40" /*"48 8B C4 4C 89 40 ? 48 89 48 ? 55 53 57"*/)), reinterpret_cast<void*>(&CreateMove)))
		return false;
	L_PRINT(LOG_INFO) << CS_XOR("\"CreateMove\" hook has been created");

	if (!hkMouseInputEnabled.Create(MEM::GetVFunc(I::Input, VTABLE::CLIENT::MOUSEINPUTENABLED), reinterpret_cast<void*>(&MouseInputEnabled)))
		return false;
	L_PRINT(LOG_INFO) << CS_XOR("\"MouseInputEnabled\" hook has been created");

	if (!hkFrameStageNotify.Create(MEM::GetVFunc(I::Client, VTABLE::CLIENT::FRAMESTAGENOTIFY), reinterpret_cast<void*>(&FrameStageNotify)))
		return false;
	L_PRINT(LOG_INFO) << CS_XOR("\"FrameStageNotify\" hook has been created");

	// in ida it will go in order as
	// @ida: #STR: ; "game_newmap"
	// @ida: #STR: ; "mapname"
	// @ida: #STR: ; "transition"
	// and the pattern is in the first one "game_newmap"
	if (!hkLevelInit.Create(MEM::FindPattern(CLIENT_DLL, CS_XOR("48 89 5C 24 ? 56 48 83 EC ? 48 8B 0D ? ? ? ? 48 8B F2")), reinterpret_cast<void*>(&LevelInit)))
		return false;
	L_PRINT(LOG_INFO) << CS_XOR("\"LevelInit\" hook has been created");

	// @ida: ClientModeShared -> #STR: "map_shutdown"
	if (!hkLevelShutdown.Create(MEM::FindPattern(CLIENT_DLL, CS_XOR("48 83 EC ? 48 8B 0D ? ? ? ? 48 8D 15 ? ? ? ? 45 33 C9 45 33 C0 48 8B 01 FF 50 ? 48 85 C0 74 ? 48 8B 0D ? ? ? ? 48 8B D0 4C 8B 01 41 FF 50 ? 48 83 C4")), reinterpret_cast<void*>(&LevelShutdown)))
		return false;
	L_PRINT(LOG_INFO) << CS_XOR("\"LevelShutdown\" hook has been created");

	// @note: seems to do nothing for now...
	// @ida: ClientModeCSNormal->OverrideView idx 15
	//v21 = flSomeWidthSize * 0.5;
	//v22 = *flSomeHeightSize * 0.5;
	//*(float*)(pSetup + 0x49C) = v21; // m_OrthoRight
	//*(float*)(pSetup + 0x494) = -v21; // m_OrthoLeft
	//*(float*)(pSetup + 0x498) = -v22; // m_OrthoTop
	//*(float*)(pSetup + 0x4A0) = v22; // m_OrthoBottom
	if (!hkOverrideView.Create(MEM::FindPattern(CLIENT_DLL, CS_XOR("48 89 5C 24 ? 48 89 6C 24 ? 48 89 74 24 ? 57 41 56 41 57 48 83 EC ? 48 8B FA E8")), reinterpret_cast<void*>(&OverrideView)))
		return false;
	L_PRINT(LOG_INFO) << CS_XOR("\"OverrideView\" hook has been created");

	if (!hkValidateInput.Create(MEM::FindPattern(CLIENT_DLL, CS_XOR("40 53 48 83 EC ? 48 8B D9 E8 ? ? ? ? C6 83 ? ? ? ? ? 33 C0 48 C7 83")), reinterpret_cast<void*>(&ValidateInput)))
		return false;
	L_PRINT(LOG_INFO) << CS_XOR("\"ValidateInput\" hook has been created");

	// Credit: https://www.unknowncheats.me/forum/4253223-post6185.html
	if (!hkDrawObject.Create(MEM::FindPattern(SCENESYSTEM_DLL, CS_XOR("48 8B C4 48 89 50 ? 53")), reinterpret_cast<void*>(&DrawObject)))
		return false;
	L_PRINT(LOG_INFO) << CS_XOR("\"DrawObject\" hook has been created");

	if (!hkIsRelativeMouseMode.Create(MEM::GetVFunc(I::InputSystem, VTABLE::INPUTSYSTEM::ISRELATIVEMOUSEMODE), reinterpret_cast<void*>(&IsRelativeMouseMode)))
		return false;
	L_PRINT(LOG_INFO) << CS_XOR("\"IsRelativeMouseMode\" hook has been created");

	if (!hkRenderBatchList.Create(MEM::FindPattern(SCENESYSTEM_DLL, CS_XOR("40 53 48 83 EC ? 83 79 ? ? 48 8B D9 0F 8C")), reinterpret_cast<void*>(&RenderBatchList)))
		return false;
	L_PRINT(LOG_INFO) << CS_XOR("\"RenderBatchList\" hook has been created");

	if (!hkDrawScopeOverlay.Create(MEM::GetAbsoluteAddress(MEM::FindPattern(CLIENT_DLL, CS_XOR("E8 ? ? ? ? 80 7C 24 ? ? 74 ? F3 0F 10 0D")), 0x1), reinterpret_cast<void*>(&DrawScopeOverlay)))
		return false;
	L_PRINT(LOG_INFO) << CS_XOR("\"DrawScopeOverlay\" hook has been created");

	if (!hkGetFOV.Create(MEM::GetAbsoluteAddress(MEM::FindPattern(CLIENT_DLL, CS_XOR("E8 ? ? ? ? F3 0F 11 45 ? 48 8B 5C 24")), 0x1), reinterpret_cast<void*>(&GetFOV)))
		return false;
	L_PRINT(LOG_INFO) << CS_XOR("\"GetFOV\" hook has been created");

	if (!hkOnAddEntity.Create(MEM::GetVFunc(I::GameResourceService->pGameEntitySystem, VTABLE::CLIENT::ONADDENTITY), reinterpret_cast<void*>(&OnAddEntity)))
		return false;
	L_PRINT(LOG_INFO) << CS_XOR("\"OnAddEntity\" hook has been created");

	if (!hkOnRemoveEntity.Create(MEM::GetVFunc(I::GameResourceService->pGameEntitySystem, VTABLE::CLIENT::ONREMOVEENTITY), reinterpret_cast<void*>(&OnRemoveEntity)))
		return false;
	L_PRINT(LOG_INFO) << CS_XOR("\"OnRemoveEntity\" hook has been created");

	return true;
}

void H::Destroy()
{
	MH_DisableHook(MH_ALL_HOOKS);
	MH_RemoveHook(MH_ALL_HOOKS);

	MH_Uninitialize();
}

HRESULT __stdcall H::Present(IDXGISwapChain* pSwapChain, UINT uSyncInterval, UINT uFlags)
{
	const auto oPresent = hkPresent.GetOriginal();

	// recreate it if it's not valid
	if (I::RenderTargetView == nullptr)
	{
		L_PRINT(LOG_INFO) << CS_XOR("Target view is nullptr. Recreating...");
		I::CreateRenderTarget();
	}

	// set our render target
	if (I::RenderTargetView != nullptr)
		I::DeviceContext->OMSetRenderTargets(1, &I::RenderTargetView, nullptr);

	F::OnPresent();

	return oPresent(I::SwapChain->pDXGISwapChain, uSyncInterval, uFlags);
}

HRESULT CS_FASTCALL H::ResizeBuffers(IDXGISwapChain* pSwapChain, std::uint32_t nBufferCount, std::uint32_t nWidth, std::uint32_t nHeight, DXGI_FORMAT newFormat, std::uint32_t nFlags)
{
	const auto oResizeBuffer = hkResizeBuffers.GetOriginal();

	auto hResult = oResizeBuffer(pSwapChain, nBufferCount, nWidth, nHeight, newFormat, nFlags);
	if (SUCCEEDED(hResult))
	{
		L_PRINT(LOG_INFO) << CS_XOR("Recreating target view after resizing");
		I::CreateRenderTarget();
	}

	return hResult;
}

HRESULT __stdcall H::CreateSwapChain(IDXGIFactory* pFactory, IUnknown* pDevice, DXGI_SWAP_CHAIN_DESC* pDesc, IDXGISwapChain** ppSwapChain)
{
	const auto oCreateSwapChain = hkCreateSwapChain.GetOriginal();

	I::DestroyRenderTarget();
	L_PRINT(LOG_INFO) << CS_XOR("render target view has been destroyed");

	return oCreateSwapChain(pFactory, pDevice, pDesc, ppSwapChain);
}

long H::WndProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	if (D::OnWndProc(hWnd, uMsg, wParam, lParam))
		return 1L;

	return ::CallWindowProcW(IPT::pOldWndProc, hWnd, uMsg, wParam, lParam);
}

ViewMatrix_t* CS_FASTCALL H::GetMatrixForView(CRenderGameSystem* pRenderGameSystem, IViewRender* pViewRender, ViewMatrix_t* pOutWorldToView, ViewMatrix_t* pOutViewToProjection, ViewMatrix_t* pOutWorldToProjection, ViewMatrix_t* pOutWorldToPixels)
{
	const auto oGetMatrixForView = hkGetMatrixForView.GetOriginal();
	ViewMatrix_t* matResult = oGetMatrixForView(pRenderGameSystem, pViewRender, pOutWorldToView, pOutViewToProjection, pOutWorldToProjection, pOutWorldToPixels);

	// get view matrix
	SDK::ViewMatrix = *pOutWorldToProjection;
	// get camera position
	// @note: ida @GetMatrixForView(global_pointer, pRenderGameSystem + 16, ...)
	SDK::CameraPosition = pViewRender->vecOrigin;

	return matResult;
}

void DebugInputHistory(CCSGOInputHistoryEntryPB* pInputEntry)
{
	if (pInputEntry == nullptr)
		return;
	CMsgQAngle* view = pInputEntry->pViewAngles;
	if (view)
		L_PRINT(LOG_INFO) << "pViewAngles: " << view->angValue;

	CMsgVector* shootpos = pInputEntry->pShootPosition;
	if (shootpos)
		L_PRINT(LOG_INFO) << "pShootPosition: " << shootpos->vecValue;

	CMsgVector* headpos = pInputEntry->pTargetHeadPositionCheck;
	if (headpos)
		L_PRINT(LOG_INFO) << "pTargetHeadPositionCheck: " << headpos->vecValue;

	CMsgVector* targetpos = pInputEntry->pTargetAbsPositionCheck;
	if (targetpos)
		L_PRINT(LOG_INFO) << "pTargetAbsPositionCheck: " << targetpos->vecValue;

	CMsgQAngle* targetang = pInputEntry->pTargetAngPositionCheck;
	if (targetang)
		L_PRINT(LOG_INFO) << "pTargetAngPositionCheck: " << targetang->angValue;

	CCSGOInterpolationInfoPB_CL* cl_interp = pInputEntry->cl_interp;
	if (cl_interp)
	{
		L_PRINT(LOG_INFO) << "cl_interp:";
		L_PRINT(LOG_INFO) << "flFraction = " << cl_interp->flFraction;
	}
	CCSGOInterpolationInfoPB* sv_interp0 = pInputEntry->sv_interp0;
	if (sv_interp0)
	{
		L_PRINT(LOG_INFO) << "sv_interp0:";
		L_PRINT(LOG_INFO) << "flFraction = " << sv_interp0->flFraction;
		L_PRINT(LOG_INFO) << "nSrcTick = " << sv_interp0->nSrcTick;
		L_PRINT(LOG_INFO) << "nDstTick = " << sv_interp0->nDstTick;
	}
	CCSGOInterpolationInfoPB* sv_interp1 = pInputEntry->sv_interp1;
	if (sv_interp1)
	{
		L_PRINT(LOG_INFO) << "sv_interp1:";
		L_PRINT(LOG_INFO) << "flFraction = " << sv_interp1->flFraction;
		L_PRINT(LOG_INFO) << "nSrcTick = " << sv_interp1->nSrcTick;
		L_PRINT(LOG_INFO) << "nDstTick = " << sv_interp1->nDstTick;
	}
	CCSGOInterpolationInfoPB* player_interp = pInputEntry->player_interp;
	if (player_interp)
	{
		L_PRINT(LOG_INFO) << "player_interp:";
		L_PRINT(LOG_INFO) << "flFraction = " << player_interp->flFraction;
		L_PRINT(LOG_INFO) << "nSrcTick = " << player_interp->nSrcTick;
		L_PRINT(LOG_INFO) << "nDstTick = " << player_interp->nDstTick;
	}

	double flTick = static_cast<double>(pInputEntry->nPlayerTickCount) + static_cast<double>(pInputEntry->flPlayerTickFraction);
	L_PRINT(LOG_INFO) << "flTick = " << flTick;

	L_PRINT(LOG_INFO) << "nTargetEntIndex = " << pInputEntry->nTargetEntIndex;
}



static void double_tap()
{
	if (!C_GET(bool, Vars.bRageEnable) || !C_GET(bool, Vars.bDoubletap))
		return;

	C_CSWeaponBaseGun* active_weapon = SDK::LocalPawn->GetCurrentWeapon();
	if (active_weapon == nullptr)
		return;
	CUserCmd* cmd = SDK::Cmd;
	static int shot_count{};
	float offset_tick{};
	float temp = active_weapon->GetNextPrimaryAttackTickRatio() + modff(active_weapon->GetWatOffset(), &offset_tick);
	int next_attack = active_weapon->GetNextPrimaryAttackTick();
	float shoot_tick = next_attack + offset_tick;
	if (temp >= 1)
		++shoot_tick;
	if (temp < 0)
		--shoot_tick;

	bool f = shot_count % 2;

	if (cmd->csgoUserCmd.nAttack1StartHistoryIndex > -1)
		shot_count++;

	for (int i = 0; i < cmd->csgoUserCmd.inputHistoryField.pRep->nAllocatedSize; i++)
	{
		auto input_history = cmd->GetInputHistoryEntry(i);
		if (input_history == nullptr)
			continue;

		input_history->nPlayerTickCount = f ? shoot_tick - 1 : 0;
		input_history->flPlayerTickFraction = 0.f;
	}
	cmd->csgoUserCmd.nAttack1StartHistoryIndex = -1;
}

std::deque<C_SmokeGrenadeProjectile*> smokeProjectiles{};

void HandleRemovals(C_CSPlayerPawn* pLocalPawn)
{
	for (C_SmokeGrenadeProjectile* smoke : smokeProjectiles)
	{
		if (C_GET(RemovalsDropdown_t, Vars.nRemovals) & ERemovalsDropdown::REMOVALS_SMOKE)
		{
			smoke->DidSmokeEffect() = true;
			smoke->GetSmokeDetonationPos() = Vector_t{};
			continue;
		}

		if (C_GET(bool, Vars.bWorldModulation))
		{
			Color_t smokeColor = C_GET(ColorPickerVar_t, Vars.colParticles).colValue;
			smoke->GetSmokeColor() = Vector_t(smokeColor.r, smokeColor.g, smokeColor.b);
		}
	}

	if (C_GET(unsigned int, Vars.nRemovals) & REMOVALS_FLASH)
		pLocalPawn->GetFlashMaxAlpha() = 0.0f;
	else
		pLocalPawn->GetFlashMaxAlpha() = 255.0f;
}

//bool CS_FASTCALL H::CreateMove(CCSGOInput* pInput, int nSlot, CUserCmd* pCmd)
void CS_FASTCALL H::CreateMove(CCSGOInput* pInput, int nSlot, bool bActive)
{
	const auto oCreateMove = hkCreateMove.GetOriginal();
	oCreateMove(pInput, nSlot, bActive);
	if (!I::Engine->IsConnected() || !I::Engine->IsInGame())
		return;

	CUserCmd* pCmd = pInput->GetUserCmd();
	SDK::Cmd = pCmd;
	I::Input = pInput;
	if (SDK::Cmd == nullptr)
		return;

	CBaseUserCmdPB* pBaseCmd = SDK::Cmd->csgoUserCmd.pBaseCmd;
	if (pBaseCmd == nullptr)
		return;

	SDK::LocalController = CCSPlayerController::GetLocalPlayerController();
	if (SDK::LocalController == nullptr)
		return;

	SDK::LocalPawn = I::GameResourceService->pGameEntitySystem->Get<C_CSPlayerPawn>(SDK::LocalController->GetPawnHandle());
	if (SDK::LocalPawn == nullptr)
		return;

	HandleRemovals(SDK::LocalPawn);
	g_PredictionSystem->Begin(pInput, pCmd);
	{
		F::OnCreateMove(SDK::Cmd, pBaseCmd, SDK::LocalController);
	}
	g_PredictionSystem->End(pInput, pCmd);

	// TODO : We need to fix CRC saving
	// 
	// There seems to be an issue within CBasePB and the classes that derive it.
	// So far, you may be unable to press specific keys such as crouch and automatic shooting.
	// A dodgy fix would be to comment it out but it still doesn't fix the bhop etc.

	//CRC::Save(pBaseCmd);
	//if (CRC::CalculateCRC(pBaseCmd) == true)
	//	CRC::Apply(SDK::Cmd);

	double_tap();
}

bool CS_FASTCALL H::MouseInputEnabled(void* pThisptr)
{
	const auto oMouseInputEnabled = hkMouseInputEnabled.GetOriginal();
	return MENU::bMainWindowOpened ? false : oMouseInputEnabled(pThisptr);
}

void CS_FASTCALL H::FrameStageNotify(void* rcx, int nFrameStage)
{
	const auto oFrameStageNotify = hkFrameStageNotify.GetOriginal();
	F::OnFrameStageNotify(nFrameStage);

	return oFrameStageNotify(rcx, nFrameStage);
}

__int64* CS_FASTCALL H::LevelInit(void* pClientModeShared, const char* szNewMap)
{
	const auto oLevelInit = hkLevelInit.GetOriginal();
	// if global variables are not captured during I::Setup or we join a new game, recapture it
	if (I::GlobalVars == nullptr)
		I::GlobalVars = *reinterpret_cast<IGlobalVars**>(MEM::GetAbsoluteAddress(MEM::FindPattern(CLIENT_DLL, CS_XOR("48 89 15 ?? ?? ?? ?? 48 89 42")), 0x3));
	
	// disable model occlusion
	I::PVS->Set(false);

	return oLevelInit(pClientModeShared, szNewMap);
}

__int64 CS_FASTCALL H::LevelShutdown()
{
	const auto oLevelShutdown = hkLevelShutdown.GetOriginal();
	// reset global variables since it got discarded by the game
	I::GlobalVars = nullptr;

	return oLevelShutdown();
}

void CS_FASTCALL H::DrawObject(void* pAnimatableSceneObjectDesc, void* pDx11, CMeshData* arrMeshDraw, int nDataCount, void* pSceneView, void* pSceneLayer, void* pUnk, void* pUnk2)
{
	const auto oDrawObject = hkDrawObject.GetOriginal();
	if (!I::Engine->IsConnected() || !I::Engine->IsInGame())
		return oDrawObject(pAnimatableSceneObjectDesc, pDx11, arrMeshDraw, nDataCount, pSceneView, pSceneLayer, pUnk, pUnk2);

	if (SDK::LocalController == nullptr || SDK::LocalPawn == nullptr)
		return oDrawObject(pAnimatableSceneObjectDesc, pDx11, arrMeshDraw, nDataCount, pSceneView, pSceneLayer, pUnk, pUnk2);

	if (!F::OnDrawObject(pAnimatableSceneObjectDesc, pDx11, arrMeshDraw, nDataCount, pSceneView, pSceneLayer, pUnk, pUnk2))
		oDrawObject(pAnimatableSceneObjectDesc, pDx11, arrMeshDraw, nDataCount, pSceneView, pSceneLayer, pUnk, pUnk2);
}

void* H::IsRelativeMouseMode(void* pThisptr, bool bActive)
{
	const auto oIsRelativeMouseMode = hkIsRelativeMouseMode.GetOriginal();

	MENU::bMainActive = bActive;

	if (MENU::bMainWindowOpened)
		return oIsRelativeMouseMode(pThisptr, false);

	return oIsRelativeMouseMode(pThisptr, bActive);
}

void H::RenderBatchList(void* a1)
{
	const auto oOriginal = hkRenderBatchList.GetOriginal();
	if (!C_GET(bool, Vars.bWorldModulation))
	{
		oOriginal(a1);
		return;
	}
	void** SceneObjectDesc = *reinterpret_cast<void***>(reinterpret_cast<uintptr_t>(a1) + 0x38);

	if (SceneObjectDesc)
	{
		const char* something = reinterpret_cast<const char*(__fastcall*)(void*)>(*reinterpret_cast<void***>(SceneObjectDesc)[0])(SceneObjectDesc);
		auto classname = std::string_view(something);
		auto v10 = *reinterpret_cast<void**>(reinterpret_cast<uintptr_t>(a1) + 0x18);

		class CAggregateSceneObjectData
		{
		public:
			MEM_PAD(0x38);
			Color_t m_rgba;
			MEM_PAD(0x8);
		};

		class CAggregateSceneObject /*: public CSceneObject*/
		{
		public:
			MEM_PAD(0x120);
			int count;
			MEM_PAD(0x4);
			CAggregateSceneObjectData* data;
		};

		class CMeshDrawPrimitive_t
		{
		public:
			MEM_PAD(0x18);
			CAggregateSceneObject* m_pObject;
			CMaterial2* m_pMaterial;
			MEM_PAD(0x28);
			Color_t m_rgba;
		};

		uint32_t count = *reinterpret_cast<uint32_t*>(reinterpret_cast<uintptr_t>(a1) + 0x30);

		if (v10)
		{
			for (uint32_t i = 0; i < count; ++i)
			{
				CMeshDrawPrimitive_t* meshDraw = (CMeshDrawPrimitive_t*)(reinterpret_cast<uintptr_t>(v10) + (0x68 * i));

				if (!meshDraw)
					continue;

				// Particles, e.g. molotov
				if (classname.find(CS_XOR("CParticleObjectDesc")) != std::string::npos)
				{
					meshDraw->m_rgba = C_GET(ColorPickerVar_t, Vars.colParticles).colValue;
				}

				// World modulation
				if (classname.find(CS_XOR("CAnimatableSceneObjectDesc")) != std::string::npos)
				{
					auto materialName = std::string_view(meshDraw->m_pMaterial->GetName());
					if (materialName.find("props") != std::string::npos)
					{
						meshDraw->m_rgba = C_GET(ColorPickerVar_t, Vars.colProps).colValue;
					}
				}

				if (classname.find(CS_XOR("CBaseSceneObjectDesc")) != std::string::npos)
				{
					meshDraw->m_rgba = C_GET(ColorPickerVar_t, Vars.colMisc).colValue;
				}

				if (classname.find(CS_XOR("CAggregateSceneObjectDesc")) != std::string::npos)
				{
					if (meshDraw->m_pObject)
					{
						for (int j = 0; j < meshDraw->m_pObject->count; ++j)
						{
							CAggregateSceneObjectData& data = meshDraw->m_pObject->data[j];
							data.m_rgba = C_GET(ColorPickerVar_t, Vars.colWorld).colValue;
						}
					}
				}
			}
		}
	}

	oOriginal(a1);
}

void CS_FASTCALL H::DrawScopeOverlay(void* rcx, void* a2)
{
	const auto oDrawScopeOverlay = hkDrawScopeOverlay.GetOriginal();
	if (C_GET(bool, Vars.bRemoveScope))
		return;

	oDrawScopeOverlay(rcx, a2);
}

float CS_FASTCALL H::GetFOV(void* pCameraServices)
{
	const auto oGetFOV = hkGetFOV.GetOriginal();
	if (pCameraServices == nullptr || !I::Engine->IsInGame() || !I::Engine->IsConnected())
		return oGetFOV(pCameraServices);
	if (SDK::LocalController == nullptr || !SDK::LocalController->IsPawnAlive())
		return oGetFOV(pCameraServices);
	if (C_GET(bool, Vars.bRemoveScope))
		return C_GET(int, Vars.nFOV);

	if (SDK::LocalPawn != nullptr && SDK::LocalPawn->IsScoped())
		return oGetFOV(pCameraServices);
	else
		return C_GET(int, Vars.nFOV);
}

void* CS_FASTCALL H::OnAddEntity(CGameEntitySystem* pThis, CEntityInstance* pInstance, CBaseHandle hHandle)
{
	const auto oOriginal = hkOnAddEntity.GetOriginal();
	if (pInstance == nullptr || !hHandle.IsValid())
		return oOriginal(pThis, pInstance, hHandle);

	SchemaClassInfoData_t* entityInfo = nullptr;
	pInstance->GetSchemaClassInfo(&entityInfo);
	if (entityInfo == nullptr)
		return oOriginal(pThis, pInstance, hHandle);

	const char* szName = entityInfo->szName;
	if (szName == nullptr)
		return oOriginal(pThis, pInstance, hHandle);

	const FNV1A_t uHashedName = FNV1A::Hash(szName);

	switch (uHashedName)
	{
	case FNV1A::HashConst("CCSPlayerController"):
	{
		CCSPlayerController* controller = I::GameResourceService->pGameEntitySystem->Get<CCSPlayerController>(hHandle);
		if (controller == nullptr)
			return oOriginal(pThis, pInstance, hHandle);

		SDK::PlayerControllers.push_back(controller);
	}
	break;
	case FNV1A::HashConst("C_SmokeGrenadeProjectile"):
	{
		C_SmokeGrenadeProjectile* smoke = I::GameResourceService->pGameEntitySystem->Get<C_SmokeGrenadeProjectile>(hHandle);
		if (smoke == nullptr)
			return oOriginal(pThis, pInstance, hHandle);

		smokeProjectiles.push_back(smoke);
	}
	break;
	}

	return oOriginal(pThis, pInstance, hHandle);
}

void CS_FASTCALL H::OnRemoveEntity(CGameEntitySystem* pThis, CEntityInstance* pInstance, CBaseHandle hHandle)
{
	const auto oOriginal = hkOnRemoveEntity.GetOriginal();
	if (pInstance == nullptr || !hHandle.IsValid())
		return oOriginal(pThis, pInstance, hHandle);

	SchemaClassInfoData_t* entityInfo = nullptr;
	pInstance->GetSchemaClassInfo(&entityInfo);
	if (entityInfo == nullptr)
		return oOriginal(pThis, pInstance, hHandle);

	const char* szName = entityInfo->szName;
	if (szName == nullptr)
		return oOriginal(pThis, pInstance, hHandle);

	const FNV1A_t uHashedName = FNV1A::Hash(szName);

	switch (uHashedName)
	{
	case FNV1A::HashConst("CCSPlayerController"):
	{
		CCSPlayerController* controller = I::GameResourceService->pGameEntitySystem->Get<CCSPlayerController>(hHandle);
		if (controller == nullptr)
			return oOriginal(pThis, pInstance, hHandle);

		auto& vec = SDK::PlayerControllers;
		auto it = std::find(vec.begin(), vec.end(), controller);
		if (it != vec.end())
			vec.erase(it);
	}
	break;

	case FNV1A::HashConst("C_SmokeGrenadeProjectile"):
	{
		C_SmokeGrenadeProjectile* smoke = I::GameResourceService->pGameEntitySystem->Get<C_SmokeGrenadeProjectile>(hHandle);
		if (smoke == nullptr)
			return oOriginal(pThis, pInstance, hHandle);

		auto& vec = smokeProjectiles;
		auto it = std::find(vec.begin(), vec.end(), smoke);
		if (it != vec.end())
			vec.erase(it);
	}
	break;
	}

	oOriginal(pThis, pInstance, hHandle);
}
