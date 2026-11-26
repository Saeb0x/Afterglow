#if !defined(AG_RHI_H)
#define AG_RHI_H

#include "Core/Types.h"

enum RHICommandType
{
	RHICOMMANDTYPE_CLEAR
};

struct RHICommand
{
	RHICommandType Type;

	union
	{
		struct
		{
			real32 Red;
			real32 Green;
			real32 Blue;
			real32 Alpha;
		} Clear;
	};
};

struct RHICommandList
{
	RHICommand* Commands;
	usize Count;
	usize Capacity;
};

bool RHIInit(void* windowHandle, uint16 windowWidth, uint16 windowHeight);
bool RHIShutdown();
void RHISetViewport(uint16 windowWidth, uint16 windowHeight);

inline void RHIPushCommand(RHICommand command, RHICommandList* commandList)
{
	if(commandList->Count < commandList->Capacity)
	{
		commandList->Commands[commandList->Count++] = command;
	}
}
inline void RHIResetCommandList(RHICommandList* commandList)
{
	commandList->Count = 0;
}
void RHISubmit(const RHICommandList* commandList);
void RHIPresent();

#endif