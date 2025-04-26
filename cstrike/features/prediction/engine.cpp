#include "engine.h"

#include "../../sdk/entity.h"
#include "../../core/interfaces.h"
#include "../../sdk/interfaces/igameresourceservice.h"
#include "../../sdk/interfaces/inetworkclientservice.h"
#include "../../sdk/interfaces/cgameentitysystem.h"
#include "../../sdk/interfaces/ccsgoinput.h"
#include "../../features/misc/movement.h"

class C_CSWeaponBaseGun;
class IGameResourceService;
class CGameEntitySystem;

void CPredictionSystem::RunPrediction(CNetworkGameClient* pNetworkGameClient, int nPredictionReason)
{
	using func_t = void(__fastcall*)(CNetworkGameClient*, int);
	static func_t fn = (func_t)MEM::FindPattern(ENGINE2_DLL, CS_XOR("40 55 41 56 48 83 EC ? 80 B9 F8 00 00 00 ?"));
	fn(pNetworkGameClient, nPredictionReason);
}

void CPredictionSystem::Begin(CCSGOInput* pInput, CUserCmd* pUserCmd)
{
	if (!I::GlobalVars || !SDK::LocalPawn || !SDK::LocalController->IsPawnAlive())
		return;

	if (this->nLastSequence == I::GlobalVars->nCurrentTick)
		return;

	CPlayer_MovementServices* pMovementServices = SDK::LocalPawn->GetMovementServices();
	if (!pMovementServices)
		return;

	CNetworkGameClient* pNetworkGameClient = I::NetworkClientService->GetNetworkGameClient();
	if (!pNetworkGameClient)
		return;

	// construct prediction data
	this->predictionData = {};

	// save pre prediction flags
	predictionData.nFlags = SDK::LocalPawn->GetFlags();

	// store current global variables

	predictionData.flCurrentTime = I::GlobalVars->flCurrentTime;
	predictionData.flCurrentTime1 = I::GlobalVars->flCurrentTime1;

	predictionData.nTickCount = I::GlobalVars->nTickCount;

	predictionData.flFrameTime = I::GlobalVars->flFrameTime;
	predictionData.flFrameTime2 = I::GlobalVars->flFrameTime2;
	predictionData.flFrameTime3 = I::GlobalVars->flFrameTime3;

	// store current tickbase
	predictionData.nTickBase = SDK::LocalController->GetTickBase();

	// store prediction state
	predictionData.bInPrediction = I::Prediction->InPrediction;
	predictionData.bFirstPrediction = I::Prediction->bFirstPrediction;
	predictionData.bHasBeenPredicted = pUserCmd->bHasBeenPredicted;
	predictionData.bShouldPredict = pNetworkGameClient->bShouldPredict;

	// set prediction state
	pUserCmd->bHasBeenPredicted = false;
	pNetworkGameClient->bShouldPredict = true;
	I::Prediction->bFirstPrediction = false;
	I::Prediction->InPrediction = true;

	pMovementServices->SetPredictionCommand(); //(pUserCmd);
	SDK::LocalController->GetCurrentCommand() = pUserCmd;

	// update prediction
	if (pNetworkGameClient->bShouldPredict && pNetworkGameClient->nDeltaTick > 0 && pNetworkGameClient->nSomePredictionTick > 0)
		RunPrediction(pNetworkGameClient, client_command_tick);

	// restore tickbase
	SDK::LocalController->GetTickBase() = predictionData.nTickBase;
}

void CPredictionSystem::End(CCSGOInput* pInput, CUserCmd* pUserCmd)
{
	if (!I::GlobalVars || !SDK::LocalPawn || !SDK::LocalController->IsPawnAlive())
		return;

	// run movement features
	F::MISC::MOVEMENT::PostPrediction(pUserCmd, pUserCmd->csgoUserCmd.pBaseCmd, SDK::LocalController, SDK::LocalPawn);

	if (this->nLastSequence == I::GlobalVars->nCurrentTick)
		return;

	CPlayer_MovementServices* pMovementServices = SDK::LocalPawn->GetMovementServices();
	if (!pMovementServices)
		return;

	CNetworkGameClient* pNetworkGameClient = I::NetworkClientService->GetNetworkGameClient();
	if (!pNetworkGameClient)
		return;

	pMovementServices->ResetPredictionCommand();
	SDK::LocalController->GetCurrentCommand() = nullptr;

	// restore global variables

	I::GlobalVars->flCurrentTime = predictionData.flCurrentTime;
	I::GlobalVars->flCurrentTime1 = predictionData.flCurrentTime1;

	I::GlobalVars->nTickCount = predictionData.nTickCount;

	I::GlobalVars->flFrameTime = predictionData.flFrameTime;
	I::GlobalVars->flFrameTime2 = predictionData.flFrameTime2;
	I::GlobalVars->flFrameTime3 = predictionData.flFrameTime3;

	// restore our tickbase
	SDK::LocalController->GetTickBase() = predictionData.nTickBase;

	// restore prediction state
	I::Prediction->bFirstPrediction = predictionData.bFirstPrediction;
	I::Prediction->InPrediction = predictionData.bInPrediction;
	pUserCmd->bHasBeenPredicted = predictionData.bHasBeenPredicted;
	pNetworkGameClient->bShouldPredict = predictionData.bShouldPredict;

	// save for next call
	this->nLastSequence = I::GlobalVars->nCurrentTick;
}
