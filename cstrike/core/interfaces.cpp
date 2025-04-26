// used: [d3d] api
#include <d3d11.h>

#include "interfaces.h"

// used: findpattern, callvirtual, getvfunc...
#include "../utilities/memory.h"

// used: l_print
#include "../utilities/log.h"

// used: iswapchaindx11
#include "../sdk/interfaces/iswapchaindx11.h"
#include "../sdk/interfaces/iresourcesystem.h"

#pragma region interfaces_get

using InstantiateInterfaceFn_t = void* (*)();

class CInterfaceRegister
{
public:
	InstantiateInterfaceFn_t fnCreate;
	const char* szName;
	CInterfaceRegister* pNext;
};

static const CInterfaceRegister* GetRegisterList(const wchar_t* wszModuleName)
{
	void* hModule = MEM::GetModuleBaseHandle(wszModuleName);
	if (hModule == nullptr)
		return nullptr;

	std::uint8_t* pCreateInterface = reinterpret_cast<std::uint8_t*>(MEM::GetExportAddress(hModule, CS_XOR("CreateInterface")));

	if (pCreateInterface == nullptr)
	{
		L_PRINT(LOG_ERROR) << CS_XOR("failed to get \"CreateInterface\" address");
		return nullptr;
	}

	return *reinterpret_cast<CInterfaceRegister**>(MEM::ResolveRelativeAddress(pCreateInterface, 0x3, 0x7));
}

template <typename T = void*>
T* Capture(const CInterfaceRegister* pModuleRegister, const char* szInterfaceName)
{
	for (const CInterfaceRegister* pRegister = pModuleRegister; pRegister != nullptr; pRegister = pRegister->pNext)
	{
		if (const std::size_t nInterfaceNameLength = CRT::StringLength(szInterfaceName);
			// found needed interface
			CRT::StringCompareN(szInterfaceName, pRegister->szName, nInterfaceNameLength) == 0 &&
			// and we've given full name with hardcoded digits
			(CRT::StringLength(pRegister->szName) == nInterfaceNameLength ||
			// or it contains digits after name
			CRT::StringToInteger<int>(pRegister->szName + nInterfaceNameLength, nullptr, 10) > 0))
		{
			// capture our interface
			void* pInterface = pRegister->fnCreate();

#ifdef _DEBUG
			// log interface address
			L_PRINT(LOG_INFO) << CS_XOR("captured \"") << pRegister->szName << CS_XOR("\" interface at address: ") << L::AddFlags(LOG_MODE_INT_SHOWBASE | LOG_MODE_INT_FORMAT_HEX) << reinterpret_cast<std::uintptr_t>(pInterface);
#else
			L_PRINT(LOG_INFO) << CS_XOR("captured \"") << pRegister->szName << CS_XOR("\" interface");
#endif

			return static_cast<T*>(pInterface);
		}
	}

	L_PRINT(LOG_ERROR) << CS_XOR("failed to find interface \"") << szInterfaceName << CS_XOR("\"");
	return nullptr;
}

#pragma endregion

bool I::Setup()
{
	bool bSuccess = true;

#pragma region interface_game_exported
	const auto pTier0Handle = MEM::GetModuleBaseHandle(TIER0_DLL);
	if (pTier0Handle == nullptr)
		return false;

	MemAlloc = *reinterpret_cast<IMemAlloc**>(MEM::GetExportAddress(pTier0Handle, CS_XOR("g_pMemAlloc")));
	bSuccess &= (MemAlloc != nullptr);

	const auto pSchemaSystemRegisterList = GetRegisterList(SCHEMASYSTEM_DLL);
	if (pSchemaSystemRegisterList == nullptr)
		return false;

	SchemaSystem = Capture<ISchemaSystem>(pSchemaSystemRegisterList, SCHEMA_SYSTEM);
	bSuccess &= (SchemaSystem != nullptr);

	const auto pInputSystemRegisterList = GetRegisterList(INPUTSYSTEM_DLL);
	if (pInputSystemRegisterList == nullptr)
		return false;

	InputSystem = Capture<IInputSystem>(pInputSystemRegisterList, INPUT_SYSTEM_VERSION);
	bSuccess &= (InputSystem != nullptr);

	const auto pEngineRegisterList = GetRegisterList(ENGINE2_DLL);
	if (pEngineRegisterList == nullptr)
		return false;

	GameResourceService = Capture<IGameResourceService>(pEngineRegisterList, GAME_RESOURCE_SERVICE_CLIENT);
	bSuccess &= (GameResourceService != nullptr);

	Engine = Capture<IEngineClient>(pEngineRegisterList, SOURCE2_ENGINE_TO_CLIENT);
	bSuccess &= (Engine != nullptr);

	NetworkClientService = Capture<INetworkClientService>(pEngineRegisterList, NETWORK_CLIENT_SERVICE);
	bSuccess &= (NetworkClientService != nullptr);

	const auto pTier0RegisterList = GetRegisterList(TIER0_DLL);
	if (pTier0RegisterList == nullptr)
		return false;

	Cvar = Capture<IEngineCVar>(pTier0RegisterList, ENGINE_CVAR);
	bSuccess &= (Cvar != nullptr);

	const auto pClientRegister = GetRegisterList(CLIENT_DLL);
	if (pClientRegister == nullptr)
		return false;

	Client = Capture<ISource2Client>(pClientRegister, SOURCE2_CLIENT);
	bSuccess &= (Client != nullptr);

	const auto pMaterialSystem2Register = GetRegisterList(MATERIAL_SYSTEM2_DLL);
	if (pMaterialSystem2Register == nullptr)
		return false;

	MaterialSystem2 = Capture<IMaterialSystem2>(pMaterialSystem2Register, MATERIAL_SYSTEM2);
	bSuccess &= (MaterialSystem2 != nullptr);

	const auto pResourceSystemRegisterList = GetRegisterList(RESOURCESYSTEM_DLL);
	if (pResourceSystemRegisterList == nullptr)
		return false;

	ResourceSystem = Capture<IResourceSystem>(pResourceSystemRegisterList, RESOURCE_SYSTEM);
	bSuccess &= (ResourceSystem != nullptr);

	if (ResourceSystem != nullptr)
	{
		ResourceHandleUtils = reinterpret_cast<CResourceHandleUtils*>(ResourceSystem->QueryInterface(RESOURCE_HANDLE_UTILS));
		bSuccess &= (ResourceHandleUtils != nullptr);
	}

	Prediction = Capture<CPrediction>(pClientRegister, CL_PREDICTION);
	bSuccess &= (Prediction != nullptr);

#pragma endregion

	// @ida:  #STR: "r_gpu_mem_stats", "-threads", "CTSListBase: Misaligned list\n", "CTSQueue: Misaligned queue\n", "Display GPU memory usage.", "-r_max_device_threads"
	// https://media.discordapp.net/attachments/1055004763328106558/1315619131109937152/image.png?ex=6758114b&is=6756bfcb&hm=a568636a5292e2a04f94972f5781d8ad88f170a38ec2a1ce82135726dec23fac&=&format=webp&quality=lossless
	SwapChain = **reinterpret_cast<ISwapChainDx11***>(MEM::ResolveRelativeAddress(MEM::FindPattern(RENDERSYSTEM_DLL, CS_XOR("66 0F 7F 0D 83 C9 43 ? 48 8B F7 66 0F 7F 05 88 C9 43 ?")), 0x4, 0x8));
	bSuccess &= (SwapChain != nullptr);

	// grab's d3d11 interfaces for later use
	if (SwapChain != nullptr)
	{
		if (FAILED(SwapChain->pDXGISwapChain->GetDevice(__uuidof(ID3D11Device), (void**)&Device)))
		{
			L_PRINT(LOG_ERROR) << CS_XOR("failed to get device from swapchain");
			CS_ASSERT(false);
			return false;
		}
		else
			// we successfully got device, so we can get immediate context
			Device->GetImmediateContext(&DeviceContext);
	}
	bSuccess &= (Device != nullptr && DeviceContext != nullptr);
	
	// #STR: CSGOInput
	Input = *reinterpret_cast<CCSGOInput**>(MEM::ResolveRelativeAddress(MEM::FindPattern(CLIENT_DLL, CS_XOR("48 8B 0D ? ? ? ? 99 2B C2")), 0x3, 0x7));
	bSuccess &= (Input != nullptr);

	// @ida: #STR: "gpGlocals->rendertime() called while IsInSimulation() is true, "gpGlocals->curtime() called while IsInSimulation() is false
	// @ida: #STR: "C_SceneEntity::SetupClientOnlyScene:  C" then go up until you see it
	GlobalVars = *reinterpret_cast<IGlobalVars**>(MEM::GetAbsoluteAddress(MEM::FindPattern(CLIENT_DLL, CS_XOR("48 89 15 ?? ?? ?? ?? 48 89 42")), 0x3));
	bSuccess &= (GlobalVars != nullptr);

	// @ida: #STR: "CRenderingWorldSession::OnLoopActivate" go down just a bit
	PVS = reinterpret_cast<CPVS*>(MEM::ResolveRelativeAddress(MEM::FindPattern(ENGINE2_DLL, CS_XOR("48 8D 0D ? ? ? ? 33 ? FF 50")), 0x3, 0x7)); 
	bSuccess &= (PVS != nullptr);

	// @ida: #STR: "Physics/TraceShape (Client)"
	// @ida: #STR: "Weapon_Knife.Stab" then go up
	/*
	XREF Signature #1 @ 1804DA671: 4C 8B 25 ? ? ? ? 24
	XREF Signature #2 @ 18024B7CE: 4C 8B 35 ? ? ? ? 24
	XREF Signature #3 @ 1806965AF: 4C 8B 2D ? ? ? ? 24
	XREF Signature #4 @ 180A05B8B: 48 8B 35 ? ? ? ? 80 E1
	XREF Signature #5 @ 18050B2C9: 48 8B 0D ? ? ? ? 80 4D
	*/
	GameTraceManager = *reinterpret_cast<CGameTraceManager**>(MEM::GetAbsoluteAddress(MEM::FindPattern(CLIENT_DLL, CS_XOR("4C 8B 25 ? ? ? ? 24")), 0x3, 0x0));
	bSuccess &= (GameTraceManager != nullptr);

	return bSuccess;
}

void I::CreateRenderTarget()
{
	if (FAILED(SwapChain->pDXGISwapChain->GetDevice(__uuidof(ID3D11Device), (void**)&Device)))
	{
		L_PRINT(LOG_ERROR) << CS_XOR("failed to get device from swapchain");
		CS_ASSERT(false);
	}
	else
		// we successfully got device, so we can get immediate context
		Device->GetImmediateContext(&DeviceContext);

	// @note: i dont use this anywhere else so lambda is fine
	static const auto GetCorrectDXGIFormat = [](DXGI_FORMAT eCurrentFormat)
	{
		switch (eCurrentFormat)
		{
		case DXGI_FORMAT_R8G8B8A8_UNORM_SRGB:
			return DXGI_FORMAT_R8G8B8A8_UNORM;
		}

		return eCurrentFormat;
	};

	DXGI_SWAP_CHAIN_DESC sd;
	SwapChain->pDXGISwapChain->GetDesc(&sd);

	ID3D11Texture2D* pBackBuffer = nullptr;
	if (SUCCEEDED(SwapChain->pDXGISwapChain->GetBuffer(0, IID_PPV_ARGS(&pBackBuffer))))
	{
		if (pBackBuffer)
		{
			D3D11_RENDER_TARGET_VIEW_DESC desc{};
			desc.Format = static_cast<DXGI_FORMAT>(GetCorrectDXGIFormat(sd.BufferDesc.Format));
			desc.ViewDimension = D3D11_RTV_DIMENSION_TEXTURE2D;
			if (FAILED(Device->CreateRenderTargetView(pBackBuffer, &desc, &RenderTargetView)))
			{
				L_PRINT(LOG_WARNING) << CS_XOR("failed to create render target view with D3D11_RTV_DIMENSION_TEXTURE2D...");
				L_PRINT(LOG_INFO) << CS_XOR("retrying to create render target view with D3D11_RTV_DIMENSION_TEXTURE2DMS...");
				desc.ViewDimension = D3D11_RTV_DIMENSION_TEXTURE2DMS;
				if (FAILED(Device->CreateRenderTargetView(pBackBuffer, &desc, &RenderTargetView)))
				{
					L_PRINT(LOG_WARNING) << CS_XOR("failed to create render target view with D3D11_RTV_DIMENSION_TEXTURE2DMS...");
					L_PRINT(LOG_INFO) << CS_XOR("retrying...");
					if (FAILED(Device->CreateRenderTargetView(pBackBuffer, NULL, &RenderTargetView)))
					{
						L_PRINT(LOG_ERROR) << CS_XOR("failed to create render target view");
						CS_ASSERT(false);
					}
				}
			}
			pBackBuffer->Release();
			pBackBuffer = nullptr;
		}
	}
}

void I::DestroyRenderTarget()
{
	if (RenderTargetView != nullptr)
	{
		RenderTargetView->Release();
		RenderTargetView = nullptr;
	}
}
