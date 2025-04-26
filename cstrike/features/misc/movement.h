#pragma once

class CUserCmd;
class CBaseUserCmdPB;
class CCSGOInputHistoryEntryPB;

class CCSPlayerController;
class C_CSPlayerPawn;

struct QAngle_t;

namespace F::MISC::MOVEMENT
{
	void OnMove(CUserCmd* pCmd, CBaseUserCmdPB* pBaseCmd, CCSPlayerController* pLocalController, C_CSPlayerPawn* pLocalPawn);
	void PostPrediction(CUserCmd* pCmd, CBaseUserCmdPB* pBaseCmd, CCSPlayerController* pLocalController, C_CSPlayerPawn* pLocalPawn);

	void HalfDuck(CUserCmd* pCmd, C_CSPlayerPawn* pLocalPawn);

	void BunnyHop(CUserCmd* pCmd, C_CSPlayerPawn* pLocalPawn);
	void JumpBug(CUserCmd* pCmd, C_CSPlayerPawn* pLocalPawn);
	void EdgeJump(CUserCmd* pCmd, C_CSPlayerPawn* pLocalPawn);
	void AutoStrafe(CUserCmd* pCmd, CBaseUserCmdPB* pUserCmd, C_CSPlayerPawn* pLocalPawn);
	void LimitSpeed(CBaseUserCmdPB* pUserCmd, C_CSPlayerPawn* pLocalPawn, const float speed);
	void QuickStop(CUserCmd* pCmd, CBaseUserCmdPB* pBaseCmd, C_CSPlayerPawn* pLocal, bool shouldForceStop);
	void SlowWalk(CUserCmd* pCmd, CBaseUserCmdPB* pUserCmd, C_CSPlayerPawn* pLocalPawn);
	void AdjustAnglesForQuantizedMovement(CUserCmd* pCmd, CBaseUserCmdPB* pBaseCmd, C_CSPlayerPawn* pLocalPawn, bool bUseRoll = false);
	void MovementCorrection(CBaseUserCmdPB* pUserCmd, CCSGOInputHistoryEntryPB* pInputHistory, const QAngle_t& angDesiredViewPoint);

	// will call MovementCorrection && validate user's angView to avoid untrusted ban
	void ValidateUserCommand(CUserCmd* pCmd, CBaseUserCmdPB* pUserCmd, CCSGOInputHistoryEntryPB* pInputEntry, QAngle_t angCorrectionView);
}
