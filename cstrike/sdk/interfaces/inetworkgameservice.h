#pragma once
#include <cstdint>
#include "../datatypes/cnetworkserializepb.h"
#include "../../utilities/memory.h"
#include "inetworkclientservice.h"

class INetworkClientService;

class INetworkGameService
{
	enum
	{
		unk = 23,
		send_voice_ = 48
	};

public:
	template <typename T>
	void send_voice(CNetworkSerializePB* message_handle, T* message)
	{
		//CALL_VIRTUAL(void, send_voice_, this, 0, message_handle, message, -1);
		MEM::CallVFunc(this(message_handle, message), send_voice);
	}
};
