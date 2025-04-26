#pragma once
#include <cstdint>

class CNetworkSerializePB
{
public:
	void* vtable;
	const char* unscopedName;
	uint32_t categoryMask;
	int unk;
	void* protobufBinding;
	const char* groupName;
	int16_t messageID;
	uint8_t groupID;
	uint8_t defaultBufferType;
	char _pad[28];
};
