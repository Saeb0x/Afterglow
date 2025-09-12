#include "Sandbox.h"

#include "layers/Sandbox2D.h"

Sandbox::Sandbox()
	: Afterglow::Application("Sandbox")
{
	PushLayer(new Sandbox2D());
}