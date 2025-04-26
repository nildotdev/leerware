#include "sdk.h"

// used: getmodulebasehandle
#include "../utilities/memory.h"

#include "../sdk/interfaces/igameresourceservice.h"
#include "../sdk/interfaces/cgameentitysystem.h"
#include "../sdk/interfaces/ischemasystem.h"
#include "../sdk/entity.h"

bool SDK::Setup()
{
	bool bSuccess = true;

	const void* hTier0Lib = MEM::GetModuleBaseHandle(TIER0_DLL);
	if (hTier0Lib == nullptr)
		return false;

	const void* hClientLib = MEM::GetModuleBaseHandle(CLIENT_DLL);
	if (hClientLib == nullptr)
		return false;

	fnConColorMsg = reinterpret_cast<decltype(fnConColorMsg)>(MEM::GetExportAddress(hTier0Lib, CS_XOR("?ConColorMsg@@YAXAEBVColor@@PEBDZZ")));
	bSuccess &= fnConColorMsg != nullptr;

	fnMsg = reinterpret_cast<decltype(fnMsg)>(MEM::GetExportAddress(hTier0Lib, CS_XOR("Msg")));
	bSuccess &= fnMsg != nullptr;

	const int nHighestIndex = I::GameResourceService->pGameEntitySystem->GetHighestEntityIndex();
	for (int nIndex = 1; nIndex <= nHighestIndex; nIndex++)
	{
		C_BaseEntity* pEntity = I::GameResourceService->pGameEntitySystem->Get(nIndex);
		if (pEntity == nullptr)
			continue;

		SchemaClassInfoData_t* pClassInfo = nullptr;
		pEntity->GetSchemaClassInfo(&pClassInfo);
		if (pClassInfo == nullptr)
			continue;

		const FNV1A_t uHashedName = FNV1A::Hash(pClassInfo->szName);
		if (uHashedName == FNV1A::HashConst("CCSPlayerController"))
			PlayerControllers.push_back(reinterpret_cast<CCSPlayerController*>(pEntity));
	}
	L_PRINT(LOG_NONE) << L::SetColor(LOG_COLOR_FORE_GREEN | LOG_COLOR_FORE_INTENSITY) << CS_XOR("controller cache updated");

	//InterpolationCLVT = (uint8_t*)hClientLib + 0x181319BA8;
	//InterpolationVT = (uint8_t*)hClientLib + 0x181319B08;

	return bSuccess;
}
