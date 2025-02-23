#include "ccsgoinput.h"
#include "../entity.h"

CUserCmd* CCSGOInput::GetUserCmd()
{
	// Get the local player controller
	auto controller = CCSPlayerController::GetLocalPlayerController();
	if (!controller)
		return nullptr;

	// Define and resolve the GetCommandIndex function
	using GetCommandIndexFn = void(__fastcall*)(void* controller, int* index);
	static GetCommandIndexFn GetCommandIndex = reinterpret_cast<GetCommandIndexFn>(
	MEM::GetAbsoluteAddress(
	MEM::FindPattern(CLIENT_DLL, CS_XOR("E8 ? ? ? ? 8B 8D ? ? ? ? 8D 51")),
	0x1));
	if (!GetCommandIndex)
		return nullptr;

	// Get the command index
	int index = 0;
	GetCommandIndex(controller, &index);
	int commandIndex = (index == 0 || index == -1) ? 0xFFFFFFFF : index - 1;

	// Define and resolve the GetUserCmdBase function
	using GetUserCmdBaseFn = void*(__fastcall*)(void* base, int index);
	static GetUserCmdBaseFn GetUserCmdBase = reinterpret_cast<GetUserCmdBaseFn>(
	MEM::GetAbsoluteAddress(
	MEM::FindPattern(CLIENT_DLL, CS_XOR("E8 ? ? ? ? 48 8B CF 4C 8B E8 44 8B B8")),
	0x1));
	if (!GetUserCmdBase)
		return nullptr;

	// Resolve the global offset
	static void* userCmdBaseOffset = *reinterpret_cast<void**>(
	MEM::ResolveRelativeAddress(
	reinterpret_cast<uint8_t*>(
	MEM::FindPattern(CLIENT_DLL, CS_XOR("48 8B 0D ? ? ? ? E8 ? ? ? ? 48 8B CF 4C 8B E8"))),
	0x3, 0x7));
	if (!userCmdBaseOffset)
		return nullptr;

	// Retrieve the user command base
	void* userCmdBase = GetUserCmdBase(userCmdBaseOffset, commandIndex);
	if (!userCmdBase)
		return nullptr;

	// Get the sequence number
	unsigned int sequenceNumber = *reinterpret_cast<unsigned int*>(reinterpret_cast<uintptr_t>(userCmdBase) + 0x5C00);

	// Define and resolve the GetUserCmd function
	using GetUserCmdFn = CUserCmd*(__fastcall*)(void* controller, unsigned int sequenceNumber);
	static GetUserCmdFn GetUserCmd = reinterpret_cast<GetUserCmdFn>(
	MEM::GetAbsoluteAddress(
	MEM::FindPattern(CLIENT_DLL, CS_XOR("E8 ? ? ? ? 48 8B 0D ? ? ? ? 45 33 E4 48 89 44 24")),
	0x1));
	if (!GetUserCmd)
		return nullptr;

	// Retrieve the user command
	CUserCmd* userCmd = GetUserCmd(controller, sequenceNumber);
	return userCmd;
}
