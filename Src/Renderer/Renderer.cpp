#include "Renderer.h"

#include "RHI/OpenGL/GLRHI.cpp"

#define AG_RENDERER_MAX_COMMANDS 64

static struct
{
	RHICommand Commands[AG_RENDERER_MAX_COMMANDS];
	RHICommandList CommandList;
} RendererState;

bool RendererInit(void* windowHandle, uint16 width, uint16 height)
{
	if(!RHIInit(windowHandle, width, height))
	{
		return (false);
	}

	RendererState.CommandList.Commands = RendererState.Commands;
	RendererState.CommandList.Capacity = AG_RENDERER_MAX_COMMANDS;

	return (true);
}

bool RendererShutdown()
{
	if(!RHIShutdown())
	{
		return (false);
	}

	return (true);
}

void RendererResize(uint16 width, uint16 height)
{
	RHIResize(width, height);
}

void RendererBegin()
{
	RHIResetCommandList(&RendererState.CommandList);
	RHICommand clearCommand = {RHICOMMANDTYPE_CLEAR, {0.63f, 0.87f, 1.0f, 1.0f}};
	RHIPushCommand(clearCommand, &RendererState.CommandList);
}

void RendererEnd()
{
	RHISubmit(&RendererState.CommandList);
	RHIPresent();
}